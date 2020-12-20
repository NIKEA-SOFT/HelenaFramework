#ifndef COMMON_CORE_IPP
#define COMMON_CORE_IPP

namespace Helena
{
#if HF_PLATFORM == HF_PLATFORM_WIN
	inline auto WINAPI Core::CtrlHandler(DWORD) -> BOOL {
		Core::GetContext()->m_Signal = true;
		return TRUE;
	}

	inline auto Core::SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException) -> int {
		std::cout << "SEH Handler, Code: " << code << std::endl;
	}

#elif HF_PLATFORM == HF_PLATFORM_LINUX
	inline auto Core::SigHandler(int signal) -> void {
		Core::GetContext()->m_Signal = true;
	}
#endif

	inline auto Core::SetContext(const std::shared_ptr<Ctx>& ctx) noexcept -> void {
		GetContext() = ctx;
	}

	inline auto Core::GetContext() -> std::shared_ptr<Ctx>& {
		static auto instance{[]() {
			auto ctx = std::make_shared<Ctx>();
			ctx->m_Entity = ctx->m_Registry.create();
			// register signal's
		#if HF_PLATFORM == HF_PLATFORM_WIN
			SetConsoleCtrlHandler(CtrlHandler, TRUE);
		#elif HF_PLATFORM == HF_PLATFORM_LINUX
			signal(SIGTERM, Service::SigHandler);
			signal(SIGSTOP, Service::SigHandler);
			signal(SIGINT,  Service::SigHandler);
			signal(SIGKILL, Service::SigHandler);
		#else
			#error Unknown platform
		#endif
			return ctx;
		}()};
		return instance;
	}

	inline auto Core::SetArgs(std::size_t argc, char** argv) noexcept -> void {
		auto& ctx = GetContext();
		ctx->m_ArgCount = argc;
		ctx->m_Args = argv;
	}

	inline auto Core::GetArgs(std::size_t offset) noexcept -> const char* {
		auto& ctx = GetContext();
		HF_ASSERT(offset < ctx->m_ArgCount, "Args offset out of bounds!");
		HF_ASSERT(ctx->m_Args[offset], "Args is nullptr, use SetArgs for set arguments!");
		return ctx->m_Args[offset];
	}

	inline auto Core::GetArgsCount() noexcept -> std::size_t {
		return GetContext()->m_ArgCount;
	}

	inline auto Core::GetSignal() noexcept -> bool {
		return GetContext()->m_Signal;
	}

	template <typename Type, typename... Args>
	inline auto Core::CreateContext(Args&&... args) -> Type& {
		HF_ASSERT(!HasContext<Type>(), "Context type already exist!");
		auto& ctx = GetContext();
		return ctx->m_Registry.emplace<Type>(ctx->m_Entity, std::forward<Args>(args)...);
	}

	template <typename Type>
	inline auto Core::GetContext() noexcept -> Type& {
		HF_ASSERT(HasContext<Type>(), "Context type not exist!");
		auto& ctx = GetContext();
		return ctx->m_Registry.get<Type>(ctx->m_Entity);
	}

	template <typename Type>
	inline auto Core::HasContext() noexcept -> bool {
		auto& ctx = GetContext();
		return ctx->m_Registry.has<Type>(ctx->m_Entity);
	}

	template <typename Type>
	inline auto Core::RemoveContext() noexcept -> void {
		HF_ASSERT(HasContext<Type>(), "Context type not exist!");
		auto& ctx = GetContext();
		return ctx->m_Registry.remove<Type>(ctx->m_Entity);
	}

	inline auto Core::GetContextIndex(entt::id_type typeIndex) -> entt::id_type {
		auto& indexes = GetContext()->m_TypeIndexes;
		if(const auto it = indexes.find(typeIndex); it != indexes.cend()) {
			return it->second;
		}
		return indexes.emplace(typeIndex, indexes.size()).first->second;
	}
}

#endif // COMMON_CORE_IPP