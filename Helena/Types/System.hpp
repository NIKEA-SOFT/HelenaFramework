#ifndef HELENA_TYPES_SYSTEM_HPP
#define HELENA_TYPES_SYSTEM_HPP

#include <Helena/Engine/Engine.hpp>

namespace Helena::Types
{
    template <typename T>
    requires std::is_class_v<T>
    struct System
    {
        System() noexcept = default;
        ~System() noexcept = default;
        System(const System&) noexcept = delete;
        System& operator=(const System&) noexcept = delete;
        System(System&&) noexcept = delete;
        System& operator=(System&&) noexcept = delete;

        [[nodiscard]] static T& CurrentSystem() {
            HELENA_ASSERT(Engine::HasSystem<T>(), "System: {} not registered!", Traits::NameOf<T>{});
            return Engine::GetSystem<T>();
        }

        [[nodiscard]] static bool HasSystem() {
            return Engine::HasSystem<T>();
        }
    };
}

#endif // HELENA_TYPES_SYSTEM_HPP