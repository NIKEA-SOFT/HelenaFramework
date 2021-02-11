#ifndef COMMON_CORE_HPP
#define COMMON_CORE_HPP

namespace Helena
{
	class Core HF_FINAL
	{
		static inline std::shared_ptr<CoreCtx> m_Ctx {};

		template<typename Type, typename = void>
		friend struct ENTT_API entt::type_seq;

		struct CoreCtx {
			entt::entity m_Entity {entt::null};
			entt::registry m_Registry;
			std::atomic_bool m_Signal;
			std::vector<std::string_view> m_Args;
			std::unordered_map<entt::id_type, entt::id_type> m_TypeIndexes;
		};

	private:
	#if HF_PLATFORM == HF_PLATFORM_WIN
		static auto WINAPI CtrlHandler(DWORD) -> BOOL;
		static auto SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException) -> int;
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

		static auto SetArgs(const std::size_t size, char** argv) -> void;

		[[nodiscard]] static auto GetArgs() noexcept -> const std::vector<std::string_view>&;
		[[nodiscard]] static auto GetSignal() noexcept -> bool;
		[[nodiscard]] static auto GetCoreCtx() noexcept -> const std::shared_ptr<CoreCtx>&;

		template <typename Type, typename... Args>
		static auto CreateCtx(Args&&... args) -> Type*;

		template <typename Type>
		[[nodiscard]] static auto GetCtx() noexcept -> Type*;

		template <typename Type>
		static auto RemoveCtx() noexcept -> void;
	};

	namespace Internal {
		template<typename Type>
		struct entt::type_seq<Type> {
			[[nodiscard]] static auto value() {
				static const auto value = Helena::Core::GetCtxId(entt::type_hash<Type>::value());
				return value;
			}
		};
	}
}

#include <Common/Core.ipp>

#endif // COMMON_CORE_HPP