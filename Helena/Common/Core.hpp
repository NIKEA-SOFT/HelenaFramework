#ifndef COMMON_CORE_HPP
#define COMMON_CORE_HPP

namespace Helena
{
	namespace Events
	{
		namespace Core {
			struct HeartbeatBegin {};
			struct HeartbeatEnd {};

			struct TickPre {};
			struct Tick {};
			struct TickPost {};
		}
	}
	
	class Core
	{
		template <typename, typename>
		friend struct ENTT_API entt::type_seq;

		template <typename Type>
		using delegate_t = entt::delegate<Type>;

		template<typename Type, typename... Args>
		using mem_ptr = void(Type::* const)(Args...);

		template <typename Type>
		struct SystemIndex {
			[[nodiscard]] static std::size_t GetIndex() {
				static const auto value = Core::GetTypeIndex(m_Context->m_SystemManager.m_Indexes, entt::type_hash<Type>().value());
				return value;
			}
		};

		struct System {
			template <typename Type, typename... Args>
			using fn_create_t	= decltype(std::declval<Type>().Create(std::declval<Args>()...));
			template <typename Type, typename... Args>
			using fn_update_t	= decltype(std::declval<Type>().Update(std::declval<Args>()...));
			template <typename Type, typename... Args>
			using fn_destroy_t	= decltype(std::declval<Type>().Destroy(std::declval<Args>()...));

			using ev_create_t	= delegate_t<void ()>;
			using ev_update_t	= delegate_t<void ()>;
			using ev_destroy_t	= delegate_t<void ()>;

			System() = default;
			~System() = default;
			System(const System&) noexcept = default;
			System(System&&) noexcept = default;
			System& operator = (System&&) = delete;
			System& operator = (const System&) = delete;

			entt::any m_Instance {};
			ev_create_t m_EventCreate {};
			ev_update_t m_EventUpdate {};
			ev_destroy_t m_EventDestroy {};
		};

		struct SystemManager {
			std::vector<System> m_Systems;
			std::queue<std::size_t> m_SystemsBegin;
			std::queue<std::size_t> m_SystemsUpdatable;
			std::unordered_map<entt::id_type, std::size_t> m_Indexes;
		};

		struct Context {
			friend class Core;

			template <typename, typename>
			friend struct ENTT_API entt::type_seq;

			entt::registry m_Registry {};
			entt::dispatcher m_Dispatcher {};

		private:
			std::chrono::steady_clock::time_point m_TimeStart {};
			std::chrono::steady_clock::time_point m_TimeNow {};
			std::chrono::steady_clock::time_point m_TimePrev {};

			double m_TimeDelta {};
			double m_TickRate {};

			std::atomic_bool m_Shutdown;
			std::vector<std::string_view> m_Args;
			std::unordered_map<entt::id_type, std::size_t> m_TypeIndexes;
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

		static auto Heartbeat() -> void;

		[[nodiscard]] static auto GetTypeIndex(std::unordered_map<entt::id_type, std::size_t>& container, const entt::id_type typeIndex) -> std::size_t;

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
		[[nodiscard]] static auto GetTickrate() noexcept;
		[[nodiscard]] static auto GetTimeElapsed() noexcept;
		[[nodiscard]] static auto GetTimeDelta() noexcept;
		[[nodiscard]] static auto GetContext() noexcept -> const std::shared_ptr<Context>&;

		template <typename Type, typename... Args>
		static auto RegisterSystem([[maybe_unused]] Args&&... args) -> Type*;

		template <typename Type>
		[[nodiscard]] static auto GetSystem() -> Type*;

		template <typename Type>
		static auto RemoveSystem() -> void;

		template <typename Event, auto Method>
		static auto RegisterEvent() -> void;

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