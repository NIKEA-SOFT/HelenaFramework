#ifndef HELENA_ENGINE_ENGINE_IPP
#define HELENA_ENGINE_ENGINE_IPP

#include <Helena/Engine/Engine.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/Hash.hpp>
#include <Helena/Traits/CVRefPtr.hpp>
#include <Helena/Util/Format.hpp>
#include <Helena/Util/Sleep.hpp>

#include <algorithm>

namespace Helena
{
#if defined(HELENA_PLATFORM_WIN)
    inline BOOL WINAPI Engine::CtrlHandler(DWORD dwCtrlType)
    {
        const auto ctx = Core::Context::Get();
        if(ctx)
        {
            if(ctx->m_State == Core::EState::Init) {
                Engine::Shutdown("Ctrl handler");
            }
        }

        return TRUE;
    }

    inline LONG Engine::MiniDumpSEH(EXCEPTION_POINTERS* pException) 
    {
        const auto context  = Core::Context::Get();
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
        DWORD processId = GetProcessId(hProcess);
        MINIDUMP_TYPE flag = MINIDUMP_TYPE::MiniDumpWithIndirectlyReferencedMemory;
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo {
            .ThreadId = GetCurrentThreadId(),
            .ExceptionPointers = pException,
            .ClientPointers = TRUE
        };

        BOOL result = MiniDumpWriteDump(hProcess, processId, hFile, flag, &exceptionInfo, NULL, NULL);
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
    inline void Core::SigHandler(int signal)
    {
        const auto ctx = Core::Context::Get();
        if(ctx)
        {
            if(ctx->m_State == Core::EState::Init) {
                Engine::Shutdown("sig handler");
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
        auto& ctx = Core::Context::GetInstance();
        const auto state = ctx.m_State;

    #if defined(HELENA_PLATFORM_WIN)
        __try {
    #endif
        switch(state) 
        {
            case Core::EState::Undefined:  
            {
                //HELENA_ASSERT(ctx.m_CallbackInit,       "Init callback is empty in Context");
                HELENA_ASSERT(ctx.m_CallbackTick,       "Tick callback is empty in Context");
                HELENA_ASSERT(ctx.m_CallbackUpdate,     "Update callback is empty in Context");
                //HELENA_ASSERT(ctx.m_CallbackShutdown,   "Shutdown callback is empty in Context");

                RegisterHandlers();

                ctx.m_State     = Core::EState::Init;
                ctx.m_TimeStart = std::chrono::steady_clock::now();
                ctx.m_TimeNow   = ctx.m_TimeStart;
                ctx.m_TimePrev  = ctx.m_TimeStart;

                if(ctx.m_CallbackInit) {
                    ctx.m_CallbackInit();
                }
            } break;

            case Core::EState::Init: 
            {
                //static float timeElapsed {};
                //static float timeLeftFPS {1.f};
                //static std::uint32_t countFPS {};

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

                ctx.m_CallbackTick();

                if(ctx.m_TimeElapsed >= ctx.m_Tickrate) {
                    ctx.m_TimeElapsed -= ctx.m_Tickrate;
                    ctx.m_CallbackUpdate();
                    ++ctx.m_CountFPS;
                }

                if(ctx.m_TimeElapsed < ctx.m_Tickrate) {
                    Util::Sleep(std::chrono::milliseconds{1});
                }

            } break;

            case Core::EState::Shutdown: 
            {
                if(ctx.m_CallbackShutdown) {
                    ctx.m_CallbackShutdown();
                }

                ctx.m_State = Core::EState::Undefined;

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
        return Core::Context::GetInstance().m_State == Core::EState::Init;
    }

    [[nodiscard]] inline Core::EState Engine::GetState() noexcept {
        return Core::Context::GetInstance().m_State;
    }

    template <typename... Args>
    void Engine::Shutdown(const std::string_view format, Args&&... args) 
    {
        auto& ctx = Core::Context::GetInstance();
        const std::lock_guard lock{ctx.m_ShutdownMutex};

        if(ctx.m_State != Core::EState::Shutdown) {
            ctx.m_State = Core::EState::Shutdown;
            ctx.m_ShutdownReason = Types::Format<256>(format, std::forward<Args>(args)...);
        }
    }

    template <typename T, typename... Args>
    void Engine::RegisterSystem(Args&&... args) {
        auto& ctx = Core::Context::GetInstance();
        ctx.m_Systems.Create<T>(std::forward<Args>(args)...);
    }

    template <typename... T>
    [[nodiscard]] bool Engine::HasSystem() noexcept {
        auto& ctx = Core::Context::GetInstance();
        return ctx.m_Systems.Has<T...>();
    }

    template <typename... T>
    [[nodiscard]] bool Engine::AnySystem() noexcept {
        auto& ctx = Core::Context::GetInstance();
        return ctx.m_Systems.Any<T...>();
    }

    template <typename... T>
    [[nodiscard]] decltype(auto) Engine::GetSystem() noexcept {
        auto& ctx = Core::Context::GetInstance();
        return ctx.m_Systems.Get<T...>();
    }

    template <typename... T>
    void Engine::RemoveSystem() noexcept {
        auto& ctx = Core::Context::GetInstance();
        return ctx.m_Systems.Remove<T...>();
    }

    template <typename Event, typename T>
    static void Engine::SubscribeEvent(Callback<Event, T> callback) 
    {
        using System = Traits::RemoveCVRefPtr<T>;
        using Pool = EventPool<Event>;
        using Callback = Callback<Event, T>;

        // Create new pool if not exist
        auto& ctx = Core::Context::GetInstance();
        if(!ctx.m_Events.Has<Pool>()) {
            ctx.m_Events.Create<Pool>();
        }

        // Pool on callbacks
        auto& pool = ctx.m_Events.Get<Pool>();

        // Already registered check
        HELENA_ASSERT(std::find_if(pool.cbegin(), pool.cend(), [id = Hash::template Get<decltype(callback)>()](const auto& instance) {
            return instance(nullptr, id);
        }) == pool.cend(), "System {} already registered on event type: {}", Traits::NameOf<System>::value, Traits::NameOf<Event>::value);

        // Add lambda invoke inside pool
        pool.emplace_back([callback](Event* event, std::uint64_t other_id) -> bool
        {
            HELENA_ASSERT(Engine::HasSystem<System>(), "System {} not exist in pool of event type: {}", Traits::NameOf<System>::value, Traits::NameOf<Event>::value);

            // This 'if' used as compare predicate for remove listener of signal
            // if other_id != OperationCall it's mean lambda called from RemoveEvent
            // if other_id == RemoveEvent it's mean lambda called from SignalEvent
            if(other_id) {
                constexpr auto id = Hash::template Get<decltype(callback)>();
                return id == other_id;
            }

            // Check exist System
            if(Engine::HasSystem<System>()) {
                // Invoke callback
                (Engine::GetSystem<System>().*callback)(*event);
                return true;
            }

            return false;
        });
    }

    template <typename Event, typename... Args>
    static void Engine::SignalEvent(Args&&... args) noexcept 
    {
        using Pool = EventPool<Event>;

        // Get pool of events
        auto& ctx = Core::Context::GetInstance();
        if(ctx.m_Events.Has<Pool>()) 
        {
            // Get pool of callbacks
            auto& pool = ctx.m_Events.Get<Pool>();
            if(pool.size())
            {
                // Create Event with args
                auto event = Event{std::forward<Args>(args)...};
                for(auto size = pool.size(); size; --size) 
                {
                    // Signal
                    const auto& callback = pool[size - 1];
                    if(!callback(&event, OperationCall)) {
                        pool.erase(pool.cbegin() + (size - 1));
                    }
                }
            }
        }
    }

    template <typename Event, typename T>
    static void Engine::RemoveEvent(Callback<Event, T> callback) noexcept 
    {
        using Pool = EventPool<Event>;

        // Get hash id for type of callback
        constexpr auto id = Hash::template Get<decltype(callback)>();

        // Get pool of events
        auto& ctx = Core::Context::GetInstance();
        if(ctx.m_Events.Has<Pool>()) 
        {
            // Get pool of callbacks
            auto& pool = ctx.m_Events.Get<Pool>();
            for(auto size = pool.size(); size; --size) 
            {
                // Call lambda with compare signal
                // when Event is nullptr and id != OperationCall
                // used compare inside lambda.
                // We compare current instance hash id with lambda
                // instance hash id and if equal return true
                const auto& instance = pool[size - 1];
                if(instance(nullptr, id)) {
                    pool.erase(pool.cbegin() + (size - 1));
                }
            }
        }
    }
}

#endif // HELENA_ENGINE_ENGINE_IPP