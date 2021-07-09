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

    }

    inline BOOL WINAPI Core::CtrlHandler(DWORD dwCtrlType)
    {
        if(m_Context)
        {
            std::unique_lock lock(m_Context->m_ShutdownMutex);
            if(m_Context->m_State != ECoreState::Shutdown) {
                Core::Shutdown();
            }
            m_Context->m_ShutdownCondition.wait(lock);
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
        if(m_Context && m_Context->m_State != ECoreState::Shutdown) {
            Core::Shutdown();
        }
    }
#endif

    inline void Core::HookSignals()
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

    template <typename Type>
    [[nodiscard]] inline auto Core::SystemIndex<Type>::GetIndex(map_indexes_t& container) -> std::size_t {
        static const auto value = Internal::AddOrGetTypeIndex(container, Hash::Type<Type>);
        return value;
    }

    template <typename Func>
    inline void Core::Initialize(Func&& callback, const std::shared_ptr<Context>& ctx) noexcept
    {
        HF_ASSERT(!m_Context, "Core is already initialized!");

        try
        {
            CreateContext(ctx);
            callback();

            if(!ctx)
            {
                if(m_Context->m_State == ECoreState::Init) {
                    Heartbeat();
                }

                if(!m_Context->m_ShutdownReason.empty()) {
                    HF_MSG_FATAL("Shutdown reason: {}", m_Context->m_ShutdownReason);
                }

                #if HF_PLATFORM == HF_PLATFORM_WIN
                    m_Context->m_ShutdownCondition.notify_all();
                #endif
            }
        } catch(const std::exception& error) {
            HF_MSG_FATAL("Exception code: {}", error.what());
        } catch(...) {
            HF_MSG_FATAL("Unknown exception!");
        }
    }

    inline void Core::CreateContext(const std::shared_ptr<Context>& ctx)
    {
        if(!ctx) {
            m_Context				= std::make_shared<Context>();
            m_Context->m_State      = ECoreState::Init;
            m_Context->m_TimeStart	= std::chrono::steady_clock::now();	// Start time (used for calculate elapsed time)
            m_Context->m_TimeNow	= m_Context->m_TimeStart;
            m_Context->m_TimePrev	= m_Context->m_TimeNow;
            m_Context->m_TickRate	= 1.0 / 30.0;

            HeartbeatTimeCalc();
            HookSignals();
        } else {
            m_Context = ctx;
        }
    }

    inline void Core::Heartbeat()
    {
        double timeElapsed {};
        double timeFPS {};
        std::uint32_t fps {};

        m_Context->m_Dispatcher.template trigger<Events::Initialize>();
        while(m_Context->m_State != ECoreState::Shutdown)
        {
            // Get time and delta
            timeElapsed	+= HeartbeatTimeCalc();

            EventSystems(SystemEvent::Create);
            EventSystems(SystemEvent::Execute);
            EventSystems(SystemEvent::Tick);

            if(const auto time = GetTimeElapsed(); time > timeFPS) {
                timeFPS = time + 1.0;
            #if HF_PLATFORM == HF_PLATFORM_WIN
                constexpr const std::size_t size = 16;
                char title[size]{"FPS: "};
                std::to_chars(title + 5, title + size, fps);
                SetConsoleTitle(title);
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

        EventSystems(SystemEvent::Destroy);
        m_Context->m_Dispatcher.template trigger<Events::Finalize>();

    }

    inline void Core::EventSystems(const SystemEvent type)
    {
        const auto eventid = static_cast<std::underlying_type_t<SystemEvent>>(type);
        auto& container = m_Context->m_EventScheduler[eventid];
        auto& events    = m_Context->m_SystemsEvents;

        for(auto size = container.size(); size; --size)
        {
            const auto index = container.front();
            const auto& event = events[eventid];

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

    [[nodiscard]] inline auto Core::GetContext() noexcept -> std::shared_ptr<Context> {
        HF_ASSERT(m_Context, "Core is not initialized");
        return m_Context;
    }

    [[nodiscard]] inline auto Core::GetCoreState() noexcept -> ECoreState {
        HF_ASSERT(m_Context, "Core is not initialized");
        return m_Context->m_State;
    }

    inline void Core::Shutdown() noexcept {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_State = ECoreState::Shutdown;
    }

    inline void Core::Shutdown(const std::string& msg) noexcept {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_ShutdownReason = msg;
        m_Context->m_State = ECoreState::Shutdown;
    }

    inline void Core::SetArgs(const std::size_t argc, const char* const* argv)
    {
        HF_ASSERT(m_Context, "Core is not initialized");

        m_Context->m_Args.clear();
        m_Context->m_Args.reserve(argc);

        for(std::size_t i = 0; i < argc; ++i) {
            m_Context->m_Args.emplace_back(argv[i]);
        }
    }

    inline void Core::SetTickrate(double tickrate) noexcept {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_TickRate = 1.0 / std::max(1.0, tickrate);
    }

    [[nodiscard]] inline auto Core::GetArgs() noexcept -> std::vector<std::string_view>& {
        HF_ASSERT(m_Context, "Core is not initialized");
        return m_Context->m_Args;
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
    inline void Core::RegisterSystem([[maybe_unused]] Args&&... args)
    {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Resource type cannot be const/ptr/ref");

        HF_ASSERT(m_Context, "Core is not initialized");

        auto& systems = m_Context->m_Systems;
        auto& events = m_Context->m_SystemsEvents;
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
                events[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Create)].template connect<&Type::OnSystemCreate>(entt::any_cast<Type&>(instance));
                scheduler[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Create)].emplace(index);
            }

            if constexpr(Internal::is_detected_v<fn_execute_t, Type>) {
                events[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Execute)].template connect<&Type::OnSystemExecute>(entt::any_cast<Type&>(instance));
                scheduler[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Execute)].emplace(index);
            }

            if constexpr(Internal::is_detected_v<fn_update_t, Type>) {
                events[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Update)].template connect<&Type::OnSystemUpdate>(entt::any_cast<Type&>(instance));
                scheduler[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Update)].emplace(index);
            }

            if constexpr(Internal::is_detected_v<fn_tick_t, Type>) {
                events[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Tick)].template connect<&Type::OnSystemTick>(entt::any_cast<Type&>(instance));
                scheduler[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Tick)].emplace(index);
            }

            if constexpr(Internal::is_detected_v<fn_destroy_t, Type>) {
                events[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Destroy)].template connect<&Type::OnSystemDestroy>(entt::any_cast<Type&>(instance));
                scheduler[static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Destroy)].emplace(index);
            }
        }
    }

    template <typename Type>
    [[nodiscard]] inline bool Core::HasSystem() noexcept {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Resource type cannot be const/ptr/ref");

        HF_ASSERT(m_Context, "Core is not initialized");

        const auto index = SystemIndex<Type>::GetIndex(m_Context->m_TypeIndexes);
        return index < m_Context->m_Systems.size() && m_Context->m_Systems[index];
    }

    template <typename Type>
    [[nodiscard]] inline auto Core::GetSystem() noexcept -> Type& {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Resource type cannot be const/ptr/ref");

        HF_ASSERT(m_Context, "Core is not initialized");

        auto& systems = m_Context->m_Systems;
        const auto index = SystemIndex<Type>::GetIndex(m_Context->m_TypeIndexes);

        HF_ASSERT(index < systems.size() && systems[index], "Instance of system {} does not exist", Internal::NameOf<Type>);
        return entt::any_cast<Type&>(systems[index]);
    }

    template <typename Type>
    inline void Core::RemoveSystem() noexcept
    {
        static_assert(std::is_same_v<Internal::remove_cvrefptr_t<Type>, Type>, "Resource type cannot be const/ptr/ref");

        HF_ASSERT(m_Context, "Core is not initialized");

        auto& systems = m_Context->m_Systems;
        auto& events = m_Context->m_SystemsEvents;
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
    inline void Core::RegisterEvent() {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.sink<Event>().template connect<Method>();
    }

    template <typename Event, auto Method, typename Type>
    inline auto Core::RegisterEvent(Type&& instance) -> void {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.sink<Event>().template connect<Method>(instance);
    }

    template <typename Event, typename... Args>
    inline void Core::TriggerEvent([[maybe_unused]] Args&&... args) {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.template trigger<Event>(std::forward<Args>(args)...);
    }

    template <typename Event, typename... Args>
    inline void Core::EnqueueEvent([[maybe_unused]] Args&&... args) {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.template enqueue<Event>(std::forward<Args>(args)...);
    }

    template <typename Event>
    inline void Core::UpdateEvent() {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.template update<Event>();
    }

    inline void Core::UpdateEvent() {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.update();
    }

    template <typename Event, auto Method>
    inline void Core::RemoveEvent() {
        HF_ASSERT(m_Context, "Core is not initialized");
        m_Context->m_Dispatcher.sink<Event>().template disconnect<Method>();
    }

    template <typename Event, auto Method, typename Type>
    inline void Core::RemoveEvent(Type&& instance) {
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
