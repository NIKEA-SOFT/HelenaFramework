#ifndef HELENA_TYPES_VECTORANY_HPP
#define HELENA_TYPES_VECTORANY_HPP

#include <Helena/Types/VectorKVAny.hpp>

namespace Helena::Types
{
    template <typename UniqueKey>
    class VectorAny : public VectorKVAny<UniqueKey>
    {
        using Base = VectorKVAny<UniqueKey>;

    public:
        using Base::Has;
        using Base::Any;
        using Base::Remove;
        using Base::Clear;

    public:
        VectorAny() = default;
        ~VectorAny() = default;
        VectorAny(const VectorAny&) = delete;
        VectorAny(VectorAny&&) noexcept = default;
        VectorAny& operator=(const VectorAny&) = delete;
        VectorAny& operator=(VectorAny&&) noexcept = default;

        template <typename T, typename... Args>
        requires (Base::template RequiredParams<T, T, Args...>)
        void Create(Args&&... args) {
            Base::template Create<T, T>(std::forward<Args>(args)...);
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (Base::template AllowedParam<T> && ...))
        [[nodiscard]] decltype(auto) Get()
        {
            if constexpr(Traits::Arguments<T...>::Single) {
                return Base::template Get<T..., T...>();
            } else {
                return std::forward_as_tuple(Get<T>()...);
            }
        }

        template <typename... T>
        requires (!Traits::Arguments<T...>::Orphan && (Base::template AllowedParam<T> && ...))
        [[nodiscard]] decltype(auto) Get() const
        {
            if constexpr(Traits::Arguments<T...>::Single) {
                return Base::template Get<T..., T...>();
            } else {
                return std::forward_as_tuple(Get<T>()...);
            }
        }
    };
}

#endif // HELENA_TYPES_VECTORANY_HPP