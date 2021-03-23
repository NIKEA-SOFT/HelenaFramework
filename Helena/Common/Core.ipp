#ifndef COMMON_CORE_IPP
#define COMMON_CORE_IPP

namespace Helena
{
#if HF_PLATFORM == HF_PLATFORM_WIN
	inline BOOL WINAPI Core::CtrlHandler(DWORD dwCtrlType) 
	{
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

	[[nodiscard]] inline auto Core::Initialize(const std::function<bool ()>& callback, const std::shared_ptr<Context>& ctx) -> bool 
	{
		if(m_Context) {
			HF_MSG_ERROR("Core context already exist");
			return false;
		}

		// Create context or set (set used for support modules dll/lib)
		if(!ctx)
		{
			// Hook signals
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

			// Create instance of context
			if(m_Context = std::make_shared<Context>(); !m_Context) {
				HF_MSG_ERROR("Allocate memory for core context failed!");
				return false;
			}
			
			m_Context->m_TimeStart	= std::chrono::steady_clock::now();	// Start time (used for calculate elapsed time)
			m_Context->m_TimeNow	= m_Context->m_TimeStart;
			m_Context->m_TimePrev	= m_Context->m_TimeNow; 
			m_Context->m_TickRate	= m_Context->m_TickRate ? m_Context->m_TickRate : (1.0 / 30.0);

		} else {
			// Set instance of context (for modules)
			m_Context = ctx;
		}

		// callback for initialize and register systems
		if(!callback || !callback()) {
			return false;
		}

		// Core loop
		if(!ctx) {
			Heartbeat();
		}

		return true;
	}

	inline auto Core::Heartbeat() -> void 
	{		
		// Call the delegates of listeners
		m_Context->m_Dispatcher.trigger<Events::Core::HeartbeatBegin>();

		double timeElapsed {};
		while(!m_Context->m_Shutdown) 
		{
			// Get time and delta
			m_Context->m_TimePrev	= m_Context->m_TimeNow;
			m_Context->m_TimeNow	= std::chrono::steady_clock::now();
			m_Context->m_TimeDelta	= std::chrono::duration<double>{m_Context->m_TimeNow - m_Context->m_TimePrev}.count();
			timeElapsed				+= m_Context->m_TimeDelta;

			// Check new registered systems
			while(!m_Context->m_SystemManager.m_SystemsBegin.empty()) 
			{
				// Pop system object and check has method "Create" for call
				if(const auto& system = m_Context->m_SystemManager.m_Systems[m_Context->m_SystemManager.m_SystemsBegin.front()]; system.m_EventCreate) {
					system.m_EventCreate();	// Call method "Create"
				}

				// Remove front system object from container
				m_Context->m_SystemManager.m_SystemsBegin.pop();
			}

			// Iterate all the systems objects with the Update method
			for(std::size_t i = 0, size = m_Context->m_SystemManager.m_SystemsUpdatable.size(); i < size; ++i)
			{
				// Get reference on the system object
				const auto index = m_Context->m_SystemManager.m_SystemsUpdatable.front();
				const auto& system = m_Context->m_SystemManager.m_Systems[index];

				// Pop element from updatable list
				// If the m_EventUpdate is empty, but it is in the list, 
				// then this means that the System object has been destroyed and in next tick
				// it will be removed from current updatable list
				if(m_Context->m_SystemManager.m_SystemsUpdatable.pop(); system.m_EventUpdate) {
					system.m_EventUpdate();	// Call method "Create"
					m_Context->m_SystemManager.m_SystemsUpdatable.emplace(index); // emplace index in updatable queue
				}
			}

			// if elapsed time is more the tickrate time 
			if(timeElapsed >= m_Context->m_TickRate) {
				// Call of the delegates of events
				m_Context->m_Dispatcher.trigger<Events::Core::TickPre>();
				m_Context->m_Dispatcher.trigger<Events::Core::Tick>();
				m_Context->m_Dispatcher.trigger<Events::Core::TickPost>();

				// 
			#if HF_PLATFORM == HF_PLATFORM_WIN
				if(static std::size_t counter{}; !(++counter % 3)) {
					const auto title = HF_FORMAT("Helena | Delta: {:.4f} sec | Elapsed: {:.4f} sec", m_Context->m_TimeDelta, timeElapsed);
					SetConsoleTitle(title.c_str());
					counter = 0;
				}
			#endif

				timeElapsed -= m_Context->m_TickRate;
			}
			
			if(timeElapsed < m_Context->m_TickRate) {
				//const auto sleepTime = static_cast<std::uint32_t>((m_Context->m_TickRate - m_Context->m_TimeElapsed) * 1000.0);
				Util::Sleep(std::chrono::milliseconds{1});
			}
		}

		// // Call the delegates of listeners
		m_Context->m_Dispatcher.trigger<Events::Core::HeartbeatEnd>();

		// 
		for(std::size_t i = 0; i < m_Context->m_SystemManager.m_Systems.size(); ++i) 
		{
			if(const auto& system = m_Context->m_SystemManager.m_Systems[i]; system.m_EventDestroy) {
				system.m_EventDestroy();
			}
		}
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

		auto& ctx = *m_Context;
		ctx.m_Args.clear();
		ctx.m_Args.reserve(argc);

		for(std::size_t i = 0; i < argc; ++i) {
			ctx.m_Args.emplace_back(argv[i]);
		}
	}

	inline auto Core::SetTickrate(const double tickrate) -> void {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		m_Context->m_TickRate = 1.0 / tickrate;
		HF_MSG_DEBUG("Hearbeat tickrate new value: {:.4f}", m_Context->m_TickRate);
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

	[[nodiscard]] inline auto Core::GetTickrate() noexcept {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		return m_Context->m_TickRate;
	}

	[[nodiscard]] inline auto Core::GetTimeElapsed() noexcept {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		return std::chrono::duration<double>{std::chrono::steady_clock::now() - m_Context->m_TimeStart}.count();
	}

	[[nodiscard]] inline auto Core::GetTimeDelta() noexcept {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		return m_Context->m_TimeDelta;
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

		system.m_Instance = entt::any{std::in_place_type<Type>, std::forward<Args>(args)...};

		if constexpr (Internal::is_detected_v<System::fn_create_t, TSystem>) {
			system.m_EventCreate.template connect<&TSystem::Create>(entt::any_cast<TSystem&>(system.m_Instance));
			m_Context->m_SystemManager.m_SystemsBegin.emplace(index);
		}

		if constexpr (Internal::is_detected_v<System::fn_update_t, TSystem>) {
			system.m_EventUpdate.template connect<&TSystem::Update>(entt::any_cast<TSystem&>(system.m_Instance));
			m_Context->m_SystemManager.m_SystemsUpdatable.emplace(index);
		}

		if constexpr (Internal::is_detected_v<System::fn_destroy_t, TSystem>) {
			system.m_EventDestroy.template connect<&TSystem::Destroy>(entt::any_cast<TSystem&>(system.m_Instance));
		}
		
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

	/**
	* @brief Remove system from the container of systems
	* @tparam Type Type of object to use to remove the system
	*/
	template <typename Type>
	auto Core::RemoveSystem() -> void
	{
		using TSystem = std::remove_cv_t<std::remove_reference_t<Type>>;

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		// Get the reference on container of the system instances
		auto& systems = m_Context->m_SystemManager.m_Systems;

		// Get the type index from the static cache and check has of the system
		if(const auto index = SystemIndex<TSystem>::GetIndex(); index < systems.size() && systems[index].m_Instance) 
		{
			// Get the reference on the system by index
			auto& system = systems[index];

			// Check if exist the "Destroy" method for the system
			if(system.m_EventDestroy) {

				// Call the method "Destroy"
				system.m_EventDestroy();
			}

			// Destroy current system
			system.m_Instance = entt::any{};
			system.m_EventCreate.reset();
			system.m_EventUpdate.reset();
			system.m_EventDestroy.reset();
		} else {
			HF_MSG_ERROR("System: {} not exist for remove!", entt::type_name<Type>().value());
		}
	}

	template <typename Event, auto Method>
	auto Core::RegisterEvent() -> void 
	{
		// Check core initialiation
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			return;
		}

		m_Context->m_Dispatcher.sink<Event>().template connect<Method>();
	}


	template <typename Event, auto Method, typename Type>
	auto Core::RegisterEvent(Type&& instance) -> void 
	{
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

	[[nodiscard]] inline auto Core::GetTypeIndex(std::unordered_map<entt::id_type, std::size_t>& container, const entt::id_type typeIndex) -> std::size_t
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