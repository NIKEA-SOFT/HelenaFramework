#ifndef COMMON_ECSYSTEM_HPP
#define COMMON_ECSYSTEM_HPP

namespace Helena
{
	class ECSystem final 
	{
	public:
		using Entity = entt::entity;

		template <typename... Type>
		using ExcludeType = entt::exclude_t<Type...>;

		template <typename... Type>
		using GetType = entt::get_t<Type...>;

		template<typename... Type>
		static inline constexpr ExcludeType<Type...> Exclude{};

		template<typename... Type>
		static inline constexpr GetType<Type...> Get{};

		ECSystem() : m_Registry{Core::GetContext()->m_Registry} {}
		~ECSystem() = default;
		ECSystem(const ECSystem&) = default;
		ECSystem(ECSystem&&) noexcept = default;
		ECSystem& operator=(const ECSystem&) = default;
		ECSystem& operator=(ECSystem&&) noexcept = default;

	public:
		auto CreateEntity() -> Entity;

		auto CreateEntity(const Entity id) -> Entity;

		template<typename It>
		auto CreateEntity(It first, It last) -> void;

		auto HasEntity(const Entity id) const -> bool;

		auto SizeEntity() const -> std::size_t;

		auto RemoveEntity(const Entity id) -> void;

		template <typename Func>
		auto Each(Func&& callback) const -> void;

		template <typename Component, typename... Args>
		auto AddComponent(const Entity id, Args &&... args) -> Component&;

		template <typename Component, typename... Args>
		auto AddOrGetComponent(const Entity id, Args &&... args) -> Component&;

		template <typename Component, typename... Args>
		auto AddOrReplaceComponent(const Entity id, Args &&... args) -> Component&;

		template <typename... Components>
		[[nodiscard]] auto GetComponent(const Entity id) -> decltype(auto);

		template <typename... Components>
		[[nodiscard]] auto GetComponent(const Entity id) const -> decltype(auto);

		template <typename... Components>
		[[nodiscard]] auto GetComponentPtr(const Entity id);

		template <typename... Components>
		[[nodiscard]] auto GetComponentPtr(const Entity id) const;

		template <typename... Components>
		[[nodiscard]] auto HasComponent(const Entity id) -> bool;

		template <typename... Components, typename... ExcludeFilter>
		[[nodiscard]] auto ViewComponent(ExcludeType<ExcludeFilter...> = {}) -> decltype(auto);

		template <typename... Components, typename... ExcludeFilter>
		[[nodiscard]] auto ViewComponent(ExcludeType<ExcludeFilter...> = {}) const -> decltype(auto);

		template <typename... Owned, typename... ExcludeFilter>
		[[nodiscard]] auto GroupComponent(ExcludeType<ExcludeFilter...> = {}) -> decltype(auto);

		template <typename... Owned, typename... ExcludeFilter>
		[[nodiscard]] auto GroupComponent(ExcludeType<ExcludeFilter...> = {}) const -> decltype(auto);

		template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
		[[nodiscard]] auto GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...> = {}) -> decltype(auto);

		template <typename... Owned, typename... GetFilter, typename... ExcludeFilter>
		[[nodiscard]] auto GroupComponent(GetType<GetFilter...>, ExcludeType<ExcludeFilter...> = {}) const -> decltype(auto);

		template <typename... Components>
		auto RemoveComponent(const Entity id) -> void;

		template <typename... Components, typename It>
		auto RemoveComponent(It first, It last) -> void;

		auto RemoveComponents(const Entity id) -> void;

		template <typename... Components>
		auto ClearComponent() -> void;

		template <typename Component>
		[[nodiscard]] auto SizeComponent() const -> std::size_t;

		template <typename... Components>
		[[nodiscard]] auto AnyComponent(const Entity id) const -> bool;

	private:
		entt::registry& m_Registry;
	};
}

#include <Common/Systems/ECSystem.ipp>

#endif // COMMON_ECSYSTEM_HPP