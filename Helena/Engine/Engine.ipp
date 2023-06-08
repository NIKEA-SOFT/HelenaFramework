#ifndef HELENA_ENGINE_ENGINE_IPP
#define HELENA_ENGINE_ENGINE_IPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Engine/Events.hpp>
#include <Helena/Traits/Function.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Util/Format.hpp>
#include <Helena/Util/Sleep.hpp>

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
                                dateTime.GetHour(), dateTime.GetMinutes(), dateTime.GetSeconds());

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
    requires std::constructible_from<T, Args...>
    void Engine::Initialize([[maybe_unused]] Args&&... args)
    {
        HELENA_ASSERT_RUNTIME(!HasContext(), "Context already initialized!");
        InitContext({new (std::nothrow) T(std::forward<Args>(args)...), +[](const Context* ctx) {
            delete ctx;
        }});
        HELENA_ASSERT_RUNTIME(HasContext(), "Initialize Context failed!");
        MainContext().Main();
    }

    inline void Engine::Initialize(Context& ctx) noexcept {
    #if defined(HELENA_COMPILER_GCC)
        // WARNING: For plugins compiled on GCC, you must provide the flag: -fno-gnu-unique
        // Otherwise, false positives of the assert are possible.
        HELENA_ASSERT_RUNTIME(!HasContext(), "Context already initialized or compiler flag: -fno-gnu-unique not used!");
    #else
        HELENA_ASSERT_RUNTIME(!HasContext(), "Context already initialized!");
    #endif

        InitContext({std::addressof(ctx), +[](const Context*){}});
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

    [[nodiscard]] inline bool Engine::Heartbeat(std::size_t sleepMS, std::uint8_t accumulator)
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
            case EState::Undefined: [[unlikely]]
            {
                RegisterHandlers();

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
                ctx.m_TimeDelta = (ctx.m_TimeNow - ctx.m_TimePrev) / 1000.;
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

                std::uint32_t accumulated{};
                while(ctx.m_TimeElapsed >= ctx.m_TickRate && accumulated++ < accumulator) {
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

            #ifndef HELENA_ENGINE_NOSLEEP
                if(!accumulated) {
                    Util::Sleep(std::chrono::milliseconds{sleepMS});
                }
            #endif

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
        if(const auto state = ctx.m_State.exchange(EState::Shutdown, std::memory_order_acq_rel); state != EState::Shutdown) {
            ctx.m_ShutdownMessage->m_Location = msg.m_Location;
            ctx.m_ShutdownMessage->m_Message = Util::Format(msg.m_Msg, std::forward<Args>(args)...);
        }
    }

    [[nodiscard]] inline auto Engine::ShutdownReason() noexcept
    {;
        if(GetState() == EState::Shutdown)
        {
            if(const auto& [message, location] = *MainContext().m_ShutdownMessage; !message.empty()) {
                return Util::Format("[{}::{}::{}] {}", location.GetFile(), location.GetFunction(), location.GetLine(), message);
            }
        }

        return std::string{};
    }

    template <typename T, typename... Args>
    requires std::constructible_from<T, Args...>
    void Engine::RegisterSystem(Args&&... args) {
        if(GetState() == EState::Shutdown) [[unlikely]] return;
    #if defined(HELENA_THREADSAFE_SYSTEMS)
        const std::lock_guard lock{MainContext().m_LockSystems};
    #endif
        MainContext().m_Systems.template Create<T>(std::forward<Args>(args)...);
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
    void Engine::RemoveSystem() {
    #if defined(HELENA_THREADSAFE_SYSTEMS)
        const std::lock_guard lock{MainContext().m_LockSystems};
    #endif
        MainContext().m_Systems.template Remove<T...>();
    }

    template <typename Event, typename... Args>
    requires Engine::RequiresCallback<Event, Args...>
    void Engine::SubscribeEvent(void (*callback)(Args...))
    {
        using StorageArg = typename Traits::Function<CallbackStorage::Callback>::template Get<0>;
        using PayloadArg = typename Traits::Function<CallbackStorage::Callback>::template Get<1>;

        SubscribeEvent<Event>(callback, +[](StorageArg storage, [[maybe_unused]] PayloadArg data) {
            if constexpr(std::is_empty_v<Event>) {
                std::invoke(storage.As<decltype(callback)>());
            } else {
                std::invoke(storage.As<decltype(callback)>(), *static_cast<Traits::RemoveRef<
                    typename Traits::Arguments<Args...>::template Get<0>>*>(data));
            }
        });
    }

    template <typename Event, typename System, typename... Args>
    requires Engine::RequiresCallback<Event, Args...>
    void Engine::SubscribeEvent(void (System::*callback)(Args...))
    {
        using StorageArg = typename Traits::Function<CallbackStorage::Callback>::template Get<0>;
        using PayloadArg = typename Traits::Function<CallbackStorage::Callback>::template Get<1>;

        SubscribeEvent<Event>(callback, +[](StorageArg storage, [[maybe_unused]] PayloadArg data) {
            auto& ctx = MainContext();
            if(!ctx.m_Systems.template Has<System>()) [[unlikely]] {
                UnsubscribeEvent<Event>(storage.As<decltype(callback)>());
                return;
            }

            if constexpr(std::is_empty_v<Event>) {
                std::invoke(storage.As<decltype(callback)>(), ctx.m_Systems.template Get<System>());
            } else {
                std::invoke(storage.As<decltype(callback)>(), ctx.m_Systems.template Get<System>(),
                    *static_cast<Traits::RemoveRef<typename Traits::Arguments<Args...>::template Get<0>>*>(data));
            }
        });
    }

    template <typename Event, typename Callback, typename SignalFunctor>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::SubscribeEvent(Callback&& callback, SignalFunctor&& fn)
    {
        auto& ctx = MainContext();
        if(!ctx.m_Signals.template Has<Event>()) {
            ctx.m_Signals.template Create<Event>();
        }

        auto& pool = ctx.m_Signals.template Get<Event>();
#if defined(HELENA_DEBUG)
        [[maybe_unused]] const auto empty = pool.cend() == std::find_if(pool.cbegin(), pool.cend(),
            [callback = std::forward<Callback>(callback)](const auto& storage) {
                return storage == callback;
        });
        HELENA_ASSERT(empty, "Listener: {} already registered!", Traits::NameOf<Callback>{});
#endif
        pool.emplace_back(std::forward<Callback>(callback), std::forward<SignalFunctor>(fn));
    }

    template <typename... Event>
    requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
    [[nodiscard]]
    decltype(auto) Engine::SubscribersEvent()
    {
        const auto& ctx = MainContext();
        if constexpr(Traits::Arguments<Event...>::Single) {
            return ctx.m_Signals.template Has<Event...>() ? ctx.m_Signals.template Get<Event...>().size() : 0;
        } else {
            return std::make_tuple(SubscribersEvent<Event>()...);
        }
    }

    template <typename... Event>
    requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
    [[nodiscard]]
    decltype(auto) Engine::HasSubscribersEvent()
    {
        const auto& ctx = MainContext();
        if constexpr(Traits::Arguments<Event...>::Single) {
            return ctx.m_Signals.template Has<Event...>() && !ctx.m_Signals.template Get<Event...>().empty();
        } else {
            return std::make_tuple(HasSubscribersEvent<Event>()...);
        }
    }

    template <typename... Event>
    requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
    [[nodiscard]]
    decltype(auto) Engine::AnySubscribersEvent() {
        return (HasSubscribersEvent<Event>() || ...);
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
                const auto& [callback, storage] = pool[pos - 1];
                std::invoke(callback, storage, &event);
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

    template <typename Event, typename... Args>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::UnsubscribeEvent(void (*callback)(Args...)) {
        UnsubscribeEvent<Event>([callback](const auto& storage) noexcept {
            return storage == callback;
        });
    }

    template <typename Event, typename System, typename... Args>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::UnsubscribeEvent(void (System::* callback)(Args...)) {
        UnsubscribeEvent<Event>([callback](const auto& storage) noexcept {
            return storage == callback;
        });
    }

    template <typename Event, typename Comparator>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::UnsubscribeEvent(Comparator&& comparator) {
        if(const auto poolStorage = MainContext().m_Signals.template Ptr<Event>()) {
            if(const auto it = std::find_if(poolStorage->cbegin(), poolStorage->cend(), std::forward<Comparator>(comparator));
                it != poolStorage->cend()) {
                poolStorage->erase(it);
            };
        }
    }

}

#endif // HELENA_ENGINE_ENGINE_IPP