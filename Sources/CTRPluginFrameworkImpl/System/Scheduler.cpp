#include "CTRPluginFrameworkImpl/System/Scheduler.hpp"
#include "CTRPluginFramework/System/System.hpp"
#include "CTRPluginFramework/System/Lock.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFrameworkImpl/System/SystemImpl.hpp"

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
            core.ctx = Scheduler::_PollTask(core.id);

            if (core.ctx == nullptr)
            {
                core.flags = Scheduler::Core::Idle;
                LightEvent_Wait(&core.newTaskEvent);
                LightEvent_Clear(&core.newTaskEvent);

                if (SystemImpl::Status())
                    return;
            }
            else if (SystemImpl::Status())
                return;

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
    }

    int     Scheduler::Schedule(const Task &task)
    {
        CTRPluginFramework::Lock    lock(_singleton._mutex);
        Core    *cores = _singleton._cores;

        if (task.context == nullptr)
            return -1;

        TaskContext *ctx = task.context;

        if (SystemImpl::IsNew3DS && ctx->affinity > AllCores)
            ctx->affinity = AllCores;

        if (!SystemImpl::IsNew3DS && ctx->affinity > OldCores)
            ctx->affinity = AllCores;

        if (ctx->affinity == -1)
            ctx->affinity = AllCores;

        // Search for an idle core matching the Task affinity
        for (s32 i = 3; i >= 0; --i)
        {
            if (cores[i].flags == Core::Idle && cores[i].id & ctx->affinity)
            {
                cores[i].Assign(task);
                return 0;
            }
        }

        // Enqueue the task
        AtomicIncrement(&ctx->refcount);
        ctx->flags = Task::Scheduled;
        _singleton._tasks.push_back(ctx);

        return 0;
    }

    void    Scheduler::Initialize(void)
    {
        Core *_cores = _singleton._cores;

        // Create handler on Core0
        _cores[0].id = AppCore;
        _cores[0].thread = threadCreate(Scheduler__CoreHandler, &_cores[0], 0x1000, 0x20, 0, true);

        // Create handler on Core1
        _cores[1].id = SysCore;
        _cores[1].thread = threadCreate(Scheduler__CoreHandler, &_cores[1], 0x1000, 10, 1, true);


        // Create handler on Core2 & Core3 (N3DS only)
        if (!System::IsNew3DS())
        {
            _cores[2].flags = Core::Exit;
            _cores[3].flags = Core::Exit;
        }
        else
        {
            _cores[2].id = NewAppCore;
            _cores[2].thread = threadCreate(Scheduler__CoreHandler, &_cores[2], 0x1000, 0x18, 2, true);

            _cores[3].id = NewSysCore;
            _cores[3].thread = threadCreate(Scheduler__CoreHandler, &_cores[3], 0x1000, 0x18, 3, true);
        }
    }

    void    Scheduler::Exit(void)
    {
        Lock lock(_singleton._mutex);

        for (Core &core : _singleton._cores)
        {
            core.flags |= Core::Exit;
            LightEvent_Signal(&core.newTaskEvent);
        }
    }

    Scheduler::Scheduler(void)
    {
    }

    TaskContext *   Scheduler::_PollTask(u32 coreId)
    {
        std::list<TaskContext *>   &tasks = _singleton._tasks;

        // If the Scheduler is in use, then abort
        if (_singleton._mutex.TryLock())
            return nullptr;

        // If there's tasks in the queue
        for (TaskContext *ctx : tasks)
        {
            // If the task's affinity match this core
            if (ctx->affinity & coreId)
            {
                // No need to decrement TaskContext::refcount, the core takes ownership
                _singleton._mutex.Unlock();
                return ctx;
            }
        }

        _singleton._mutex.Unlock();
        return nullptr;
    }
}
