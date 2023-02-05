#ifndef HELENA_ENGINE_ENGINE_IPP
#define HELENA_ENGINE_ENGINE_IPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Traits/Function.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Util/Format.hpp>
#include <Helena/Util/Sleep.hpp>

namespace Helena
{
    inline void Engine::InitContext(ContextStorage context) noexcept {
        m_Context = std::move(context);
    }

    [[nodiscard]] inline bool Engine::HasContext() noexcept {
        return static_cast<bool>(m_Context);
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
        const auto dateTime = Types::DateTime::FromLocalTime();
        const auto dumpName = Util::Format("Crash_{:04d}{:02d}{:02d}_{:02d}_{:02d}_{:02d}.dmp",
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
        if(HWND hWnd = ::GetConsoleWindow(); hWnd) {
            HMENU hMenu = ::GetSystemMenu(hWnd, FALSE);
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
        static LARGE_INTEGER s_frequency;
        static BOOL s_use_qpc = ::QueryPerformanceFrequency(&s_frequency);
        LARGE_INTEGER now;
        std::uint64_t ms = s_use_qpc && ::QueryPerformanceCounter(&now)
            ? ((1000LL * now.QuadPart) / s_frequency.QuadPart)
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
    void Engine::Initialize(Args&&... args)
    {
        HELENA_ASSERT(!HasContext(), "Context already initialized!");
        InitContext(ContextStorage{new (std::nothrow) T, +[](Context* ctx) {
            delete ctx;
        }});
        HELENA_ASSERT(HasContext(), "Initialize Context failed!");

        if(!MainContext().Main())
        {
            constexpr const auto message = "Initialize Main of Context: {} failed!";
            if(GetState() != EState::Shutdown) {
                Shutdown(message, Traits::NameOf<T>{});
                return;
            }

            HELENA_MSG_FATAL(message, Traits::NameOf<T>{});
        }
    }

    inline void Engine::Initialize(Context& ctx) noexcept {
    #if defined(HELENA_COMPILER_GCC)
        // WARNING: For plugins compiled on GCC, you must provide the flag: -fno-gnu-unique
        // Otherwise, false positives of the assert are possible.
        HELENA_ASSERT(!HasContext(), "Context already initialized or compiler flag: -fno-gnu-unique not used!");
    #else
        HELENA_ASSERT(!HasContext(), "Context already initialized");
    #endif

        InitContext(ContextStorage{std::addressof(ctx), +[](Context*){}});
    }

    template <std::derived_from<Engine::Context> T>
    [[nodiscard]] T& Engine::GetContext() noexcept {
        return static_cast<T&>(MainContext());
    }

    [[nodiscard]] inline Engine::EState Engine::GetState() noexcept {
        return MainContext().m_State.load(std::memory_order_acquire);
    }

    inline void Engine::SetTickrate(double tickrate) noexcept {
        MainContext().m_Tickrate = 1. / (std::max)(tickrate, 1.);
    }

    [[nodiscard]] inline double Engine::GetTickrate() noexcept {
        return MainContext().m_Tickrate;
    }

    [[nodiscard]] inline std::uint64_t Engine::GetTimeElapsed() noexcept {
        return GetTickTime() - MainContext().m_TimeStart;
    }

    [[nodiscard]] inline bool Engine::Heartbeat(std::size_t sleepMS, std::uint8_t accumulator)
    {
        auto& ctx = MainContext();
        const auto state = GetState();
        const auto signal = []<typename... Args, typename... Events>(Signals<Events...>, Args&&... args) {
            (SignalEvent<Events>(args...), ...);
        };

    #if defined(HELENA_PLATFORM_WIN)
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

                signal(Signals<
                    Events::Engine::PreTick,
                    Events::Engine::Tick,
                    Events::Engine::PostTick
                >{}, ctx.m_TimeDelta);

                std::uint32_t accumulated{};
                while(ctx.m_TimeElapsed >= ctx.m_Tickrate && accumulated++ < accumulator) {
                    ctx.m_TimeElapsed -= ctx.m_Tickrate;
                    signal(Signals<
                        Events::Engine::PreUpdate,
                        Events::Engine::Update,
                        Events::Engine::PostUpdate
                    >{}, ctx.m_Tickrate);
                }

                signal(Signals<
                    Events::Engine::PreRender,
                    Events::Engine::Render,
                    Events::Engine::PostRender
                >{}, ctx.m_TimeElapsed / ctx.m_Tickrate, ctx.m_TimeDelta);

            #ifndef HELENA_ENGINE_NOSLEEP
                if(!accumulated) {
                    Util::Sleep(std::chrono::milliseconds{sleepMS});
                }
            #endif

            } break;

            case Engine::EState::Shutdown: [[unlikely]]
            {
                signal(Signals<
                    Events::Engine::PreFinalize,
                    Events::Engine::Finalize,
                    Events::Engine::PostFinalize,
                    Events::Engine::PreShutdown,
                    Events::Engine::Shutdown,
                    Events::Engine::PostShutdown
                >{});

                ctx.m_Events.Clear();
                ctx.m_Systems.Clear();
                if(!ctx.m_ShutdownMessage->m_Message.empty()) {
                    Log::Console(Log::Formater<Log::Shutdown>{
                        ctx.m_ShutdownMessage->m_Message,
                        ctx.m_ShutdownMessage->m_Location});
                }

                ctx.m_State.store(EState::Undefined, std::memory_order_release);
                return false;
            }
        }

    #if defined(HELENA_PLATFORM_WIN)
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
    void Engine::Shutdown(const Types::LocationString& msg, Args&&... args)
    {
        auto& ctx = MainContext();
        const auto state = ctx.m_State.exchange(EState::Shutdown, std::memory_order_acq_rel);
        if(state != EState::Shutdown) {
            ctx.m_ShutdownMessage->m_Location = msg.m_Location;
            ctx.m_ShutdownMessage->m_Message = Util::Format(msg.m_Msg, std::forward<Args>(args)...);
        }
    }

    [[nodiscard]] inline auto Engine::ShutdownReason() noexcept
    {;
        if(GetState() == EState::Shutdown) {
            auto& ctx = MainContext();
            const auto& msg = ctx.m_ShutdownMessage->m_Message;
            const auto& location = ctx.m_ShutdownMessage->m_Location;
            return Util::Format("[{}::{}::{}] {}", location.GetFile(), location.GetFunction(), location.GetLine(), msg);
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
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::SubscribeEvent(void (*callback)(Args...))
    {
        using StorageArg = typename Traits::Function<CallbackStorage::Callback>::template Get<0>;
        using PayloadArg = typename Traits::Function<CallbackStorage::Callback>::template Get<1>;

        SubscribeEvent<Event>(callback, +[](StorageArg storage, [[maybe_unused]] PayloadArg data) {
            if constexpr(std::is_empty_v<Event>) {
                static_assert(Traits::Arguments<Args...>::Orphan, "Args should be dropped for optimization");
                std::invoke(storage.As<decltype(callback)>());
            } else {
                static_assert(Traits::Arguments<Args...>::Single, "Args incorrect");
                static_assert((Traits::SameAs<Event, Traits::RemoveCVR<Args>> && ...), "Args type incorrect");
                std::invoke(storage.As<decltype(callback)>(), *static_cast<Traits::RemoveRef<
                    typename Traits::Arguments<Args...>::template Get<0>>*>(data));
            }
        });
    }

    template <typename Event, typename System, typename... Args>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
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
                static_assert(Traits::Arguments<Args...>::Orphan, "Args should be dropped for optimization");
                std::invoke(storage.As<decltype(callback)>(), ctx.m_Systems.template Get<System>());
            } else {
                static_assert(Traits::Arguments<Args...>::Single, "Args incorrect");
                static_assert((Traits::SameAs<Event, Traits::RemoveCVRP<Args>> && ...), "Args type incorrect");
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
        if(!ctx.m_Events.template Has<Event>()) {
            ctx.m_Events.template Create<Event>();
        }

        auto& eventPool = ctx.m_Events.template Get<Event>();
        const auto empty = eventPool.cend() == std::find_if(eventPool.cbegin(), eventPool.cend(),
            [callback = std::forward<Callback>(callback)](const auto& storage) {
                return storage == callback;
        });
        HELENA_ASSERT(empty, "Listener: {} already registered!", Traits::NameOf<Callback>{});

        if(!empty) [[unlikely]] return;
        eventPool.emplace_back(std::forward<Callback>(callback), std::forward<SignalFunctor>(fn));
    }

    template <typename Event, typename... Args>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::SignalEvent(Args&&... args)
    {
        if constexpr(std::is_empty_v<Event>) {
            union { Event event; };
            SignalEvent(event);
        } else if constexpr(requires {Event(std::forward<Args>(args)...);}) {
            auto event = Event(std::forward<Args>(args)...);
            SignalEvent(event);
        } else if constexpr(requires {Event{std::forward<Args>(args)...};}) {
            auto event = Event{std::forward<Args>(args)...};
            SignalEvent(event);
        } else {
            []<bool constructible = false>() {
                static_assert(constructible, "Event type not constructible from args");
            }();
        }
    }

    template <typename Event>
    requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
    void Engine::SignalEvent(Event& event)
    {
        auto& ctx = MainContext();
        if(auto poolStorage = ctx.m_Events.GetStorage<Event>())
        {
            auto& eventPool = *poolStorage;
            for(std::size_t pos = eventPool.size(); pos; --pos)
            {
                const auto& [callback, storage] = eventPool[pos - 1];
                if constexpr(std::is_empty_v<Event>) {
                    std::invoke(callback, storage, nullptr);
                } else {
                    std::invoke(callback, storage, static_cast<void*>(&event));
                }
            }

            if constexpr(Traits::AnyOf<Event,
                Events::Engine::PreInit,        Events::Engine::Init,       Events::Engine::PostInit,
                Events::Engine::PreConfig,      Events::Engine::Config,     Events::Engine::PostConfig,
                Events::Engine::PreExecute,     Events::Engine::Execute,    Events::Engine::PostExecute,
                Events::Engine::PreFinalize,    Events::Engine::Finalize,   Events::Engine::PostFinalize,
                Events::Engine::PreShutdown,    Events::Engine::Shutdown,   Events::Engine::PostShutdown>) {
                eventPool.clear();
            }
        }
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
        if(const auto poolStorage = MainContext().m_Events.GetStorage<Event>()) {
            if(const auto it = std::find_if(poolStorage->cbegin(), poolStorage->cend(), std::forward<Comparator>(comparator));
                it != poolStorage->cend()) {
                poolStorage->erase(it);
            };
        }
    }
}

#endif // HELENA_ENGINE_ENGINE_IPP