#ifndef COMMON_CORE_HPP
#define COMMON_CORE_HPP

namespace Helena
{
	namespace Events
	{
		enum class Initialize {};
		enum class Finalize {};
	}
	
	class Core
	{
		template <typename, typename>
		friend struct ENTT_API entt::type_seq;

		enum SystemEvent : std::uint8_t {
			Create,
			Execute,
			Tick,
			Update,
			Destroy,
			Size
		};

		template <typename Type>
		using delegate_t = entt::delegate<Type>;

		//template<typename Type, typename... Args>
		//using mem_ptr = void(Type::* const)(Args...);

		template <typename Type, typename... Args>
		using fn_create_t	= decltype(std::declval<Type>().OnSystemCreate(std::declval<Args>()...));
		template <typename Type, typename... Args>
		using fn_execute_t	= decltype(std::declval<Type>().OnSystemExecute(std::declval<Args>()...));
		template <typename Type, typename... Args>
		using fn_tick_t		= decltype(std::declval<Type>().OnSystemTick(std::declval<Args>()...));
		template <typename Type, typename... Args>
		using fn_update_t	= decltype(std::declval<Type>().OnSystemUpdate(std::declval<Args>()...));
		template <typename Type, typename... Args>
		using fn_destroy_t	= decltype(std::declval<Type>().OnSystemDestroy(std::declval<Args>()...));

		struct System {
			using ev_array_t = std::array<delegate_t<void ()>, SystemEvent::Size>;

			entt::any	m_Instance;
			ev_array_t	m_Events;
		};

		using map_indexes_t		= robin_hood::unordered_flat_map<entt::id_type, std::size_t>;
		using vec_args_t		= std::vector<std::string_view>;
		using vec_systems_t		= std::vector<System>;
		using array_events_t	= std::array<std::queue<std::size_t>, SystemEvent::Size>;

		template <typename Type>
		struct SystemIndex {
			[[nodiscard]] static auto GetIndex(map_indexes_t& container) -> std::size_t;
		};

	public:

		/*! Context used for share memory in modules (dll/lib) */
		struct Context {
		private:
			friend class Core;

			template <typename, typename>
			friend struct ENTT_API entt::type_seq;

			std::chrono::steady_clock::time_point m_TimeStart {};
			std::chrono::steady_clock::time_point m_TimeNow {};
			std::chrono::steady_clock::time_point m_TimePrev {};

			std::atomic_bool	m_Shutdown;
			entt::dispatcher	m_Dispatcher {};
			vec_args_t	m_Args;

			double m_TimeDelta {};
			double m_TickRate {};

			vec_systems_t	m_Systems;
			array_events_t	m_EventScheduler;
			map_indexes_t	m_TypeIndexes;

			map_indexes_t	m_SequenceIndexes;
		};

	private:
		static inline std::shared_ptr<Context> m_Context {};

	private:
	#if HF_PLATFORM == HF_PLATFORM_WIN
		static void Terminate();
		static BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
		static int SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException);
	#elif HF_PLATFORM == HF_PLATFORM_LINUX
		static auto SigHandler(int signal) -> void;
	#endif

		[[nodiscard]] static auto CreateOrSetContext(const std::shared_ptr<Context>& ctx) -> bool;
		static auto HeartbeatTimeCalc() -> double;
		static auto HookSignals() -> void;
		static auto Heartbeat() -> void;
		static auto EventSystems(const SystemEvent type) -> void;

		//[[nodiscard]] static auto GetSystemManager() noexcept -> SystemManager&;
		//[[nodiscard]] static auto GetTypeIndex(std::unordered_map<entt::id_type, std::size_t>& container, const entt::id_type typeIndex) -> std::size_t;

	public:
		Core() = delete;
		~Core() = delete;
		Core(const Core&) = delete;
		Core(Core&&) = delete;
		Core& operator=(const Core&) = delete;
		Core& operator=(Core&&) = delete;

		[[nodiscard]] static auto Initialize(const std::function<bool ()>& callback, const std::shared_ptr<Context>& ctx = {}) -> bool;
		static auto Shutdown() noexcept -> void;

		static auto SetArgs(const std::size_t size, const char* const* argv) -> void;
		static auto SetTickrate(const double tickrate) -> void;

		[[nodiscard]] static auto GetArgs() noexcept -> decltype(auto);
		[[nodiscard]] static auto GetTickrate() noexcept -> double;
		[[nodiscard]] static auto GetTimeElapsed() noexcept -> double;
		[[nodiscard]] static auto GetTimeDelta() noexcept -> double;
		[[nodiscard]] static auto GetContext() noexcept -> const std::shared_ptr<Context>&;

		template <typename Type, typename... Args>
		static auto RegisterSystem([[maybe_unused]] Args&&... args) -> void;

		template <typename Type>
		[[nodiscard]] static auto GetSystem() -> Type&;

		template <typename Type>
		static auto RemoveSystem() -> void;

		template <typename Event, auto Method>
		static auto RegisterEvent() -> void;

		template <typename Event, auto Method, typename Type>
		static auto RegisterEvent(Type&& instance) -> void;

		template <typename Event, typename... Args>
		static auto TriggerEvent([[maybe_unused]] Args&&... args) -> void;

		template <typename Event, typename... Args>
		static auto EnqueueEvent([[maybe_unused]] Args&&... args) -> void;

		template <typename Event>
		static auto UpdateEvent() -> void;

		static auto UpdateEvent() -> void;

		template <typename Event, auto Method>
		static auto RemoveEvent() -> void;

		template <typename Event, auto Method, typename Type>
		static auto RemoveEvent(Type&& instance) -> void;
	};
}

#include <Common/Core.ipp>

#endif // COMMON_CORE_HPP