#ifndef HELENA_TYPES_ALIGNEDSTORAGE_HPP
#define HELENA_TYPES_ALIGNEDSTORAGE_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Defines.hpp>
#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/Specialization.hpp>

#include <memory>
#include <iterator>
#include <utility>

namespace Helena::Types
{
    class AlignedStorage
    {
    public:
        template <typename T>
        struct Storage {
            using Type = T;
            alignas(alignof(T)) std::byte m_Memory[sizeof(T)];

            // By default, it is deleted for the safety of your ass
            Storage() = default;
            Storage(const Storage&) = delete;
            Storage(Storage&&) noexcept = delete;
            Storage& operator=(const Storage&) = delete;
            Storage& operator=(Storage&&) noexcept = delete;
        };

    private:
        template <typename T, typename... Args>
        static void Create(Storage<T>& storage, std::size_t index, Args&&... args)
        {
            using value_type = std::remove_extent_t<T>;
            if constexpr(std::is_array_v<T>) {
                HELENA_ASSERT(index < std::extent_v<T>, "Out of bounds!");
            }

            if constexpr(std::is_aggregate_v<value_type>) {
                new (storage.m_Memory + index * sizeof(value_type)) value_type{std::forward<Args>(args)...};
            } else {
                new (storage.m_Memory + index * sizeof(value_type)) value_type(std::forward<Args>(args)...);
            }
        }

    public:
        template <typename T, typename Tuple>
        struct ConstructibleFrom : std::false_type {};

        template <typename T, template <typename...> typename Tuple, typename... Args>
        struct ConstructibleFrom<T, Tuple<Args...>> : std::bool_constant<std::is_constructible_v<T, Args...>> {};

        template <typename T, typename Tuple>
        static constexpr auto Constructible = ConstructibleFrom<T, Tuple>::value;

        template <typename T, typename... Args>
        requires std::conjunction_v<std::negation<std::is_array<T>>, std::is_constructible<T, Args...>>
        static void Construct(Storage<T>& storage, Args&&... args)
            noexcept(std::is_nothrow_constructible_v<T, Args...>) {
            Create<T>(storage, 0, std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        requires std::conjunction_v<std::is_array<T>>
        static void Construct(Storage<T>& storage) noexcept(std::is_nothrow_default_constructible_v<T>) {
            return Construct<T>(storage, std::piecewise_construct);
        }

        template <typename T, typename... Args>
        requires std::conjunction_v<std::is_array<T>, std::is_constructible<std::remove_extent_t<T>, Args...>>
        static void Construct(Storage<T>& storage, std::size_t index, Args&&... args)
            noexcept(std::is_nothrow_constructible_v<std::remove_extent_t<T>, Args...>) {
            Create<T>(storage, index, std::forward<Args>(args)...);
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_default_constructible<std::remove_extent_t<T>>>
        static void Construct(Storage<T>& storage, std::size_t pos, std::size_t size)
            noexcept(std::is_nothrow_default_constructible_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT((pos + size) <= std::extent_v<T>, "Out of bounds!");
            for(std::size_t index = 0; index < size; ++index) {
                Create<T>(storage, index + pos);
            }
        }

        template <typename T, typename... Args>
        requires std::conjunction_v<std::is_array<T>>
        static void Construct(Storage<T>& storage, std::piecewise_construct_t, Args&&... tuples)
            requires(((Traits::Specialization<Args, std::tuple>
                && Constructible<std::remove_extent_t<T>, Args>) && ...)
                && Traits::Arguments<Args...>::Size <= std::extent_v<T>) {
            if constexpr(Traits::Arguments<Args...>::Size) {
                std::size_t index{};
                (std::apply([&storage, &index]<typename... Ts>(Ts&&... args) {
                    Create<T>(storage, index++, std::forward<Ts>(args)...);
                }, std::forward<Args>(tuples)), ...);
            } else {
                std::uninitialized_default_construct(std::begin(Ref(storage)), std::end(Ref(storage)));
            }
        }

        template <typename T>
        static void Destruct(Storage<T>& storage) noexcept(std::is_nothrow_destructible_v<T>) {
            if constexpr(!std::is_array_v<T>) {
                if constexpr(!std::is_trivially_destructible_v<T>) {
                    std::destroy_at(Ptr(storage));
                }
            } else {
                for(std::size_t index = 0; index < std::extent_v<T>; ++index) {
                    Destruct<T>(storage, index);
                }
            }
        }

        template <typename T>
        requires std::is_array_v<T>
        static void Destruct(Storage<T>& storage, std::size_t index)
            noexcept(std::is_nothrow_destructible_v<std::remove_extent_t<T>>) {
            using value_type = std::remove_extent_t<T>;

            if constexpr(!std::is_trivially_destructible_v<value_type>) {
                Ref(storage)[index].~value_type();
            }
        }

        template <typename T>
        requires std::is_array_v<T>
        static void Destruct(Storage<T>& storage, std::size_t pos, std::size_t size)
            noexcept(std::is_nothrow_destructible_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT((pos + size) <= std::extent_v<T>, "Out of bounds!");
            if constexpr(!std::is_trivially_destructible_v<std::remove_extent_t<T>>) {
                std::destroy_n(Ref(storage) + pos, size);
            }
        }

        template <typename T>
        requires std::conjunction_v<std::negation<std::is_array<T>>, std::is_copy_constructible<T>>
        static void ConstructCopy(const Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to)
            noexcept(std::is_nothrow_copy_constructible_v<T>) {
            std::uninitialized_copy_n(Ptr(from), 1, Ptr(to));
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_copy_constructible<std::remove_all_extents_t<T>>>
        static void ConstructCopy(const Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to)
            noexcept(std::is_nothrow_copy_constructible_v<std::remove_extent_t<T>>) {
            std::uninitialized_copy(std::begin(Ref(from)), std::end(Ref(from)), std::begin(Ref(to)));
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_copy_constructible<std::remove_all_extents_t<T>>>
        static void ConstructCopy(const Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to, std::size_t index)
            noexcept(std::is_nothrow_copy_constructible_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT(index < std::extent_v<T>, "Out of bounds!");
            std::uninitialized_copy_n(Ref(from) + index, 1, Ref(to) + index);
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_copy_constructible<std::remove_all_extents_t<T>>>
        static void ConstructCopy(const Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to, std::size_t pos, std::size_t size)
            noexcept(std::is_nothrow_copy_constructible_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT((pos + size) <= std::extent_v<T>, "Out of bounds!");
            std::uninitialized_copy_n(Ref(from) + pos, size, Ref(to) + pos);
        }

        template <typename T>
        requires std::conjunction_v<std::negation<std::is_array<T>>, std::is_move_constructible<T>>
        static void ConstructMove(Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to)
            noexcept(std::is_nothrow_move_constructible_v<T>) {
            std::uninitialized_move_n(Ptr(from), 1, Ptr(to));
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_move_constructible<std::remove_extent_t<T>>>
        static void ConstructMove(Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to)
            noexcept(std::is_nothrow_move_constructible_v<std::remove_extent_t<T>>) {
            std::uninitialized_move(std::begin(Ref(from)), std::end(Ref(from)), std::begin(Ref(to)));
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_move_constructible<std::remove_extent_t<T>>>
        static void ConstructMove(Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to, std::size_t index)
            noexcept(std::is_nothrow_move_constructible_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT(index < std::extent_v<T>, "Out of bounds!");
            std::uninitialized_move_n(Ref(from) + index, 1, Ref(to) + index);
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_move_constructible<std::remove_extent_t<T>>>
        static void ConstructMove(Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to, std::size_t pos, std::size_t size)
            noexcept(std::is_nothrow_move_constructible_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT((pos + size) <= std::extent_v<T>, "Out of bounds!");
            std::uninitialized_move_n(Ref(from) + pos, size, Ref(to) + pos);
        }

        template <typename T>
        requires std::conjunction_v<std::negation<std::is_array<T>>, std::is_copy_assignable<T>>
        static void OperatorCopy(const Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to)
            noexcept(std::is_nothrow_copy_assignable_v<T>) {
            Ref(to) = Ref(from);
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_copy_assignable<std::remove_all_extents_t<T>>>
        static void OperatorCopy(const Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to)
            noexcept(std::is_nothrow_copy_assignable_v<std::remove_extent_t<T>>) {
            std::copy(std::cbegin(Ref(from)), std::cend(Ref(from)), std::begin(Ref(to)));
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_copy_assignable<std::remove_all_extents_t<T>>>
        static void OperatorCopy(const Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to, std::size_t index)
            noexcept(std::is_nothrow_copy_assignable_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT(index < std::extent_v<T>, "Out of bounds!");
            Ref(to)[index] = Ref(from)[index];
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_copy_assignable<std::remove_all_extents_t<T>>>
        static void OperatorCopy(const Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to, std::size_t pos, std::size_t size)
            noexcept(std::is_nothrow_copy_assignable_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT((pos + size) <= std::extent_v<T>, "Out of bounds!");
            std::copy(Ref(from) + pos, Ref(from) + pos + size, Ref(to) + pos);
        }

        template <typename T>
        requires std::conjunction_v<std::negation<std::is_array<T>>, std::is_move_assignable<T>>
        static void OperatorMove(Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to)
            noexcept(std::is_nothrow_move_assignable_v<T>) {
            Ref(to) = std::move(Ref(from));
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_move_assignable<std::remove_extent_t<T>>>
        static void OperatorMove(Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to)
            noexcept(std::is_nothrow_move_assignable_v<std::remove_extent_t<T>>) {
            std::move(std::cbegin(Ref(from)), std::cend(Ref(from)), std::begin(Ref(to)));
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_move_assignable<std::remove_extent_t<T>>>
        static void OperatorMove(Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to, std::size_t index)
            noexcept(std::is_nothrow_move_assignable_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT(index < std::extent_v<T>, "Out of bounds!");
            Ref(to)[index] = std::move(Ref(from)[index]);
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_move_assignable<std::remove_extent_t<T>>>
        static void OperatorMove(Storage<T>& HELENA_RESTRICT from, Storage<T>& HELENA_RESTRICT to, std::size_t pos, std::size_t size)
            noexcept(std::is_nothrow_move_assignable_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT((pos + size) <= std::extent_v<T>, "Out of bounds!");
            std::move(Ref(from) + pos, Ref(from) + pos + size, Ref(to) + pos);
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_copy_assignable<std::remove_all_extents_t<T>>>
        static void Copy(Storage<T>& storage, std::size_t from, std::size_t to, std::size_t size)
            noexcept(std::is_nothrow_copy_assignable_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT((from + size) <= std::extent_v<T> && (to + size) <= std::extent_v<T>, "Out of bounds!");
            std::copy(Ref(storage) + from, Ref(storage) + from + size, Ref(storage) + to);
        }

        template <typename T>
        requires std::conjunction_v<std::is_array<T>, std::is_move_assignable<std::remove_all_extents_t<T>>>
        static void Move(Storage<T>& storage, std::size_t from, std::size_t to, std::size_t size)
            noexcept(std::is_nothrow_move_assignable_v<std::remove_extent_t<T>>) {
            HELENA_ASSERT((from + size) <= std::extent_v<T> && (to + size) <= std::extent_v<T>, "Out of bounds!");
            std::move(Ref(storage) + from, Ref(storage) + from + size, Ref(storage) + to);
        }

        template <typename T>
        static T* Ptr(Storage<T>& storage) noexcept {
            return std::launder(reinterpret_cast<T*>(std::addressof(storage.m_Memory)));
        }

        template <typename T>
        static const T* Ptr(const Storage<T>& storage) noexcept {
            return std::launder(reinterpret_cast<const T*>(std::addressof(storage.m_Memory)));
        }

        template <typename T>
        static T& Ref(Storage<T>& storage) noexcept {
            return *std::launder(reinterpret_cast<T*>(std::addressof(storage.m_Memory)));
        }

        template <typename T>
        static const T& Ref(const Storage<T>& storage) noexcept {
            return *std::launder(reinterpret_cast<const T*>(std::addressof(storage.m_Memory)));
        }
    };
}

#endif // HELENA_TYPES_ALIGNEDSTORAGE_HPP