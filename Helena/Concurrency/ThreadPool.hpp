#ifndef HELENA_CONCURRENCY_THREADPOOL_HPP
#define HELENA_CONCURRENCY_THREADPOOL_HPP

#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <optional>
#include <future>
#include <type_traits>

#include <Helena/Concurrency/SPSCQueue.hpp>
#include <Helena/Concurrency/Spinlock.hpp>

#include <Dependencies/function2/function2.hpp>

namespace Helena::Concurrency
{
    class ThreadPool final
    {
        using Callback = fu2::unique_function<void ()>;

        template <typename Func, typename... Args>
        using ResultOf = typename std::result_of<std::decay_t<Func>(std::decay_t<Args>...)>::type;

        template <typename Func, typename... Args>
        using FutureOf = std::future<ResultOf<Func, Args...>>;

    public:
        ThreadPool(const std::size_t threads = std::thread::hardware_concurrency(), const std::uint32_t jobSize = 1024);
        ~ThreadPool();
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) noexcept = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) noexcept = delete;

        template <typename Func, typename... Args>
        [[nodiscard]] bool EnqueueJob(Func&& func, Args&&... args) noexcept;

        template <typename Func, typename... Args>
        [[nodiscard]] auto EnqueueTask(Func&& func, Args&&... args)->std::optional<FutureOf<Func, Args...>>;

        [[nodiscard]] bool Empty() noexcept;
        [[nodiscard]] auto Size() noexcept;

    private:
        void Worker();

    private:
        Spinlock m_Lock;
        std::atomic_bool m_Shutdown;
        char padding[Internal::cache_line - sizeof(m_Shutdown)];
        SPSCQueue<Callback> m_Jobs;
        std::vector<std::thread> m_Threads;

    };
}

#include <Helena/Concurrency/ThreadPool.ipp>

#endif // HELENA_CONCURRENCY_THREADPOOL_HPP