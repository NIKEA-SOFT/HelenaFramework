#ifndef HELENA_ENGINE_ENGINE_IPP
#define HELENA_ENGINE_ENGINE_IPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Traits/AnyOf.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/Format.hpp>
#include <Helena/Util/ConstexprIf.hpp>
#include <Helena/Util/Format.hpp>
#include <Helena/Util/Sleep.hpp>

#include <execution>

namespace Helena
{
#if defined(HELENA_PLATFORM_WIN)
    inline BOOL WINAPI Engine::CtrlHandler(DWORD)
    {
        const auto ctx = Engine::Context::Get();
        if(ctx)
        {
            if(ctx->m_State == Engine::EState::Init) {
                Engine::Shutdown();
            }
        }

        return TRUE;
    }

    inline LONG WINAPI Engine::MiniDumpSEH(EXCEPTION_POINTERS* pException)
    {
        std::optional<std::string_view> appName;
        const auto context = Engine::Context::Get();
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
        const auto buffer = Types::Format<30>(msg, std::forward<Args>(args)...);
        SetConsoleTitleA(buffer.GetData());
    }

#elif defined(HELENA_PLATFORM_LINUX)
    inline void Engine::SigHandler(int signal)
    {
        const auto ctx = Engine::Context::Get();
        if(ctx)
        {
            if(ctx->m_State == Engine::EState::Init) {
                Engine::Shutdown();
            }
        }
    }

    inline void Engine::RegisterHandlers() {
        signal(SIGTERM, SigHandler);
        signal(SIGSTOP, SigHandler);
        signal(SIGINT,  SigHandler);
        signal(SIGKILL, SigHandler);
        signal(SIGHUP,  SigHandler);
    }
#endif

    [[nodiscard]] inline bool Engine::Heartbeat() 
    {
        auto& ctx = Engine::Context::GetInstance();
        const auto state = ctx.m_State.load(std::memory_order_relaxed);
        auto fnGetTimeMS = []() noexcept {
            std::uint64_t ms {};
        #if defined(HELENA_PLATFORM_WIN)
            static LARGE_INTEGER s_frequency;
            static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
            LARGE_INTEGER now;
            if(s_use_qpc && QueryPerformanceCounter(&now)) {
                ms = (1000LL * now.QuadPart) / s_frequency.QuadPart;
            } else {
                ms = GetTickCount64();
            }
        #else
            struct timeval te;
            gettimeofday(&te, NULL);
            ms = te.tv_sec * 1000LL + te.tv_usec / 1000;
        #endif
            return ms;
        };

    #if defined(HELENA_PLATFORM_WIN)
        __try {
    #endif
        switch(state) 
        {
            case Engine::EState::Undefined: [[unlikely]]
            {
                RegisterHandlers();
                
                ctx.m_ShutdownMessage.m_Location.m_File = "";
                ctx.m_ShutdownMessage.m_Location.m_Function = "";
                ctx.m_ShutdownMessage.m_Location.m_Line = 0;
                ctx.m_ShutdownMessage.m_Message.clear();

                ctx.m_State     = Engine::EState::Init;
                ctx.m_TimeStart = fnGetTimeMS();
                ctx.m_TimeNow   = ctx.m_TimeStart;
                ctx.m_TimePrev  = ctx.m_TimeStart;

                if(ctx.m_Callback) {
                    ctx.m_Callback();
                }

                if(Running()) SignalEvent<Events::Engine::Init>();
                if(Running()) SignalEvent<Events::Engine::Config>();
                if(Running()) SignalEvent<Events::Engine::Execute>();

            } break;

            case Engine::EState::Init: [[likely]]
            {
                ctx.m_TimePrev  = ctx.m_TimeNow;
                ctx.m_TimeNow   = fnGetTimeMS();
                ctx.m_DeltaTime = (ctx.m_TimeNow - ctx.m_TimePrev) / 1000.f;

                ctx.m_TimeElapsed += ctx.m_DeltaTime;

                static std::uint32_t accumulator = 0;
                static constexpr std::uint32_t accumulatorMax = 2;

                // Base signal called only once for new listeners
                SignalEvent<Events::Engine::Init>();
                SignalEvent<Events::Engine::Config>();
                SignalEvent<Events::Engine::Execute>();
                SignalEvent<Events::Engine::Tick>(ctx.m_DeltaTime);

                accumulator = 0;
                while(ctx.m_TimeElapsed >= ctx.m_Tickrate) 
                {
                    ctx.m_TimeElapsed -= ctx.m_Tickrate;
                    if(accumulator++ >= accumulatorMax) {
                        break;
                    }

                    SignalEvent<Events::Engine::Update>(ctx.m_Tickrate);
                }

                SignalEvent<Events::Engine::Render>(ctx.m_TimeElapsed / ctx.m_Tickrate);

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
                ctx.m_State = Engine::EState::Undefined;

                if(!ctx.m_ShutdownMessage.m_Message.empty()) {
                    const auto format = Log::Formater<Log::Fatal>{ctx.m_ShutdownMessage.m_Message, ctx.m_ShutdownMessage.m_Location};
                    Log::Console<Log::Fatal>(format);
                }

                return false;
            }
        }

    #if defined(HELENA_PLATFORM_WIN)
        } __except (MiniDumpSEH(GetExceptionInformation())) {
            if(ctx.m_State == Engine::EState::Shutdown) {
                return false;
            }

            Engine::Shutdown("Unhandled Exception");
        }
    #endif

        return true;
    }

    [[nodiscard]] inline bool Engine::Running() noexcept {
        return GetState() == Engine::EState::Init;
    }

    [[nodiscard]] inline Engine::EState Engine::GetState() noexcept {
        return Engine::Context::GetInstance().m_State.load(std::memory_order_relaxed);
    }

    template <typename... Args>
    void Engine::Shutdown(const Types::LocationString& msg, [[maybe_unused]] Args&&... args)
    {
        auto& ctx = Engine::Context::GetInstance();
        const std::lock_guard lock{ctx.m_ShutdownMessage.m_Mutex};

        if(ctx.m_State != Engine::EState::Shutdown) {
            ctx.m_State = Engine::EState::Shutdown;

            if(!msg.m_Msg.empty()) {
                ctx.m_ShutdownMessage.m_Location = msg.m_Location;
                ctx.m_ShutdownMessage.m_Message = "Shutdown Engine with reason: " + Util::Format(msg.m_Msg, std::forward<Args>(args)...);
            }
        }
    }

    template <typename T, typename... Args>
    void Engine::RegisterSystem([[maybe_unused]] Args&&... args) {
        Engine::Context::GetInstance().m_Systems.template Create<T>(std::forward<Args>(args)...);
    }

    template <typename... T>
    [[nodiscard]] bool Engine::HasSystem() {
        return Engine::Context::GetInstance().m_Systems.template Has<T...>();
    }

    template <typename... T>
    [[nodiscard]] bool Engine::AnySystem() {
        return Engine::Context::GetInstance().m_Systems.template Any<T...>();
    }

    template <typename... T>
    [[nodiscard]] decltype(auto) Engine::GetSystem() {
        return Engine::Context::GetInstance().m_Systems.template Get<T...>();
    }

    template <typename... T>
    void Engine::RemoveSystem() {
        return Engine::Context::GetInstance().m_Systems.template Remove<T...>();
    }

    template <typename Event, typename... Args>
    void Engine::SubscribeEvent(void (*callback)(Args...))
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        auto& ctx = Engine::Context::GetInstance();
        if(!ctx.m_Events.template Has<Event>()) {
            ctx.m_Events.template Create<Event>();
        }

        auto& pool = ctx.m_Events.template Get<Event>();
        const auto id = reinterpret_cast<std::uintptr_t>(callback);
        const auto empty = pool.cend() == std::find_if(pool.begin(), pool.end(), [id](const auto& data) { return data.m_Key == id; });
        HELENA_ASSERT(empty, "Listener already registered!");
        if(empty) 
        {
            pool.emplace_back(id, [callback]([[maybe_unused]] void* event) {
                if constexpr(std::is_empty_v<Event>) {
                    static_assert(sizeof... (Args) == 0, "Args should be dropped for optimization");

                    callback();
                } else {
                    static_assert(sizeof...(Args) == 1, "Args incorrect");
                    static_assert((std::is_same_v<Event, Traits::RemoveCVRefPtr<Args>> && ...), "Args type incorrect");

                    callback(static_cast<std::tuple_element_t<0, std::tuple<Args...>>>(*static_cast<Event*>(event)));
                }
            });
        }
    }

    template <typename Event, typename System, typename... Args>
    void Engine::SubscribeEvent(void (System::*callback)(Args...))
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        auto& ctx = Engine::Context::GetInstance();
        if(!ctx.m_Events.template Has<Event>()) {
            ctx.m_Events.template Create<Event>();
        }

        auto& pool = ctx.m_Events.template Get<Event>();
        constexpr auto id = Types::Hash::template Get<decltype(callback), std::uint64_t>();
        const auto empty = pool.cend() == std::find_if(pool.begin(), pool.end(), [id](const auto& data) { return data.m_Key == id; });
        HELENA_ASSERT(empty, "Listener already registered!");
        if(empty)
        {
            pool.emplace_back(id, [callback]([[maybe_unused]] void* event) {
                if constexpr(std::is_empty_v<Event>) {
                    static_assert(sizeof... (Args) == 0, "Args should be dropped for optimization");

                    if(HasSystem<System>()) {
                        (GetSystem<System>().*callback)();
                    }
                } else {
                    static_assert(sizeof...(Args) == 1, "Args incorrect");
                    static_assert((std::is_same_v<Event, Traits::RemoveCVRefPtr<Args>> && ...), "Args type incorrect");

                    if(HasSystem<System>()) {
                        (GetSystem<System>().*callback)(static_cast<std::tuple_element_t<0, std::tuple<Args...>>>(*static_cast<Event*>(event)));
                    }
                }
            });
        }
    }

    template <typename Event, typename... Args>
    void Engine::SignalEvent([[maybe_unused]] Args&&... args)
    {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");

        auto& ctx = Engine::Context::GetInstance();
        if(ctx.m_Events.template Has<Event>()) 
        {
            [[maybe_unused]] Event event{std::forward<Args>(args)...};
            void* data = Util::ConstexprIf<std::is_empty_v<Event>>(nullptr, &event);
            auto& pool = ctx.m_Events.template Get<Event>();

            for(std::size_t pos = pool.size(); pos; --pos) {
                pool[pos - 1].m_Callback(data);
            }

            if constexpr(Traits::IsAnyOf<Event,
                Events::Engine::Init,
                Events::Engine::Config,
                Events::Engine::Execute,
                Events::Engine::Finalize,
                Events::Engine::Shutdown>::value) {
                pool.clear();
            }
        }
    }

    template <typename Event, typename... Args>
    void Engine::UnsubscribeEvent(void (*callback)(Args...)) {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");
        
        auto& ctx = Engine::Context::GetInstance();
        HELENA_ASSERT(ctx.m_Events.template Has<Event>(), "Event pool not exist");
        if(!ctx.m_Events.template Has<Event>()) {
            return;
        }

        auto& pool = ctx.m_Events.template Get<Event>();
        const auto id = reinterpret_cast<std::uintptr_t>(callback);
        const auto it = std::find_if(pool.cbegin(), pool.cend(), [id](const auto& data) noexcept {
            return data.m_Key == id;
        });

        if(it != pool.cend()) {
            pool.erase(it);
        }
    }

    template <typename Event, typename System, typename... Args>
    void Engine::UnsubscribeEvent(void (System::* callback)(Args...)) {
        static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");
        
        auto& ctx = Engine::Context::GetInstance();
        HELENA_ASSERT(ctx.m_Events.template Has<Event>(), "Event pool not exist");
        if(!ctx.m_Events.template Has<Event>()) {
            return;
        }

        auto& pool = ctx.m_Events.template Get<Event>();
        constexpr auto id = Types::Hash::template Get<decltype(callback), std::uint64_t>();
        const auto it = std::find_if(pool.cbegin(), pool.cend(), [id](const auto& data) noexcept {
            return data.m_Key == id;
        });

        if(it != pool.cend()) {
            pool.erase(it);
        }
    }
}

#endif // HELENA_ENGINE_ENGINE_IPP