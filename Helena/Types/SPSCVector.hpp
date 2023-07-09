#ifndef HELENA_TYPES_SPSCVECTOR_HPP
#define HELENA_TYPES_SPSCVECTOR_HPP

#include <Helena/Traits/Cacheline.hpp>

#include <atomic>
#include <vector>

namespace Helena::Types
{
    template <typename Type>
    class SPSCVector
    {
        using ContainerType = std::vector<Type>;
        using AtomicContainerType = std::atomic<ContainerType*>;
    public:
        using value_type = Type;

    public:
        SPSCVector(std::size_t reserve = 100) {
            m_WriterContainer = new ContainerType(); m_WriterContainer->reserve(reserve);
            auto readerContainer = new ContainerType(); readerContainer->reserve(reserve);
            m_Swap.store(readerContainer, std::memory_order::relaxed);
        }

        ~SPSCVector()
        {
            while(true)
            {
                if(auto ptr = m_Swap.load(std::memory_order_acquire); ptr)
                {
                    if(!m_Swap.compare_exchange_strong(ptr, nullptr)) {
                        continue;
                    }

                    // First we release swap (in CAS since the reader can Handle the swap buffer)
                    delete ptr;

                    // We can then ensure that m_WriterContainer is present
                    delete std::exchange(m_WriterContainer, nullptr);
                    break;
                }
            }
        }

        SPSCVector(const SPSCVector&) = delete;
        SPSCVector(SPSCVector&&) noexcept = delete;
        SPSCVector& operator=(const SPSCVector&) = delete;
        SPSCVector& operator=(SPSCVector&&) noexcept = delete;

        // ----- [WRITER] -----
        template <typename... Args>
        void Add(Args&&... args) {
            m_WriterContainer->emplace_back(std::forward<Args>(args)...);
        }

        bool Commit()
        {
            if(!Empty())
            {
                auto swapContainer = m_Swap.load(std::memory_order::relaxed);
                do {
                    if(!swapContainer || !swapContainer->empty())
                        return false;
                } while(!m_Swap.compare_exchange_weak(swapContainer, m_WriterContainer, std::memory_order::release, std::memory_order::relaxed));
                return m_WriterContainer = swapContainer;
            }

            return false;
        }

        [[nodiscard]] bool Empty() const noexcept {
            return m_WriterContainer->empty();
        }

        [[nodiscard]] std::size_t Size() const noexcept {
            return m_WriterContainer->size();
        }

        [[nodiscard]] decltype(auto) Container() const noexcept {
            return *m_WriterContainer;
        }

        [[nodiscard]] decltype(auto) Container() noexcept {
            return *m_WriterContainer;
        }

        // ----- [READER] -----
        template <typename Callback>
        bool Handle(Callback fn)
        {
            auto swapContainer = m_Swap.load(std::memory_order::relaxed);

            do {
                if(!swapContainer || swapContainer->empty())
                    return false;
            } while(!m_Swap.compare_exchange_weak(swapContainer, nullptr, std::memory_order::release, std::memory_order::relaxed));

            for(auto&& value : *swapContainer) {
                fn(value);
            }

            swapContainer->clear();
            m_Swap.store(swapContainer, std::memory_order::release);
            return true;
        }

    private:
        alignas(Traits::Cacheline) AtomicContainerType m_Swap;
        ContainerType* m_WriterContainer;
    };
}

#endif // HELENA_TYPES_SPSCVECTOR_HPP