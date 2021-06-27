#ifndef HELENA_SYSTEMS_PROPERTY_IPP
#define HELENA_SYSTEMS_PROPERTY_IPP

#include <Helena/Systems/PropertyManager.hpp>
#include <Helena/Core.hpp>

namespace Helena::Systems
{
    template <typename Property>
    [[nodiscard]] auto PropertyManager::PropertyIndex<Property>::GetIndex(map_index_t& container) -> std::size_t {
        static const std::size_t value = Internal::AddOrGetTypeIndex(container, Hash::Type<Property>);
        return value;
    }

    template <typename Property, typename... Args>
    auto PropertyManager::Create([[maybe_unused]] Args&&... args) -> void {
        static_assert(std::is_same_v<Property, Internal::remove_cvrefptr_t<Property>>,
                "Property type cannot be const/ptr/ref");
        static_assert(Internal::is_specialization_of_v<Property, PropertyInfo>,
                "Property type is not Property::Property type");

        using Event = Helena::Events::Systems::Property::Create<Property>;
        const auto index = PropertyIndex<Property>::GetIndex(m_PropertyIndexes);
        if(index >= m_Properties.size()) {
            m_Properties.resize(index + 1);
        }

        HF_ASSERT(!m_Properties[index], "Property: {} already exist", Internal::NameOf<Property>);
        m_Properties[index].template emplace<typename Property::Value>(std::forward<Args>(args)...);
        Core::TriggerEvent<Event>();
    }

    template <typename... Property>
    [[nodiscard]] auto PropertyManager::Has() noexcept -> bool {
        static_assert(sizeof...(Property) > 0, "Property pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Property>, Property> && ...),
                "Property type cannot be const/ptr/ref");
        static_assert((Internal::is_specialization_of_v<Property, PropertyInfo> && ...),
                "Property type is not Property::Property type");

        if constexpr(sizeof...(Property) == 1) {
            const auto index = PropertyIndex<Property...>::GetIndex(m_PropertyIndexes);
            return index < m_Properties.size() && m_Properties[index];
        } else {
            return (Has<Property>() && ...);
        }
    }

    template <typename... Property>
    [[nodiscard]] auto PropertyManager::Any() noexcept -> bool {
        static_assert(sizeof...(Property) > 1, "Exclusion-only Property are not supported");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Property>, Property> && ...),
                "Property type cannot be const/ptr/ref");
        static_assert((Internal::is_specialization_of_v<Property, PropertyInfo> && ...),
                "Property type is not Property::Property type");

        return (Has<Property>() || ...);
    }

    template <typename... Property>
    [[nodiscard]] auto PropertyManager::Get() noexcept -> decltype(auto) {
        static_assert(sizeof...(Property) > 0, "Property pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Property>, Property> && ...),
                "Property type cannot be const/ptr/ref");
        static_assert((Internal::is_specialization_of_v<Property, PropertyInfo> && ...),
                "Property type is not Property::Property type");

        if constexpr(sizeof...(Property) == 1) {
            const auto index = PropertyIndex<Property...>::GetIndex(m_PropertyIndexes);
            HF_ASSERT(index < m_Properties.size() && m_Properties[index], "Property {} not exist",
                      Internal::NameOf<Property...>);
            HF_ASSERT(entt::any_cast<typename Property::Value...>(&m_Properties[index]),
                      "Property: {} value type is incorrect", Internal::NameOf<Property...>);
            return entt::any_cast<typename Property::Value&...>(m_Properties[index]);
        } else {
            return std::forward_as_tuple(Get<Property>()...);
        }
    }

    template <typename... Property>
    auto PropertyManager::Remove() noexcept -> void {
        static_assert(sizeof...(Property) > 0, "Property pack is empty!");
        static_assert((std::is_same_v<Internal::remove_cvrefptr_t<Property>, Property> && ...),
                "Property type cannot be const/ptr/ref");
        static_assert((Internal::is_specialization_of_v<Property, PropertyInfo> && ...),
                "Property type is not Property::Property type");

        if constexpr(sizeof...(Property) == 1) {
            using Event = Helena::Events::Systems::Property::Remove<Property...>;
            const auto index = PropertyIndex<Property...>::GetIndex(m_PropertyIndexes);
            HF_ASSERT(index < m_Properties.size() && m_Properties[index],
                      "Property {} not exist", Internal::NameOf<Property...>);
            HF_ASSERT(entt::any_cast<typename Property::Value...>(&m_Properties[index]),
                      "Property: {} value type is incorrect", Internal::NameOf<Property...>);
            Core::TriggerEvent<Event>();
            m_Properties[index].reset();
        } else {
            (Remove<Property>(), ...);
        }
    }
}

#endif // HELENA_SYSTEMS_PROPERTY_IPP
