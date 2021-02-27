#ifndef COMMON_CORE_HPP
#define COMMON_CORE_HPP

namespace Helena
{
	namespace Events {
		struct EV_SYSTEM_CREATE {};
		struct EV_SYSTEM_UPDATE {};
		struct EV_SYSTEM_DESTROY {};
	}

	class Core
	{
		template <typename, typename>
		friend struct ENTT_API entt::type_seq;

		using registry_t		= entt::registry;
		using dispatcher_t		= entt::dispatcher;
		using any_t				= std::any;
		/*using unique_any_t		= std::unique_ptr<void, void (*)(void*)>;*/
		using signal_t			= std::atomic_bool;
		using args_list_t		= std::vector<std::string_view>;
		using indexes_hash_t	= std::unordered_map<entt::id_type, std::size_t>;
		
		template <typename Type>
		using delegate_t		= entt::delegate<Type>;

		//template <typename Type, typename... Args, std::enable_if_t<!std::is_array_v<Type>, int> = 0>
		//[[nodiscard]] static unique_any_t make_unique_any(Args&&... args) {
		//	return unique_any_t(new Type(_STD forward<Args>(args)...), [](void* ptr) {
		//		Type* instance = static_cast<Type*>(ptr); 
		//		delete instance; instance = nullptr;
		//	});
		//}

		template <typename Type>
		struct SystemIndex {
			[[nodiscard]] static std::size_t GetIndex() HF_NOEXCEPT {
				static const auto value = Core::GetTypeIndex(m_Context->m_SystemManager.m_Indexes, entt::type_hash<Type>().value());
				return value;
			}
		};

		struct System {
			template <typename Type, typename... Args>
			using fn_create_t	= decltype(std::declval<Type>().OnCreate(std::declval<Args>()...));
			template <typename Type, typename... Args>
			using fn_update_t	= decltype(std::declval<Type>().OnUpdate(std::declval<Args>()...));
			template <typename Type, typename... Args>
			using fn_destroy_t	= decltype(std::declval<Type>().OnDestroy(std::declval<Args>()...));

			using ev_create_t	= delegate_t<void ()>;
			using ev_update_t	= delegate_t<void (float)>;
			using ev_destroy_t	= delegate_t<void ()>;

			System() = default;
			~System() = default;
			System(const System&) noexcept = default;
			System(System&&) noexcept = default;
			System& operator = (System&&) = delete;
			System& operator = (const System&) = delete;


			any_t			m_Instance;
			ev_create_t		m_EventCreate {};
			ev_update_t		m_EventUpdate {};
			ev_destroy_t	m_EventDestroy {};
		};

		struct SystemManager {
			using systems_list_t	= std::vector<System>;

			systems_list_t		m_Systems;
			indexes_hash_t		m_Indexes;
		};

		struct Context {
			friend class Core;

			registry_t		m_Registry {};
			dispatcher_t	m_Dispatcher {};

		private:
			signal_t		m_Signal;
			args_list_t		m_Args;
			indexes_hash_t	m_TypeIndexes;
			SystemManager	m_SystemManager;
		};

		static inline std::shared_ptr<Context> m_Context {};

	private:
	#if HF_PLATFORM == HF_PLATFORM_WIN
		static BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
		static int SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException);
	#elif HF_PLATFORM == HF_PLATFORM_LINUX
		static auto SigHandler(int signal) -> void;
	#endif

		[[nodiscard]] static auto GetTypeIndex(indexes_hash_t& container, const entt::id_type typeIndex) -> std::size_t;

	public:
		Core() = delete;
		~Core() = delete;
		Core(const Core&) = delete;
		Core(Core&&) = delete;
		Core& operator=(const Core&) = delete;
		Core& operator=(Core&&) = delete;

		[[nodiscard]] static auto Initialize(const std::shared_ptr<Context>& ctx = {}) -> bool;

		static auto SetArgs(const std::size_t size, const char* const* argv) -> void;

		[[nodiscard]] static auto GetArgs() noexcept -> decltype(auto);
		[[nodiscard]] static auto GetSignal() noexcept -> bool;
		[[nodiscard]] static auto GetContext() noexcept -> const std::shared_ptr<Context>&;

		template <typename Type, typename... Args>
		static auto RegisterSystem([[maybe_unused]] Args&&... args) -> Type*;

		template <typename Type>
		[[nodiscard]] static auto GetSystem() noexcept -> Type*;

		template <typename Type>
		static auto RemoveSystem() noexcept -> void;

		template <typename Event, auto Candidate, typename Type>
		static auto RegisterEvent(Type&& instance) -> bool;
	};
}

#include <Common/Core.ipp>

#endif // COMMON_CORE_HPP