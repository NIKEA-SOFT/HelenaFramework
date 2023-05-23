#ifndef HELENA_TYPES_SUBSYSTEMS_HPP
#define HELENA_TYPES_SUBSYSTEMS_HPP

#include <Helena/Types/VectorAny.hpp>

namespace Helena::Types
{
    template <typename System>
    requires std::is_class_v<System>
    class Subsystems
    {
        struct UniqueKey {};

    public:
        Subsystems() noexcept : m_Subsystems{} {};
        ~Subsystems() noexcept = default;
        Subsystems(const Subsystems&) noexcept = delete;
        Subsystems& operator=(const Subsystems&) noexcept = delete;
        Subsystems(Subsystems&&) noexcept = delete;
        Subsystems& operator=(Subsystems&&) noexcept = delete;

        template <typename Subsystem, typename... Args>
        void RegisterSubsystem(Args&&... args) {
            if(!HasSubsystem<Subsystem>()) {
                m_Subsystems.template Create<Subsystem>(std::forward<Args>(args)...);
            }
        }

        template <typename... Subsystems>
        [[nodiscard]] bool HasSubsystem() const {
            return m_Subsystems.template Has<Subsystems...>();
        }

        template <typename... Subsystems>
        [[nodiscard]] bool AnySubsystem() const {
            return m_Subsystems.template Any<Subsystems...>();
        }

        template <typename... Subsystems>
        [[nodiscard]] decltype(auto) GetSubsystem() {
            return m_Subsystems.template Get<Subsystems...>();
        }

        template <typename... Subsystems>
        [[nodiscard]] decltype(auto) GetSubsystem() const {
            return m_Subsystems.template Get<Subsystems...>();
        }

        template <typename... Subsystems>
        void RemoveSubsystem() {
            if constexpr(Helena::Traits::Arguments<Subsystems...>::Single) {
                if(m_Subsystems.template Has<Subsystems...>()) {
                    m_Subsystems.template Remove<Subsystems...>();
                }
            } else {
                (RemoveSubsystem<Subsystems>(), ...);
            }
        }

    private:
        VectorAny<UniqueKey> m_Subsystems;
    };
}

#endif // HELENA_TYPES_SUBSYSTEMS_HPP