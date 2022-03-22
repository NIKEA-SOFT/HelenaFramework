// Modification of basic_any, taked from:
// https://github.com/skypjack/entt

#ifndef HELENA_TYPES_ANY_HPP
#define HELENA_TYPES_ANY_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Types/Hash.hpp>
#include <Helena/Traits/Constness.hpp>
#include <Helena/Traits/CVRefPtr.hpp>

#include <cstdint>
#include <memory>
#include <new>

namespace Helena::Types
{
    template<std::size_t Len = sizeof(double[2]), std::size_t = alignof(typename std::aligned_storage_t<Len + !Len>)>
    class Any;

    /**
     * @brief A SBO friendly, type-safe container for single values of any type.
     * @tparam Len Size of the storage reserved for the small buffer optimization.
     * @tparam Align Optional alignment requirement.
     */
    template<std::size_t Len, std::size_t Align>
    class Any {
        /**
         * @brief Provides the member constant `value` to true if a given type is an
         * iterator, false otherwise.
         * @tparam Type The type to test.
         */
        template<typename Type, typename = void>
        struct is_iterator : std::false_type {};


        /*! @copydoc is_iterator */
        template<typename Type>
        struct is_iterator<Type, std::void_t<typename std::iterator_traits<Type>::iterator_category>>
            : std::true_type
        {};

        /**
         * @brief Helper variable template.
         * @tparam Type The type to test.
         */
        template<typename Type>
        static constexpr bool is_iterator_v = is_iterator<Type>::value;

        template<std::size_t N>
        struct choice_t : choice_t<N - 1> {};

        template<>
        struct choice_t<0> {};

        template <std::size_t N>
        static constexpr choice_t<N> choice{};

        template<typename>
        [[nodiscard]] static constexpr bool is_equality_comparable(...) { return false; }

        template<typename Type>
        [[nodiscard]] static constexpr auto is_equality_comparable(choice_t<0>)
            -> decltype(std::declval<Type>() == std::declval<Type>()) {
            return true;
        }

        template<typename Type>
        [[nodiscard]] static constexpr auto is_equality_comparable(choice_t<1>)
            -> decltype(std::declval<typename Type::value_type>(), std::declval<Type>() == std::declval<Type>()) {
            if constexpr(is_iterator_v<Type>) {
                return true;
            } else if constexpr(std::is_same_v<typename Type::value_type, Type>) {
                return is_equality_comparable<Type>(choice<0>);
            } else {
                return is_equality_comparable<typename Type::value_type>(choice<2>);
            }
        }

        template<typename Type>
        [[nodiscard]] static constexpr auto is_equality_comparable(choice_t<2>)
            -> decltype(std::declval<typename Type::mapped_type>(), std::declval<Type>() == std::declval<Type>()) {
            return is_equality_comparable<typename Type::key_type>(choice<2>) && is_equality_comparable<typename Type::mapped_type>(choice<2>);
        }

        enum class operation : std::uint8_t { COPY, MOVE, DTOR, COMP, ADDR, CADDR, TYPE };
        enum class policy : std::uint8_t { OWNER, REF, CREF };

        using storage_type = std::aligned_storage_t<Len + !Len, Align>;
        using vtable_type = const void* (const operation, const Any&, void*);

        template<typename Type>
        static constexpr bool in_situ = Len && alignof(Type) <= alignof(storage_type)
            && sizeof(Type) <= sizeof(storage_type) && std::is_nothrow_move_constructible_v<Type>;

        template<typename Type>
        [[nodiscard]] static constexpr policy type_to_policy()
        {
            if constexpr(std::is_lvalue_reference_v<Type>)
            {
                if constexpr(std::is_const_v<std::remove_reference_t<Type>>) {
                    return policy::CREF;
                } else {
                    return policy::REF;
                }
            } else {
                return policy::OWNER;
            }
        }

        template<typename Type>
        [[nodiscard]] static bool compare(const void* lhs, const void* rhs) {
            if constexpr(!std::is_function_v<Type> && is_equality_comparable<Type>(choice<2>)) {
                return *static_cast<const Type*>(lhs) == *static_cast<const Type*>(rhs);
            } else {
                return lhs == rhs;
            }
        }

        template<typename Type>
        [[nodiscard]] static const void* basic_vtable([[maybe_unused]] const operation op, [[maybe_unused]] const Any& from, [[maybe_unused]] void* to) 
        {
            static_assert(std::is_same_v<std::remove_reference_t<std::remove_const_t<Type>>, Type>, "Invalid type");

            if constexpr(!std::is_void_v<Type>) 
            {
                const Type* instance = (in_situ<Type> && from.mode == policy::OWNER)
                    ? std::launder(reinterpret_cast<const Type*>(&from.storage))
                    : static_cast<const Type*>(from.instance);

                switch(op) 
                {
                    case operation::COPY: {
                        if constexpr(std::is_copy_constructible_v<Type>) {
                            static_cast<Any*>(to)->Create<Type>(*instance);
                        }
                    } break;
                    case operation::MOVE: 
                    {
                        if constexpr(in_situ<Type>) {
                            if(from.mode == policy::OWNER) {
                                return new (&static_cast<Any*>(to)->storage) Type{std::move(*const_cast<Type*>(instance))};
                            }
                        }

                        return (static_cast<Any*>(to)->instance = std::exchange(const_cast<Any&>(from).instance, nullptr));
                    }
                    case operation::DTOR:
                    {
                        if(from.mode == policy::OWNER) 
                        {
                            if constexpr(in_situ<Type>) {
                                instance->~Type();
                            } else if constexpr(std::is_array_v<Type>) {
                                delete[] instance;
                            } else {
                                delete instance;
                            }
                        }
                    } break;
                    case operation::COMP: return compare<Type>(instance, (*static_cast<const Any**>(to))->data()) ? to : nullptr;
                    case operation::ADDR: {
                        if(from.mode == policy::CREF) {
                            return nullptr;
                        }
                    } [[fallthrough]];
                    case operation::CADDR: return instance;
                    case operation::TYPE: { 
                        *static_cast<std::uint64_t*>(to) = Hash::Get<Type, std::uint64_t>();
                    } break;
                }
            }

            return nullptr;
        }

        template<typename Type, typename... Args>
        void initialize([[maybe_unused]] Args &&... args) 
        {
            if constexpr(!std::is_void_v<Type>) 
            {
                if constexpr(std::is_lvalue_reference_v<Type>) {
                    static_assert(sizeof...(Args) == 1u && (std::is_lvalue_reference_v<Args> && ...), "Invalid arguments");
                    instance = (std::addressof(args), ...);
                } 
                else if constexpr(in_situ<Type>) 
                {
                    if constexpr(sizeof...(Args) != 0u && std::is_aggregate_v<Type>) {
                        new (&storage) Type{std::forward<Args>(args)...};
                    } else {
                        new (&storage) Type(std::forward<Args>(args)...);
                    }
                } 
                else 
                {
                    if constexpr(sizeof...(Args) != 0u && std::is_aggregate_v<Type>) {
                        instance = new Type{std::forward<Args>(args)...};
                    } else {
                        instance = new Type(std::forward<Args>(args)...);
                    }
                }
            }
        }

        Any(const Any& other, const policy pol) noexcept : instance{other.data()}, vtable{other.vtable}, mode{pol} {}

    public:
        /*! @brief Size of the internal storage. */
        static constexpr auto length = Len;
        /*! @brief Alignment requirement. */
        static constexpr auto alignment = Align;

        /*! @brief Default constructor. */
        Any() noexcept : instance{}, vtable{&basic_vtable<void>}, mode{policy::OWNER} {}

        /**
         * @brief Constructs a wrapper by directly initializing the new object.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @tparam Args Types of arguments to use to construct the new instance.
         * @param args Parameters to use to construct the instance.
         */
        template<typename Type, typename... Args>
        explicit Any(std::in_place_type_t<Type>, Args &&... args) 
            : instance{}
            , vtable{&basic_vtable<std::remove_const_t<std::remove_reference_t<Type>>>}
            , mode{type_to_policy<Type>()} {
            initialize<Type>(std::forward<Args>(args)...);
        }

        /**
         * @brief Constructs a wrapper that holds an unmanaged object.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @param value An instance of an object to use to initialize the wrapper.
         */
        template<typename Type>
        Any(std::reference_wrapper<Type> value) noexcept : Any{} {
            // invokes deprecated assignment operator (and avoids issues with vs2017)
            *this = value;
        }

        /**
         * @brief Constructs a wrapper from a given value.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @param value An instance of an object to use to initialize the wrapper.
         */
        template<typename Type, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Type>, Any>>>
        Any(Type&& value) : instance{}, vtable{&basic_vtable<std::decay_t<Type>>}, mode{policy::OWNER} {
            initialize<std::decay_t<Type>>(std::forward<Type>(value));
        }

        /**
         * @brief Copy constructor.
         * @param other The instance to copy from.
         */
        Any(const Any& other) : instance{}, vtable{&basic_vtable<void>}, mode{policy::OWNER} {
            other.vtable(operation::COPY, other, this);
        }

        /**
         * @brief Move constructor.
         * @param other The instance to move from.
         */
        Any(Any&& other) noexcept : instance{}, vtable{other.vtable}, mode{other.mode} {
            vtable(operation::MOVE, other, this);
        }

        /*! @brief Frees the internal storage, whatever it means. */
        ~Any() {
            vtable(operation::DTOR, *this, nullptr);
        }

        /**
         * @brief Copy assignment operator.
         * @param other The instance to copy from.
         * @return This any object.
         */
        Any& operator=(const Any& other) {
            Reset();
            other.vtable(operation::COPY, other, this);
            return *this;
        }

        /**
         * @brief Move assignment operator.
         * @param other The instance to move from.
         * @return This any object.
         */
        Any& operator=(Any&& other) noexcept {
            std::exchange(vtable, other.vtable)(operation::DTOR, *this, nullptr);
            other.vtable(operation::MOVE, other, this);
            mode = other.mode;
            return *this;
        }

        /**
         * @brief Value assignment operator.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @param value An instance of an object to use to initialize the wrapper.
         * @return This any object.
         */
        template<typename Type>
        std::enable_if_t<!std::is_same_v<std::decay_t<Type>, Any>, Any&> operator=(Type&& value) {
            Create<std::decay_t<Type>>(std::forward<Type>(value));
            return *this;
        }

        /**
         * @brief Returns the type hash of the contained object.
         * @return The type hash of the contained object, if any.
         */
        [[nodiscard]] std::uint64_t type_hash() const noexcept {
            std::uint64_t info{};
            vtable(operation::TYPE, *this, &info);
            return info;
        }

        /**
         * @brief Returns an opaque pointer to the contained instance.
         * @return An opaque pointer the contained instance, if any.
         */
        [[nodiscard]] const void* data() const noexcept {
            return vtable(operation::CADDR, *this, nullptr);
        }

        /*! @copydoc data */
        [[nodiscard]] void* data() noexcept {
            return const_cast<void*>(vtable(operation::ADDR, *this, nullptr));
        }

        /**
         * @brief Replaces the contained object by creating a new instance directly.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @tparam Args Types of arguments to use to construct the new instance.
         * @param args Parameters to use to construct the instance.
         */
        template<typename Type, typename... Args>
        void Create(Args &&... args) {
            std::exchange(vtable, &basic_vtable<std::remove_const_t<std::remove_reference_t<Type>>>)(operation::DTOR, *this, nullptr);
            mode = type_to_policy<Type>();
            initialize<Type>(std::forward<Args>(args)...);
        }

        /*! @brief Destroys contained object */
        void Reset() {
            std::exchange(vtable, &basic_vtable<void>)(operation::DTOR, *this, nullptr);
            mode = policy::OWNER;
        }

        /**
         * @brief Returns false if a wrapper is empty, true otherwise.
         * @return False if the wrapper is empty, true otherwise.
         */
        [[nodiscard]] explicit operator bool() const noexcept {
            return !(vtable(operation::CADDR, *this, nullptr) == nullptr);
        }

        /**
         * @brief Checks if two wrappers differ in their content.
         * @param other Wrapper with which to compare.
         * @return False if the two objects differ in their content, true otherwise.
         */
        [[nodiscard]] bool operator==(const Any& other) const noexcept {
            const Any* trampoline = &other;
            return type_hash() == other.type_hash() && (vtable(operation::COMP, *this, &trampoline) || !other.data());
        }

        /**
         * @brief Checks if two wrappers differ in their content.
         * @tparam Len Size of the storage reserved for the small buffer optimization.
         * @tparam Align Alignment requirement.
         * @param lhs A wrapper, either empty or not.
         * @param rhs A wrapper, either empty or not.
         * @return True if the two wrappers differ in their content, false otherwise.
         */
        template<std::size_t Length, std::size_t Alignment>
        [[nodiscard]] bool operator!=(const Any<Length, Alignment>& rhs) noexcept {
            return !(*this == rhs);
        }

        /**
         * @brief Aliasing constructor.
         * @return A wrapper that shares a reference to an unmanaged object.
         */
        [[nodiscard]] Any AsRef() noexcept {
            return Any{*this, (mode == policy::CREF ? policy::CREF : policy::REF)};
        }

        /*! @copydoc as_ref */
        [[nodiscard]] Any AsRef() const noexcept {
            return Any{*this, policy::CREF};
        }

        /**
         * @brief Returns true if a wrapper owns its object, false otherwise.
         * @return True if the wrapper owns its object, false otherwise.
         */
        [[nodiscard]] bool IsOwner() const noexcept {
            return (mode == policy::OWNER);
        }

    private:
        union { const void* instance; storage_type storage; };
        vtable_type* vtable;
        policy mode;
    };

    /**
     * @brief Performs type-safe access to the contained object.
     * @tparam Type Type to which conversion is required.
     * @tparam Len Size of the storage reserved for the small buffer optimization.
     * @tparam Align Alignment requirement.
     * @param data Target any object.
     * @return The element converted to the requested type.
     */
    template<typename Type, std::size_t Len, std::size_t Align>
    [[nodiscard]] Type AnyCast(const Any<Len, Align>& data) noexcept {
        const auto* const instance = AnyCast<std::remove_reference_t<Type>>(&data);
        HELENA_ASSERT(instance, "Invalid instance");
        return static_cast<Type>(*instance);
    }


    /*! @copydoc AnyCast */
    template<typename Type, std::size_t Len, std::size_t Align>
    [[nodiscard]] Type AnyCast(Any<Len, Align>& data) noexcept {
        // forces const on non-reference types to make them work also with wrappers for const references
        auto* const instance = AnyCast<std::remove_reference_t<const Type>>(&data);
        HELENA_ASSERT(instance, "Invalid instance");
        return static_cast<Type>(*instance);
    }


    /*! @copydoc AnyCast */
    template<typename Type, std::size_t Len, std::size_t Align>
    [[nodiscard]] Type AnyCast(Any<Len, Align>&& data) noexcept {
        // forces const on non-reference types to make them work also with wrappers for const references
        auto* const instance = AnyCast<std::remove_reference_t<const Type>>(&data);
        HELENA_ASSERT(instance, "Invalid instance");
        return static_cast<Type>(std::move(*instance));
    }


    /*! @copydoc AnyCast */
    template<typename Type, std::size_t Len, std::size_t Align>
    [[nodiscard]] const Type* AnyCast(const Any<Len, Align>* data) noexcept {
        return (data->type_hash() == Hash::Get<Traits::RemoveCVRefPtr<Type>, std::uint64_t>() ? 
            static_cast<const Type*>(data->data()) : nullptr);
    }


    /*! @copydoc AnyCast */
    template<typename Type, std::size_t Len, std::size_t Align>
    [[nodiscard]] Type* AnyCast(Any<Len, Align>* data) noexcept {
        // last attempt to make wrappers for const references return their values
        return (data->type_hash() == Hash::Get<Type, std::uint64_t>() ?
            static_cast<Type*>(static_cast<typename Traits::Constness<Any<Len, Align>, Type>::type*>(data)->data()) : nullptr);
    }


    /**
     * @brief Constructs a wrapper from a given type, passing it all arguments.
     * @tparam Type Type of object to use to initialize the wrapper.
     * @tparam Len Size of the storage reserved for the small buffer optimization.
     * @tparam Align Optional alignment requirement.
     * @tparam Args Types of arguments to use to construct the new instance.
     * @param args Parameters to use to construct the instance.
     * @return A properly initialized wrapper for an object of the given type.
     */
    template<typename Type, std::size_t Len = Any<>::length, std::size_t Align = Any<Len>::alignment, typename... Args>
    [[nodiscard]] Any<Len, Align> AnyCreate(Args &&... args) {
        return Any<Len, Align>{std::in_place_type<Type>, std::forward<Args>(args)...};
    }


    /**
     * @brief Forwards its argument and avoids copies for lvalue references.
     * @tparam Len Size of the storage reserved for the small buffer optimization.
     * @tparam Align Optional alignment requirement.
     * @tparam Type Type of argument to use to construct the new instance.
     * @param value Parameter to use to construct the instance.
     * @return A properly initialized and not necessarily owning wrapper.
     */
    template<std::size_t Len = Any<>::length, std::size_t Align = Any<Len>::alignment, typename Type>
    [[nodiscard]] Any<Len, Align> AnyForward(Type&& value) {
        return Any<Len, Align>{std::in_place_type<std::conditional_t<std::is_rvalue_reference_v<Type>,
            std::decay_t<Type>, Type>>, std::forward<Type>(value)};
    }
}

#endif // HELENA_TYPES_ANY_HPP
