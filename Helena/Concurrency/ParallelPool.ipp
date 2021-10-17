#ifndef HELENA_CONCURRENCY_PARALLELPOOL_IPP
#define HELENA_CONCURRENCY_PARALLELPOOL_IPP

#include <Helena/Concurrency/ParallelPool.hpp>
#include <Helena/Assert.hpp>

namespace Helena::Concurrency
{
    inline ParallelPool::~ParallelPool() {
        Finalize();
    }

    inline void ParallelPool::Initialize(const std::size_t threads, const std::size_t reserve) 
    {
        HF_ASSERT(threads, "Size of threads cannot be zero");

        m_Workers.reserve(threads);
        for(std::size_t id = 0; id < threads; ++id) 
        {
            auto& worker = m_Workers.emplace_back(std::make_unique<Worker>());

            worker->m_Shutdown.store(false);
            worker->m_Pool.m_Jobs.reserve(reserve);
            worker->m_Pool.m_Result.reserve(reserve);
            worker->m_Thread = std::make_unique<std::thread>(std::thread(&ParallelPool::Execution, this, id));
        }
    }

    inline void ParallelPool::Finalize() 
    {
        for(auto& worker : m_Workers) 
        {
            worker->m_Shutdown.store(true);
            worker->m_Condition.notify_one();
            if(worker->m_Thread->joinable()) {
                worker->m_Thread->join();
            }
        }

        m_Workers.clear();
    }

    inline void ParallelPool::Signal(const std::size_t id) noexcept {
        HF_ASSERT(id < m_Workers.size(), "{} >= {}", id, m_Workers.size());
        m_Workers[id]->m_Condition.notify_one();
    }

    [[nodiscard]] inline bool ParallelPool::IsBusy(const std::size_t id) const noexcept {
        HF_ASSERT(id < m_Workers.size(), "{} >= {}", id, m_Workers.size());
        return m_Workers[id]->m_IsBusy.load();
    }

    template <typename Func>
    inline void ParallelPool::Each(Func&& func) const noexcept
    {
        static_assert(!std::is_same_v<ResultOf<Func ()>, void>, "Each callback is not void");

        for(std::size_t i = m_Workers.size(); i; --i) 
        {
            auto& worker = m_Workers[i - 1];
            if(!worker->m_IsBusy.load()) {
                func(i - 1);
            }
        }
    }

    [[nodiscard]] inline auto ParallelPool::GetPool(const std::size_t id) noexcept -> JobPool& {
        HF_ASSERT(id < m_Workers.size(), "{} >= {}", id, m_Workers.size());
        return m_Workers[id]->m_Pool;
    }

    inline void ParallelPool::Execution(const std::size_t id)
    {
        auto& worker = m_Workers[id];
        auto& jobs = worker->m_Pool.m_Jobs;

        while(true)
        {
            if(worker->m_Shutdown.load(std::memory_order_relaxed)) {
                return;
            }

            worker->m_IsBusy.store(false);
            std::unique_lock lock(worker->m_Mutex);
            worker->m_Condition.wait(lock);
            worker->m_IsBusy.store(true);

            if(worker->m_Shutdown.load(std::memory_order_relaxed)) {
                return;
            }

            auto first = jobs.begin();
            while(first != jobs.cend()) {
                (*first)();
                first = jobs.erase(first);
            }

        }
    }

    template <typename Func, typename... Args>
    void ParallelPool::JobPool::Enqueue(Func&& func, Args&&... args) noexcept
    {
        if constexpr(std::is_same_v<ResultOf<Func, Args...>, void>) {
            m_Jobs.emplace_back([func = std::forward<Func>(func), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                std::apply(func, std::move(args));
            });
        } else if constexpr (std::is_same_v<ResultOf<Func, Args...>, std::size_t>) {
            m_Jobs.emplace_back([this, func = std::forward<Func>(func), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                m_Result.emplace_back(std::apply(func, std::move(args)));
            });
        } else {
            static_assert(Helena::Internal::is_always_value<false, Func>, "Func can be only void or std::size_t");
        }
    }

    [[nodiscard]] auto ParallelPool::JobPool::ExtractResult() noexcept -> std::vector<std::size_t>&& {
        return std::move(m_Result);
    }
}


#endif // HELENA_CONCURRENCY_PARALLELPOOL_IPP
