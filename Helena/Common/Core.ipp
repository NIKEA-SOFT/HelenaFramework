#ifndef COMMON_CORE_IPP
#define COMMON_CORE_IPP

namespace Helena
{
#if HF_PLATFORM == HF_PLATFORM_WIN
	inline void Core::Terminate() {
		HF_MSG_WARN("Terminating");
	}

	inline BOOL WINAPI Core::CtrlHandler(DWORD dwCtrlType) 
	{
        static std::mutex mutex;
        std::lock_guard lock{mutex};

		if(m_Context && !m_Context->m_Shutdown) {
			Core::Shutdown();
		}

		while(m_Context->m_Shutdown) {
			Util::Sleep(10);
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

	template <typename Type>
	[[nodiscard]] auto Core::SystemManager::SystemIndex<Type>::GetIndex() -> std::size_t {
		static const auto value = m_Context->m_SystemManager.GetIndexer(Hash::Type<Type>);
		return value;
	}

	//[[nodiscard]] inline Core::SystemManager::System::operator bool() const noexcept {
	//	return static_cast<bool>(m_Instance);
	//}

	template <typename Type, typename... Args>
	auto Core::SystemManager::AddSystem([[maybe_unused]] Args&&... args) -> Type* 
	{
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, 
			"Resource type cannot be const/ptr/ref");

		const auto index = SystemIndex<Type>::GetIndex();
		if(index >= m_Systems.size()) {
			m_Systems.resize(index + 1);
		}

		auto& system = m_Systems[index];
		if(!system.m_Instance)
		{
			system.m_Instance.template emplace<Type>(std::forward<Args>(args)...);
		
			if constexpr (Internal::is_detected_v<fn_create_t, Type>) {
				system.m_Events[Events::Create].template connect<&Type::OnSystemCreate>(entt::any_cast<Type&>(system.m_Instance));
				m_EventScheduler[Events::Create].emplace(index);
			}

			if constexpr (Internal::is_detected_v<fn_execute_t, Type>) {
				system.m_Events[Events::Execute].template connect<&Type::OnSystemExecute>(entt::any_cast<Type&>(system.m_Instance));
				m_EventScheduler[Events::Execute].emplace(index);
			}

			if constexpr (Internal::is_detected_v<fn_update_t, Type>) {
				system.m_Events[Events::Update].template connect<&Type::OnSystemUpdate>(entt::any_cast<Type&>(system.m_Instance));
				m_EventScheduler[Events::Update].emplace(index);
			}

			if constexpr (Internal::is_detected_v<fn_tick_t, Type>) {
				system.m_Events[Events::Tick].template connect<&Type::OnSystemTick>(entt::any_cast<Type&>(system.m_Instance));
				m_EventScheduler[Events::Tick].emplace(index);
			}

			if constexpr (Internal::is_detected_v<fn_destroy_t, Type>) {
				system.m_Events[Events::Destroy].template connect<&Type::OnSystemDestroy>(entt::any_cast<Type&>(system.m_Instance));
				m_EventScheduler[Events::Destroy].emplace(index);
			}
		}

		//HF_MSG_ERROR("System: {} already has!", entt::type_name<Type>().value());
		return entt::any_cast<Type>(&system.m_Instance);
	}

	template <typename Type>
	auto Core::SystemManager::GetSystem() noexcept -> Type* {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, 
			"Resource type cannot be const/ptr/ref");

		const auto index = SystemIndex<Type>::GetIndex();
		return index < m_Systems.size() ? entt::any_cast<Type>(&m_Systems[index].m_Instance) : nullptr;
	}

	template <typename Type>
	auto Core::SystemManager::RemoveSystem() noexcept -> void {
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, 
			"Resource type cannot be const/ptr/ref");

		const auto index = SystemIndex<TSystem>::GetIndex();
		
		if(index < systems.size()) 
		{
			if(auto& system = m_Systems[index]; system.m_Instance) 
			{
				if(system.m_EventDestroy) {
					system.m_EventDestroy();
				}

				system.m_Instance.reset();
				system.m_EventCreate.reset();
				system.m_EventExecute.reset();
				system.m_EventUpdate.reset();
				system.m_EventDestroy.reset();
			}
		}
	}


	inline auto Core::SystemManager::Event(const Events type) -> void 
	{
		auto& container = m_EventScheduler[type];
		for(std::size_t size = container.size(); size; --size) 
		{
			const auto index	= container.front();
			const auto& event	= m_Systems[index].m_Events[type];

			if(event) {
				event();
			}

			container.pop();

			// For tick and update methods we emplace it again (it's scheduler for event)
			switch(type) {
				case Events::Tick: [[fallthrough]];
				case Events::Update: {
					container.emplace(index);
				} break;
				default: break;
			}
		}
	}

	[[nodiscard]] inline auto Core::SystemManager::GetSystems() noexcept -> vec_systems_t& {
		return m_Systems;
	}

	[[nodiscard]] inline auto Core::SystemManager::GetIndexer(const entt::id_type typeIndex) {
		return Util::AddOrGetTypeIndex(m_Indexes, typeIndex);
	}

	[[nodiscard]] inline auto Core::Initialize(const std::function<bool ()>& callback, const std::shared_ptr<Context>& ctx) -> bool 
	{
		if(m_Context) {
			HF_MSG_ERROR("Core context already exist");
			return false;
		}

		try 
		{
			if(!CreateOrSetContext(ctx) || !callback || !callback()) {
				return false;
			}

			if(!ctx) {
				Heartbeat();
			}

			HF_MSG_WARN("Finalize framework");
			m_Context->m_Shutdown = false;
			Util::Sleep(100);

		} catch(const std::exception& error) {
			HF_MSG_FATAL("Exception code: {}", error.what());
		} catch(...) {
			HF_MSG_FATAL("Unknown exception!");
		}

		return true;
	}

	inline auto Core::CreateOrSetContext(const std::shared_ptr<Context>& ctx) -> bool 
	{
		if(!ctx) {
			m_Context				= std::make_shared<Context>();
			m_Context->m_TimeStart	= std::chrono::steady_clock::now();	// Start time (used for calculate elapsed time)
			m_Context->m_TimeNow	= m_Context->m_TimeStart;
			m_Context->m_TimePrev	= m_Context->m_TimeNow; 
			m_Context->m_TickRate	= 1.0 / 30.0;

			HookSignals();
			HeartbeatTimeCalc();
		} else {
			m_Context = ctx;
		}

		return true;
	}

	inline auto Core::HookSignals() -> void 
	{
		#if HF_PLATFORM == HF_PLATFORM_WIN
			set_terminate(Terminate);
			SetConsoleCtrlHandler(CtrlHandler, TRUE);
		#elif HF_PLATFORM == HF_PLATFORM_LINUX
			signal(SIGTERM, Service::SigHandler);
			signal(SIGSTOP, Service::SigHandler);
			signal(SIGINT,  Service::SigHandler);
			signal(SIGKILL, Service::SigHandler);
		#else
			#error Unknown platform
		#endif
	}

	inline auto Core::Heartbeat() -> void 
	{		
		m_Context->m_Dispatcher.template trigger<Events::Initialize>();

		double timeElapsed {};
		double timeFPS {};
		std::uint32_t fps {};
		while(!m_Context->m_Shutdown) 
		{
			// Get time and delta
			timeElapsed	+= HeartbeatTimeCalc();
			//++fps;
			m_Context->m_SystemManager.Event(SystemManager::Events::Create);
			m_Context->m_SystemManager.Event(SystemManager::Events::Execute);
			m_Context->m_SystemManager.Event(SystemManager::Events::Tick);

			if(const auto time = GetTimeElapsed(); time > timeFPS) {
				timeFPS = time + 1.0;
			#if HF_PLATFORM == HF_PLATFORM_WIN
				const auto title = HF_FORMAT("Helena | FPS: {}", fps);
				SetConsoleTitle(title.c_str());
			#endif
				fps = 0;
			}

			if(timeElapsed >= m_Context->m_TickRate) {
				timeElapsed -= m_Context->m_TickRate;
				fps++;
				m_Context->m_SystemManager.Event(SystemManager::Events::Update);
			}
			
			if(timeElapsed < m_Context->m_TickRate) {
				//const auto sleepTime = static_cast<std::uint32_t>((m_Context->m_TickRate - m_Context->m_TimeElapsed) * 1000.0);
				Util::Sleep(std::chrono::milliseconds{1});
			}
		}

		m_Context->m_Dispatcher.template trigger<Events::Finalize>();
		m_Context->m_SystemManager.Event(SystemManager::Events::Destroy);
	}

	inline auto Core::HeartbeatTimeCalc() -> double {
		m_Context->m_TimePrev	= m_Context->m_TimeNow;
		m_Context->m_TimeNow	= std::chrono::steady_clock::now();
		m_Context->m_TimeDelta	= std::chrono::duration<double>{m_Context->m_TimeNow - m_Context->m_TimePrev}.count();

		return m_Context->m_TimeDelta;
	}

	/**
	* @brief Shutdown framework
	* @details Hello world
	*/
	inline auto Core::Shutdown() noexcept -> void {
		m_Context->m_Shutdown = true;
	}

	inline auto Core::SetArgs(const std::size_t argc, const char* const* argv) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_Args.clear();
		m_Context->m_Args.reserve(argc);

		for(std::size_t i = 0; i < argc; ++i) {
			m_Context->m_Args.emplace_back(argv[i]);
		}
	}

	inline auto Core::SetTickrate(const double tickrate) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_TickRate = 1.0 / tickrate;
	}

	[[nodiscard]] inline auto Core::GetArgs() noexcept -> decltype(auto)
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
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

	[[nodiscard]] inline auto Core::GetTickrate() noexcept -> double
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		return m_Context->m_TickRate;
	}

	[[nodiscard]] inline auto Core::GetTimeElapsed() noexcept -> double {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		return std::chrono::duration<double>{std::chrono::steady_clock::now() - m_Context->m_TimeStart}.count();
	}

	[[nodiscard]] inline auto Core::GetTimeDelta() noexcept -> double {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		return m_Context->m_TimeDelta;
	}

	template <typename Type, typename... Args>
	auto Core::RegisterSystem([[maybe_unused]] Args&&... args) -> Type* 
	{
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, 
			"Resource type cannot be const/ptr/ref");
		static_assert(std::is_constructible_v<Type, Args...>, 
			"Resource type cannot be constructable");

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		return m_Context->m_SystemManager.AddSystem<Internal::remove_cvref_t<Type>>(std::forward<Args>(args)...);
	}

	template <typename Type>
	[[nodiscard]] auto Core::GetSystem() -> Type* 
	{
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, 
			"Resource type cannot be const/ptr/ref");

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		return m_Context->m_SystemManager.GetSystem<Type>();
	}

	/**
	* @brief Remove system from the container of systems
	* @tparam Type Type of object to use to remove the system
	*/
	template <typename Type>
	auto Core::RemoveSystem() -> void
	{
		static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, 
			"Resource type cannot be const/ptr/ref");

		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_SystemManager.RemoveSystem<Type>();
	}

	template <typename Event, auto Method>
	auto Core::RegisterEvent() -> void 
	{
		// Check core initialiation
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_Dispatcher.sink<Event>().template connect<Method>();
	}


	template <typename Event, auto Method, typename Type>
	auto Core::RegisterEvent(Type&& instance) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}
		
		m_Context->m_Dispatcher.sink<Event>().template connect<Method>(instance);
	}

	template <typename Event, typename... Args>
	auto Core::TriggerEvent(Args&&... args) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_Dispatcher.template trigger<Event>(std::forward<Args>(args)...);
	}

	template <typename Event, typename... Args>
	auto Core::EnqueueEvent(Args&&... args) -> void
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_Dispatcher.template enqueue<Event>(std::forward<Args>(args)...);
	}

	template <typename Event>
	auto Core::UpdateEvent() -> void {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_Dispatcher.template update<Event>();
	}

	inline auto Core::UpdateEvent() -> void {
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_Dispatcher.update();
	}

	template <typename Event, auto Method>
	auto Core::RemoveEvent() -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_Dispatcher.sink<Event>().template disconnect<Method>();
	}

	template <typename Event, auto Method, typename Type>
	auto Core::RemoveEvent(Type&& instance) -> void 
	{
		if(!m_Context) {
			HF_MSG_ERROR("Core not initialized!");
			std::terminate();
		}

		m_Context->m_Dispatcher.sink<Event>().template disconnect<Method>(instance);
	}

	//[[nodiscard]] inline auto Core::GetSystemManager() noexcept -> SystemManager& {
	//	return m_Context->m_SystemManager;
	//}

	//[[nodiscard]] inline auto Core::GetTypeIndex(std::unordered_map<entt::id_type, std::size_t>& container, const entt::id_type typeIndex) -> std::size_t 
	//{
	//	if(!m_Context) {
	//		HF_MSG_ERROR("Core not initialized!");
	//		std::terminate();
	//	}

	//	if(const auto it = container.find(typeIndex); it != container.cend()) {
	//		return it->second;
	//	}

	//	if(const auto [it, result] = container.emplace(typeIndex, container.size()); !result) {
	//		HF_MSG_FATAL("Type index emplace failed!");
	//		std::terminate();
	//	}

	//	return container.size() - 1;
	//}
}

namespace entt {
	template <typename Type>
	struct ENTT_API type_seq<Type> {
		[[nodiscard]] static id_type value() ENTT_NOEXCEPT {
			static const auto value = static_cast<id_type>(
				Util::AddOrGetTypeIndex(Helena::Core::m_Context->m_TypeIndexes, 
				Hash::Type<Type>));
			return value;
		}
	};
}

#endif // COMMON_CORE_IPP