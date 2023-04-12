#ifndef HELENA_TYPES_ALIGNEDSTORAGEV2_HPP
#define HELENA_TYPES_ALIGNEDSTORAGEV2_HPP

#include <Helena/Traits/Constructible.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/PowerOf2.hpp>

#include <bit>
#include <memory>
#include <utility>

namespace Helena::Types
{
    // Non RAII aligned storage
    template <std::size_t Size, std::size_t Alignment = sizeof(std::max_align_t)>
    requires Traits::IsPowerOf2<Alignment>
    struct AlignedStorageV2
    {
        template <typename T>
        [[nodiscard]] static constexpr auto HasSpace() noexcept {
            constexpr auto mask = Alignment - 1;
            constexpr auto size = (Size + mask) & (~mask);
            return sizeof(T) <= size;
        }

        template <std::destructible T, typename... Args>
        requires (HasSpace<T>() && Traits::ConstructibleFrom<T, Args...>)
        constexpr decltype(auto) Construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
            return *std::construct_at(std::bit_cast<T*>(&m_Memory), std::forward<Args>(args)...);
        }

        template <typename T>
        requires (HasSpace<Traits::RemoveCVR<T>>())
        constexpr decltype(auto) Construct(T&& other)
            noexcept(noexcept(this->template Construct<Traits::RemoveCVR<T>>(std::forward<T>(other))))
            requires(requires{this->template Construct<Traits::RemoveCVR<T>>(std::forward<T>(other));}) {
            return Construct<Traits::RemoveCVR<T>>(std::forward<T>(other));
        }

        template <std::destructible T>
        requires (HasSpace<T>())
        constexpr void Destruct() noexcept(std::is_nothrow_destructible_v<T>) {
            std::destroy_at(std::bit_cast<T*>(&m_Memory));
        }

        template <typename T>
        requires (HasSpace<Traits::RemoveCVR<T>>())
        constexpr decltype(auto) Operator(T&& other)
            noexcept(noexcept(*std::bit_cast<Traits::RemoveCVR<T>*>(&this->m_Memory) = std::forward<T>(other)))
            requires(requires{*std::bit_cast<Traits::RemoveCVR<T>*>(&this->m_Memory) = std::forward<T>(other);}) {
            return *std::bit_cast<Traits::RemoveCVR<T>*>(&m_Memory) = std::forward<T>(other);
        }

        template <typename T>
        requires (HasSpace<Traits::RemoveCVR<T>>())
        constexpr decltype(auto) As() const {
            //requires(requires{std::bit_cast<const Traits::RemoveCVR<T>*>(&this->m_Memory);}) {
            return *std::bit_cast<const Traits::RemoveCVR<T>*>(&m_Memory);
        }

        template <typename T>
        requires (HasSpace<Traits::RemoveCVR<T>>())
        constexpr decltype(auto) As() {
            //requires(requires{std::bit_cast<Traits::RemoveCVR<T>*>(&this->m_Memory);}) {
            return *std::bit_cast<Traits::RemoveCVR<T>*>(&m_Memory);
        }

        alignas(Alignment) std::byte m_Memory[Size];
    };
}

#endif // HELENA_TYPES_ALIGNEDSTORAGEV2_HPP