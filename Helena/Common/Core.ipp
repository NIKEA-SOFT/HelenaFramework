#ifndef COMMON_CORE_IPP
#define COMMON_CORE_IPP

namespace Helena
{
#if HF_PLATFORM == HF_PLATFORM_WIN
	inline BOOL WINAPI Core::CtrlHandler(DWORD dwCtrlType) 
	{
		static std::mutex mutex {};
        std::lock_guard lock{mutex};

		if(m_Context && !m_Context->m_Shutdown) {
			Core::Shutdown();
		}

		return TRUE;
	}

	inline int Core::SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException) {
		HF_MSG_FATAL("SEH Handler, code: {}", code);
		if(pException) {
			HF_MSG_FATAL("Exception address: {}, code: {}", 
				pException->ExceptionRecord->ExceptionAddress, 
				pException->ExceptionRecord->ExceptionCode);
		}

		return EXCEPTION_EXECUTE_HANDLER;
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

	inline auto Core::Heartbeat(float tickrate) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}
		
		auto& ctx = *m_Context;
		ctx.m_TickrateMax = tickrate;

		const auto framerate = 1.0f / ctx.m_TickrateMax;
		ctx.m_Tickrate = framerate;
		auto currentTime = std::chrono::steady_clock::now();
		auto previousTime = std::chrono::steady_clock::now();

		//float frameTime = 0.0f;
		//float frameTimeAVG = 0.0f;
		//std::size_t frameCounter = 0;
		//std::size_t frameCounterStep = 100;
		//std::size_t frameCounterSize = frameCounterStep;

		while(!ctx.m_Shutdown) 
		{
			//if(frameCounter >= frameCounterSize) {
			//	//frameTimeAVG = frameTime / frameCounter;
			//	frameCounterSize = frameCounter + frameCounterStep;
			//	//HF_MSG_INFO("Average Frame Time: {:.8f}", frameTimeAVG);
			//}

			previousTime = currentTime;
			currentTime = std::chrono::steady_clock::now();
			
			ctx.m_FrameTime = std::chrono::duration<float>{currentTime - previousTime}.count();
			ctx.m_Tickrate += ctx.m_FrameTime;
			/*frameTime += m_Context->m_FrameTime;*/

			for(std::size_t i = 0; i < ctx.m_SystemManager.m_Systems.size(); ++i) 
			{
				if(auto& system = ctx.m_SystemManager.m_Systems[i]; !system.m_bInitialized && system.m_EventCreate) {
					system.m_EventCreate();
					system.m_bInitialized = true;
				}
			}

			if(ctx.m_Tickrate >= framerate - 0.001f) {
				Util::Sleep(1);
			}

			//if(!(frameTimeAVG + tickrate >= framerate - 0.001f)) {
			//	Util::Sleep(1);
			//} else {
			//	//HF_MSG_WARN("Sleep ignored");
			//}

			if(ctx.m_Tickrate >= framerate)
			{
				ctx.m_Dispatcher.trigger<Events::Core::UpdatePre>();
				ctx.m_Dispatcher.trigger<Events::Core::Update>();
				ctx.m_Dispatcher.trigger<Events::Core::UpdatePost>();

				//HF_MSG_WARN("Update Tickrate: {:.4f}, ticknow: {:.5f}, Framerate: {:.6f}, Avg: {:.6f}", tickrate, tickrate - framerate, m_Context->m_FrameTime, averageFrameTime);
				ctx.m_Tickrate -= framerate;
			}

			/*++frameCounter;*/
		}

		for(const auto& system : ctx.m_SystemManager.m_Systems) 
		{
			if(system.m_EventDestroy) {
				system.m_EventDestroy();
			}
		}
		
		HF_MSG_WARN("Heartbeat down");
	}

	inline auto Core::Shutdown() noexcept -> void {
		m_Context->m_Shutdown = true;
	}

	inline auto Core::SetArgs(const std::size_t argc, const char* const* argv) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
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

	[[nodiscard]] inline auto Core::GetContext() noexcept -> const std::shared_ptr<Context>& 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

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
		if(system.m_Instance) {
			HF_MSG_ERROR("System: {} already has!", entt::type_name<TSystem>().value());
			return nullptr;
		}

		system.m_Instance = any_t{std::in_place_type<Type>, std::forward<Args>(args)...};

		if constexpr (Internal::is_detected_v<System::fn_create_t, TSystem>) {
			system.m_EventCreate.template connect<&TSystem::OnCreate>(entt::any_cast<TSystem&>(system.m_Instance));
		}

		//if constexpr (Internal::is_detected_v<System::fn_update_t, TSystem, float>) {
		//	system.m_EventUpdate.template connect<&TSystem::OnUpdate>(entt::any_cast<TSystem&>(system.m_Instance));
		//}

		if constexpr (Internal::is_detected_v<System::fn_destroy_t, TSystem>) {
			system.m_EventDestroy.template connect<&TSystem::OnDestroy>(entt::any_cast<TSystem&>(system.m_Instance));
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
		
		return entt::any_cast<TSystem>(&system.m_Instance);
	}

	template <typename Type>
	[[nodiscard]] auto Core::GetSystem() -> Type* 
	{
		using TSystem = std::remove_cv_t<std::remove_reference_t<Type>>;

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return nullptr;
		}

		const auto index	= SystemIndex<TSystem>::GetIndex();
		auto& systems		= m_Context->m_SystemManager.m_Systems;

		if(index >= systems.size() || systems[index].m_Instance == any_t{}) {
			HF_MSG_ERROR("System: {} not exist!", entt::type_name<Type>().value());
			return nullptr;
		}

		return entt::any_cast<Type>(&systems[index].m_Instance);
	}

	template <typename Type>
	auto Core::RemoveSystem() -> void
	{
		using TSystem = std::remove_cv_t<std::remove_reference_t<Type>>;

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		const auto index	= SystemIndex<TSystem>::GetIndex();
		auto& systems		= m_Context->m_SystemManager.m_Systems;
		
		if(index >= systems.size() || systems[index].m_Instance == any_t{}) {
			HF_MSG_ERROR("System: {} not exist for remove!", entt::type_name<Type>().value());
			return;
		}

		auto& system = systems[index];
		system.m_Instance = entt::any{};
		system.m_EventCreate.reset();
		system.m_EventUpdate.reset();
		system.m_EventDestroy.reset();
	}

	//template <typename Event, auto Method>
	//auto Core::RegisterEvent() -> void 
	//{
	//	// Check core initialiation
	//	if(!m_Context) {
	//		HF_MSG_ERROR("Core not initialized!");
	//		return;
	//	}

	//	m_Context->m_Dispatcher.sink<Event>().template connect<Method>();
	//}


	template <typename Event, auto Method, typename Type>
	auto Core::RegisterEvent(Type&& instance) -> void 
	{
		// Check core initialiation
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		m_Context->m_Dispatcher.sink<Event>().template connect<Method>(instance);
	}

	template <typename Event, typename... Args>
	auto Core::TriggerEvent(Args&&... args) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		m_Context->m_Dispatcher.template trigger<Event>(std::forward<Args>(args)...);
	}

	template <typename Event, typename... Args>
	auto Core::EnqueueEvent(Args&&... args) -> void
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		m_Context->m_Dispatcher.template enqueue<Event>(std::forward<Args>(args)...);
	}

	template <typename Event>
	auto Core::UpdateEvent() -> void {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		m_Context->m_Dispatcher.template update<Event>();
	}

	template <typename Event, auto Method>
	auto Core::RemoveEvent() -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		m_Context->m_Dispatcher.sink<Event>().template disconnect<Method>();
	}

	template <typename Event, auto Method, typename Type>
	auto Core::RemoveEvent(Type&& instance) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		m_Context->m_Dispatcher.sink<Event>().template disconnect<Method>(instance);
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