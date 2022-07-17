#ifndef HELENA_TYPES_TASKSCHEDULER_HPP
#define HELENA_TYPES_TASKSCHEDULER_HPP

#include <algorithm>
#include <chrono>
#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <utility>

#include <Helena/Engine/Log.hpp>
#include <Helena/Platform/Assert.hpp>

namespace Helena::Types
{
    class TaskScheduler final
    {
        using SteadyClock = std::chrono::steady_clock;
        using Nano = std::chrono::duration<std::uint64_t, std::nano>;
        using Milli = std::chrono::duration<std::uint64_t, std::milli>;
        using Callback = std::function<void (std::uint64_t, std::uint64_t&, std::uint32_t&)>;

    private:
        struct Task {
            Task(std::uint64_t id, std::uint64_t time, std::uint64_t expired, std::uint32_t repeat, Callback cb)
                : m_Id{id}, m_Time{time}, m_Expired{expired}, m_Repeat{repeat}, m_Callback{std::move(cb)} {}
            ~Task() = default;
            Task(const Task&) = delete;
            Task(Task&&) noexcept = default;
            Task& operator=(const Task&) = delete;
            Task& operator=(Task&&) noexcept = default;

            std::uint64_t m_Id;
            std::uint64_t m_Time;
            std::uint64_t m_Expired;
            std::uint32_t m_Repeat;
            Callback m_Callback;
        };

        struct Time {
            Time(std::uint64_t time, Task* task) : m_Time{time}, m_Task{task} {}
            ~Time() = default;
            Time(const Time&) = default;
            Time(Time&&) noexcept = default;
            Time& operator=(const Time&) = default;
            Time& operator=(Time&&) noexcept = default;

            std::uint64_t m_Time;
            Task* m_Task;
        };

    public:
        TaskScheduler() = default;
        ~TaskScheduler() = default;
        TaskScheduler(const TaskScheduler&) = delete;
        TaskScheduler(TaskScheduler&&) noexcept = default;
        TaskScheduler& operator=(const TaskScheduler&) = delete;
        TaskScheduler& operator=(TaskScheduler&&) noexcept = default;

        template <typename Func, typename... Args>
        requires std::invocable<Func, std::uint64_t, std::uint64_t&, std::uint32_t&, Args...>
        void Create(std::uint64_t id, std::uint64_t ms, std::uint32_t repeat, Func&& cb, Args&&... args)
        {
            HELENA_ASSERT(repeat, "Repeat is null");
            if(!repeat) {
                HELENA_MSG_ERROR("TaskID: {} not created, repeat is null!", id);
                return;
            }

            const auto expired = TimeNow() + TimeNano(ms);
            const auto [it, result] = m_Tasks.try_emplace(id, id, ms, expired, repeat,
                [cb = std::forward<decltype(cb)>(cb), ...args = std::forward<Args>(args)]
                (std::uint64_t id, std::uint64_t& ms, std::uint32_t& repeat) mutable {
                    std::forward<decltype(cb)>(cb)(id, ms, repeat, std::forward<Args>(args)...);
            });

            HELENA_ASSERT(result);
            if(!result) {
                HELENA_MSG_ERROR("TaskID: {} already exist!", id);
                return;
            }

            m_Times.emplace(Find(expired), expired, std::addressof(it->second));
        }

        [[nodiscard]] bool Has(std::uint64_t id) const noexcept {
            return m_Tasks.contains(id);
        }

        template <typename Func, typename... Args>
        requires std::invocable<Func, std::uint64_t, std::uint64_t&, std::uint32_t&, Args...>
        void Create(std::uint64_t id, std::uint64_t ms, Func&& cb, Args&&... args) {
            return Create(id, ms, 1u, std::forward<decltype(cb)>(cb), std::forward<Args>(args)...);
        }

        void Modify(std::uint64_t id, std::uint64_t ms, std::uint32_t repeat, bool update)
        {
            HELENA_ASSERT(repeat);
            if(const auto it = m_Tasks.find(id); it != m_Tasks.cend())
            {
                auto& task = it->second;
                task.m_Repeat = (std::max)(1u, repeat);
                task.m_Time = ms;

                if(update) {
                    const auto timeNew = TimeNow() + TimeNano(ms);
                    const auto itr = Find(task.m_Expired, id);
                    HELENA_ASSERT(itr != m_Times.end());
                    // Erase and emplace only when m_Time != expiredNow
                    if(itr != m_Times.end() && itr->m_Time != timeNew) {
                        m_Times.erase(itr);
                        m_Times.emplace(Find(timeNew), timeNew, std::addressof(task));
                    }
                }
            }
        }

        void Remove(std::uint64_t id)
        {
            if(const auto it = m_Tasks.find(id); it != m_Tasks.cend())
            {
                const auto itr = Find(it->second.m_Expired, id);
                HELENA_ASSERT(itr != m_Times.end(), "WTF? Why not found?");
                if(itr != m_Times.end()) {
                    m_Times.erase(itr);
                }
                m_Tasks.erase(it);
            }
        }

        [[nodiscard]] std::size_t Count() const noexcept {
            return m_Tasks.size();
        }

        void Clear() {
            m_Tasks.clear();
            m_Times.clear();
        }

        void Update()
        {
            while(!m_Times.empty())
            {
                const auto timeNow = TimeNow();
                const auto timeTask = m_Times.back();

                // Check time and exit from loop
                if(timeTask.m_Time > timeNow) {
                    break;
                }

                auto* task = timeTask.m_Task;
                const auto id = task->m_Id;
                const auto addressOld = reinterpret_cast<std::uintptr_t>(std::addressof(*task));
                HELENA_ASSERT(task->m_Repeat, "WTF? Repeat is null");
                task->m_Callback(id, task->m_Time, --task->m_Repeat);

                // TODO: Optimization if task modificated

                // If task not found in m_Times then continue
                // because task removed from callback
                const auto it = Find(timeTask.m_Time, id);
                if(it == m_Times.end()) {
                    continue;
                }

                task = it->m_Task;
                HELENA_ASSERT(task, "WTF? Task is null");
                if(task->m_Repeat)
                {
                    // Compare old task address and new
                    // if time not changed, but task recreated we should ignore emplace
                    const auto addressNew = reinterpret_cast<std::uintptr_t>(std::addressof(*task));
                    if(addressOld == addressNew) {
                        task->m_Expired = timeNow + TimeNano(task->m_Time);
                        m_Times.erase(it);
                        m_Times.emplace(Find(task->m_Expired), task->m_Expired, std::addressof(*task));
                    }
                    continue;
                }

                m_Times.erase(it);
                m_Tasks.erase(task->m_Id);
            }
        }

    private:
        [[nodiscard]] auto Find(std::uint64_t time) -> std::vector<Time>::iterator {
            const auto it = std::lower_bound(m_Times.rbegin(), m_Times.rend(), time, [](const auto& time, const auto expired) {
                return time.m_Time < expired;
            });
            return it == m_Times.rend() ? m_Times.begin() : it.base();
        }

        [[nodiscard]] auto Find(std::uint64_t time, std::uint64_t id) -> std::vector<Time>::iterator
        {
            const auto it = Find(time);
            for(std::size_t size = std::distance(m_Times.begin(), it); size; --size)
            {
                const auto& value = m_Times[size - 1];
                if(value.m_Time != time) {
                    break;
                }

                if(value.m_Task->m_Id == id) {
                    return m_Times.begin() + (size - 1);
                }
            }

            return m_Times.end();
        }

        [[nodiscard]] static std::uint64_t TimeNow() noexcept {
            return std::chrono::duration_cast<Nano>(SteadyClock::now().time_since_epoch()).count();
        }

        [[nodiscard]] static std::uint64_t TimeNano(std::uint64_t ms) noexcept {
            return std::chrono::duration_cast<Nano>(Milli{ms}).count();
        }

    private:
        std::unordered_map<std::uint64_t, Task> m_Tasks;
        std::vector<Time> m_Times;
    };
}

#endif // HELENA_TYPES_TASKSCHEDULER_HPP