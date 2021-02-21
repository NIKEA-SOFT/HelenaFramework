#ifndef COMMON_CORE_HPP
#define COMMON_CORE_HPP

namespace Helena
{
	class Core
	{
		template <typename, typename>
		friend struct ENTT_API entt::type_seq;

		struct CoreCtx {
			entt::entity m_Entity {entt::null};
			entt::registry m_Registry;
			std::atomic_bool m_Signal;
			std::vector<std::string_view> m_Args;
			std::unordered_map<entt::id_type, entt::id_type> m_TypeIndexes;
		};

		static inline std::shared_ptr<CoreCtx> m_Ctx {};

	private:
	#if HF_PLATFORM == HF_PLATFORM_WIN
		static BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
		static int SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException);
	#elif HF_PLATFORM == HF_PLATFORM_LINUX
		static auto SigHandler(int signal) -> void;
	#endif

		[[nodiscard]] static auto GetCtxId(const entt::id_type typeIndex) -> entt::id_type;

	public:
		Core() = delete;
		~Core() = delete;
		Core(const Core&) = delete;
		Core(Core&&) = delete;
		Core& operator=(const Core&) = delete;
		Core& operator=(Core&&) = delete;

		[[nodiscard]] static auto Initialize(const std::shared_ptr<CoreCtx>& ctx = {}) -> bool;

		static auto SetArgs(const std::size_t size, const char* const* argv) -> void;

		[[nodiscard]] static auto GetArgs() noexcept -> const std::vector<std::string_view>&;
		[[nodiscard]] static auto GetSignal() noexcept -> bool;
		[[nodiscard]] static auto GetCoreCtx() noexcept -> const std::shared_ptr<CoreCtx>&;

		template <typename Type, typename... Args>
		static auto CreateCtx([[maybe_unused]] Args&&... args) -> Type*;

		template <typename Type>
		[[nodiscard]] static auto GetCtx() noexcept -> Type*;

		template <typename Type>
		static auto RemoveCtx() noexcept -> void;
	};
}

#include <Common/Core.ipp>

#endif // COMMON_CORE_HPP