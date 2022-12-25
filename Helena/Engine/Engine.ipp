#ifndef HELENA_ENGINE_ENGINE_IPP
#define HELENA_ENGINE_ENGINE_IPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/SameAs.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Util/Format.hpp>
#include <Helena/Util/Sleep.hpp>

namespace Helena
{
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

        const HANDLE hFile = ::CreateFileA(dumpName.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if(!hFile || hFile == INVALID_HANDLE_VALUE) {
            Log::Console<Log::Exception>("Create file for dump failed, error: {}", GetLastError());
            return EXCEPTION_EXECUTE_HANDLER;
        }

        const HANDLE hProcess = ::GetCurrentProcess();
        const DWORD processId = ::GetProcessId(hProcess);
        const MINIDUMP_TYPE flag = MINIDUMP_TYPE::MiniDumpWithIndirectlyReferencedMemory;
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo {
            .ThreadId = ::GetCurrentThreadId(),
            .ExceptionPointers = pException,
            .ClientPointers = TRUE
        };

        const BOOL result = ::MiniDumpWriteDump(hProcess, processId, hFile, flag, &exceptionInfo, NULL, NULL);
        if(!result) {
            (void)::DeleteFileA(dumpName.c_str());
            HELENA_MSG_EXCEPTION("Create dump failed, error: {}", GetLastError());
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
        ::gettimeofday(&te, NULL);
        std::uint64_t ms = te.tv_sec * 1000LL + te.tv_usec / 1000;
#endif
        return ms;
    }

    [[nodiscard]] inline bool Engine::Heartbeat()
    {
        auto& ctx = Context::GetInstance();
        const auto state = GetState();

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

                if(Running()) SignalEvent<Events::Engine::Init>();
                if(Running()) SignalEvent<Events::Engine::Config>();
                if(Running()) SignalEvent<Events::Engine::Execute>();

            } break;

            case EState::Init: [[likely]]
            {
                ctx.m_TimePrev  = ctx.m_TimeNow;
                ctx.m_TimeNow   = GetTickTime();
                ctx.m_TimeDelta = (ctx.m_TimeNow - ctx.m_TimePrev) / 1000.f;
                ctx.m_TimeElapsed += ctx.m_TimeDelta;

                // Base signal called only once for new listeners
                SignalEvent<Events::Engine::Init>();
                SignalEvent<Events::Engine::Config>();
                SignalEvent<Events::Engine::Execute>();
                SignalEvent<Events::Engine::Tick>(ctx.m_TimeDelta);

                constexpr std::uint32_t accumulatorMax = 5;
                std::uint32_t accumulator{};
                while(ctx.m_TimeElapsed >= ctx.m_Tickrate && accumulator++ < accumulatorMax) {
                    ctx.m_TimeElapsed -= ctx.m_Tickrate;
                    SignalEvent<Events::Engine::Update>(ctx.m_Tickrate);
                }

                SignalEvent<Events::Engine::Render>(ctx.m_TimeElapsed / ctx.m_Tickrate, ctx.m_TimeDelta);

            #ifndef HELENA_ENGINE_NOSLEEP
                Util::Sleep(std::chrono::milliseconds{1});
            #endif

            } break;

            case Engine::EState::Shutdown: [[unlikely]]
            {
                SignalEvent<Events::Engine::Finalize>();
                SignalEvent<Events::Engine::Shutdown>();

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

    [[nodiscard]] inline Engine::EState Engine::GetState() noexcept {
        return Context::GetInstance().m_State.load(std::memory_order_acquire);
    }

    template <typename... Args>
    void Engine::Shutdown(const Types::LocationString& msg, [[maybe_unused]] Args&&... args)
    {
        auto& ctx = Context::GetInstance();
        const auto state = ctx.m_State.exchange(EState::Shutdown, std::memory_order_acq_rel);
        if(state != EState::Shutdown) {
            ctx.m_ShutdownMessage->m_Location = msg.m_Location;
            ctx.m_ShutdownMessage->m_Message = Util::Format(msg.m_Msg, std::forward<Args>(args)...);
        }
    }

    [[nodiscard]] inline auto Engine::ShutdownReason() noexcept
    {;
        if(GetState() == EState::Shutdown) {
            auto& ctx = Context::GetInstance();
            const auto& msg = ctx.m_ShutdownMessage->m_Message;
            const auto& location = ctx.m_ShutdownMessage->m_Location;
            return Util::Format("[{}::{}::{}] {}", location.GetFile(), location.GetFunction(), location.GetLine(), msg);
        }

        return std::string{};
    }

    template <typename T, typename... Args>
    void Engine::RegisterSystem([[maybe_unused]] Args&&... args) {
        if(GetState() == EState::Shutdown) [[unlikely]] return;
        Context::GetInstance().m_Systems.template Create<T>(std::forward<Args>(args)...);
    }

    template <typename... T>
    [[nodiscard]] bool Engine::HasSystem() {
        return Context::GetInstance().m_Systems.template Has<T...>();
    }

    template <typename... T>
    [[nodiscard]] bool Engine::AnySystem() {
        return Context::GetInstance().m_Systems.template Any<T...>();
    }

    template <typename... T>
    [[nodiscard]] decltype(auto) Engine::GetSystem() {
        return Context::GetInstance().m_Systems.template Get<T...>();
    }

    template <typename... T>
    void Engine::RemoveSystem() {
        Context::GetInstance().m_Systems.template Remove<T...>();
    }

    template <typename Event, typename... Args>
    void Engine::SubscribeEvent(void (*callback)([[maybe_unused]] Args...))
    {
        static_assert(Traits::SameAs<Event, Traits::RemoveCVRP<Event>>, "Event type incorrect");

        auto& ctx = Context::GetInstance();
        if(!ctx.m_Events.template Has<Event>()) {
            ctx.m_Events.template Create<Event>();
        }

        auto& eventPool = ctx.m_Events.template Get<Event>();
        const auto empty = eventPool.cend() == std::find_if(eventPool.begin(), eventPool.end(), [callback](const auto& storage) {
            return storage == callback;
        });
        HELENA_ASSERT(empty, "Listener already registered!");

        if(empty)
        {
            eventPool.emplace_back(callback, +[](CallbackStorage::Storage& storage, [[maybe_unused]] void* data) 
            {
                if constexpr(std::is_empty_v<Event>) {
                    static_assert(Traits::Arguments<Args...>::Orphan, "Args should be dropped for optimization");

                    storage.m_Callback();
                } else {
                    static_assert(Traits::Arguments<Args...>::Single, "Args incorrect");
                    static_assert((Traits::SameAs<Event, Traits::RemoveCVR<Args>> && ...), "Args type incorrect");

                    decltype(callback) fn{}; new (&fn) decltype(storage.m_Callback){storage.m_Callback};
                    fn(*static_cast<Traits::RemoveCVR<typename Traits::Arguments<Args...>::template Get<0>>*>(data));
                }
            });
        }
    }

    template <typename Event, typename System, typename... Args>
    void Engine::SubscribeEvent(void (System::*callback)([[maybe_unused]] Args...))
    {
        static_assert(Traits::SameAs<Event, Traits::RemoveCVRP<Event>>, "Event type incorrect");

        auto& ctx = Context::GetInstance();
        if(!ctx.m_Events.template Has<Event>()) {
            ctx.m_Events.template Create<Event>();
        }

        auto& eventPool = ctx.m_Events.template Get<Event>();
        const auto empty = eventPool.cend() == std::find_if(eventPool.begin(), eventPool.end(), [callback](const auto& storage) {
            return storage == callback;
        });
        HELENA_ASSERT(empty, "Listener already registered!");

        if(empty)
        {
            eventPool.emplace_back(callback, +[](CallbackStorage::Storage& storage, [[maybe_unused]] void* data) 
            {
                auto& ctx = Context::GetInstance();
                if(!ctx.m_Systems.template Has<System>()) [[unlikely]] {
                    decltype(callback) fn{}; new (&fn) decltype(storage.m_CallbackMember){storage.m_CallbackMember};
                    UnsubscribeEvent<Event>(fn);
                    return;
                }

                if constexpr(std::is_empty_v<Event>) {
                    static_assert(Traits::Arguments<Args...>::Orphan, "Args should be dropped for optimization");

                    decltype(callback) fn{}; new (&fn) decltype(storage.m_CallbackMember){storage.m_CallbackMember};
                    (ctx.m_Systems.template Get<System>().*fn)();
                } else {
                    static_assert(Traits::Arguments<Args...>::Single, "Args incorrect");
                    static_assert((Traits::SameAs<Event, Traits::RemoveCVRP<Args>> && ...), "Args type incorrect");

                    decltype(callback) fn{}; new (&fn) decltype(storage.m_CallbackMember){storage.m_CallbackMember};
                    (ctx.m_Systems.template Get<System>().*fn)(*static_cast<Traits::RemoveCVR<typename Traits::Arguments<Args...>::template Get<0>>*>(data));
                }
            });
        }
    }

    template <typename Event, typename... Args>
    void Engine::SignalEvent([[maybe_unused]] Args&&... args)
    {
        static_assert(Traits::SameAs<Event, Traits::RemoveCVRP<Event>>, "Event type incorrect");

        if constexpr(std::is_empty_v<Event>) {
            union {
                Event event;
            };
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
    void Engine::SignalEvent(Event& event)
    {
        auto& ctx = Context::GetInstance();
        if(ctx.m_Events.template Has<Event>())
        {
            auto& eventPool = ctx.m_Events.template Get<Event>();
            for(std::size_t pos = eventPool.size(); pos; --pos)
            {
                if constexpr(std::is_empty_v<Event>) {
                    eventPool[pos - 1].m_Callback(eventPool[pos - 1].m_Storage, nullptr);
                } else {
                    eventPool[pos - 1].m_Callback(eventPool[pos - 1].m_Storage, static_cast<void*>(&event));
                }
            }

            if constexpr(Traits::IsAnyOf<Event,
                Events::Engine::Init,
                Events::Engine::Config,
                Events::Engine::Execute,
                Events::Engine::Finalize,
                Events::Engine::Shutdown>::value) {
                eventPool.clear();
            }
        }
    }

    template <typename Event, typename... Args>
    void Engine::UnsubscribeEvent(void (*callback)([[maybe_unused]] Args...)) {
        static_assert(Traits::SameAs<Event, Traits::RemoveCVRP<Event>>, "Event type incorrect");

        auto& ctx = Context::GetInstance();
        if(!ctx.m_Events.template Has<Event>()) {
            return;
        }

        auto& eventPool = ctx.m_Events.template Get<Event>();
        const auto it = std::find_if(eventPool.cbegin(), eventPool.cend(), [callback](const auto& storage) noexcept {
            return storage == callback;
        });

        if(it != eventPool.cend()) {
            eventPool.erase(it);
        }
    }

    template <typename Event, typename System, typename... Args>
    void Engine::UnsubscribeEvent(void (System::* callback)([[maybe_unused]] Args...)) {
        static_assert(Traits::SameAs<Event, Traits::RemoveCVRP<Event>>, "Event type incorrect");

        auto& ctx = Context::GetInstance();
        if(!ctx.m_Events.template Has<Event>()) {
            return;
        }

        auto& eventPool = ctx.m_Events.template Get<Event>();
        const auto it = std::find_if(eventPool.cbegin(), eventPool.cend(), [callback](const auto& storage) noexcept {
            return storage == callback;
        });

        if(it != eventPool.cend()) {
            eventPool.erase(it);
        }
    }
}

#endif // HELENA_ENGINE_ENGINE_IPP