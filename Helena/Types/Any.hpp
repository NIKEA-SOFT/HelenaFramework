#ifndef HELENA_TYPES_ANY_HPP
#define HELENA_TYPES_ANY_HPP

#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/Specialization.hpp>
#include <Helena/Traits/PowerOf2.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Types/CompressedPair.hpp>
#include <Helena/Types/Hash.hpp>

#include <bit>
#include <cstddef>
#include <new>
#include <memory>
#include <typeinfo>
#include <tuple>
#include <utility>

namespace Helena::Types
{
    template <std::size_t Capacity = sizeof(void*), std::size_t Alignment = alignof(void*), typename Alloc = std::allocator<std::byte>>
    requires Traits::IsPowerOf2<Alignment>
    class Any
    {
        static_assert(std::is_same_v<typename Alloc::value_type, std::byte>, "Allocator value_type != std::byte");

    public:
        using allocator_type    = Alloc;
        using allocator_traits  = std::allocator_traits<allocator_type>;
        using value_type        = typename allocator_traits::value_type;
        using pointer           = typename allocator_traits::pointer;
        using const_pointer     = typename allocator_traits::const_pointer;

        using Hasher            = Hash<std::uint64_t>;
        using HasherValue       = Hasher::value_type;

        static constexpr std::size_t DefaultCapacity = sizeof(void*);
        static constexpr std::size_t DefaultAlignment = alignof(void*);
        static constexpr std::size_t DisableSBO = 0;

        template <typename T>
        static constexpr bool AllowedParam = std::conjunction_v<
            std::negation<std::is_same<std::decay_t<T>, void>>,
            std::negation<std::is_lvalue_reference<T>>,
            std::negation<std::is_function<std::remove_pointer_t<T>>>,
            std::negation<std::is_member_function_pointer<T>>>;

        template <typename T, typename... Args>
        static constexpr bool RequiredParams = std::conjunction_v<
            std::bool_constant<AllowedParam<T>>,
            std::conditional_t<Traits::Arguments<Args...>::Orphan,
                std::is_default_constructible<std::decay_t<T>>,
                std::is_constructible<std::decay_t<T>, Args...>>>;

        template <typename T>
        static constexpr bool RequiredValue = std::conjunction_v<
            std::negation<std::is_same<std::decay_t<T>, void>>,
            std::negation<std::is_same<std::decay_t<T>, Any>>,
            std::negation<std::is_function<std::remove_pointer_t<T>>>,
            std::negation<std::is_member_function_pointer<T>>,
            std::negation<Traits::SpecializationOf<std::decay_t<T>, std::in_place_type_t>>>;

    private:
        template <typename T>
        static constexpr auto HashOf = Hasher::template From<T>();

        enum class ECommand : std::uint8_t {
            Copy,
            Move,
            Destroy
        };

        template <auto... Fn>
        struct CaptureFunctions {};

        template <ECommand>
        struct Command {};

        struct Commands
        {
            using Tuple = std::tuple<
                void (*)(const Any&, Any&),     // Copy
                void (*)(Any&, Any&),           // Move
                void (*)(Any&)                  // Destroy
            >;

            template <typename T>
            static void Copy(const Any& current, Any& other)
            {
                if constexpr(Placementable<T>) {
                    std::construct_at(std::bit_cast<T*>(std::addressof(other.m_Storage.m_Buffer)),
                        *std::bit_cast<const T*>(std::addressof(current.m_Storage.m_Buffer)));
                    other.m_Pair.First() = current.m_Pair.First();
                } else {
                    other.template Initialize<T>(*static_cast<const T*>(current.m_Instance));
                }
            }

            template <typename T>
            static void Move(Any& current, Any& other) noexcept
            {
                if constexpr(Placementable<T>) {
                    std::construct_at(std::bit_cast<T*>(std::addressof(other.m_Storage.m_Buffer)),
                        std::move(*std::bit_cast<T*>(std::addressof(current.m_Storage.m_Buffer))));
                    other.m_Pair.First() = std::exchange(current.m_Pair.First(), nullptr);
                    return;
                } else if constexpr(!allocator_traits::is_always_equal::value) {
                    if(current.m_Pair.Second() != other.m_Pair.Second()) {
                        other.template Initialize<T>(std::move(*static_cast<T*>(current.m_Instance)));
                        current.VTable<ECommand::Destroy>(current);
                        return;
                    }
                }

                other.m_Instance = std::exchange(current.m_Instance, nullptr);
                other.m_Pair.First() = std::exchange(current.m_Pair.First(), nullptr);
            }

            template <typename T>
            static void Destroy(Any& current) noexcept
            {
                if constexpr(Placementable<T>) {
                    std::destroy_at(std::bit_cast<T*>(std::addressof(current.m_Storage.m_Buffer)));
                } else {
                    allocator_traits::destroy(current.m_Pair.Second(), static_cast<T*>(current.m_Instance));
                    allocator_traits::deallocate(current.m_Pair.Second(), static_cast<pointer>(current.m_Instance), sizeof(T));
                    current.m_Instance = nullptr;
                }

                current.m_Pair.First() = nullptr;
            }

            template <ECommand Index, typename... Args>
            constexpr decltype(auto) operator()(Command<Index>, Args&&... args) const noexcept {
                return std::get<static_cast<std::underlying_type_t<ECommand>>(Index)>(m_Functors)(std::forward<Args>(args)...);
            }

            template <typename T, auto... Fn>
            constexpr Commands(std::type_identity<T>, const CaptureFunctions<Fn...>) noexcept
                : m_Functors{Fn...}
                , m_Name{Traits::NameOf<T>}
                , m_Hash{HashOf<T>}
                , m_Size{sizeof(T)}
                , m_Placementable{Placementable<T>} {}

            [[nodiscard]] constexpr auto Data(const Any& current) const noexcept {
                const void* result[]{current.m_Instance, std::addressof(current.m_Storage.m_Buffer)};
                return result[m_Placementable];
            }

            [[nodiscard]] constexpr auto TypeName() const noexcept {
                return m_Name;
            }

            [[nodiscard]] constexpr auto TypeHash() const noexcept {
                return m_Hash;
            }

            [[nodiscard]] constexpr auto Size() const noexcept {
                return m_Size;
            }

            [[nodiscard]] constexpr bool UsesSBO() const noexcept {
                return m_Placementable;
            }

            const Tuple m_Functors;
            const char* const m_Name;
            const HasherValue m_Hash;
            const std::size_t m_Size;
            const bool m_Placementable;
        };

        template <typename T>
        static constexpr auto CommandSwitch = Commands{
            std::type_identity<T>{},
            CaptureFunctions<
                &Commands::template Copy<T>,
                &Commands::template Move<T>,
                &Commands::template Destroy<T>
            >{}
        };

        struct Storage {
            alignas(Alignment) value_type m_Buffer[Capacity + !Capacity];
        };

        template <typename T>
        static constexpr bool Placementable = Capacity && sizeof(T) <= sizeof(Storage) && alignof(T) <= alignof(Storage)
                                            && std::is_nothrow_move_constructible_v<T>;

    public:
        explicit Any() noexcept : m_Instance{}, m_Pair{} {}

        explicit Any(std::allocator_arg_t, const Alloc& alloc) noexcept
            : m_Instance{}
            , m_Pair{std::piecewise_construct
                , std::forward_as_tuple(nullptr)
                , std::forward_as_tuple(alloc)} {}

        template <typename T, typename... Args>
        requires RequiredParams<T, Args...>
        explicit Any(std::in_place_type_t<T>, Args&&... args)
            : m_Instance{}
            , m_Pair{std::piecewise_construct
                , std::forward_as_tuple(nullptr)
                , std::forward_as_tuple(Alloc())} {
            Initialize<std::decay_t<T>>(std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        requires RequiredParams<T, Args...>
        explicit Any(std::allocator_arg_t, const Alloc& alloc, std::in_place_type_t<T>, Args&&... args)
            : m_Instance{}
            , m_Pair{std::piecewise_construct
                , std::forward_as_tuple(nullptr)
                , std::forward_as_tuple(alloc)} {
            Initialize<std::decay_t<T>>(std::forward<Args>(args)...);
        }

        template <typename T>
        requires RequiredValue<T>
        explicit Any(T&& value)
            : Any(std::in_place_type<std::decay_t<T>>, std::forward<T>(value)) {}

        template <typename T>
        requires RequiredValue<T>
        explicit Any(T&& value, const Alloc& alloc)
            : Any(std::allocator_arg, alloc, std::in_place_type<std::decay_t<T>>, std::forward<T>(value)) {}

        ~Any() noexcept {
            Reset();
        }

        Any(const Any& other) : Any(std::allocator_arg,
            allocator_traits::select_on_container_copy_construction(other.m_Pair.Second()))
        {
            if(!other.Empty()) {
                other.template VTable<ECommand::Copy>(other, *this);
            }
        }

        Any(const Any& other, const Alloc& alloc) : Any(std::allocator_arg, alloc)
        {
            if(!other.Empty()) {
                other.template VTable<ECommand::Copy>(other, *this);
            }
        }

        Any(Any&& other) noexcept : Any(std::allocator_arg, std::move(other.m_Pair.Second()))
        {
            if(!other.Empty()) {
                other.template VTable<ECommand::Move>(other, *this);
            }
        }

        Any(Any&& other, const Alloc& alloc) noexcept : Any(std::allocator_arg, alloc)
        {
            if(!other.Empty()) {
                other.template VTable<ECommand::Move>(other, *this);
            }
        }

        Any& operator=(const Any& other)
        {
            Reset();

            if constexpr(!allocator_traits::is_always_equal::value
                && allocator_traits::propagate_on_container_copy_assignment::value) {
                if(m_Pair.Second() != other.m_Pair.Second()) {
                    m_Pair.Second() = other.m_Pair.Second();
                }
            }

            if(!other.Empty()) {
                other.template VTable<ECommand::Copy>(other, *this);
            }

            return *this;
        }

        Any& operator=(Any&& other) noexcept
        {
            Reset();

            if constexpr(!allocator_traits::is_always_equal::value
                && allocator_traits::propagate_on_container_move_assignment::value) {
                if(m_Pair.Second() != other.m_Pair.Second()) {
                    m_Pair.Second() = std::move(other.m_Pair.Second());
                }
            }

            if(!other.Empty()) {
                other.template VTable<ECommand::Move>(other, *this);
            }

            return *this;
        }

        template <typename T, typename... Args>
        requires RequiredParams<T, Args...>
        void Create(Args&&... args) {
            Reset();
            Initialize<std::decay_t<T>>(std::forward<Args>(args)...);
        }

        [[nodiscard]] constexpr bool Empty() const noexcept {
            return !m_Pair.First();
        }

        // Check the type that is stored in Any
        template <typename T>
        [[nodiscard]] constexpr bool Contain() const noexcept {
            if(!Empty()) [[likely]] {
                constexpr auto hash = HashOf<std::decay_t<T>>;
                return m_Pair.First()->TypeHash() == hash;
            }
            return false;
        }

        [[nodiscard]] constexpr auto TypeName() const noexcept -> const char*
        {
            if(!Empty()) {
                return m_Pair.First()->TypeName();
            }

            return nullptr;
        }

        [[nodiscard]] constexpr auto TypeHash() const noexcept
        {
            if(!Empty()) {
                return m_Pair.First()->TypeHash();
            }
            return HasherValue{};
        }

        [[nodiscard]] constexpr bool UsesSBO() const noexcept {
            return m_Pair.First() && m_Pair.First()->UsesSBO();
        }

        // Works similar to any_cast from STL
        // if Any contain the value then As<int>, As<int&> and As<int*> is used.
        // If Any contain the pointer then As<int*&> or As<int**> is used.
        template <typename T>
        [[nodiscard]] constexpr decltype(auto) As()
        {
            void* instance = DataPtr<std::remove_cvref_t<std::remove_pointer_t<T>>>();
            if constexpr(std::is_lvalue_reference_v<T>) {
                CheckAndThrow(instance);
                return *static_cast<std::remove_cvref_t<T>*>(instance);
            } else if constexpr(std::is_pointer_v<T>) {
                return static_cast<T>(instance);
            } else {
                CheckAndThrow(instance);
                return *static_cast<std::remove_const_t<std::remove_cvref_t<T>>*>(instance);
            }
        }

        template <typename T>
        [[nodiscard]] constexpr decltype(auto) As() const
        {
            const void* instance = DataPtr<std::remove_cvref_t<std::remove_pointer_t<T>>>();
            if constexpr(std::is_lvalue_reference_v<T>) {
                CheckAndThrow(instance);
                return *static_cast<const std::remove_cvref_t<T>*>(instance);
            } else if constexpr(std::is_pointer_v<T>) {
                return *static_cast<const std::remove_pointer_t<T>**>(instance);
            } else {
                CheckAndThrow(instance);
                return *static_cast<const std::remove_cvref_t<T>*>(instance);
            }
        }

        [[nodiscard]] constexpr bool operator==(const Any& other) const noexcept {
            return m_Pair.First() && other.m_Pair.First()
                && m_Pair.First()->TypeHash() == other.m_Pair.First()->TypeHash();
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return !Empty();
        }

        constexpr void Reset() noexcept
        {
            if(!Empty()) {
                VTable<ECommand::Destroy>(*this);
            }
        }

    private:
        template <typename T, typename... Args>
        requires RequiredParams<T, Args...>
        constexpr void Initialize(Args&&... args)
            noexcept(noexcept(Construct<std::decay_t<T>>(std::forward<Args>(args)...))) {
            using Type = std::decay_t<T>;
            m_Pair.First() = std::addressof(CommandSwitch<T>);
            Construct<Type>(std::forward<Args>(args)...);
        }

        template <typename T>
        [[nodiscard]] constexpr void* DataPtr() noexcept {
            return const_cast<void*>(static_cast<const Any*>(this)->DataPtr<T>());
        }

        template <typename T>
        [[nodiscard]] constexpr const void* DataPtr() const noexcept
        {
            // branch suppressing
            if(const auto vtable = m_Pair.First()) [[likely]] {
                const void* result[]{nullptr, vtable->Data(*this)};
                return result[vtable->TypeHash() == HashOf<T>];
            }

            return nullptr;
        }

        template <ECommand Index, typename... Args>
        constexpr decltype(auto) VTable(Args&&... args) const {
            constexpr auto cmd = Command<Index>{};
            return (*m_Pair.First())(cmd, std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        constexpr void Construct(Args&&... args)
        {
            if constexpr(Placementable<T>) {
                std::construct_at(std::bit_cast<T*>(&m_Storage.m_Buffer), std::forward<Args>(args)...);
            } else {
                m_Instance = allocator_traits::allocate(m_Pair.Second(), sizeof(T));
                try {
                    allocator_traits::construct(m_Pair.Second(), static_cast<T*>(m_Instance), std::forward<Args>(args)...);
                } catch(...) {
                    allocator_traits::deallocate(m_Pair.Second(), static_cast<pointer>(m_Instance), sizeof(T));
                    m_Instance = nullptr;
                    throw;
                }
            }
        }

        static void CheckAndThrow(const void* data) {
            if(!data) [[unlikely]] {
                ThrowBadCast();
            }
        }

        [[noreturn]] static void ThrowBadCast()
        {
            class BadCast : public std::bad_cast {
                [[nodiscard]] const char* what() const noexcept override {
                    return "Bad cast in Any container";
                }
            };

            throw BadCast{};
        }

    private:
        union {
            void* m_Instance;
            Storage m_Storage;
        };

        CompressedPair<const Commands*, Alloc> m_Pair;
    };
}

#endif // HELENA_TYPES_ANY_HPP
