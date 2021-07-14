#ifndef HELENA_CONCURRENCY_PARALLELPOOL_HPP
#define HELENA_CONCURRENCY_PARALLELPOOL_HPP

#include <thread>
#include <mutex>
#include <vector>
#include <memory>

#include <Helena/Concurrency/Internal.hpp>

#include <Dependencies/function2/function2.hpp>

namespace Helena::Concurrency
{
    class ParallelPool final
    {
        using Callback = fu2::unique_function<void()>;

        template <typename Func, typename... Args>
        using ResultOf = typename std::result_of<std::decay_t<Func>(std::decay_t<Args>...)>::type;

        struct alignas(Internal::cache_line) JobPool {

            template <typename Func, typename... Args>
            [[nodiscard]] void Enqueue(Func&& func, Args&&... args) noexcept;
            [[nodiscard]] auto ExtractResult() noexcept -> std::vector<std::size_t>&&;

        private:
            friend class ParallelPool;
            friend struct Worker;

            std::vector<Callback> m_Jobs;
            std::vector<std::size_t> m_Result;
        };

        struct Worker {
            Worker() = default;
            ~Worker() = default;
            Worker(const Worker&) = delete;
            Worker(Worker&&) noexcept = default;
            Worker& operator=(const Worker&) = delete;
            Worker& operator=(Worker&&) noexcept = default;

            std::atomic_bool m_Shutdown{};
            char paddingA[Internal::cache_line - sizeof(m_Shutdown)]{};
            std::atomic_bool m_IsBusy{};
            char paddingB[Internal::cache_line - sizeof(m_IsBusy)]{};

            JobPool m_Pool;

            std::mutex m_Mutex;
            std::condition_variable m_Condition;
            std::unique_ptr<std::thread> m_Thread;
        };

    public:
        ParallelPool() = default;
        ~ParallelPool();
        ParallelPool(const ParallelPool&) = delete;
        ParallelPool(ParallelPool&&) noexcept = delete;
        ParallelPool& operator=(const ParallelPool&) = delete;
        ParallelPool& operator=(ParallelPool&&) noexcept = delete;

        void Initialize(const std::size_t threads = std::thread::hardware_concurrency(), const std::size_t reserve = 1024);

        void Finalize();

        void Signal(const std::size_t id) noexcept;
        [[nodiscard]] bool IsBusy(const std::size_t id) const noexcept;
        
        template <typename Func>
        void Each(Func&& func) const noexcept;

        [[nodiscard]] auto GetPool(const std::size_t id) noexcept -> JobPool&;

    private:
        void Execution(const std::size_t id);

    private:
        std::vector<std::unique_ptr<Worker>> m_Workers;
    };
}

#include <Helena/Concurrency/ParallelPool.ipp>

#endif // HELENA_CONCURRENCY_PARALLELPOOL_HPP