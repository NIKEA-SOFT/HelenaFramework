#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorKVAny.hpp>
#include <Helena/Types/VectorUnique.hpp>

#include <mutex>
#include <functional>

namespace Helena
{
    namespace Events::Engine
    {
        struct Init {};
        struct Config {};
        struct Execute {};
        struct Tick {
            float deltaTime;
        };
        struct Update {
            float fixedTime;
        };
        struct Finalize {};
        struct Shutdown {};
    }

    class Engine final
    {
        template <typename Event>
        using EventPool = Types::VectorUnique<std::function<void(const Event&)>>;

    public:
        enum class EState : std::uint8_t
        {
            Undefined,
            Init,
            Shutdown
        };

        class Context
        {
            friend class Engine;

            [[nodiscard]] static Context& GetInstance() noexcept {
                HELENA_ASSERT(m_Context, "Context not initilized");
                return *m_Context.get();
            }

        public:
            using Callback = std::function<void()>;

            Context() noexcept : m_Tickrate{ 1.f / 30.f }, m_DeltaTime{}, m_TimeElapsed{}, m_TimeLeftFPS{}, m_CountFPS{}, m_State{ EState::Undefined } {}
            virtual ~Context() = default;
            Context(const Context&) = delete;
            Context(Context&&) noexcept = delete;
            Context& operator=(const Context&) = delete;
            Context& operator=(Context&&) noexcept = delete;

            template <typename T = Context, typename... Args>
            requires std::is_base_of_v<Context, T>&& std::is_constructible_v<T, Args...>
            [[nodiscard]] static void Initialize([[maybe_unused]] Args&&... args) {
                HELENA_ASSERT(!m_Context, "EngineContext already initialized!");
                m_Context = std::make_shared<T>(std::forward<Args>(args)...);
            }

            template <typename T = Context>
            requires std::is_base_of_v<Context, T>
            static void Initialize(const std::shared_ptr<T>& ctx) noexcept {
                HELENA_ASSERT(!m_Context, "Context already initialized!");
                m_Context = ctx;
            }

            template <typename T = Context>
            requires std::is_base_of_v<Context, T>
            [[nodiscard]] static std::shared_ptr<T> Get() noexcept {
                HELENA_ASSERT(m_Context, "Context not initialized");
                return std::static_pointer_cast<T>(m_Context);
            }

            template <typename... Args>
            static void SetAppName(std::string name) noexcept {
                auto& ctx = GetInstance();
                ctx.m_ApplicationName = std::move(name);
            }

            static std::string_view GetAppName() noexcept {
                const auto& ctx = GetInstance();
                return ctx.m_ApplicationName;
            }

            static void SetTickrate(float tickrate) noexcept {
                auto& ctx = GetInstance();
                ctx.m_Tickrate = 1.f / std::max(tickrate, 1.f);
            }

            static void SetCallback(Callback callback) noexcept {
                auto& ctx = GetInstance();
                ctx.m_Callback = std::move(callback);
            }

            [[nodiscard]] static float GetTickrate() noexcept {
                const auto& ctx = GetInstance();
                return ctx.m_Tickrate;
            }

        private:
            Types::VectorAny<64> m_Systems;
            Types::VectorKVAny<sizeof(double)> m_Events;

            Callback m_Callback;

            std::mutex  m_ShutdownMutex;
            std::string m_ShutdownReason;
            std::string m_ApplicationName;

            float m_Tickrate;
            float m_DeltaTime;

            float m_TimeElapsed;
            float m_TimeLeftFPS;

            std::chrono::steady_clock::time_point m_TimeStart;
            std::chrono::steady_clock::time_point m_TimeNow;
            std::chrono::steady_clock::time_point m_TimePrev;

            std::uint32_t m_CountFPS;

            EState m_State;


            inline static std::shared_ptr<Context> m_Context;   // Global context
        };


        template <std::uint64_t id>
        struct EventID {};

        template <typename Event, typename System>
        using EventCallbackSystem = void (System::*)(const Event&);

        template <typename Event>
        using EventCallback = void (*)(const Event&);

    private:
        template <typename Event>
        [[nodiscard]] static decltype(auto) GetCreatePool();

        template <typename Event, typename Type>
        static void RemoveEventByKey();

        template <typename Event, typename... Args>
        static void SignalBase(Args&&... args);

    public:
        [[nodiscard]] static bool Heartbeat();
        [[nodiscard]] static bool Running() noexcept;
        [[nodiscard]] static EState GetState() noexcept;

        template <typename... Args>
        static void Shutdown(const std::string_view format = {}, Args&&... args);

    public:
        template <typename T, typename... Args>
        static void RegisterSystem(Args&&... args);

        template <typename... T>
        [[nodiscard]] static bool HasSystem();

        template <typename... T>
        [[nodiscard]] static bool AnySystem();

        template <typename... T>
        [[nodiscard]] static decltype(auto) GetSystem();

        template <typename... T>
        static void RemoveSystem();

    public:
        template <typename Event>
        static void SubscribeEvent(EventCallback<Event> callback);

        template <typename Event, typename System>
        static void SubscribeEvent(EventCallbackSystem<Event, System> callback);

        template <typename Event, typename... Args>
        static void SignalEvent(Args&&... args);

        template <typename Event>
        static void RemoveEvent(EventCallback<Event> callback);

        template <typename Event, typename System>
        static void RemoveEvent(EventCallbackSystem<Event, System> callback);

    private:
    #if defined(HELENA_PLATFORM_WIN)
        static BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
        static LONG WINAPI MiniDumpSEH(EXCEPTION_POINTERS* pException);

        template <typename... Args>
        static void ConsoleInfo(std::string_view msg, Args&&... args);
    #elif defined(HELENA_PLATFORM_LINUX)
        static auto SigHandler(int signal) -> void;
    #endif

        static void RegisterHandlers();
    };
}

#include <Helena/Engine/Engine.ipp>

#endif // HELENA_ENGINE_ENGINE_HPP
