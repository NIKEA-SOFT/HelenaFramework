#ifndef HELENA_TYPES_MODERNDESIGN_HPP
#define HELENA_TYPES_MODERNDESIGN_HPP

#include <type_traits>

namespace Helena::Types
{
    template <typename T>
    requires std::is_class_v<T>
    class ModernDesign
    {
    public:
        ModernDesign() noexcept {
            System = static_cast<T*>(this);
        }
        ~ModernDesign() noexcept {
            System = nullptr;
        }
        ModernDesign(const ModernDesign&) noexcept = delete;
        ModernDesign& operator=(const ModernDesign&) noexcept = delete;
        ModernDesign(ModernDesign&&) noexcept = delete;
        ModernDesign& operator=(ModernDesign&&) noexcept = delete;

    protected:
        static inline T* System = nullptr;
    };
}

#endif // HELENA_TYPES_MODERNDESIGN_HPP