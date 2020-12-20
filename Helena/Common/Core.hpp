#ifndef COMMON_CORE_HPP
#define COMMON_CORE_HPP

namespace Helena
{
	//template <typename T>
	//using SafePtr = sf::safe_ptr<T, std::shared_mutex, std::unique_lock<std::shared_mutex>, std::shared_lock<std::shared_mutex>>;

	class Core final
	{
	private:
	#if HF_PLATFORM == HF_PLATFORM_WIN
		static auto WINAPI CtrlHandler(DWORD) -> BOOL;
		static auto SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException) -> int;
	#elif HF_PLATFORM == HF_PLATFORM_LINUX
		static auto SigHandler(int signal) -> void;
	#endif

	public:
		Core() = delete;
		~Core() = delete;
		Core(const Core&) = delete;
		Core(Core&&) = delete;
		Core& operator=(const Core&) = delete;
		Core& operator=(Core&&) = delete;

		struct Ctx 
		{
			entt::registry m_Registry;
			std::unordered_map<entt::id_type, entt::id_type> m_TypeIndexes;
			entt::entity m_Entity;
			std::size_t m_ArgCount;
			char** m_Args;
			std::atomic_bool m_Signal;
		};

		static auto SetContext(const std::shared_ptr<Ctx>& ctx) noexcept -> void;
		static auto GetContext() -> std::shared_ptr<Ctx>&;
		static auto SetArgs(std::size_t argc, char** argv) noexcept -> void;
		static auto GetArgs(std::size_t offset) noexcept -> const char*;
		static auto GetArgsCount() noexcept -> std::size_t;
		static auto GetSignal() noexcept -> bool;
		template <typename Type, typename... Args>
		static auto CreateContext(Args&&... args) -> Type&;
		template <typename Type>
		static auto GetContext() noexcept -> Type&;
		template <typename Type>
		static auto HasContext() noexcept -> bool;
		template <typename Type>
		static auto RemoveContext() noexcept -> void;
		static auto GetContextIndex(entt::id_type typeIndex) -> entt::id_type;
	};

	namespace Internal {
		template<typename Type>
		struct entt::type_seq<Type> {
			[[nodiscard]] static auto value() {
				static const auto value = Helena::Core::GetContextIndex(entt::type_hash<Type>::value());
				return value;
			}
		};
	}
}

#include <Common/Core.ipp>

#endif // COMMON_CORE_HPP