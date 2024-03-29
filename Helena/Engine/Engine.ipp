#ifndef HELENA_ENGINE_ENGINE_IPP
#define HELENA_ENGINE_ENGINE_IPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Engine/Events.hpp>
#include <Helena/Traits/Function.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Util/Format.hpp>

namespace Helena
{
    inline void Engine::InitContext(ContextStorage context) noexcept {
        m_Context = std::move(context);
    }

    [[nodiscard]] inline Engine::Context& Engine::MainContext() noexcept {
        HELENA_ASSERT(m_Context, "Context not initialized!");
        return *m_Context;
    }

#if defined(HELENA_PLATFORM_WIN)
    inline BOOL WINAPI Engine::CtrlHandler([[maybe_unused]] DWORD dwCtrlType) {
        if(GetState() == EState::Init) Shutdown();
        return TRUE;
    }

    inline LONG WINAPI Engine::MiniDumpSEH(EXCEPTION_POINTERS* pException)
    {
        const auto dateTime  = Types::DateTime::FromLocalTime();
        const auto& dumpName = Util::Format("Crash_{:04d}{:02d}{:02d}_{:02d}_{:02d}_{:02d}.dmp",
                                dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
                                dateTime.GetHours(), dateTime.GetMinutes(), dateTime.GetSeconds());

        const auto hFile = ::CreateFileA(dumpName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if(!hFile || hFile == INVALID_HANDLE_VALUE) {
            HELENA_MSG_EXCEPTION("Create file for dump failed, error: {}", ::GetLastError());
            return EXCEPTION_EXECUTE_HANDLER;
        }

        const auto hProcess     = ::GetCurrentProcess();
        const auto processId    = ::GetProcessId(hProcess);
        const auto flag         = MINIDUMP_TYPE::MiniDumpWithIndirectlyReferencedMemory;
        auto exceptionInfo      = MINIDUMP_EXCEPTION_INFORMATION {
            .ThreadId = ::GetCurrentThreadId(),
            .ExceptionPointers = pException,
            .ClientPointers = TRUE
        };

        if(!::MiniDumpWriteDump(hProcess, processId, hFile, flag, &exceptionInfo, nullptr, nullptr)) {
            (void)::DeleteFileA(dumpName.c_str());
            HELENA_MSG_EXCEPTION("Create dump failed, error: {}", ::GetLastError());
        } else {
            HELENA_MSG_EXCEPTION("SEH Handler Dump: \"{}\" created!", dumpName);
        }

        (void)::CloseHandle(hFile);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    inline void Engine::RegisterHandlers()
    {
        static ULONG stackSize = 64 * 1024;

        ::SetThreadStackGuarantee(&stackSize);
        ::SetConsoleCtrlHandler(CtrlHandler, TRUE);
        ::SetUnhandledExceptionFilter(MiniDumpSEH);

        // Disable X button
        if(const auto hWnd = ::GetConsoleWindow(); hWnd) {
            const auto hMenu = ::GetSystemMenu(hWnd, FALSE);
            ::EnableMenuItem(hMenu, SC_CLOSE, MF_DISABLED | MF_BYCOMMAND);
        }
    }

#elif defined(HELENA_PLATFORM_LINUX)
    inline void Engine::SigHandler([[maybe_unused]] int signal) {
        if(GetState() == EState::Init) Shutdown();
    }

    inline void Engine::RegisterHandlers() {
        signal(SIGTERM, SigHandler);
        signal(SIGSTOP, SigHandler);
        signal(SIGINT,  SigHandler);
        signal(SIGKILL, SigHandler);
        signal(SIGHUP,  SigHandler);
    }
#endif

    [[nodiscard]] inline std::uint64_t Engine::GetTickTime() noexcept
    {
#if defined(HELENA_PLATFORM_WIN)
        static LARGE_INTEGER frequency{};
        static const auto queryFrequency = ::QueryPerformanceFrequency(&frequency);
        LARGE_INTEGER now;
        std::uint64_t ms = queryFrequency && ::QueryPerformanceCounter(&now)
            ? ((1000LL * now.QuadPart) / frequency.QuadPart)
            : ::GetTickCount64();
#else
        struct timeval te;
        ::gettimeofday(&te, nullptr);
        std::uint64_t ms = te.tv_sec * 1000LL + te.tv_usec / 1000;
#endif
        return ms;
    }

    template <std::derived_from<Engine::Context> T, typename... Args>
    requires Traits::ConstructibleAggregateFrom<T, Args...>
    void Engine::Initialize([[maybe_unused]] Args&&... args)
    {
        HELENA_ASSERT_RUNTIME(!HasContext(), "Context already initialized!");
        InitContext({new (std::nothrow) T(std::forward<Args>(args)...), +[](const Context* ctx) {
            delete static_cast<const T*>(ctx);
        }});
        HELENA_ASSERT_RUNTIME(HasContext(), "Initialize Context failed!");
        RegisterHandlers();
        MainContext().Main();
    }

    inline void Engine::Initialize(Context& ctx) noexcept {
        if(m_Context) {
            InitContext({std::addressof(ctx), +[](const Context*){}});
        }
    }

    [[nodiscard]] inline bool Engine::HasContext() noexcept {
        return static_cast<bool>(m_Context);
    }

    template <std::derived_from<Engine::Context> T>
    [[nodiscard]] T& Engine::GetContext() noexcept {
        return static_cast<T&>(MainContext());
    }

    [[nodiscard]] inline Engine::EState Engine::GetState() noexcept {
        return MainContext().m_State.load(std::memory_order_acquire);
    }

    inline void Engine::SetTickrate(double tickrate) noexcept {
        MainContext().m_TickRate = 1. / (std::max)(tickrate, 1.);
    }

    [[nodiscard]] inline double Engine::GetTickrate() noexcept {
        return 1. / MainContext().m_TickRate;
    }

    [[nodiscard]] inline std::uint64_t Engine::GetTimeElapsed() noexcept {
        return GetTickTime() - MainContext().m_TimeStart;
    }

    template <typename HeartbeatConfig>
    requires Engine::RequiresConfig<HeartbeatConfig>
    [[nodiscard]] bool Engine::Heartbeat()
    {
        auto& ctx = MainContext();
        const auto state = GetState();
        const auto signal = []<typename... Args, typename... Events>(Signals<Events...>, [[maybe_unused]] Args&&... args) {
            (SignalEvent<Events>(args...), ...);
        };

    #if defined(HELENA_PLATFORM_WIN) && defined(HELENA_COMPILER_MSVC)
        __try {
    #endif
        switch(state)
        {
            case EState::Undefined: [[unlikely]] {
                ctx.m_TimeStart = GetTickTime();
                ctx.m_TimeNow   = ctx.m_TimeStart;
                ctx.m_TimePrev  = ctx.m_TimeStart;
                ctx.m_ShutdownMessage->m_Location = {};
                ctx.m_ShutdownMessage->m_Message.clear();
                ctx.m_State.store(EState::Init, std::memory_order_release);
            } break;

            case EState::Init: [[likely]]
            {
                ctx.m_TimePrev  = ctx.m_TimeNow;
                ctx.m_TimeNow   = GetTickTime();
                ctx.m_TimeDelta = static_cast<double>(ctx.m_TimeNow - ctx.m_TimePrev) / 1000.;
                ctx.m_TimeElapsed += ctx.m_TimeDelta;

                signal(Signals<
                    Events::Engine::PreInit,
                    Events::Engine::Init,
                    Events::Engine::PostInit,
                    Events::Engine::PreConfig,
                    Events::Engine::Config,
                    Events::Engine::PostConfig,
                    Events::Engine::PreExecute,
                    Events::Engine::Execute,
                    Events::Engine::PostExecute
                >{});

                for(const auto& message : ctx.m_DeferredSignals) {
                    message();
                } ctx.m_DeferredSignals.clear();

                signal(Signals<
                    Events::Engine::PreTick,
                    Events::Engine::Tick,
                    Events::Engine::PostTick
                >{}, ctx.m_TimeDelta);

                std::uint32_t accumulated{HeartbeatConfig::Accumulate};
                while(ctx.m_TimeElapsed >= ctx.m_TickRate && accumulated--) {
                    ctx.m_TimeElapsed -= ctx.m_TickRate;
                    signal(Signals<
                        Events::Engine::PreUpdate,
                        Events::Engine::Update,
                        Events::Engine::PostUpdate
                    >{}, ctx.m_TickRate);
                }

                signal(Signals<
                    Events::Engine::PreRender,
                    Events::Engine::Render,
                    Events::Engine::PostRender
                >{}, ctx.m_TimeElapsed / ctx.m_TickRate, ctx.m_TimeDelta);

                if(accumulated) {
                    HeartbeatConfig::Sleep();
                }

            } break;

            case EState::Shutdown: [[unlikely]]
            {
                signal(Signals<
                    Events::Engine::PreFinalize,
                    Events::Engine::Finalize,
                    Events::Engine::PostFinalize,
                    Events::Engine::PreShutdown,
                    Events::Engine::Shutdown,
                    Events::Engine::PostShutdown
                >{});

                ctx.m_Signals.Clear();
                ctx.m_DeferredSignals.clear();
                ctx.m_Systems.Clear();

                if(!ctx.m_ShutdownMessage->m_Message.empty()) {
                    Log::Message<Log::Shutdown>({ctx.m_ShutdownMessage->m_Message,
                        ctx.m_ShutdownMessage->m_Location});
                }

                ctx.m_State.store(EState::Undefined, std::memory_order_release);

                return false;
            }
        }

    #if defined(HELENA_PLATFORM_WIN) && defined(HELENA_COMPILER_MSVC)
        } __except (MiniDumpSEH(GetExceptionInformation())) {
            if(GetState() == EState::Shutdown) {
                return false;
            }

            Shutdown("Unhandled Exception");
        }
    #endif

        return true;
    }

    [[nodiscard]] inline bool Engine::Running() noexcept {
        return GetState() == EState::Init;
    }

    template <typename... Args>
    void Engine::Shutdown(const Types::LocationString& msg, [[maybe_unused]] Args&&... args)
    {
        auto& ctx = MainContext();
        const auto state = ctx.m_State.exchange(EState::Shutdown, std::memory_order_acq_rel);
        if(state != EState::Shutdown && !msg.m_Msg.empty()) {
            ctx.m_ShutdownMessage->m_Location = msg.m_Location;
            ctx.m_ShutdownMessage->m_Message = Util::Format(msg.m_Msg, std::forward<Args>(args)...);
        }
    }

    [[nodiscard]] inline auto Engine::ShutdownReason() noexcept
    {;
        if(GetState() == EState::Shutdown)
        {
            const auto& [message, location] = *MainContext().m_ShutdownMessage;
            if(!message.empty()) {
                return Util::Format("[{}::{}::{}] {}", location.GetFile(), location.GetFunction(), location.GetLine(), message);
            }
        }

        return std::string{};
    }

    template <typename T, typename... Args>
    requires Traits::ConstructibleAggregateFrom<T, Args...>
    void Engine::RegisterSystem(decltype(NoSignal), Args&&... args) {
        if(GetState() == EState::Shutdown) [[unlikely]] return;
    #if defined(HELENA_THREADSAFE_SYSTEMS)
        const std::lock_guard lock{MainContext().m_LockSystems};
    #endif
        MainContext().m_Systems.template Create<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    requires Traits::ConstructibleAggregateFrom<T, Args...>
    void Engine::RegisterSystem(Args&&... args) {
        if(GetState() == EState::Shutdown) [[unlikely]] return;
        SignalEvent<Events::Engine::PreRegisterSystem<T>>();
        RegisterSystem<T>(NoSignal, std::forward<Args>(args)...);
        SignalEvent<Events::Engine::PostRegisterSystem<T>>();
    }

    template <typename... T>
    [[nodiscard]] bool Engine::HasSystem() {
    #if defined(HELENA_THREADSAFE_SYSTEMS)
        const std::lock_guard lock{MainContext().m_LockSystems};
    #endif
        return MainContext().m_Systems.template Has<T...>();
    }

    template <typename... T>
    [[nodiscard]] bool Engine::AnySystem() {
    #if defined(HELENA_THREADSAFE_SYSTEMS)
        const std::lock_guard lock{MainContext().m_LockSystems};
    #endif
        return MainContext().m_Systems.template Any<T...>();
    }

    template <typename... T>
    [[nodiscard]] decltype(auto) Engine::GetSystem() {
    #if defined(HELENA_THREADSAFE_SYSTEMS)
        const std::lock_guard lock{MainContext().m_LockSystems};
    #endif
        return MainContext().m_Systems.template Get<T...>();
    }

    template <typename... T>
    void Engine::RemoveSystem(decltype(NoSignal)) {
    #if defined(HELENA_THREADSAFE_SYSTEMS)
        const std::lock_guard lock{MainContext().m_LockSystems};
    #endif
        MainContext().m_Systems.template Remove<T...>();
    }

    template <typename... T>
    void Engine::RemoveSystem()
    {
        if constexpr(Traits::Arguments<T...>::Single)
        {
            if(!HasSystem<T...>())
                return;

            SignalEvent<Events::Engine::PreRemoveSystem<T...>>();
            RemoveSystem<T...>(NoSignal);
            SignalEvent<Events::Engine::PostRemoveSystem<T...>>();

        } else (RemoveSystem<T>(), ...);
    }

    template <typename T, typename... Args>
    requires Traits::ConstructibleAggregateFrom<T, Args...>
    void Engine::RegisterComponent(decltype(NoSignal), Args&&... args) {
    #if defined(HELENA_THREADSAFE_COMPONENTS)
        const std::lock_guard lock{MainContext().m_LockComponents};
    #endif
        MainContext().m_Components.template Create<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    requires Traits::ConstructibleAggregateFrom<T, Args...>
    void Engine::RegisterComponent(Args&&... args) {
        SignalEvent<Events::Engine::PreRegisterComponent<T>>();
        RegisterComponent<T>(NoSignal, std::forward<Args>(args)...);
        SignalEvent<Events::Engine::PostRegisterComponent<T>>();
    }

    template <typename... T>
    [[nodiscard]] bool Engine::HasComponent() {
    #if defined(HELENA_THREADSAFE_COMPONENTS)
        const std::lock_guard lock{MainContext().m_LockComponents};
    #endif
        return MainContext().m_Components.template Has<T...>();
    }

    template <typename... T>
    [[nodiscard]] bool Engine::AnyComponent() {
    #if defined(HELENA_THREADSAFE_COMPONENTS)
        const std::lock_guard lock{MainContext().m_LockComponents};
    #endif
        return MainContext().m_Components.template Any<T...>();
    }

    template <typename... T>
    [[nodiscard]] decltype(auto) Engine::GetComponent() {
    #if defined(HELENA_THREADSAFE_COMPONENTS)
        const std::lock_guard lock{MainContext().m_LockComponents};
    #endif
        return MainContext().m_Components.template Get<T...>();
    }

    template <typename... T>
    void Engine::RemoveComponent(decltype(NoSignal)) {
    #if defined(HELENA_THREADSAFE_COMPONENTS)
        const std::lock_guard lock{MainContext().m_LockComponents};
    #endif
        MainContext().m_Components.template Remove<T...>();
    }

    template <typename... T>
    void Engine::RemoveComponent()
    {
        if constexpr(Traits::Arguments<T...>::Single)
        {
            if(!HasComponent<T...>())
                return;

            SignalEvent<Events::Engine::PreRemoveComponent<T...>>();
            RemoveComponent<T...>(NoSignal);
            SignalEvent<Events::Engine::PostRemoveComponent<T...>>();

        } else (RemoveComponent<T>(), ...);
    }

    template <typename Event, auto Callback>
    requires Engine::RequiresCallback<Event, Callback, /* Member function */ false>
    void Engine::SubscribeEvent() {
        return SubscribeEvent(typename Delegate::Args<Event, Callback>{}, nullptr);
    }

    template <typename Event, auto Callback>
    requires Engine::RequiresCallback<Event, Callback, /* Member function */ true>
    void Engine::SubscribeEvent(typename Traits::Function<decltype(Callback)>::Class* instance) {
        return SubscribeEvent(typename Delegate::Args<Event, Callback>{}, instance);
    }

    template <typename Event, auto Callback>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::SubscribeEvent(Delegate::Args<Event, Callback>, void* instance)
    {
        auto& ctx = MainContext();
        if(!ctx.m_Signals.template Has<Event>()) {
            ctx.m_Signals.template Create<Event>();
        }

        auto& pool = ctx.m_Signals.template Get<Event>();
    #if defined(HELENA_DEBUG)
        [[maybe_unused]]
        const auto empty = pool.cend() == std::find_if(pool.cbegin(), pool.cend(), [instance](const auto& delegate) {
            return delegate.template Compare<Event, Callback>(instance);
        });
        HELENA_ASSERT(empty, "Listener: {} already registered!", Traits::NameOf<decltype(Callback)>);
    #endif
        pool.emplace_back(typename Delegate::Args<Event, Callback>{}, instance);
    }

    template <typename... Event>
    requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
    [[nodiscard]] auto Engine::Subscribers()
    {
        const auto& ctx = MainContext();
        if constexpr(Traits::Arguments<Event...>::Single) {
            return ctx.m_Signals.template Has<Event...>() ? ctx.m_Signals.template Get<Event...>().size() : 0;
        } else {
            return std::make_tuple(Subscribers<Event>()...);
        }
    }

    template <typename... Event>
    requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
    [[nodiscard]] auto Engine::HasSubscribers()
    {
        const auto& ctx = MainContext();
        if constexpr(Traits::Arguments<Event...>::Single) {
            return ctx.m_Signals.template Has<Event...>() && !ctx.m_Signals.template Get<Event...>().empty();
        } else {
            return std::make_tuple(HasSubscribers<Event>()...);
        }
    }

    template <typename... Event>
    requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
    [[nodiscard]] auto Engine::AnySubscribers() {
        return (HasSubscribers<Event>() || ...);
    }

    template <typename Event, typename... Args>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::SignalEvent([[maybe_unused]] Args&&... args)
    {
        if constexpr(std::is_empty_v<Event>) {
            union { Event event; };
            SignalEvent(event);
        } else if constexpr(requires {Event(std::forward<Args>(args)...);}) {
            Event event(std::forward<Args>(args)...);
            SignalEvent(event);
        } else if constexpr(requires {Event{std::forward<Args>(args)...};}) {
            Event event{std::forward<Args>(args)...};
            SignalEvent(event);
        } else {
            []<bool Constructible = false>() {
                static_assert(Constructible, "Event type not constructible from args");
            }();
        }
    }

    template <typename Event>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::SignalEvent(Event& event)
    {
        auto& ctx = MainContext();
        if(auto poolStorage = ctx.m_Signals.template Ptr<Event>())
        {
            auto& pool = *poolStorage;
            for(std::size_t pos = pool.size(); pos; --pos) {
                const auto& delegate = pool[pos - 1];
                std::invoke(delegate, &event);
            }

            if constexpr(Traits::AnyOf<Event,
                Events::Engine::PreInit,        Events::Engine::Init,       Events::Engine::PostInit,
                Events::Engine::PreConfig,      Events::Engine::Config,     Events::Engine::PostConfig,
                Events::Engine::PreExecute,     Events::Engine::Execute,    Events::Engine::PostExecute,
                Events::Engine::PreFinalize,    Events::Engine::Finalize,   Events::Engine::PostFinalize,
                Events::Engine::PreShutdown,    Events::Engine::Shutdown,   Events::Engine::PostShutdown>) {
                pool.clear();
            }
        }
    }

    template <typename Event, typename... Args>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::EnqueueSignal(Args&&... args) {
        MainContext().m_DeferredSignals.emplace_back([... args = std::forward<Args>(args)]() mutable {
            SignalEvent<Event>(std::forward<Args>(args)...);
        });
    }

    template <typename Event, auto Callback>
    requires Engine::RequiresCallback<Event, Callback, /* Member function */ false>
    void Engine::UnsubscribeEvent() {
        return UnsubscribeEvent(typename Delegate::Args<Event, Callback>{}, nullptr);
    }

    template <typename Event, auto Callback>
    requires Engine::RequiresCallback<Event, Callback, /* Member function */ true>
    void Engine::UnsubscribeEvent(typename Traits::Function<decltype(Callback)>::Class* instance) {
        return UnsubscribeEvent(typename Delegate::Args<Event, Callback>{}, instance);
    }

    template <typename Event, auto Callback>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::UnsubscribeEvent(Delegate::Args<Event, Callback>, void* instance)
    {
        if(const auto pool = MainContext().m_Signals.template Ptr<Event>())
        {
            const auto it = std::find_if(pool->cbegin(), pool->cend(), [instance](const auto& delegate) {
                return delegate.template Compare<Event, Callback>(instance);
            });

            if(it != pool->cend()) {
                pool->erase(it);
            };
        }
    }
}

#endif // HELENA_ENGINE_ENGINE_IPP