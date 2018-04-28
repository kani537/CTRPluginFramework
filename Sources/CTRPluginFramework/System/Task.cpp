#include "CTRPluginFramework/System/Task.hpp"
#include "CTRPluginFrameworkImpl/System/Scheduler.hpp"

namespace CTRPluginFramework
{
    Task::Task(TaskFunc func, void *arg, s32 affinity)
    {
        context = new TaskContext();

        AtomicIncrement(&context->refcount);
        context->affinity = affinity;
        context->func = func;
        context->arg = arg;
        LightEvent_Init(&context->event, RESET_STICKY);
    }

    Task::Task(const Task &task)
    {
        context = task.context;
        AtomicIncrement(&context->refcount);
    }

    Task::Task(Task &&task) noexcept
    {
        context = task.context;
        task.context = nullptr;
    }

    Task::~Task(void)
    {
        if (context != nullptr && AtomicDecrement(&context->refcount) == 0)
            delete context;
    }

    int     Task::Start(void) const
    {
        if (context == nullptr)
            return -1;

        LightEvent_Clear(&context->event);
        return Scheduler::Schedule(*this);
    }

    s32     Task::Wait(void) const
    {
        if (context == nullptr)
            return -1;

        if (context->flags == Task::Finished)
            return context->result;

        LightEvent_Wait(&context->event);
        return context->result;
    }

    u32     Task::Status(void) const
    {
        if (context == nullptr)
            return -1;

        return context->flags;
    }
}
