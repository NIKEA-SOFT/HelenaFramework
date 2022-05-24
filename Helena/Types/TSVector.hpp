#ifndef HELENA_TYPES_TSVECTOR_HPP
#define HELENA_TYPES_TSVECTOR_HPP

#include <Helena/Traits/Cacheline.hpp>
#include <Helena/Types/Spinlock.hpp>

#include <vector>

namespace Helena::Types
{
    template <typename Type>
    class TSVector
    {
    public:
        TSVector() = default;
        ~TSVector() = default;
        TSVector(const TSVector&) = delete;
        TSVector(TSVector&&) noexcept = delete;
        TSVector& operator=(const TSVector&) = delete;
        TSVector& operator=(TSVector&&) noexcept = delete;

        [[nodiscard]] std::size_t Size() const noexcept {
            std::lock_guard lock{m_Lock};
            return m_Container.size();
        }

        template <typename... Args>
        void Push(Args&&... args) {
            std::lock_guard lock{m_Lock};
            m_Container.emplace_back(std::forward<Args>(args)...);
        }

        auto Pop() const
        {
            std::vector<Type> container;

            {
                std::lock_guard lock{m_Lock};
                if(!m_Container.empty()) {
                    std::swap(m_Container, container);
                }
            }

            return container;
        }

        template <typename Callback>
        void Each(Callback&& func)
        {
            std::vector<Type> container;

            {
                std::lock_guard lock{m_Lock};
                if(!m_Container.empty()) {
                    std::swap(m_Container, container);
                }
            }

            for(auto& instance : container) {
                func(instance);
            }
        }

    private:
        alignas(Traits::Cacheline::value) std::vector<Type> m_Container {};
        mutable Spinlock m_Lock {};
    };
}

#endif // HELENA_TYPES_TSVECTOR_HPP