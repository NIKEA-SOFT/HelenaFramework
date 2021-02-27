#ifndef COMMON_CORE_IPP
#define COMMON_CORE_IPP

namespace Helena
{
#if HF_PLATFORM == HF_PLATFORM_WIN
	inline BOOL WINAPI Core::CtrlHandler(DWORD dwCtrlType) 
	{
		if(m_Context) {
			m_Context->m_Signal = true;
		}

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
	inline auto Core::SigHandler(int signal) -> void
	{
		if(m_Context) {
			m_Context->m_Signal = true;
		}
	}
#endif

	[[nodiscard]] inline auto Core::Initialize(const std::shared_ptr<Context>& ctx) -> bool 
	{
		if(m_Context) {
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

			if(m_Context = std::make_shared<Context>(); !m_Context) {
				HF_MSG_ERROR("Allocate memory for core context failed!");
				return false;
			}

		} else {
			m_Context = ctx;
		}

		return true;
	}

	inline auto Core::SetArgs(const std::size_t argc, const char* const* argv) -> void 
	{
		if(m_Context) {
			HF_MSG_ERROR("Core context already exist");
			return;
		}

		m_Context->m_Args.clear();
		m_Context->m_Args.reserve(argc);

		for(std::size_t i = 0; i < argc; ++i) {
			m_Context->m_Args.emplace_back(argv[i]);
		}
	}

	[[nodiscard]] inline auto Core::GetArgs() noexcept -> decltype(auto)
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return std::vector<std::string_view>{};
		}

		return m_Context->m_Args;
	}

	[[nodiscard]] inline auto Core::GetSignal() noexcept -> bool {
		// todo signal dispatcher add
		return m_Context->m_Signal;
	}

	[[nodiscard]] inline auto Core::GetContext() noexcept -> const std::shared_ptr<Context>& {
		return m_Context;
	}

	template <typename Type, typename... Args>
	auto Core::RegisterSystem([[maybe_unused]] Args&&... args) -> Type* 
	{
		using TSystem = std::remove_cv_t<std::remove_reference_t<Type>>;

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return nullptr;
		}

		const auto index	= SystemIndex<TSystem>::GetIndex();
		auto& systems		= m_Context->m_SystemManager.m_Systems;

		if(index >= systems.size()) {
			systems.resize(index + 1);
		}
		
		auto& system = systems[index];
		if(system.m_Instance.has_value()) {
			HF_MSG_ERROR("System: {} already has!", entt::type_name<TSystem>().value());
			return nullptr;
		}

		system.m_Instance = std::make_any<Test>(std::forward<Args>(args)...);

		if constexpr (Internal::is_detected_v<System::fn_create_t, TSystem>) {
			system.m_EventCreate.template connect<&TSystem::OnCreate>(std::any_cast<TSystem&>(system.m_Instance));
		}

		if constexpr (Internal::is_detected_v<System::fn_update_t, TSystem, float>) {
			system.m_EventUpdate.template connect<&TSystem::OnUpdate>(std::any_cast<TSystem&>(system.m_Instance));
		}

		if constexpr (Internal::is_detected_v<System::fn_destroy_t, TSystem>) {
			system.m_EventDestroy.template connect<&TSystem::OnDestroy>(std::any_cast<TSystem&>(system.m_Instance));
		}

		// Test callback's
		//if(system.m_EventCreate) {
		//	system.m_EventCreate();
		//}

		//if(system.m_EventUpdate) {
		//	system.m_EventUpdate(1.0f);
		//}

		//if(system.m_EventDestroy) {
		//	system.m_EventDestroy();
		//}

		return std::any_cast<TSystem>(&system.m_Instance);
	}

	template <typename Type>
	[[nodiscard]] auto Core::GetSystem() noexcept -> Type* 
	{
		using TSystem = std::remove_cv_t<std::remove_reference_t<Type>>;

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return nullptr;
		}

		const auto index	= SystemIndex<TSystem>::GetIndex();
		auto& systems		= m_Context->m_SystemManager.m_Systems;

		if(index >= systems.size() || !systems[index].m_Instance.has_value()) {
			HF_MSG_ERROR("System: {} not exist!", entt::type_name<Type>().value());
			return nullptr;
		}

		return std::any_cast<Type>(&systems[index].m_Instance);
	}

	template <typename Type>
	auto Core::RemoveSystem() noexcept -> void
	{
		using TSystem = std::remove_cv_t<std::remove_reference_t<Type>>;

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		const auto index	= SystemIndex<TSystem>::GetIndex();
		auto& systems		= m_Context->m_SystemManager.m_Systems;

		if(index >= systems.size() || !systems[index].m_Instance.has_value()) {
			HF_MSG_ERROR("System: {} not exist for remove!", entt::type_name<Type>().value());
			return;
		}

		auto& system = systems[index];
		system.m_Instance.reset();
		system.m_EventCreate.reset();
		system.m_EventUpdate.reset();
		system.m_EventDestroy.reset();
	}

	template <typename Event, auto Method, typename Type>
	auto Core::RegisterEvent(Type&& instance) -> bool 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return false;
		}

		auto wtf = m_Context->m_Dispatcher.sink<Event>().connect<Method>(instance);
		return true;
	}

	[[nodiscard]] inline auto Core::GetTypeIndex(indexes_hash_t& container, const entt::id_type typeIndex) -> std::size_t
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		if(const auto it = container.find(typeIndex); it != container.cend()) {
			return it->second;
		}

		//if(container.size() >= static_cast<entt::id_type>(std::numeric_limits<entt::id_type>::max())) {
		//	HF_MSG_FATAL("Limit of type index");
		//	std::terminate();
		//}
		
		if(const auto [it, result] = container.emplace(typeIndex, container.size()); !result) {
			HF_MSG_FATAL("Type index emplace failed!");
			std::terminate();
		}

		return container.size() - 1;
	}
}

namespace entt {
	template <typename Type>
	struct ENTT_API type_seq<Type> {
		[[nodiscard]] static id_type value() ENTT_NOEXCEPT {
			static const auto value = static_cast<entt::id_type>(Helena::Core::GetTypeIndex(Helena::Core::GetContext()->m_TypeIndexes, entt::type_hash<Type>::value()));
			return value;
		}
	};
}

#endif // COMMON_CORE_IPP