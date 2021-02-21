#ifndef COMMON_CORE_IPP
#define COMMON_CORE_IPP

namespace Helena
{
#if HF_PLATFORM == HF_PLATFORM_WIN
	inline BOOL WINAPI Core::CtrlHandler(DWORD dwCtrlType) {
		Core::GetCoreCtx()->m_Signal = true;
		HF_MSG_FATAL("CtrlHander called");
		return TRUE;
	}

	inline int Core::SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException) {
		HF_MSG_FATAL("SEH Handler, code: {}", code);
		if(pException) {
			HF_MSG_FATAL("Exception address: {}, code: {}", 
				pException->ExceptionRecord->ExceptionAddress, 
				pException->ExceptionRecord->ExceptionCode);
		}

		return TRUE;
	}

#elif HF_PLATFORM == HF_PLATFORM_LINUX
	inline auto Core::SigHandler(int signal) -> void {
		Core::GetContext()->m_Signal = true;
	}
#endif

	[[nodiscard]] inline auto Core::Initialize(const std::shared_ptr<CoreCtx>& ctx) -> bool 
	{
		if(m_Ctx) {
			HF_MSG_ERROR("Core context already exist");
			return false;
		}

		if(!ctx)
		{
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

			m_Ctx = std::make_shared<CoreCtx>();
			m_Ctx->m_Entity = m_Ctx->m_Registry.create();
		} else {
			m_Ctx = ctx;
		}

		return true;
	}

	inline auto Core::SetArgs(const std::size_t argc, const char* const* argv) -> void 
	{
		if(const auto& ctx = GetCoreCtx(); ctx) 
		{
			ctx->m_Args.clear();
			ctx->m_Args.reserve(argc);

			for(std::size_t i = 0; i < argc; ++i) {
				ctx->m_Args.emplace_back(argv[i]);
			}
		}
	}

	[[nodiscard]] inline auto Core::GetArgs() noexcept -> const std::vector<std::string_view>& 
	{
		const auto& ctx = GetCoreCtx();
		return ctx->m_Args;
	}

	[[nodiscard]] inline auto Core::GetSignal() noexcept -> bool 
	{
		const auto& ctx = GetCoreCtx();
		return ctx->m_Signal;
	}

	[[nodiscard]] inline auto Core::GetCoreCtx() noexcept -> const std::shared_ptr<CoreCtx>& 
	{
		HF_ASSERT(m_Ctx, "Core not initialized");

	#ifndef HF_DEBUG
		if(!m_Ctx) {
			HF_MSG_FATAL("Core not initialized");
			std::terminate();
		}
	#endif

		return m_Ctx;
	}

	template <typename Type, typename... Args>
	auto Core::CreateCtx([[maybe_unused]] Args&&... args) -> Type* 
	{
		static_assert(!std::is_empty_v<Type>, "CreateCtx with empty class");

		if(const auto& ctx = GetCoreCtx(); ctx) 
		{
			if(!ctx->m_Registry.has<Type>(ctx->m_Entity)) {
				return &ctx->m_Registry.emplace<Type>(ctx->m_Entity, std::forward<Args>(args)...);
			}

			HF_MSG_ERROR("Context: {} already exist!", HF_CLASSNAME_RT(Type));
		}

		return nullptr;
	}

	template <typename Type>
	[[nodiscard]] auto Core::GetCtx() noexcept -> Type* 
	{
		if(const auto& ctx = GetCoreCtx(); ctx) 
		{
			if(const auto ptr = ctx->m_Registry.try_get<Type>(ctx->m_Entity); ptr) {
				return ptr;
			}

			HF_MSG_ERROR("Context: {} not exist!", HF_CLASSNAME_RT(Type));
		}

		return nullptr;
	}

	template <typename Type>
	auto Core::RemoveCtx() noexcept -> void 
	{
		if(const auto& ctx = GetCoreCtx(); ctx) 
		{
			if(ctx->m_Registry.has<Type>(ctx->m_Entity)) {
				ctx->m_Registry.remove<Type>(ctx->m_Entity);
			} else {
				HF_MSG_ERROR("Context: {} not exist!", HF_CLASSNAME_RT(Type));
			}
		}
	}

	[[nodiscard]] inline auto Core::GetCtxId(const entt::id_type typeId) -> entt::id_type 
	{
		auto& indexes = GetCoreCtx()->m_TypeIndexes;

		if(const auto it = indexes.find(typeId); it != indexes.cend()) {
			return it->second;
		}

		if(indexes.size() >= static_cast<entt::id_type>(std::numeric_limits<entt::id_type>::max())) {
			HF_MSG_FATAL("Limit of type index");
			std::terminate();
		}
		
		if(const auto [it, result] = indexes.emplace(typeId, static_cast<entt::id_type>(indexes.size())); !result) {
			HF_MSG_FATAL("Emplace type index failed");
			std::terminate();
		}

		return static_cast<entt::id_type>(indexes.size() - 1);
	}
}

namespace entt {
	template <typename Type>
	struct ENTT_API entt::type_seq<Type> {
		[[nodiscard]] static auto value() {
			static const auto value = Helena::Core::GetCtxId(entt::type_hash<Type>::value());
			return value;
		}
	};
}

#endif // COMMON_CORE_IPP