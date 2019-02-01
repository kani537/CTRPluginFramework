#ifndef CTRPLUGINFRAMEWORKIMPL_SYSTEM_SCHEDULER_HPP
#define CTRPLUGINFRAMEWORKIMPL_SYSTEM_SCHEDULER_HPP

#include "types.h"
#include "CTRPluginFramework/System/Mutex.hpp"
#include "CTRPluginFramework/System/Task.hpp"
#include "CTRPluginFramework/System/Thread.hpp"
#include <list>

namespace CTRPluginFramework
{
    class Scheduler
    {
        enum
        {
            AppCore = 1 << 0,
            AppCore1 = 1 << 1,
            NewAppCore = 1 << 2,
            NewSysCore = 1 << 3,

            AppCores = AppCore | AppCore1 | NewAppCore,
            SysCores = NewSysCore,
            OldCores = AppCore | AppCore1,
            NewCores = NewAppCore | NewSysCore,
            AllCores = AppCores | SysCores
        };

        friend void Scheduler__CoreHandler(void *arg);
        struct Core
        {
            enum
            {
                Idle = 0,
                Busy = 1,
                Exit = 2
            };

            u32         id{0};
            u32         flags{Idle};
            ThreadEx    thread;
            TaskContext *ctx{nullptr};
            LightEvent  newTaskEvent{};

            Core(void);
            ~Core(void) = default;

            // Non copyable
            Core(const Core &core) = delete;
            Core(Core &&core) = delete;
            Core& operator=(const Core &core) = delete;

            void    Assign(const Task &task);
        };

    public:
        ~Scheduler(void);

        // Non copyable
        Scheduler(const Scheduler &scheduler) = delete;
        Scheduler(Scheduler &&scheduler) = delete;
        Scheduler& operator=(const Scheduler &scheduler) = delete;

        /**
         * \brief Schedule a new Task to be executed
         * \param task The Task to be executed
         * \return If the operation is a success
         */
        static int      Schedule(const Task &task);
        static void     Initialize(void);
        static void     Exit(void);

    private:
        Scheduler(void);

        Core    _cores[4];
        Mutex   _mutex;
        std::list<TaskContext *>   _tasks{};

        static Scheduler _singleton;

        static TaskContext * _PollTask(u32 coreId);
    };
}

#endif
