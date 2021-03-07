#ifndef COMMON_CORE_HPP
#define COMMON_CORE_HPP

namespace Helena
{
	namespace Events
	{
		namespace Core {
			struct UpdatePre {};
			struct Update {};
			struct UpdatePost {};
		}
	}
	
	class Core
	{
		template <typename, typename>
		friend struct ENTT_API entt::type_seq;

		using registry_t		= entt::registry;
		using dispatcher_t		= entt::dispatcher;
		using any_t				= entt::any;
		using signal_t			= std::atomic_bool;
		using args_list_t		= std::vector<std::string_view>;
		using indexes_hash_t	= std::unordered_map<entt::id_type, std::size_t>;
		
		template <typename Type>
		using delegate_t		= entt::delegate<Type>;

		template <typename Type>
		struct SystemIndex {
			[[nodiscard]] static std::size_t GetIndex() {
				static const auto value = Core::GetTypeIndex(m_Context->m_SystemManager.m_Indexes, entt::type_hash<Type>().value());
				return value;
			}
		};

		struct System {
			template <typename Type, typename... Args>
			using fn_create_t	= decltype(std::declval<Type>().OnCreate(std::declval<Args>()...));
			//template <typename Type, typename... Args>
			//using fn_update_t	= decltype(std::declval<Type>().OnUpdate(std::declval<Args>()...));
			template <typename Type, typename... Args>
			using fn_destroy_t	= decltype(std::declval<Type>().OnDestroy(std::declval<Args>()...));

			using ev_create_t	= delegate_t<void ()>;
			/*using ev_update_t	= delegate_t<void (float)>;*/
			using ev_destroy_t	= delegate_t<void ()>;

			System() = default;
			~System() = default;
			System(const System&) noexcept = default;
			System(System&&) noexcept = default;
			System& operator = (System&&) = delete;
			System& operator = (const System&) = delete;

			bool m_bInitialized {};
			any_t m_Instance {};
			ev_create_t m_EventCreate {};
			/*ev_update_t m_EventUpdate {};*/
			ev_destroy_t m_EventDestroy {};
		};

		struct SystemManager {
			using systems_list_t = std::vector<System>;

			systems_list_t m_Systems;
			indexes_hash_t m_Indexes;
		};

		struct Context {
			friend class Core;

			template <typename, typename>
			friend struct ENTT_API entt::type_seq;

			registry_t m_Registry {};
			dispatcher_t m_Dispatcher {};
			float m_FrameTime {};
			float m_Tickrate {};
			float m_TickrateMax {};

		private:
			signal_t m_Shutdown;
			args_list_t m_Args;
			indexes_hash_t m_TypeIndexes;
			SystemManager m_SystemManager;
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
		static auto Heartbeat(float tickrate) -> void;
		static auto Shutdown() noexcept -> void;

		static auto SetArgs(const std::size_t size, const char* const* argv) -> void;

		[[nodiscard]] static auto GetArgs() noexcept -> decltype(auto);
		[[nodiscard]] static auto GetContext() noexcept -> const std::shared_ptr<Context>&;

		template <typename Type, typename... Args>
		static auto RegisterSystem([[maybe_unused]] Args&&... args) -> Type*;

		template <typename Type>
		[[nodiscard]] static auto GetSystem() -> Type*;

		template <typename Type>
		static auto RemoveSystem() -> void;

		//template <typename Event, auto Method>
		//static auto RegisterEvent() -> void;

		template <typename Event, auto Method, typename Type>
		static auto RegisterEvent(Type&& instance) -> void;

		template <typename Event, typename... Args>
		static auto TriggerEvent(Args&&... args) -> void;

		template <typename Event, typename... Args>
		static auto EnqueueEvent(Args&&... args) -> void;

		template <typename Event>
		static auto UpdateEvent() -> void;

		template <typename Event, auto Method>
		static auto RemoveEvent() -> void;

		template <typename Event, auto Method, typename Type>
		static auto RemoveEvent(Type&& instance) -> void;
	};
}

#include <Common/Core.ipp>

#endif // COMMON_CORE_HPP