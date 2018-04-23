#include "CTRPluginFrameworkImpl/System/Scheduler.hpp"
#include "CTRPluginFramework/System/System.hpp"
#include "CTRPluginFramework/System/Lock.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"

namespace CTRPluginFramework
{
#define DEBUG 0
    Scheduler   Scheduler::_singleton;

    Scheduler::Core::Core(void)
    {
        LightEvent_Init(&newTaskEvent, RESET_STICKY);
    }

    void    Scheduler::Core::Assign(const Task &task)
    {
        if (flags != Core::Idle)
            return;

        flags = Core::Busy;
        ctx = task.context;
        ctx->flags = Task::Scheduled;
        AtomicIncrement(&ctx->refcount);
        LightEvent_Signal(&newTaskEvent);
    }

    Scheduler::~Scheduler(void)
    {
        // TODO: signal core threads to exit (technically this code is never reached)
    }

    void     Scheduler__CoreHandler(void *arg)
    {
        Scheduler::Core &core = *reinterpret_cast<Scheduler::Core *>(arg);

        core.flags = Scheduler::Core::Idle;

        while (core.flags != Scheduler::Core::Exit)
        {
            // Fetch a task from the queue
            core.ctx = Scheduler::_PollTask();

            if (core.ctx == nullptr)
            {
                core.flags = Scheduler::Core::Idle;
                LightEvent_Wait(&core.newTaskEvent);
                LightEvent_Clear(&core.newTaskEvent);
            }

#if DEBUG
            OSD::Notify(Utils::Format("New task on core: %d", core.id));
#endif
            core.flags = Scheduler::Core::Busy;

            TaskContext *ctx = core.ctx;

            if (ctx != nullptr && ctx->func != nullptr)
            {
                ctx->flags = Task::Processing;

                ctx->result = ctx->func(ctx->arg);
            }

            // If TaskContext::refcount is 0 then free the resources as it's an alone obj
            if (!AtomicDecrement(&ctx->refcount))
                delete ctx;
            // Else, signal it's done
            else
            {
                ctx->flags = Task::Finished;
                LightEvent_Signal(&ctx->event);
            }
#if DEBUG
            OSD::Notify(Utils::Format("Task ended on core: %d", core.id));
#endif
        }

        svcExitThread();
    }

    int     Scheduler::Schedule(const Task &task)
    {
        Lock    lock(_singleton._mutex);
        Core    *cores = _singleton._cores;

        if (task.context == nullptr)
            return -1;

        if (_singleton._tasks.empty())
        {
            // Priority to N3DS extra core
            if (cores[2].flags == Core::Idle)
            {
                cores[2].Assign(task);
                return 0;
            }
            if (cores[1].flags == Core::Idle)
            {
                cores[1].Assign(task);
                return 0;
            }
        }

        TaskContext *ctx = task.context;

        AtomicIncrement(&ctx->refcount);
        ctx->flags = Task::Scheduled;
        _singleton._tasks.push(ctx);

        return 0;
    }

    Scheduler::Scheduler(void)
    {
        // Create handler on Core0 ??
        //_cores[0].stack = static_cast<u8 *>(::operator new(0x1000));
        //_cores[0].thread = threadCreate(CoreHandler, &_cores[0], 0x1000, 0x18, 0, false);
        _cores[0].flags = Core::Exit;

        // Create handler on Core1
        _cores[1].id = 1;
        _cores[1].stack = static_cast<u8 *>(::operator new(0x1000));
        _cores[1].thread = threadCreate(Scheduler__CoreHandler, &_cores[1], _cores[1].stack, 0x1000, 0x18, 1);


        // Create handler on Core2 (N3DS only)
        if (!System::IsNew3DS())
        {
            _cores[2].flags = Core::Exit;
        }
        else
        {
            _cores[2].id = 2;
            _cores[2].stack = static_cast<u8 *>(::operator new(0x1000));
            _cores[2].thread = threadCreate(Scheduler__CoreHandler, &_cores[2], _cores[2].stack, 0x1000, 0x18, 2);
        }
    }

    TaskContext *   Scheduler::_PollTask(void)
    {
        TaskContext                 *ctx = nullptr;
        std::queue<TaskContext *>   &tasks = _singleton._tasks;

        // If the Scheduler is in use, then abort
        if (_singleton._mutex.TryLock())
            return ctx;

        // If there's tasks in the queue
        if (!tasks.empty())
        {
            ctx = tasks.front();
            tasks.pop();

            // No need to decrement TaskContext::refcount, the core takes it
        }

        _singleton._mutex.Unlock();
        return ctx;
    }
}
