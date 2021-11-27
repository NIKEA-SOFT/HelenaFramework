#ifndef HELENA_CONCURRENCY_THREADPOOL_IPP
#define HELENA_CONCURRENCY_THREADPOOL_IPP

#include <Helena/Concurrency/ThreadPool.hpp>
#include <Helena/Concurrency/Spinlock.hpp>
#include <Helena/Defines.hpp>
#include <Helena/Assert.hpp>

#include <algorithm>
#include <tuple>

namespace Helena::Concurrency
{
    inline ThreadPool::ThreadPool(const std::size_t threads, const std::uint32_t jobSize)
        : m_Lock{}, m_Shutdown{}, m_Jobs{jobSize}, m_Threads{}
    {
        HF_ASSERT(jobSize, "Size of queue cannot be zero");

        m_Threads.reserve(threads);
        for(std::size_t i = 0; i < threads; ++i) {
            m_Threads.emplace_back(&ThreadPool::Worker, this);
        }
    }

    inline ThreadPool::~ThreadPool()
    {
        m_Shutdown.store(true);

        for(auto& thread : m_Threads)
        {
            if(thread.joinable()) {
                thread.join();
            }
        }
    }

    template <typename Func, typename... Args>
    [[nodiscard]] bool ThreadPool::EnqueueJob(Func&& func, Args&&... args) noexcept {
        static_assert(std::is_same_v<ResultOf<Func, Args...>, void>, "Func is not void, did you mean EnqueueTask?");
        return m_Jobs.push([func = std::forward<Func>(func), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            std::apply(func, std::move(args));
        });
    }

    template <typename Func, typename... Args>
    [[nodiscard]] auto ThreadPool::EnqueueTask(Func&& func, Args&&... args) -> std::optional<FutureOf<Func, Args...>> {
        static_assert(!std::is_same_v<ResultOf<Func, Args...>, void>, "Func is void, did you mean EnqueueJob?");

        using packaged_task = std::packaged_task<ResultOf<Func, Args...>()>;

        packaged_task task{[func = std::forward<Func>(func), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            return std::apply(func, std::move(args));
        }};

        auto value = std::make_optional<FutureOf<Func, Args...>>(task.get_future());
        if(!m_Jobs.push([task = std::move(task)]() mutable {
            task();
        })) {
            value.reset();
        }

        return value;
    }

    [[nodiscard]] bool ThreadPool::Empty() noexcept {
        return m_Jobs.empty();
    }

    [[nodiscard]] auto ThreadPool::Size() noexcept {
        return m_Jobs.size();
    }

    inline void ThreadPool::Worker()
    {
        Callback func;
        bool result;
        while(true)
        {
            m_Lock.lock();
            result = m_Jobs.pop(func);
            m_Lock.unlock();

            if(m_Shutdown.load()) {
                return;
            }

            if(result) {
                func();
            }
        }
    }
}


#endif // HELENA_CONCURRENCY_THREADPOOL_IPP
