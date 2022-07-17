#ifndef HELENA_TYPES_MONOSTATE_HPP
#define HELENA_TYPES_MONOSTATE_HPP

#include <Helena/Types/FixedBuffer.hpp>

namespace Helena::Types
{
    template <Types::FixedBuffer Key>
    struct Monostate
    {
        template <typename Type>
        void operator=(Type value) const noexcept {
            m_Data<Type> = value;
        }

        template <typename Type>
        operator Type() const noexcept {
            return m_Data<Type>;
        }

    private:
        template <typename Type>
        inline static Type m_Data = {};
    };
}

#endif // HELENA_TYPES_MONOSTATE_HPP