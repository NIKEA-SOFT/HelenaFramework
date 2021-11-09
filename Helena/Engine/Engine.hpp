#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Engine/Context.hpp>
#include <Helena/Types/Function.hpp>
#include <Helena/Types/FixedString.hpp>

#include <vector>

namespace Helena
{
    class Engine final
    {
        template <typename Event, typename T>
        using Callback = void (T::*)(Event&);

        template <typename Event>
        using EventCallback = std::function<bool (Event*, std::uint64_t)>;

        template <typename Event>
        using EventPool = std::vector<EventCallback<Event>>;

        static constexpr std::uint64_t OperationCall = 0uLL;

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
        [[nodiscard]] static bool HasSystem() noexcept;

        template <typename... T>
        [[nodiscard]] static bool AnySystem() noexcept;

        template <typename... T>
        [[nodiscard]] static decltype(auto) GetSystem() noexcept;

        template <typename... T>
        static void RemoveSystem() noexcept;

    public:
        template <typename Event, typename T>
        static void SubscribeEvent(Callback<Event, T> callback);

        template <typename Event, typename... Args>
        static void SignalEvent(Args&&... args) noexcept;

        template <typename Event, typename T>
        static void RemoveEvent(Callback<Event, T> callback) noexcept;

    private:
    #if defined(HELENA_PLATFORM_WIN)
        static BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
        static LONG MiniDumpSEH(EXCEPTION_POINTERS* pException);
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
