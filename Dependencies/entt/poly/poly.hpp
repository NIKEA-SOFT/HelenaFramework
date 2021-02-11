#ifndef ENTT_POLY_POLY_HPP
#define ENTT_POLY_POLY_HPP


#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "../config/config.h"
#include "../core/any.hpp"
#include "../core/type_info.hpp"
#include "../core/type_traits.hpp"


namespace entt {


/*! @brief Inspector class used to infer the type of the virtual table. */
struct poly_inspector {
    /**
     * @brief Generic conversion operator (definition only).
     * @tparam Type Type to which conversion is requested.
     */
    template <class Type>
    operator Type &&() const;

    /**
     * @brief Dummy invocation function (definition only).
     * @tparam Member Index of the function to invoke.
     * @tparam Args Types of arguments to pass to the function.
     * @param args The arguments to pass to the function.
     * @return A poly inspector convertible to any type.
     */
    template<auto Member, typename... Args>
    poly_inspector invoke(Args &&... args) const;

    /*! @copydoc invoke */
    template<auto Member, typename... Args>
    poly_inspector invoke(Args &&... args);
};


/**
 * @brief Static virtual table factory.
 * @tparam Concept Concept descriptor.
 */
template<typename Concept>
class poly_vtable {
    using inspector = typename Concept::template type<poly_inspector>;

    template<typename Ret, typename... Args>
    static auto vtable_entry(Ret(*)(inspector &, Args...)) -> Ret(*)(any &, Args...);

    template<typename Ret, typename... Args>
    static auto vtable_entry(Ret(*)(const inspector &, Args...)) -> Ret(*)(const any &, Args...);

    template<typename Ret, typename... Args>
    static auto vtable_entry(Ret(*)(Args...)) -> Ret(*)(const any &, Args...);

    template<typename Ret, typename... Args>
    static auto vtable_entry(Ret(inspector:: *)(Args...)) -> Ret(*)(any &, Args...);

    template<typename Ret, typename... Args>
    static auto vtable_entry(Ret(inspector:: *)(Args...) const) -> Ret(*)(const any &, Args...);

    template<auto... Candidate>
    static auto make_vtable(value_list<Candidate...>)
    -> std::tuple<decltype(vtable_entry(Candidate))...>;

    template<typename... Func>
    [[nodiscard]] static constexpr auto make_vtable(type_list<Func...>)  {
        if constexpr(sizeof...(Func) == 0) {
            return decltype(make_vtable(typename Concept::template impl<inspector>{})){};
        } else if constexpr((std::is_function_v<Func> && ...)) {
            return decltype(std::make_tuple(vtable_entry(std::declval<Func inspector:: *>())...)){};
        }
    }

    template<typename Type, auto Candidate, typename Ret, typename Any, typename... Args>
    static void fill_vtable_entry(Ret(* &entry)(Any &, Args...)) {
        if constexpr(std::is_invocable_r_v<Ret, decltype(Candidate), Args...>) {
            entry = +[](Any &, Args... args) -> Ret {
                return std::invoke(Candidate, std::forward<Args>(args)...);
            };
        } else {
            entry = +[](Any &any, Args... args) -> Ret {
                return static_cast<Ret>(std::invoke(Candidate, any_cast<constness_as_t<Type, Any> &>(any), std::forward<Args>(args)...));
            };
        }
    }

    template<typename Type, auto... Index>
    [[nodiscard]] static auto fill_vtable(std::index_sequence<Index...>) {
        type impl{};
        (fill_vtable_entry<Type, value_list_element_v<Index, typename Concept::template impl<Type>>>(std::get<Index>(impl)), ...);
        return impl;
    }

public:
    /*! @brief Virtual table type. */
    using type = decltype(make_vtable(Concept{}));

    /**
     * @brief Returns a static virtual table for a specific concept and type.
     * @tparam Type The type for which to generate the virtual table.
     * @return A static virtual table for the given concept and type.
     */
    template<typename Type>
    [[nodiscard]] static const auto * instance() {
        static_assert(std::is_same_v<Type, std::decay_t<Type>>, "Type differs from its decayed form");
        static const auto vtable = fill_vtable<Type>(std::make_index_sequence<Concept::template impl<Type>::size>{});
        return &vtable;
    }
};


/**
 * @brief Poly base class used to inject functionalities into concepts.
 * @tparam Poly The outermost poly class.
 */
template<typename Poly>
struct poly_base {
    /**
     * @brief Invokes a function from the static virtual table.
     * @tparam Member Index of the function to invoke.
     * @tparam Args Types of arguments to pass to the function.
     * @param self A reference to the poly object that made the call.
     * @param args The arguments to pass to the function.
     * @return The return value of the invoked function, if any.
     */
    template<auto Member, typename... Args>
    [[nodiscard]] decltype(auto) invoke(const poly_base &self, Args &&... args) const {
        const auto &poly = static_cast<const Poly &>(self);
        return std::get<Member>(*poly.vtable)(poly.storage, std::forward<Args>(args)...);
    }

    /*! @copydoc invoke */
    template<auto Member, typename... Args>
    [[nodiscard]] decltype(auto) invoke(poly_base &self, Args &&... args) {
        auto &poly = static_cast<Poly &>(self);
        return std::get<Member>(*poly.vtable)(poly.storage, std::forward<Args>(args)...);
    }
};


/**
 * @brief Shortcut for calling `poly_base<Type>::invoke`.
 * @tparam Member Index of the function to invoke.
 * @tparam Poly A fully defined poly object.
 * @tparam Args Types of arguments to pass to the function.
 * @param self A reference to the poly object that made the call.
 * @param args The arguments to pass to the function.
 * @return The return value of the invoked function, if any.
 */
template<auto Member, typename Poly, typename... Args>
decltype(auto) poly_call(Poly &&self, Args &&... args) {
    return std::forward<Poly>(self).template invoke<Member>(self, std::forward<Args>(args)...);
}


/**
 * @brief Static polymorphism made simple and within everyone's reach.
 *
 * Static polymorphism is a very powerful tool in C++, albeit sometimes
 * cumbersome to obtain.<br/>
 * This class aims to make it simple and easy to use.
 *
 * @note
 * Both deduced and defined static virtual tables are supported.<br/>
 * Moreover, the `poly` class template also works with unmanaged objects.
 *
 * @tparam Concept Concept descriptor.
 */
template<typename Concept>
class poly: private Concept::template type<poly_base<poly<Concept>>> {
    /*! @brief A poly base is allowed to snoop into a poly object. */
    friend struct poly_base<poly<Concept>>;

    using vtable_type = typename poly_vtable<Concept>::type;

public:
    /*! @brief Concept type. */
    using concept_type = typename Concept::template type<poly_base<poly<Concept>>>;

    /*! @brief Default constructor. */
    poly() ENTT_NOEXCEPT
        : storage{},
          vtable{}
    {}

    /**
     * @brief Constructs a poly by directly initializing the new object.
     * @tparam Type Type of object to use to initialize the poly.
     * @tparam Args Types of arguments to use to construct the new instance.
     * @param args Parameters to use to construct the instance.
     */
    template<typename Type, typename... Args>
    explicit poly(std::in_place_type_t<Type>, Args &&... args)
        : storage{std::in_place_type<Type>, std::forward<Args>(args)...},
          vtable{poly_vtable<Concept>::template instance<std::remove_const_t<std::remove_reference_t<Type>>>()}
    {}

    /**
     * @brief Constructs a poly that holds an unmanaged object.
     * @tparam Type Type of object to use to initialize the poly.
     * @param value An instance of an object to use to initialize the poly.
     */
    template<typename Type>
    poly(std::reference_wrapper<Type> value)
        : poly{std::in_place_type<Type &>, &value.get()}
    {}

    /**
     * @brief Constructs a poly from a given value.
     * @tparam Type Type of object to use to initialize the poly.
     * @param value An instance of an object to use to initialize the poly.
     */
    template<typename Type, typename = std::enable_if_t<!std::is_same_v<std::remove_cv_t<std::remove_reference_t<Type>>, poly>>>
    poly(Type &&value) ENTT_NOEXCEPT
        : poly{std::in_place_type<std::remove_cv_t<std::remove_reference_t<Type>>>, std::forward<Type>(value)}
    {}

    /**
     * @brief Copy constructor.
     * @param other The instance to copy from.
     */
    poly(const poly &other) = default;

    /**
     * @brief Move constructor.
     * @param other The instance to move from.
     */
    poly(poly &&other) ENTT_NOEXCEPT
        : poly{}
    {
        swap(*this, other);
    }

    /**
     * @brief Assignment operator.
     * @param other The instance to assign from.
     * @return This poly object.
     */
    poly & operator=(poly other) {
        swap(other, *this);
        return *this;
    }

    /**
     * @brief Returns the type of the contained object.
     * @return The type of the contained object, if any.
     */
    [[nodiscard]] type_info type() const ENTT_NOEXCEPT {
        return storage.type();
    }

    /**
     * @brief Returns an opaque pointer to the contained instance.
     * @return An opaque pointer the contained instance, if any.
     */
    [[nodiscard]] const void * data() const ENTT_NOEXCEPT {
        return storage.data();
    }

    /*! @copydoc data */
    [[nodiscard]] void * data() ENTT_NOEXCEPT {
        return storage.data();
    }

    /**
     * @brief Replaces the contained object by creating a new instance directly.
     * @tparam Type Type of object to use to initialize the poly.
     * @tparam Args Types of arguments to use to construct the new instance.
     * @param args Parameters to use to construct the instance.
     */
    template<typename Type, typename... Args>
    void emplace(Args &&... args) {
        storage.emplace<Type>(std::forward<Args>(args)...);
        vtable = poly_vtable<Concept>::template instance<Type>();
    }

    /**
     * @brief Returns false if a poly is empty, true otherwise.
     * @return False if the poly is empty, true otherwise.
     */
    [[nodiscard]] explicit operator bool() const ENTT_NOEXCEPT {
        return !(vtable == nullptr);
    }

    /**
     * @brief Returns a pointer to the underlying concept.
     * @return A pointer to the underlying concept.
     */
    [[nodiscard]] concept_type * operator->() ENTT_NOEXCEPT {
        return this;
    }

    /*! @copydoc operator-> */
    [[nodiscard]] const concept_type * operator->() const ENTT_NOEXCEPT {
        return this;
    }

    /**
     * @brief Swaps two poly objects.
     * @param lhs A valid poly object.
     * @param rhs A valid poly object.
     */
    friend void swap(poly &lhs, poly &rhs) {
        using std::swap;
        swap(lhs.storage, rhs.storage);
        swap(lhs.vtable, rhs.vtable);
    }

    /**
     * @brief Aliasing constructor.
     * @param other A reference to an object that isn't necessarily initialized.
     * @return A poly that shares a reference to an unmanaged object.
     */
    [[nodiscard]] friend poly as_ref(poly &other) ENTT_NOEXCEPT {
        poly ref;
        ref.storage = as_ref(other.storage);
        ref.vtable = other.vtable;
        return ref;
    }

    /*! @copydoc as_ref */
    [[nodiscard]] friend poly as_ref(const poly &other) ENTT_NOEXCEPT {
        poly ref;
        ref.storage = as_ref(other.storage);
        ref.vtable = other.vtable;
        return ref;
    }

private:
    any storage;
    const vtable_type *vtable;
};


}


#endif
