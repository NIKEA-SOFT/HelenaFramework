#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Platform/Platform.hpp>
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorKVAny.hpp>
#include <Helena/Types/VectorUnique.hpp>
#include <Helena/Types/LocationString.hpp>
#include <Helena/Types/Mutex.hpp>

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

        struct Render {
            float deltaTime;
        };

        struct Finalize {};
        struct Shutdown {};
    }

    class Engine final
    {
        template <std::size_t Value> 
        struct IUniqueKey {};

        using UKEventPool       = IUniqueKey<0>;
        using UKEventStorage    = IUniqueKey<1>;
        using UKSystems         = IUniqueKey<2>;

        struct EventData {
            std::uintptr_t m_Key;
            std::function<void (void*)> m_Callback;
        };

        using EventPool = std::vector<EventData>;

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

            struct ShutdownMessage {
                std::string m_Message;
                Types::SourceLocation m_Location;
                Types::Mutex m_Mutex;
            };

            static constexpr auto DefaultTickrate = 1.f / 30.f;

        protected:
            using Callback = std::function<void ()>;

            template <typename T = Context>
            [[nodiscard]] static T& GetInstance() noexcept {
                HELENA_ASSERT(m_Context, "Context not initilized");
                return *static_cast<T*>(m_Context.get());
            }

        public:

            Context() noexcept : m_Tickrate{DefaultTickrate}, m_DeltaTime{}, m_TimeElapsed{}, m_State{ EState::Undefined } {}
            ~Context() {
                m_Events.Clear();
                m_Systems.Clear();
            }
            Context(const Context&) = delete;
            Context(Context&&) noexcept = delete;
            Context& operator=(const Context&) = delete;
            Context& operator=(Context&&) noexcept = delete;

            template <typename T = Context, typename... Args>
            requires std::is_base_of_v<Context, T> && std::is_constructible_v<T, Args...>
            static void Initialize([[maybe_unused]] Args&&... args) {
                HELENA_ASSERT(!m_Context, "Context already initialized!");
                m_Context = std::make_shared<T>(std::forward<Args>(args)...);
            }

            template <typename T = Context>
            requires std::is_base_of_v<Context, T>
            static void Initialize(const std::shared_ptr<T>& ctx) noexcept {
                HELENA_ASSERT(ctx, "Context is empty!");
                HELENA_ASSERT(!m_Context || ctx && m_Context == ctx, "Context already initialized!");
                m_Context = ctx;
            }

            template <typename T = Context>
            requires std::is_base_of_v<Context, T>
            [[nodiscard]] static std::shared_ptr<T> Get() noexcept {
                HELENA_ASSERT(m_Context, "Context not initialized");
                return std::static_pointer_cast<T>(m_Context);
            }

            static void SetAppName(std::string name) noexcept {
                auto& ctx = GetInstance();
                ctx.m_ApplicationName = std::move(name);
            }

            static void SetTickrate(float tickrate) noexcept {
                auto& ctx = GetInstance();
                ctx.m_Tickrate = 1.f / std::max(tickrate, 1.f);
            }

            static void SetMain(Callback callback) noexcept {
                auto& ctx = GetInstance();
                ctx.m_Callback = std::move(callback);
            }

            static const std::string& GetAppName() noexcept {
                const auto& ctx = GetInstance();
                return ctx.m_ApplicationName;
            }

            [[nodiscard]] static float GetTickrate() noexcept {
                const auto& ctx = GetInstance();
                return ctx.m_Tickrate;
            }

        private:
            Types::VectorAny<UKSystems> m_Systems;
            Types::VectorUnique<UKEventStorage, EventPool> m_Events;

            Callback m_Callback;

            ShutdownMessage m_ShutdownMessage;
            std::string m_ApplicationName;

            float m_Tickrate;
            float m_DeltaTime;
            float m_TimeElapsed;

            std::chrono::steady_clock::time_point m_TimeStart;
            std::chrono::steady_clock::time_point m_TimeNow;
            std::chrono::steady_clock::time_point m_TimePrev;

            std::atomic<EState> m_State;

            inline static std::shared_ptr<Context> m_Context;
        };

    private:
    #if defined(HELENA_PLATFORM_WIN)
        static BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
        static LONG WINAPI MiniDumpSEH(EXCEPTION_POINTERS* pException);

        template <typename... Args>
        static void ConsoleInfo(std::string_view msg, [[maybe_unused]] Args&&... args);
    #elif defined(HELENA_PLATFORM_LINUX)
        static auto SigHandler(int signal) -> void;
    #endif

        static void RegisterHandlers();

        template <typename Event, typename Callback>
        static void RegisterEvent(std::uintptr_t id, Callback&& callback);

        template <typename Iterator>
        static Iterator FindEvent(Iterator begin, Iterator end, std::uintptr_t id) noexcept;

        template <typename Event>
        static decltype(auto) GetEventPool();

        template <typename Event>
        static void RemoveEvent(std::uintptr_t id) noexcept;

    public:
        [[nodiscard]] static bool Heartbeat();
        [[nodiscard]] static bool Running() noexcept;
        [[nodiscard]] static EState GetState() noexcept;

        template <typename... Args>
        static void Shutdown(std::string_view msg = {}, [[maybe_unused]] Args&&... args, Types::SourceLocation location = Types::SourceLocation::Create());

        template <typename T, typename... Args>
        static void RegisterSystem([[maybe_unused]] Args&&... args);

        template <typename... T>
        [[nodiscard]] static bool HasSystem();

        template <typename... T>
        [[nodiscard]] static bool AnySystem();

        template <typename... T>
        [[nodiscard]] static decltype(auto) GetSystem();

        template <typename... T>
        static void RemoveSystem();

        template <typename Event, typename... Args>
        static void SubscribeEvent(void (*callback)(Args...));

        template <typename Event, typename System, typename... Args>
        static void SubscribeEvent(void (System::*callback)(Args...));

        template <typename Event, typename... Args>
        static void SignalEvent([[maybe_unused]] Args&&... args);

        template <typename Event, typename... Args>
        static void UnsubscribeEvent(void (*callback)(Args...));

        template <typename Event, typename System, typename... Args>
        static void UnsubscribeEvent(void (System::*callback)(Args...));
    };
}

#include <Helena/Engine/Engine.ipp>

#endif // HELENA_ENGINE_ENGINE_HPP
