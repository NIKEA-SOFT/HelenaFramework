#ifndef HELENA_TYPES_REFERENCEPOINTER_HPP
#define HELENA_TYPES_REFERENCEPOINTER_HPP

#include <optional>
#include <Helena/Platform/Assert.hpp>

namespace Helena::Types
{
    template <typename T>
    class ReferencePointer
    {
        struct Container {
            std::optional<T> m_Storage;
            std::size_t m_Refs;
        };

    public:
        ReferencePointer() : m_Container{} {};

        template <typename... Args>
        ReferencePointer(std::in_place_t, Args&&... args) : m_Container{new (std::nothrow) Container} {
            if(m_Container) {
                m_Container->m_Storage = std::make_optional<T>(std::forward<Args>(args)...);
                m_Container->m_Refs = 1uLL;
            }
        }

        ~ReferencePointer() {
            Reset();
        }

        ReferencePointer(const ReferencePointer& other) : m_Container{other.m_Container} {
            if(m_Container) {
                ++m_Container->m_Refs;
            }
        }

        ReferencePointer(ReferencePointer&& other) noexcept : m_Container{other.m_Container} {
            other.m_Container = nullptr;
        }

        ReferencePointer& operator=(const ReferencePointer& other)
        {
            Reset();

            m_Container = other.m_Container;
            if(m_Container) {
                ++m_Container->m_Refs;
            }

            return *this;
        }

        ReferencePointer& operator=(ReferencePointer&& other) noexcept {
            m_Container = other.m_Container;
            other.m_Container = nullptr;
        }

        template <typename... Args>
        [[nodiscard]] static ReferencePointer Create([[maybe_unused]] Args&&... args) noexcept {
            return ReferencePointer{std::in_place, std::forward<Args>(args)...};
        }

        [[nodiscard]] T& Ref() const {
            HELENA_ASSERT(m_Container, "Container is empty!");
            return m_Container->m_Storage.value();
        }

        [[nodiscard]] T* Ptr() const {
            return m_Container ? &m_Container->m_Storage.value() : nullptr;
        }

        [[nodiscard]] bool Shared() const noexcept {
            HELENA_ASSERT(m_Container, "Container is empty!");
            return m_Container && m_Container->m_Refs > 1;
        }

        [[nodiscard]] std::size_t Count() const noexcept {
            HELENA_ASSERT(m_Container, "Container is empty!");
            return m_Container->m_Refs;
        }

        void Reset() noexcept
        {
            if(m_Container && !--m_Container->m_Refs) {
                m_Container->m_Storage.reset();
                delete m_Container; m_Container = nullptr;
            }
        }

        [[nodiscard]] operator bool() const noexcept {
            return m_Container;
        }

        [[nodiscard]] T& operator*() const {
            HELENA_ASSERT(m_Container);
            return m_Container->m_Storage.value();
        }

    private:
        Container* m_Container;
    };
}

#endif // HELENA_TYPES_REFERENCEPOINTER_HPP