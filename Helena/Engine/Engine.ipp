#ifndef HELENA_ENGINE_ENGINE_IPP
#define HELENA_ENGINE_ENGINE_IPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Traits/AnyOf.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/Format.hpp>
#include <Helena/Util/ConstexprIf.hpp>
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
                Engine::Shutdown("sig handler");
            }
        }

        return TRUE;
    }

    inline LONG WINAPI Engine::MiniDumpSEH(EXCEPTION_POINTERS* pException)
    {
        std::optional<std::string_view> appName;
        const auto context = Context::Get();
        if(!context->GetAppName().empty()) {
            appName = std::make_optional<std::string_view>(context->GetAppName());
        }

        const auto dateTime = Types::DateTime::FromLocalTime();
        const auto dumpName = Util::Format("{}_{:04d}{:02d}{:02d}_{:02d}_{:02d}_{:02d}.dmp", 
            appName.value_or("Helena"),
            dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
            dateTime.GetHour(), dateTime.GetMinutes(), dateTime.GetSeconds());

        HANDLE hFile = CreateFileA(dumpName.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if(!hFile || hFile == INVALID_HANDLE_VALUE) {
            Log::Console<Log::Exception>("Create file for dump failed, error: {}", GetLastError());
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
            Log::Console<Log::Exception>("Create dump failed, error: {}", GetLastError());
            DeleteFileA(dumpName.c_str());
        } else {
            Log::Console<Log::Exception>("SEH Handler Dump: \"{}\" created!", dumpName.c_str());
        }

        CloseHandle(hFile);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    inline void Engine::RegisterHandlers() {
        static ULONG stackSize = 64 * 1024;

        SetThreadStackGuarantee(&stackSize);
        SetConsoleCtrlHandler(CtrlHandler, TRUE);
        SetUnhandledExceptionFilter(MiniDumpSEH);

        // Disable X button
        if(HWND hWnd = GetConsoleWindow(); hWnd) {
            HMENU hMenu = GetSystemMenu(hWnd, FALSE);
            EnableMenuItem(hMenu, SC_CLOSE, MF_DISABLED | MF_BYCOMMAND);
        }
    }

    template <typename... Args>
    void Engine::ConsoleInfo(std::string_view msg, [[maybe_unused]] Args&&... args) {
        const auto buffer = Types::Format<32>(msg, std::forward<Args>(args)...);
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

    [[nodiscard]] inline bool Engine::Heartbeat() 
    {
        auto& ctx = Context::GetInstance();
        const auto state = ctx.m_State.load(std::memory_order_relaxed);

    #if defined(HELENA_PLATFORM_WIN)
        __try {
    #endif
        switch(state) 
        {
            case EState::Undefined: [[unlikely]]
            {
                RegisterHandlers();

                ctx.m_State     = EState::Init;
                ctx.m_TimeStart = std::chrono::steady_clock::now();
                ctx.m_TimeNow   = ctx.m_TimeStart;
                ctx.m_TimePrev  = ctx.m_TimeStart;

                if(ctx.m_Callback) {
                    ctx.m_Callback();
                }

                SignalEvent<Events::Engine::Init>();
                SignalEvent<Events::Engine::Config>();
                SignalEvent<Events::Engine::Execute>();

            } break;

            case EState::Init: [[likely]] 
            {
                ctx.m_TimePrev  = ctx.m_TimeNow;
                ctx.m_TimeNow   = std::chrono::steady_clock::now();
                ctx.m_DeltaTime = std::chrono::duration<float>{ctx.m_TimeNow - ctx.m_TimePrev}.count();

                ctx.m_TimeElapsed += ctx.m_DeltaTime;

                static std::uint32_t accumulator = 0;
                static constexpr std::uint32_t accumulatorMax = 2;
            #if defined(HELENA_PLATFORM_WIN)
                static std::uint32_t countFPS = 0;
                static float timeFPS = 0.f; 

                timeFPS += ctx.m_DeltaTime;
                if(timeFPS > 1.f) {
                    timeFPS -= 1.f;
                    ConsoleInfo("App: {} | FPS: {}{}", ctx.GetAppName(), countFPS, '\0');
                    countFPS = 0;
                }
            #endif

                // Base signal called only once for new listeners
                SignalEvent<Events::Engine::Init>();
                SignalEvent<Events::Engine::Config>();
                SignalEvent<Events::Engine::Execute>();
                SignalEvent<Events::Engine::Tick>(ctx.m_DeltaTime);

                accumulator = 0;
                while(ctx.m_TimeElapsed >= ctx.m_Tickrate) {
                    ctx.m_TimeElapsed -= ctx.m_Tickrate;

                #if defined(HELENA_PLATFORM_WIN)
                    countFPS++;
                #endif

                    SignalEvent<Events::Engine::Update>(ctx.m_Tickrate);

                    if(accumulator >= accumulatorMax) {
                        break;
                    }

                    ++accumulator;
                }

                SignalEvent<Events::Engine::Render>(ctx.m_TimeElapsed / ctx.m_Tickrate);

            #ifndef HELENA_ENGINE_NOSLEEP
                Util::Sleep(std::chrono::milliseconds{1});
            #endif

            } break;

            case EState::Shutdown: [[unlikely]]
            {
                SignalEvent<Events::Engine::Finalize>();
                SignalEvent<Events::Engine::Shutdown>();

                ctx.m_Events.Clear();
                ctx.m_Systems.Clear();
                ctx.m_State = EState::Undefined;

                if(!ctx.m_ShutdownMessage.m_Message.empty()) {
                    const auto format = Types::BasicLogger::Formater<Log::Fatal>{ctx.m_ShutdownMessage.m_Message, ctx.m_ShutdownMessage.m_Location};
                    Log::Console(format);
                    ctx.m_ShutdownMessage.m_Message.clear();
                }

                return false;
            }
        }

    #if defined(HELENA_PLATFORM_WIN)
        } __except (MiniDumpSEH(GetExceptionInformation())) {
            if(ctx.m_State == EState::Shutdown) {
                return false;
            }

            Engine::Shutdown("Unhandled Exception");
        }
    #endif

        return true;
    }

    [[nodiscard]] inline bool Engine::Running() noexcept {
        return GetState() == EState::Init;
    }

    [[nodiscard]] inline Engine::EState Engine::GetState() noexcept {
        return Context::GetInstance().m_State.load(std::memory_order_relaxed);
    }

    template <typename... Args>
    void Engine::Shutdown(std::string_view msg, [[maybe_unused]] Args&&... args, Types::SourceLocation location)
    {
        auto& ctx = Context::GetInstance();
        const std::lock_guard lock{ctx.m_ShutdownMessage.m_Mutex};

        if(ctx.m_State != EState::Shutdown) {
            ctx.m_State = EState::Shutdown;

            ctx.m_ShutdownMessage.m_Location = location;
            ctx.m_ShutdownMessage.m_Message = "Shutdown Engine";

            if(!msg.empty()) {
                ctx.m_ShutdownMessage.m_Message += " with reason: " + Util::Format(msg, std::forward<Args>(args)...);
            }
        }
    }

    template <typename T, typename... Args>
    void Engine::RegisterSystem([[maybe_unused]] Args&&... args) {
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
    decltype(auto) Engine::GetEventPool() 
    {
        auto& ctx = Context::GetInstance();
        if(!ctx.m_Events.Has<Event>()) {
            ctx.m_Events.Create<Event>();
        }
        return ctx.m_Events.Get<Event>();
    }

    template <typename Event, typename Callback>
    void Engine::RegisterEvent(std::uintptr_t id, Callback&& callback) 
    {
        auto& pool = GetEventPool<Event>();
        const auto empty = FindEvent(pool.cbegin(), pool.cend(), id) == pool.cend();
        HELENA_ASSERT(empty, "Listener already registered!");
        if(empty){
            pool.emplace_back(id, std::forward<Callback>(callback));
        }
    }

    template <typename Iterator>
    Iterator Engine::FindEvent(Iterator begin, Iterator end, std::uintptr_t id) noexcept {
        return std::find_if(begin, end, [id](const auto& data) { return data.m_Key == id; });
    }

    template <typename Event, typename... Args>
    void Engine::SubscribeEvent(void (*callback)(Args...))
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        RegisterEvent<Event>(reinterpret_cast<std::uintptr_t>(callback), [callback]([[maybe_unused]] void* event) 
        {
            if constexpr(std::is_empty_v<Event>) {
                static_assert(sizeof... (Args) == 0, "Args should be dropped for optimization");

                callback();
            } else {
                static_assert(sizeof...(Args) == 1, "Args incorrect");
                static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Args...>>, "Args type incorrect");

                callback(static_cast<Args...>(*static_cast<Event*>(event)));
            }
        });
    }

    template <typename Event, typename System, typename... Args>
    void Engine::SubscribeEvent(void (System::*callback)(Args...))
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        RegisterEvent<Event>(Hash::Get<decltype(callback)>(), [callback]([[maybe_unused]] void* event) 
        {
            if constexpr(std::is_empty_v<Event>) {
                static_assert(sizeof... (Args) == 0, "Args should be dropped for optimization");

                if(HasSystem<System>()) {
                    (GetSystem<System>().*callback)();
                }
            } else {
                static_assert(sizeof...(Args) == 1, "Args incorrect");
                static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Args...>>, "Args type incorrect");

                if(HasSystem<System>()) {
                    (GetSystem<System>().*callback)(static_cast<Args...>(*static_cast<Event*>(event)));
                }
            }
        });
    }

    template <typename Event, typename... Args>
    void Engine::SignalEvent([[maybe_unused]] Args&&... args)
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        [[maybe_unused]] auto event = Event{std::forward<Args>(args)...};
        void* data = Util::ConstexprIf<std::is_empty_v<Event>>(nullptr, static_cast<void*>(&event));
        auto& pool = GetEventPool<Event>();

        for(std::size_t pos = pool.size(); pos; --pos) {
            pool[pos - 1].m_Callback(data);
        }

        if constexpr (Traits::IsAnyOf<Event,
            Events::Engine::Init, 
            Events::Engine::Config,
            Events::Engine::Execute,
            Events::Engine::Finalize,
            Events::Engine::Shutdown>::value) {
            pool.clear();
        }
    }

    template <typename Event>
    void Engine::RemoveEvent(std::uintptr_t id) noexcept
    {
        const auto& pool = GetEventPool<Event>();
        if(const auto it = FindEvent(pool.cbegin(), pool.cend(), id); it != pool.cend()) {
            pool.erase(it);
        }
    }

    template <typename Event, typename... Args>
    void Engine::UnsubscribeEvent(void (*callback)(Args...)) {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        RemoveEvent<Event>(reinterpret_cast<std::uintptr_t>(callback));
    }

    template <typename Event, typename System, typename... Args>
    void Engine::UnsubscribeEvent(void (System::* callback)(Args...)) {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        RemoveEvent<Event>(Hash::Get<decltype(callback)>());
    }
}

#endif // HELENA_ENGINE_ENGINE_IPP