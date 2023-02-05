#ifndef HELENA_TYPES_REFERENCEPOINTER_HPP
#define HELENA_TYPES_REFERENCEPOINTER_HPP

#include <Helena/Types/CompressedPair.hpp>
#include <Helena/Platform/Assert.hpp>

namespace Helena::Types
{
    template <typename T>
    class ReferencePointer
    {
        using Container = CompressedPair<T, std::size_t>;

    public:
        ReferencePointer() : m_Container{} {};

        template <typename... Args>
        ReferencePointer(std::in_place_t, Args&&... args)
            : m_Container{new (std::nothrow) Container{
                std::piecewise_construct,
                std::forward_as_tuple(std::forward<Args>(args)...),
                std::forward_as_tuple(1uLL)}} {}

        ~ReferencePointer() {
            Reset();
        }

        ReferencePointer(const ReferencePointer& other) : m_Container{other.m_Container} {
            if(m_Container) {
                ++m_Container->Second();
            }
        }

        ReferencePointer(ReferencePointer&& other) noexcept : m_Container{other.m_Container} {
            other.m_Container = nullptr;
        }

        ReferencePointer& operator=(const ReferencePointer& other)
        {
            Reset();

            if(other.m_Container) {
                m_Container = other.m_Container;
                ++m_Container->Second();
            }

            return *this;
        }

        ReferencePointer& operator=(ReferencePointer&& other) noexcept
        {
            Reset();

            if(other.m_Container) {
                m_Container = other.m_Container;
                other.m_Container = nullptr;
            }

            return *this;
        }

        template <typename... Args>
        [[nodiscard]] static ReferencePointer Create(Args&&... args) noexcept {
            return ReferencePointer{std::in_place, std::forward<Args>(args)...};
        }

        [[nodiscard]] T& Ref() const {
            HELENA_ASSERT(m_Container, "Container is empty!");
            return m_Container->First();
        }

        [[nodiscard]] T* Ptr() const {
            return m_Container ? &m_Container->First() : nullptr;
        }

        [[nodiscard]] bool Shared() const noexcept {
            HELENA_ASSERT(m_Container, "Container is empty!");
            return m_Container && m_Container->Second() > 1;
        }

        [[nodiscard]] std::size_t Count() const noexcept {
            HELENA_ASSERT(m_Container, "Container is empty!");
            return m_Container->Second();
        }

        void Reset() noexcept
        {
            if(m_Container && !--m_Container->Second()) {
                delete m_Container;
                m_Container = nullptr;
            }
        }

        [[nodiscard]] operator bool() const noexcept {
            return m_Container;
        }

        [[nodiscard]] T& operator*() const {
            HELENA_ASSERT(m_Container);
            return m_Container->First();
        }

        [[nodiscard]] T* operator->() const {
            HELENA_ASSERT(m_Container);
            return &m_Container->First();
        }

    private:
        Container* m_Container;
    };
}

#endif // HELENA_TYPES_REFERENCEPOINTER_HPP