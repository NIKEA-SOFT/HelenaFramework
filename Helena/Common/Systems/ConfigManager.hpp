#ifndef COMMON_SYSTEMS_CONFIGMANAGER_HPP
#define COMMON_SYSTEMS_CONFIGMANAGER_HPP

namespace Helena::Systems
{
    class ConfigManager final
    {
//        struct Hasher {
//            std::size_t operator()(const entt::id_type key) const {
//                return static_cast<std::size_t>(key);
//            }
//        };

//        struct Equal {
//            bool operator()(const entt::id_type lhs, const entt::id_type rhs) const {
//                return lhs == rhs;
//            }
//        };

//        using map_property_t = robin_hood::unordered_flat_map<entt::id_type, entt::any, Hasher, Equal>;
        using map_index_t = robin_hood::unordered_flat_map<entt::id_type, std::size_t>;

        template <typename Resource>
        struct ResourceIndex {
            [[nodiscard]] static auto GetIndex(map_index_t& container) -> std::size_t;
        };

        template <typename Resource, typename Property>
        struct PropertyIndex {
            [[nodiscard]] static auto GetIndex(map_index_t& container) -> std::size_t;
        };

    public:
        using ID = entt::id_type;

        template <ID Value>
        using Resource = entt::tag<Value>;

        template <ID Value>
        using Key = entt::tag<Value>;

    public:
        ConfigManager() = default;
        ~ConfigManager() = default;
        ConfigManager(const ConfigManager&) = delete;
        ConfigManager(ConfigManager&&) noexcept = delete;
        ConfigManager& operator=(const ConfigManager&) = delete;
        ConfigManager& operator=(ConfigManager&&) noexcept = delete;

        template <typename Resource, typename... Args>
        auto CreateResource([[maybe_unused]] Args&&... args) -> void;

//		template <typename Resource>
//        [[nodiscard]] auto HasResource() noexcept -> bool;

        template <typename... Resources>
        [[nodiscard]] auto HasResource() noexcept -> bool;

        template <typename... Resources>
        [[nodiscard]] auto AnyResource() noexcept -> bool;

//		template <typename Resource>
//        [[nodiscard]] auto GetResource() noexcept -> Resource&;

        template <typename... Resources>
        [[nodiscard]] auto GetResource() noexcept -> decltype(auto);

        template <typename... Resources>
        auto RemoveResource() noexcept -> void;

        // Compiletime properties
        template <typename Resource, typename Key, typename Type, typename... Args>
        auto AddProperty([[maybe_unused]] Args&&... args) -> void;

        template <typename Resource, typename Key>
        [[nodiscard]] auto HasProperty() noexcept -> bool;

        template <typename Resource, typename Key, typename Type>
        [[nodiscard]] auto GetProperty() -> Type&;

        template <typename Resource, typename Key>
        auto RemoveProperty() noexcept -> void;

        // Runtime properties
//        template <typename Type, typename... Args>
//        auto AddProperty(const std::string_view key, [[maybe_unused]] Args&&... args) -> void;

//        [[nodiscard]] auto HasProperty(const std::string_view key) const noexcept -> bool;

//        template <typename Type>
//        [[nodiscard]] auto GetProperty(const std::string_view key) const noexcept -> Type*;

//        auto RemoveProperty(const std::string_view key) noexcept -> void;

    private:
        std::vector<entt::any> m_Resources;
        std::vector<entt::any> m_Properties;
        //map_property_t m_PropertyMap;
        map_index_t m_ResourceIndexes;
        map_index_t m_PropertyIndexes;
    };
}

#include <Common/Systems/ConfigManager.ipp>

#endif // COMMON_SYSTEMS_CONFIGMANAGER_HPP
