#ifndef HELENA_CORE_IPP
#define HELENA_CORE_IPP

#include <Helena/Core.hpp>
#include <Helena/Assert.hpp>
#include <Helena/Hash.hpp>
#include <Helena/Util.hpp>
#include <Helena/Internal.hpp>

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

        while(m_Context) {
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
    inline auto Core::SigHandler([[maybe_unused]] int signal) -> void
    {
        HF_MSG_WARN("Signal: {} received", signal);
        if(m_Context && !m_Context->m_Shutdown) {
            Core::Shutdown();
        }
    }
#endif

    template <typename Type>
    [[nodiscard]] auto Core::SystemIndex<Type>::GetIndex(map_indexes_t& container) -> std::size_t {
        static const auto value = Internal::AddOrGetTypeIndex(container, Hash::Type<Type>);
        return value;
    }

    [[nodiscard]] inline auto Core::CreateOrSetContext(const std::shared_ptr<Context>& ctx) -> bool
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
            signal(SIGTERM, SigHandler);
            signal(SIGSTOP, SigHandler);
            signal(SIGINT,  SigHandler);
            signal(SIGKILL, SigHandler);
            signal(SIGHUP,  SigHandler);
        #else
            #error Unknown platform
		#endif
	}

    inline auto Core::HeartbeatTimeCalc() -> double
    {
        m_Context->m_TimePrev	= m_Context->m_TimeNow;
        m_Context->m_TimeNow	= std::chrono::steady_clock::now();
        m_Context->m_TimeDelta	= std::chrono::duration<double>{m_Context->m_TimeNow - m_Context->m_TimePrev}.count();

        return m_Context->m_TimeDelta;
    }

    template <typename Func>
    [[nodiscard]] auto Core::Initialize(Func&& callback, const std::shared_ptr<Context>& ctx) -> bool
    {
        static_assert(std::is_invocable_v<Func>, "Callback is not a callable type");
        HF_ASSERT(!m_Context, "Core is already initialized!");

        try
        {
            if(!CreateOrSetContext(ctx) || !callback()) {
                return false;
            }

            if(!ctx) {
                Heartbeat();
            }

            m_Context->m_Shutdown = false;
            m_Context.reset();

            HF_MSG_DEBUG("Finalize framework");
        } catch(const std::exception& error) {
            HF_MSG_FATAL("Exception code: {}", error.what());
        } catch(...) {
            HF_MSG_FATAL("Unknown exception!");
        }

        return true;
    }

    inline auto Core::Heartbeat() -> void
    {
        double timeElapsed {};
        double timeFPS {};
        std::uint32_t fps {};

        EventSystems(SystemEvent::Create);
        m_Context->m_Dispatcher.template trigger<Events::Initialize>();

        while(!m_Context->m_Shutdown)
        {
            // Get time and delta
            timeElapsed	+= HeartbeatTimeCalc();

            EventSystems(SystemEvent::Create);
            EventSystems(SystemEvent::Execute);
            EventSystems(SystemEvent::Tick);

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
                EventSystems(SystemEvent::Update);
            }

            if(timeElapsed < m_Context->m_TickRate) {
                //const auto sleepTime = static_cast<std::uint32_t>((m_Context->m_TickRate - m_Context->m_TimeElapsed) * 1000.0);
                Util::Sleep(std::chrono::milliseconds{1});
            }
		}

        m_Context->m_Dispatcher.template trigger<Events::Finalize>();
        EventSystems(SystemEvent::Destroy);
    }

    /**
     * @brief Core::EventSystems
     * @param type Type of event for call
     */
    inline auto Core::EventSystems(const SystemEvent type) -> void
    {
        auto& container = m_Context->m_EventScheduler[type];
        for(std::size_t size = container.size(); size; --size)
        {
            const auto index = container.front();
            const auto& event = m_Context->m_SystemsEvent[type];

            if(event) {
                event();
            }

            container.pop();

            // For tick and update methods we emplace it again (it's scheduler for event)
            switch(type)
            {
                case SystemEvent::Tick: [[fallthrough]];
                case SystemEvent::Update: {
                    container.emplace(index);
                } break;
                default: break;
            }
        }
    }

    inline auto Core::Shutdown() noexcept -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Shutdown = true;
    }

    inline auto Core::SetArgs(const std::size_t argc, const char* const* argv) -> void
    {
        HF_ASSERT(m_Context, "Core is not initialized");

        m_Context->m_Args.clear();
        m_Context->m_Args.reserve(argc);

        for(std::size_t i = 0; i < argc; ++i) {
            m_Context->m_Args.emplace_back(argv[i]);
        }
    }

    inline auto Core::SetTickrate(double tickrate) -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_TickRate = 1.0 / tickrate;
    }

    [[nodiscard]] inline auto Core::GetArgs() noexcept -> decltype(auto) {
        HF_ASSERT(m_Context, "Core is not initialized");
        return m_Context->m_Args;
    }

    [[nodiscard]] inline auto Core::GetContext() noexcept -> const std::shared_ptr<Context>& {
        HF_ASSERT(m_Context, "Core is not initialized");
        return m_Context;
    }

    [[nodiscard]] inline auto Core::GetTickrate() noexcept -> double {
        HF_ASSERT(m_Context, "Core is not initialized");
        return m_Context->m_TickRate;
    }

    [[nodiscard]] inline auto Core::GetTimeElapsed() noexcept -> double {
        HF_ASSERT(m_Context, "Core is not initialized");
        return std::chrono::duration<double>{std::chrono::steady_clock::now() - m_Context->m_TimeStart}.count();
    }

    [[nodiscard]] inline auto Core::GetTimeDelta() noexcept -> double {
        HF_ASSERT(m_Context, "Core is not initialized");
        return m_Context->m_TimeDelta;
    }

    template <typename Type, typename... Args>
    auto Core::RegisterSystem([[maybe_unused]] Args&&... args) -> void {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Resource type cannot be const/ptr/ref");

        HF_ASSERT(m_Context, "Core is not initialized");

        auto& systems = m_Context->m_Systems;
        auto& events = m_Context->m_SystemsEvent;
        auto& scheduler = m_Context->m_EventScheduler;
        const auto index = SystemIndex<Type>::GetIndex(m_Context->m_TypeIndexes);

        if(index >= systems.size()) {
            systems.resize(index + 1);
        }

        HF_ASSERT(!systems[index], "Instance of system {} is already registered", Internal::NameOf<Type>);

        if(auto& instance = systems[index]; !instance)
        {
            instance.template emplace<Type>(std::forward<Args>(args)...);
            if constexpr(Internal::is_detected_v<fn_create_t, Type>) {
                events[SystemEvent::Create].template connect<&Type::OnSystemCreate>(entt::any_cast<Type&>(instance));
                scheduler[SystemEvent::Create].emplace(index);
            }

            if constexpr(Internal::is_detected_v<fn_execute_t, Type>) {
                events[SystemEvent::Execute].template connect<&Type::OnSystemExecute>(entt::any_cast<Type&>(instance));
                scheduler[SystemEvent::Execute].emplace(index);
            }

            if constexpr(Internal::is_detected_v<fn_update_t, Type>) {
                events[SystemEvent::Update].template connect<&Type::OnSystemUpdate>(entt::any_cast<Type&>(instance));
                scheduler[SystemEvent::Update].emplace(index);
            }

            if constexpr(Internal::is_detected_v<fn_tick_t, Type>) {
                events[SystemEvent::Tick].template connect<&Type::OnSystemTick>(entt::any_cast<Type&>(instance));
                scheduler[SystemEvent::Tick].emplace(index);
            }

            if constexpr(Internal::is_detected_v<fn_destroy_t, Type>) {
                events[SystemEvent::Destroy].template connect<&Type::OnSystemDestroy>(entt::any_cast<Type&>(instance));
                scheduler[SystemEvent::Destroy].emplace(index);
            }
        }
    }

    template <typename Type>
    [[nodiscard]] auto Core::HasSystem() noexcept -> bool {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Resource type cannot be const/ptr/ref");

        HF_ASSERT(m_Context, "Core is not initialized");

        const auto index = SystemIndex<Type>::GetIndex(m_Context->m_TypeIndexes);
        return index < m_Context->m_Systems.size() && m_Context->m_Systems[index];
    }

    template <typename Type>
    [[nodiscard]] auto Core::GetSystem() noexcept -> Type& {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Resource type cannot be const/ptr/ref");

        HF_ASSERT(m_Context, "Core is not initialized");

        auto& systems = m_Context->m_Systems;
        const auto index = SystemIndex<Type>::GetIndex(m_Context->m_TypeIndexes);

        HF_ASSERT(index < systems.size() && systems[index], "Instance of system {} does not exist", Internal::NameOf<Type>);
        return entt::any_cast<Type&>(systems[index]);
    }

    template <typename Type>
    auto Core::RemoveSystem() noexcept -> void
    {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Resource type cannot be const/ptr/ref");

        HF_ASSERT(m_Context, "Core is not initialized");

        auto& systems = m_Context->m_Systems;
        auto& events = m_Context->m_SystemsEvent;
        const auto index = SystemIndex<Type>::GetIndex(m_Context->m_TypeIndexes);

        HF_ASSERT(index < systems.size() && systems[index], "Instance of system {} does not exist for remove", Internal::NameOf<Type>);
        if(index < systems.size())
        {
            if(auto& instance = systems[index]; instance)
            {
                if(events[SystemEvent::Destroy]) {
                    events[SystemEvent::Destroy]();
                }

                instance.reset();
                events[SystemEvent::Create].reset();
                events[SystemEvent::Execute].reset();
                events[SystemEvent::Tick].reset();
                events[SystemEvent::Update].reset();
                events[SystemEvent::Destroy].reset();
            }
        }
    }

    template <typename Event, auto Method>
    auto Core::RegisterEvent() -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.sink<Event>().template connect<Method>();
    }

    template <typename Event, auto Method, typename Type>
    auto Core::RegisterEvent(Type&& instance) -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.sink<Event>().template connect<Method>(instance);
    }

    template <typename Event, typename... Args>
    auto Core::TriggerEvent([[maybe_unused]] Args&&... args) -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.template trigger<Event>(std::forward<Args>(args)...);
    }

    template <typename Event, typename... Args>
    auto Core::EnqueueEvent([[maybe_unused]] Args&&... args) -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.template enqueue<Event>(std::forward<Args>(args)...);
    }

    template <typename Event>
    auto Core::UpdateEvent() -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.template update<Event>();
    }

    inline auto Core::UpdateEvent() -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.update();
    }

    template <typename Event, auto Method>
    auto Core::RemoveEvent() -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.sink<Event>().template disconnect<Method>();
    }

    template <typename Event, auto Method, typename Type>
    auto Core::RemoveEvent(Type&& instance) -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.sink<Event>().template disconnect<Method>(instance);
    }
}

namespace entt {
    template <typename Type>
    struct ENTT_API type_seq<Type> {
        [[nodiscard]] static id_type value() ENTT_NOEXCEPT {
            static const auto value = static_cast<id_type>(Helena::Internal::AddOrGetTypeIndex(
                Helena::Core::m_Context->m_SequenceIndexes, Helena::Hash::Type<Type>));
            return value;
        }
    };
}

#endif // HELENA_CORE_IPP
