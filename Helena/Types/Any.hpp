// Modification of Any, taked from:
// https://github.com/skypjack/entt

#ifndef HELENA_TYPES_ANY_HPP
#define HELENA_TYPES_ANY_HPP

#include <Helena/Types/Hash.hpp>
#include <Helena/Platform/Assert.hpp>

#include <cstddef>
#include <memory>
#include <new>
#include <utility>

namespace Helena::Types
{
    namespace Internal {
        /**
         * @brief Utility class to disambiguate overloaded functions.
         * @tparam N Number of choices available.
         */
        template<std::size_t N>
        struct choice_t
            // Unfortunately, doxygen cannot parse such a construct.
            : /*! @cond TURN_OFF_DOXYGEN */ choice_t<N - 1> /*! @endcond */
        {};

        /*! @copybrief choice_t */
        template<>
        struct choice_t<0> {};
    }

    template<std::size_t Len = sizeof(double[2]), std::size_t = alignof(max_align_t)>
    class Any;

    /**
     * @brief A SBO friendly, type-safe container for single values of any type.
     * @tparam Len Size of the storage reserved for the small buffer optimization.
     * @tparam Align Optional alignment requirement.
     */
    template<std::size_t Len, std::size_t Align>
    class Any
    {
        enum class EOperation : std::uint8_t {
            Copy,
            Move,
            Transfer,
            Assign,
            Destroy,
            Compare,
            Get
        };

        enum class EPolicy : std::uint8_t {
            Owner,
            Ref,
            CRef
        };

        template<typename, typename = void>
        struct has_iterator_category: std::false_type {};

        template<typename Type>
        struct has_iterator_category<Type, std::void_t<typename std::iterator_traits<Type>::iterator_category>>: std::true_type {};

         /**
          * @brief Provides the member constant `value` to true if a given type is an
          * iterator, false otherwise.
          * @tparam Type The type to test.
          */
        template<typename Type, typename = void>
        struct is_iterator : std::false_type {};

        /*! @copydoc is_iterator */
        template <typename Type>
        struct is_iterator<Type, std::enable_if_t<!std::is_same_v<std::remove_const_t<std::remove_pointer_t<Type>>, void>>>
            : has_iterator_category<Type> {};

        /**
         * @brief Helper variable template.
         * @tparam Type The type to test.
         */
        template<typename Type>
        static inline constexpr bool is_iterator_v = is_iterator<Type>::value;

        /**
         * @brief Provides the member constant `value` to true if a given type is
         * complete, false otherwise.
         * @tparam Type The type to test.
         */
        template<typename Type, typename = void>
        struct is_complete : std::false_type {};

        /*! @copydoc is_complete */
        template<typename Type>
        struct is_complete<Type, std::void_t<decltype(sizeof(Type))>> : std::true_type {};

        /**
         * @brief Helper variable template.
         * @tparam Type The type to test.
         */
        template<typename Type>
        static inline constexpr bool is_complete_v = is_complete<Type>::value;

        /**
        * @brief Variable template for the choice trick.
        * @tparam N Number of choices available.
        */
        template<std::size_t N>
        static inline constexpr Internal::choice_t<N> choice{};

        template<typename, typename = void>
        struct has_tuple_size_value : std::false_type {};

        template<typename Type>
        struct has_tuple_size_value<Type, std::void_t<decltype(std::tuple_size<const Type>::value)>> : std::true_type {};

        /**
         * @brief Provides the member constant `value` to true if a given type is
         * equality comparable, false otherwise.
         * @tparam Type The type to test.
         */
        template<typename Type, typename = void>
        struct is_equality_comparable : std::false_type {};

        template<typename>
        [[nodiscard]] static constexpr bool maybe_equality_comparable(Internal::choice_t<0>) {
            return true;
        }

        template<typename Type>
        [[nodiscard]] static constexpr auto maybe_equality_comparable(Internal::choice_t<1>) -> decltype(std::declval<typename Type::value_type>(), bool{}) {
            if constexpr(is_iterator_v<Type>) {
                return true;
            } else if constexpr(std::is_same_v<typename Type::value_type, Type>) {
                return maybe_equality_comparable<Type>(choice<0>);
            } else {
                return is_equality_comparable<typename Type::value_type>::value;
            }
        }

        template<typename Type>
        [[nodiscard]] constexpr std::enable_if_t<is_complete_v<std::tuple_size<std::remove_const_t<Type>>>, bool> maybe_equality_comparable(Internal::choice_t<2>) {
            if constexpr(has_tuple_size_value<Type>::value) {
                return unpack_maybe_equality_comparable<Type>(std::make_index_sequence<std::tuple_size<Type>::value>{});
            } else {
                return maybe_equality_comparable<Type>(choice<1>);
            }
        }

        /*! @copydoc is_equality_comparable */
        template<typename Type>
        struct is_equality_comparable<Type, std::void_t<decltype(std::declval<Type>() == std::declval<Type>())>>
            : std::bool_constant<maybe_equality_comparable<Type>(choice<2>)> {};

        /**
         * @brief Helper variable template.
         * @tparam Type The type to test.
         */
        template<typename Type>
        static inline constexpr bool is_equality_comparable_v = is_equality_comparable<Type>::value;

        struct Storage {
            alignas(Align) std::byte data[Len + !Len];
        };

        using VTable = const void* (const EOperation, const Any&, const void*);

    public:
        using hash_type = Hash<std::uint64_t>;

    private:
        template<typename Type>
        static constexpr bool in_situ = Len && alignof(Type) <= alignof(Storage) && sizeof(Type) <= sizeof(Storage) && std::is_nothrow_move_constructible_v<Type>;

        template<typename Type>
        static const void* VTableHandler([[maybe_unused]] const EOperation op, [[maybe_unused]] const Any& value, [[maybe_unused]] const void* other)
        {
            static_assert(!std::is_same_v<Type, void> && std::is_same_v<std::remove_cvref_t<Type>, Type>, "Invalid type");
            const Type* element = nullptr;

            if constexpr(in_situ<Type>) {
                element = value.Owner() ? reinterpret_cast<const Type*>(&value.storage) : static_cast<const Type*>(value.instance);
            } else {
                element = static_cast<const Type*>(value.instance);
            }

            switch(op)
            {
                case EOperation::Copy: {
                    if constexpr(std::is_copy_constructible_v<Type>) {
                        static_cast<Any*>(const_cast<void*>(other))->Initialize<Type>(*element);
                    }
                } break;
                case EOperation::Move:
                {
                    if constexpr(in_situ<Type>) {
                        if(value.Owner()) {
                            return new(&static_cast<Any*>(const_cast<void*>(other))->storage) Type{std::move(*const_cast<Type*>(element))};
                        }
                    }

                    return (static_cast<Any*>(const_cast<void*>(other))->instance = std::exchange(const_cast<Any&>(value).instance, nullptr));
                }
                case EOperation::Transfer: {
                    if constexpr(std::is_move_assignable_v<Type>) {
                        *const_cast<Type*>(element) = std::move(*static_cast<Type*>(const_cast<void*>(other)));
                        return other;
                    }
                } [[fallthrough]];
                case EOperation::Assign: {
                    if constexpr(std::is_copy_assignable_v<Type>) {
                        *const_cast<Type*>(element) = *static_cast<const Type*>(other);
                        return other;
                    }
                } break;
                case EOperation::Destroy: {
                    if constexpr(in_situ<Type>) {
                        element->~Type();
                    } else if constexpr(std::is_array_v<Type>) {
                        delete[] element;
                    } else {
                        delete element;
                    }
                } break;
                case EOperation::Compare: {
                    if constexpr(!std::is_function_v<Type> && !std::is_array_v<Type> && is_equality_comparable_v<Type>) {
                        return *element == *static_cast<const Type *>(other) ? other : nullptr;
                    } else {
                        return (element == other) ? other : nullptr;
                    }
                } break;
                case EOperation::Get: {
                    return element;
                }
            }

            return nullptr;
        }

        template<typename Type, typename... Args>
        void Initialize([[maybe_unused]] Args&&...args)
        {
            using ValueType = std::remove_cvref_t<Type>;
            if constexpr(!std::is_void_v<Type>)
            {
                vtable = VTableHandler<ValueType>;
                key = hash_type::template From<ValueType>();

                if constexpr(std::is_lvalue_reference_v<Type>) {
                    static_assert(sizeof...(Args) == 1u && (std::is_lvalue_reference_v<Args> && ...), "Invalid arguments");
                    mode = std::is_const_v<std::remove_reference_t<Type>> ? EPolicy::CRef : EPolicy::Ref;
                    instance = (std::addressof(args), ...);
                } else if constexpr(in_situ<ValueType>) {
                    if constexpr(sizeof...(Args) != 0u && std::is_aggregate_v<ValueType>) {
                        new(&storage) ValueType{std::forward<Args>(args)...};
                    } else {
                        new(&storage) ValueType(std::forward<Args>(args)...);
                    }
                } else {
                    if constexpr(sizeof...(Args) != 0u && std::is_aggregate_v<ValueType>) {
                        instance = new ValueType{std::forward<Args>(args)...};
                    } else {
                        instance = new ValueType(std::forward<Args>(args)...);
                    }
                }
            }
        }

        Any(const Any& other, const EPolicy pol) noexcept
            : instance{other.Data()}
            , key{other.key}
            , vtable{other.vtable}
            , mode{pol} {}

    public:
        /*! @brief Size of the internal storage. */
        static constexpr auto length = Len;
        /*! @brief Alignment requirement. */
        static constexpr auto alignment = Align;

        /*! @brief Default constructor. */
        constexpr Any() noexcept
            : instance{}
            , key{hash_type::template From<void>()}
            , vtable{}
            , mode{EPolicy::Owner} {}

        /**
         * @brief Constructs a wrapper by directly initializing the new object.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @tparam Args Types of arguments to use to construct the new instance.
         * @param args Parameters to use to construct the instance.
         */
        template<typename Type, typename... Args>
        explicit Any(std::in_place_type_t<Type>, Args &&...args) : Any{} {
            Initialize<Type>(std::forward<Args>(args)...);
        }

        /**
         * @brief Constructs a wrapper from a given value.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @param value An instance of an object to use to initialize the wrapper.
         */
        template<typename Type, typename = std::enable_if_t<std::is_same_v<std::decay_t<Type>, Any>>>
        Any(Type&& value) : Any{} {
            Initialize<std::decay_t<Type>>(std::forward<Type>(value));
        }

        /**
         * @brief Copy constructor.
         * @param other The instance to copy from.
         */
        Any(const Any& other) : Any{} {
            if(other.vtable) {
                other.vtable(EOperation::Copy, other, this);
            }
        }

        /**
         * @brief Move constructor.
         * @param other The instance to move from.
         */
        Any(Any&& other) noexcept : instance{}, key{other.key}, vtable{other.vtable}, mode{other.mode} {
            if(other.vtable) {
                other.vtable(EOperation::Move, other, this);
            }
        }

        /*! @brief Frees the internal storage, whatever it means. */
        ~Any() {
            if(vtable && Owner()) {
                vtable(EOperation::Destroy, *this, nullptr);
            }
        }

        /**
         * @brief Copy assignment operator.
         * @param other The instance to copy from.
         * @return This any object.
         */
        Any& operator=(const Any& other) {
            Reset();

            if(other.vtable) {
                other.vtable(EOperation::Copy, other, this);
            }

            return *this;
        }

        /**
         * @brief Move assignment operator.
         * @param other The instance to move from.
         * @return This any object.
         */
        Any& operator=(Any&& other) noexcept {
            Reset();

            if(other.vtable) {
                other.vtable(EOperation::Move, other, this);
                key = other.key;
                vtable = other.vtable;
                mode = other.mode;
            }

            return *this;
        }

        /**
         * @brief Value assignment operator.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @param value An instance of an object to use to initialize the wrapper.
         * @return This any object.
         */
        template<typename Type, typename = std::enable_if_t<std::is_same_v<std::decay_t<Type>, Any>>>
        Any& operator=(Type&& value) {
            Create<std::decay_t<Type>>(std::forward<Type>(value));
            return *this;
        }

        /**
         * @brief Returns the object type if any, `hash_type::template From<void>()` otherwise.
         * @return The object type if any, `hash_type::template From<void>()` otherwise.
         */
        [[nodiscard]] hash_type::value_type Key() const noexcept {
            return key;
        }

        /**
         * @brief Returns an opaque pointer to the contained instance.
         * @return An opaque pointer the contained instance, if any.
         */
        [[nodiscard]] const void* Data() const noexcept {
            return vtable ? vtable(EOperation::Get, *this, nullptr) : nullptr;
        }

        /**
         * @brief Returns an opaque pointer to the contained instance.
         * @param hash Expected type.
         * @return An opaque pointer the contained instance, if any.
         */
        [[nodiscard]] const void* Data(hash_type::value_type hash) const noexcept {
            return this->key == hash ? Data() : nullptr;
        }

        /**
         * @brief Returns an opaque pointer to the contained instance.
         * @return An opaque pointer the contained instance, if any.
         */
        [[nodiscard]] void* Data() noexcept {
            return (!vtable || mode == EPolicy::CRef) ? nullptr : const_cast<void*>(vtable(EOperation::Get, *this, nullptr));
        }

        /**
         * @brief Returns an opaque pointer to the contained instance.
         * @param hash Expected type.
         * @return An opaque pointer the contained instance, if any.
         */
        [[nodiscard]] void* Data(hash_type::value_type hash) noexcept {
            return this->key == hash ? Data() : nullptr;
        }

        /**
         * @brief Replaces the contained object by creating a new instance directly.
         * @tparam Type Type of object to use to initialize the wrapper.
         * @tparam Args Types of arguments to use to construct the new instance.
         * @param args Parameters to use to construct the instance.
         */
        template<typename Type, typename... Args>
        void Create(Args &&...args) {
            Reset();
            Initialize<Type>(std::forward<Args>(args)...);
        }

        /**
         * @brief Assigns a value to the contained object without replacing it.
         * @param other The value to assign to the contained object.
         * @return True in case of success, false otherwise.
         */
        bool Assign(const Any& other) {
            if(vtable && mode != EPolicy::CRef && key == other.key) {
                return (vtable(EOperation::Assign, *this, other.Data()) != nullptr);
            }

            return false;
        }

        /*! @copydoc assign */
        bool Assign(Any&& other) {
            if(vtable && mode != EPolicy::CRef && key == other.key) {
                if(auto* val = other.Data(); val) {
                    return (vtable(EOperation::Transfer, *this, val) != nullptr);
                } else {
                    return (vtable(EOperation::Assign, *this, std::as_const(other).Data()) != nullptr);
                }
            }

            return false;
        }

        /*! @brief Destroys contained object */
        void Reset()
        {
            if(vtable && Owner()) {
                vtable(EOperation::Destroy, *this, nullptr);
            }

            key = hash_type::template From<void>();
            vtable = nullptr;
            mode = EPolicy::Owner;
        }

        /**
         * @brief Returns false if a wrapper is empty, true otherwise.
         * @return False if the wrapper is empty, true otherwise.
         */
        [[nodiscard]] explicit operator bool() const noexcept {
            return vtable != nullptr;
        }

        /**
         * @brief Checks if two wrappers differ in their content.
         * @param other Wrapper with which to compare.
         * @return False if the two objects differ in their content, true otherwise.
         */
        bool operator==(const Any& other) const noexcept {
            if(vtable && key == other.key) {
                return (vtable(EOperation::Compare, *this, other.Data()) != nullptr);
            }

            return (!vtable && !other.vtable);
        }

        /**
         * @brief Aliasing constructor.
         * @return A wrapper that shares a reference to an unmanaged object.
         */
        [[nodiscard]] Any AsRef() noexcept {
            return Any{*this, (mode == EPolicy::CRef ? EPolicy::CRef : EPolicy::Ref)};
        }

        /*! @copydoc as_ref */
        [[nodiscard]] Any AsRef() const noexcept {
            return Any{*this, EPolicy::CRef};
        }

        /**
         * @brief Returns true if a wrapper owns its object, false otherwise.
         * @return True if the wrapper owns its object, false otherwise.
         */
        [[nodiscard]] bool Owner() const noexcept {
            return (mode == EPolicy::Owner);
        }

        /**
        * @brief Returns true if a hash of T and the hash of a current object are equal, otherwise false.
        * @return True if hash of T and hash of current object are equal, false otherwise.
        */
        template <typename T>
        [[nodiscard]] bool EqualHash() const noexcept {
            return key == hash_type::template From<std::remove_cvref_t<T>>();
        }

    private:
        union {
            const void* instance;
            Storage storage;
        };
        hash_type::value_type key;
        VTable* vtable;
        EPolicy mode;
    };

    /**
     * @brief Checks if two wrappers differ in their content.
     * @tparam Len Size of the storage reserved for the small buffer optimization.
     * @tparam Align Alignment requirement.
     * @param lhs A wrapper, either empty or not.
     * @param rhs A wrapper, either empty or not.
     * @return True if the two wrappers differ in their content, false otherwise.
     */
    template<std::size_t Len, std::size_t Align>
    [[nodiscard]] inline bool operator!=(const Any<Len, Align>& lhs, const Any<Len, Align>& rhs) noexcept {
        return !(lhs == rhs);
    }

    /**
     * @brief Performs type-safe access to the contained object.
     * @tparam Type Type to which conversion is required.
     * @tparam Len Size of the storage reserved for the small buffer optimization.
     * @tparam Align Alignment requirement.
     * @param data Target any object.
     * @return The element converted to the requested type.
     */
    template<typename Type, std::size_t Len, std::size_t Align>
    Type AnyCast(const Any<Len, Align>& data) noexcept {
        const auto* const instance = AnyCast<std::remove_reference_t<Type>>(&data);
        HELENA_ASSERT(instance, "Invalid instance");
        return static_cast<Type>(*instance);
    }

    /*! @copydoc AnyCast */
    template<typename Type, std::size_t Len, std::size_t Align>
    Type AnyCast(Any<Len, Align>& data) noexcept {
        // forces const on non-reference types to make them work also with wrappers for const references
        auto* const instance = AnyCast<std::remove_reference_t<const Type>>(&data);
        HELENA_ASSERT(instance, "Invalid instance");
        return static_cast<Type>(*instance);
    }

    /*! @copydoc AnyCast */
    template<typename Type, std::size_t Len, std::size_t Align>
    Type AnyCast(Any<Len, Align>&& data) noexcept {
        if constexpr(std::is_copy_constructible_v<std::remove_cvref_t<Type>>) {
            if(auto* const instance = AnyCast<std::remove_reference_t<Type>>(&data); instance) {
                return static_cast<Type>(std::move(*instance));
            } else {
                return AnyCast<Type>(data);
            }
        } else {
            auto* const instance = AnyCast<std::remove_reference_t<Type>>(&data);
            HELENA_ASSERT(instance, "Invalid instance");
            return static_cast<Type>(std::move(*instance));
        }
    }

    /*! @copydoc AnyCast */
    template<typename Type, std::size_t Len, std::size_t Align>
    const Type* AnyCast(const Any<Len, Align>* data) noexcept {
        constexpr auto key = Any<>::hash_type::template From<std::remove_cvref_t<Type>>();
        return static_cast<const Type*>(data->Data(key));
    }

    /*! @copydoc AnyCast */
    template<typename Type, std::size_t Len, std::size_t Align>
    Type* AnyCast(Any<Len, Align>* data) noexcept {
      if constexpr(std::is_const_v<Type>) {
          // last attempt to make wrappers for const references return their values
          return AnyCast<Type>(&std::as_const(*data));
      } else {
          constexpr auto key = Any<>::hash_type::template From<std::remove_cvref_t<Type>>();
          return static_cast<Type*>(data->Data(key));
      }
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
    Any<Len, Align> AnyCreate(Args &&...args) {
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
    Any<Len, Align> AnyForward(Type&& value) {
        return Any<Len, Align>{std::in_place_type<Type>, std::forward<Type>(value)};
    }
}

#endif // HELENA_TYPES_ANY_HPP
