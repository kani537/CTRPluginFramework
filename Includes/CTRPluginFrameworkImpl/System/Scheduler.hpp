#ifndef CTRPLUGINFRAMEWORKIMPL_SYSTEM_SCHEDULER_HPP
#define CTRPLUGINFRAMEWORKIMPL_SYSTEM_SCHEDULER_HPP

#include "types.h"
#include "CTRPluginFramework/System/Mutex.hpp"
#include "CTRPluginFramework/System/Task.hpp"
#include "ctrulib/thread.h"
#include <queue>

namespace CTRPluginFramework
{
    class Scheduler
    {
        friend void Scheduler__CoreHandler(void *arg);
        struct Core
        {
            enum
            {
                Idle = 0,
                Busy = 1,
                Exit = 2
            };

            u32         flags{Idle};
            u8          *stack{nullptr};
            Thread      thread{nullptr};
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

    private:
        Scheduler(void);

        Core    _cores[3];
        Mutex   _mutex;
        std::queue<TaskContext *>   _tasks{};

        static Scheduler _singleton;

        static TaskContext * _PollTask(void);
    };
}

#endif
