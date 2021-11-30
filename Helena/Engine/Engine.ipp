#ifndef HELENA_ENGINE_ENGINE_IPP
#define HELENA_ENGINE_ENGINE_IPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/Format.hpp>
#include <Helena/Util/Format.hpp>
#include <Helena/Util/Sleep.hpp>

#include <algorithm>

namespace Helena
{
#if defined(HELENA_PLATFORM_WIN)
    inline BOOL WINAPI Engine::CtrlHandler(DWORD)
    {
        const auto ctx = Context::Get();
        if(ctx)
        {
            if(ctx->m_State == EState::Init) {
                Engine::Shutdown("Ctrl handler");
            }
        }

        return TRUE;
    }

    inline LONG WINAPI Engine::MiniDumpSEH(EXCEPTION_POINTERS* pException)
    {
        const auto context  = Context::Get();
        const auto dateTime = Types::DateTime::FromLocalTime();
        const auto dumpName = Util::Format("{}_{:04d}{:02d}{:02d}_{:02d}_{:02d}_{:02d}.dmp", 
            context && !context->GetAppName().empty() ? context->GetAppName() : "Helena",
            dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
            dateTime.GetHour(), dateTime.GetMinutes(), dateTime.GetSeconds());

        HANDLE hFile = CreateFileA(dumpName.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if(!hFile || hFile == INVALID_HANDLE_VALUE) {
            HELENA_MSG_EXCEPTION("Create file for dump failed, error: {}", GetLastError());
            return EXCEPTION_EXECUTE_HANDLER;
        }

        HANDLE hProcess = GetCurrentProcess();
        const DWORD processId = GetProcessId(hProcess);
        const MINIDUMP_TYPE flag = MINIDUMP_TYPE::MiniDumpWithIndirectlyReferencedMemory;
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo {
            .ThreadId = GetCurrentThreadId(),
            .ExceptionPointers = pException,
            .ClientPointers = TRUE
        };

        const BOOL result = MiniDumpWriteDump(hProcess, processId, hFile, flag, &exceptionInfo, NULL, NULL);
        if(!result) {
            HELENA_MSG_EXCEPTION("Create dump failed, error: {}", GetLastError());
            DeleteFileA(dumpName.c_str());
        } else {
            HELENA_MSG_EXCEPTION("SEH Handler Dump: \"{}\" created!", dumpName.c_str());
        }

        CloseHandle(hFile);
        return EXCEPTION_EXECUTE_HANDLER;
    }



    inline void Engine::RegisterHandlers() {
        static ULONG stackSize = 64 * 1024;

        SetThreadStackGuarantee(&stackSize);
        SetConsoleCtrlHandler(CtrlHandler, TRUE);
        SetUnhandledExceptionFilter(MiniDumpSEH);
    }

    template <typename... Args>
    void Engine::ConsoleInfo(std::string_view msg, Args&&... args) {
        const auto buffer = Types::Format<MAX_PATH>(msg, std::forward<Args>(args)...);
        SetConsoleTitleA(buffer.GetData());
    }

#elif defined(HELENA_PLATFORM_LINUX)
    inline void SigHandler(int signal)
    {
        const auto ctx = Context::Get();
        if(ctx)
        {
            if(ctx->m_State == EState::Init) {
                Engine::Shutdown("sig handler");
            }
        }
    }

    inline void Engine::RegisterHandlers() noexcept {
        signal(SIGTERM, SigHandler);
        signal(SIGSTOP, SigHandler);
        signal(SIGINT,  SigHandler);
        signal(SIGKILL, SigHandler);
        signal(SIGHUP,  SigHandler);
    }
#endif

    template <typename Event>
    [[nodiscard]] decltype(auto) Engine::GetCreatePool()
    {
        using Pool  = EventPool<Event>;
        auto& ctx   = Context::GetInstance();
        if(!ctx.m_Events.Has<Event>()) {
            ctx.m_Events.Create<Event, Pool>();
        }

        return ctx.m_Events.Get<Event, Pool>();
    }

    template <typename Event, typename Type>
    void Engine::RemoveEventByKey() 
    {
        constexpr auto id = Hash::Get<Type>();
        using Key   = EventID<id>;
        auto& pool  = GetCreatePool<Event>();
        if(pool.Has<Key>()) {
            pool.Remove<Key>();
        }
    }

    template <typename Event, typename... Args>
    void Engine::SignalBase(Args&&... args) 
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        auto& pool = GetCreatePool<Event>();        
        if(!pool.Empty()) 
        {
            const auto event = Event{std::forward<Args>(args)...};
            const auto& cb = [&event](const auto& callback) {
                callback(event);
            };

            if constexpr(std::is_same_v<Event, Events::Engine::Tick> || std::is_same_v<Event, Events::Engine::Update>) {
                pool.Each(cb, false);
            } else {
                pool.Each(cb, true);
            }
        }
    }


    [[nodiscard]] inline bool Engine::Heartbeat() 
    {
        auto& ctx = Context::GetInstance();
        const auto state = ctx.m_State;

    #if defined(HELENA_PLATFORM_WIN)
        __try {
    #endif
        switch(state) 
        {
            case EState::Undefined:  
            {
                RegisterHandlers();

                ctx.m_State     = EState::Init;
                ctx.m_TimeStart = std::chrono::steady_clock::now();
                ctx.m_TimeNow   = ctx.m_TimeStart;
                ctx.m_TimePrev  = ctx.m_TimeStart;

                if(ctx.m_Callback) {
                    ctx.m_Callback();
                }

                SignalBase<Events::Engine::Init>();
                SignalBase<Events::Engine::Config>();
                SignalBase<Events::Engine::Execute>();

            } break;

            case EState::Init: 
            {
                ctx.m_TimePrev  = ctx.m_TimeNow;
                ctx.m_TimeNow   = std::chrono::steady_clock::now();
                ctx.m_DeltaTime = std::chrono::duration<float>{ctx.m_TimeNow - ctx.m_TimePrev}.count();

                ctx.m_TimeElapsed += ctx.m_DeltaTime;

            #if defined(HELENA_PLATFORM_WIN)
                const auto timeElapsedFPS = std::chrono::duration<float>{ctx.m_TimeNow - ctx.m_TimeStart}.count();
                if(timeElapsedFPS > ctx.m_TimeLeftFPS) {
                    ctx.m_TimeLeftFPS = timeElapsedFPS + 1.f;
                    ConsoleInfo("App: {} | FPS: {}", ctx.GetAppName(), ctx.m_CountFPS);
                    ctx.m_CountFPS = 0;
                }
            #endif

                // Base signal called only once for new listeners
                SignalBase<Events::Engine::Init>();
                SignalBase<Events::Engine::Config>();
                SignalBase<Events::Engine::Execute>();
                SignalBase<Events::Engine::Tick>(ctx.m_DeltaTime);

                if(ctx.m_TimeElapsed >= ctx.m_Tickrate) {
                    ctx.m_TimeElapsed -= ctx.m_Tickrate;
                    ctx.m_CountFPS++;

                    SignalBase<Events::Engine::Update>(ctx.m_Tickrate);
                }

                if(ctx.m_TimeElapsed < ctx.m_Tickrate) {
                    Util::Sleep(std::chrono::milliseconds{1});
                }

            } break;

            case EState::Shutdown: 
            {
                SignalBase<Events::Engine::Finalize>();
                SignalBase<Events::Engine::Shutdown>();

                ctx.m_State = EState::Undefined;

                const auto reason = static_cast<std::string_view>(ctx.m_ShutdownReason);
                if(!reason.empty()) {
                    HELENA_MSG_FATAL("Shutdown Engine with reason: {}", reason);
                } else {
                    HELENA_MSG_WARNING("Shutdown Engine");
                }

                return false;
            }
        }

    #if defined(HELENA_PLATFORM_WIN)
        } __except (MiniDumpSEH(GetExceptionInformation())) {
            Engine::Shutdown("Unhandled Exception");
        }
    #endif

        return true;
    }

    [[nodiscard]] inline bool Engine::Running() noexcept {
        return Context::GetInstance().m_State == EState::Init;
    }

    [[nodiscard]] inline Engine::EState Engine::GetState() noexcept {
        return Context::GetInstance().m_State;
    }

    template <typename... Args>
    void Engine::Shutdown(const std::string_view format, Args&&... args)
    {
        auto& ctx = Context::GetInstance();
        const std::lock_guard lock{ctx.m_ShutdownMutex};

        if(ctx.m_State != EState::Shutdown) {
            ctx.m_State = EState::Shutdown;
            ctx.m_ShutdownReason = Util::Format(format, std::forward<Args>(args)...);
        }
    }

    template <typename T, typename... Args>
    void Engine::RegisterSystem(Args&&... args) {
        auto& ctx = Context::GetInstance();
        ctx.m_Systems.Create<T>(std::forward<Args>(args)...);
    }

    template <typename... T>
    [[nodiscard]] bool Engine::HasSystem() {
        auto& ctx = Context::GetInstance();
        return ctx.m_Systems.Has<T...>();
    }

    template <typename... T>
    [[nodiscard]] bool Engine::AnySystem() {
        auto& ctx = Context::GetInstance();
        return ctx.m_Systems.Any<T...>();
    }

    template <typename... T>
    [[nodiscard]] decltype(auto) Engine::GetSystem() {
        auto& ctx = Context::GetInstance();
        return ctx.m_Systems.Get<T...>();
    }

    template <typename... T>
    void Engine::RemoveSystem() {
        auto& ctx = Context::GetInstance();
        return ctx.m_Systems.Remove<T...>();
    }

    template <typename Event>
    void Engine::SubscribeEvent(EventCallback<Event> callback)
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        constexpr auto id = Hash::template Get<decltype(callback)>();

        using Key   = EventID<id>;
        auto& pool  = GetCreatePool<Event>();
        if(!pool.Has<Key>()) {
            pool.Create<Key>([callback](const Event& event) {
                callback(event);
            });
        }
    }

    template <typename Event, typename System>
    void Engine::SubscribeEvent(EventCallbackSystem<Event, System> callback) 
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");
        static_assert(std::is_same_v<System, Traits::RemoveCVRefPtr<System>>, "System type incorrect");

        constexpr auto id = Hash::template Get<decltype(callback)>();

        using Key   = EventID<id>;
        auto& pool  = GetCreatePool<Event>();
        if(!pool.Has<Key>()) {
            pool.Create<Key>([callback](const Event& event) {
                if(Engine::HasSystem<System>()) {
                    (Engine::GetSystem<System>().*callback)(event);
                }
            });
        }
    }

    template <typename Event, typename... Args>
    void Engine::SignalEvent(Args&&... args) 
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        const auto event = Event{std::forward<Args>(args)...};
        auto& pool = GetCreatePool<Event>();
        pool.Each([&event](const auto& callback) {
            callback(event);
        });
    }

    template <typename Event>
    void Engine::RemoveEvent(EventCallback<Event> callback) {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        RemoveEventByKey<Event, decltype(callback)>();
    }

    template <typename Event, typename System>
    void Engine::RemoveEvent(EventCallbackSystem<Event, System> callback) {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");
        static_assert(std::is_same_v<System, Traits::RemoveCVRefPtr<System>>, "System type incorrect");

        RemoveEventByKey<Event, decltype(callback)>();
    }
}

#endif // HELENA_ENGINE_ENGINE_IPP