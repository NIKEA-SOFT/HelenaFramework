#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Core/Context.hpp>
#include <Helena/Traits/CVRefPtr.hpp>

namespace Helena
{
    class Engine final
    {
        template <std::uint64_t id>
        struct EventID {};

        template <typename Event>
        struct EventPool : Core::IEventPool {
            EventPool() noexcept = default;
            ~EventPool() = default;
            EventPool(const EventPool&) = delete;
            EventPool(EventPool&&) noexcept = delete;
            EventPool& operator=(const EventPool&) = delete;
            EventPool& operator=(EventPool&&) noexcept = delete;
            static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");
            Types::VectorUnique<std::function<void (const Event&)>> m_Callbacks;
        };

        template <typename Event, typename System>
        using EventCallbackSystem = void (System::*)(const Event&);

        template <typename Event>
        using EventCallback = void (*)(const Event&);

    private:
        template <typename Pool>
        [[nodiscard]] static Pool& GetCreatePool();

        template <typename Pool, typename Key>
        static void RemoveEventByKey();

        template <typename Event, typename... Args>
        static void SignalBase(Args&&... args);

    public:
        [[nodiscard]] static bool Heartbeat();
        [[nodiscard]] static bool Running() noexcept;
        [[nodiscard]] static Core::EState GetState() noexcept;

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
