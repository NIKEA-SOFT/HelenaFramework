#ifndef HELENA_TYPES_MODERNDESIGN_HPP
#define HELENA_TYPES_MODERNDESIGN_HPP

#include <Helena/Engine/Engine.hpp>

namespace Helena::Types
{
    template <typename T>
    requires std::is_class_v<T>
    class ModernDesign
    {
    public:
        ModernDesign() noexcept = default;
        ~ModernDesign() noexcept = default;
        ModernDesign(const ModernDesign&) noexcept = delete;
        ModernDesign& operator=(const ModernDesign&) noexcept = delete;
        ModernDesign(ModernDesign&&) noexcept = delete;
        ModernDesign& operator=(ModernDesign&&) noexcept = delete;

    protected:
        [[nodiscard]] static T& CurrentSystem() {
            HELENA_ASSERT(Engine::HasSystem<T>(), "System: {} not registered!", Traits::NameOf<T>{});
            return Engine::GetSystem<T>();
        }
    };
}

#endif // HELENA_TYPES_MODERNDESIGN_HPP