#ifndef HELENA_CORE_CORE_HPP
#define HELENA_CORE_CORE_HPP

#include <Helena/Core/Context.hpp>

#include <Helena/Dependencies/EnTT.hpp>

namespace Helena
{
    enum class EStateEngine : std::uint8_t 
    {
        Init,
        Executing,
        Finish,
        Shutdown
    };

    class Engine final
    {
        using Callback = void (*)(EStateEngine);

        static bool Init(Callback func) noexcept;
    };


    namespace EventsXXX
    {
        enum class Initialize {};
        enum class Finalize {};
    }

    class CoreXXX final
    {
        template <typename, typename>
        friend struct ENTT_API entt::type_seq;

        enum class SystemEvent : std::uint8_t {
            Create,
            Execute,
            Tick,
            Update,
            Destroy,

            Size
        };

        template <typename Type, typename... Args>
        using fn_create_t	= decltype(std::declval<Type>().HFSystemInit(std::declval<Args>()...));

        template <typename Type, typename... Args>
        using fn_execute_t	= decltype(std::declval<Type>().HFSystemExecute(std::declval<Args>()...));

        template <typename Type, typename... Args>
        using fn_tick_t		= decltype(std::declval<Type>().HFSystemTick(std::declval<Args>()...));

        template <typename Type, typename... Args>
        using fn_update_t	= decltype(std::declval<Type>().HFSystemUpdate(std::declval<Args>()...));

        template <typename Type, typename... Args>
        using fn_finish_t	= decltype(std::declval<Type>().HFSystemFinish(std::declval<Args>()...));

        template <typename Type, typename... Args>
        using fn_destroy_t	= decltype(std::declval<Type>().HFSystemDestroy(std::declval<Args>()...));


        using map_indexes_t = robin_hood::unordered_flat_map<entt::id_type, std::size_t, Hash::Hasher<entt::id_type>, Hash::Comparator<entt::id_type>>;

        template <typename Type>
        struct SystemIndex {
            [[nodiscard]] static auto GetIndex(map_indexes_t& container) -> std::size_t;
        };

    public:

        enum class ECoreState : std::uint8_t {
            Init,
            Running,
            Shutdown
        };

        /**
         * @brief Core context class
         */
        class Context final {
            friend class Core;

            template <typename, typename>
            friend struct ENTT_API entt::type_seq;

            std::array<std::queue<std::size_t>, static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Size)> m_EventScheduler {};
            std::array<entt::delegate<void ()>, static_cast<std::underlying_type_t<SystemEvent>>(SystemEvent::Size)> m_SystemsEvents {};

            map_indexes_t m_TypeIndexes {};
            map_indexes_t m_SequenceIndexes {};

            std::vector<entt::any> m_Systems {};
            std::vector<std::string_view> m_Args {};

            entt::dispatcher m_Dispatcher {};

            std::chrono::steady_clock::time_point m_TimeStart {};
            std::chrono::steady_clock::time_point m_TimeNow {};
            std::chrono::steady_clock::time_point m_TimePrev {};

            double m_TimeDelta {};
            double m_TickRate {};

            ECoreState m_State {};

            std::string m_ShutdownReason;
        #if defined(HF_PLATFORM_WIN)
            std::mutex m_ShutdownMutex {};
            std::condition_variable m_ShutdownCondition {};
        #endif
        };



    private:
        static inline std::shared_ptr<Context> m_Context {};

    private:
    #if defined(HF_PLATFORM_WIN)
        static void Terminate();
        static BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
        static int SEHHandler(unsigned int code, _EXCEPTION_POINTERS* pException);
    #elif defined(HF_PLATFORM_LINUX)
        static auto SigHandler(int signal) -> void;
    #endif

        static auto HeartbeatTimeCalc() -> double;
        static void HookSignals();
        static void CreateContext(const std::shared_ptr<Context>& ctx);
        static void Heartbeat();
        static void EventSystems(const SystemEvent type);

    public:
        Core() = delete;
        ~Core() = delete;
        Core(const Core&) = delete;
        Core(Core&&) = delete;
        Core& operator=(const Core&) = delete;
        Core& operator=(Core&&) = delete;

        /**
         * @brief Initialize Core.
         *
         * You don't have to initialize ctx yourself.
         * The ctx transfer is used only if you initialize it in the dynamic library (plugin) for share memory.
         *
         * The signature of the function should be equivalent to the following:
         *
         * @code{.cpp}
         * bool();
         * @endcode
         *
         * @warning
         * Attempting to pass your ctx instance results in undefined behavior.
         * The Core will initialize it on its own.
         * Use ctx only for Initialize core in the plugins (dll/so).
         *
         * @note Details about context look at GetContext() method.
         *
         * @tparam Func Type of callback.
         * @param callback A valid function object.
         * @param ctx Share context memory (only if called from Modules).
         * @return True if success otherwise false.
         */
        template <typename Func>
        static void Initialize(Func&& callback, const std::shared_ptr<Context>& ctx = {}) noexcept;

        /**
         * @brief Returns a context of Core.
         *
         * @note
         * Use GetContext to get const reference on the context of Core.
         * Context is designed to support "across boundary" for share memory between plugins (dll/so).
         *
         * @return A const reference on the Core context.
         */
        [[nodiscard]] static auto GetContext() noexcept -> std::shared_ptr<Context>;

        /**
         * @brief Return Core state
         * @return ECoreState
         */
        [[nodiscard]] static auto GetCoreState() noexcept -> ECoreState;

        /**
        * @brief Shutdown framework.
        */
        static void Shutdown() noexcept;

        /**
        * @brief Shutdown framework.
        *
        */
        static void Shutdown(const std::string& reason) noexcept;

        /**
         * @brief Register args in the Core.
         * @param argc Count of arguments.
         * @param argv Pointer to arguments.
         */
        static void SetArgs(const std::size_t size, const char* const* argv);

        /**
         * @brief Sets the tickrate of the hearbeat.
         * @note Default value of tickrate 30.0 fps.
         * @param tickrate Value of tickrate for fixed step of tick event.
         */
        static void SetTickrate(const double tickrate) noexcept;

        /**
         * @brief Returns a vector of args.
         * @return A temporary vector of args.
         */
        [[nodiscard]] static auto GetArgs() noexcept -> std::vector<std::string_view>&;

        /**
         * @brief Returns a value to the given tickrate in milliseconds.
         * @return Tickrate in milliseconds (1.0 / tickrate).
         */
        [[nodiscard]] static auto GetTickrate() noexcept -> double;

        /**
         * @brief Returns a value of the elapsed time since Core initialization.
         * @return Elapsed time in milliseconds.
         */
        [[nodiscard]] static auto GetTimeElapsed() noexcept -> double;

        /**
         * @brief Returns a value of the delta time.
         * @return Delta time in milliseconds.
         */
        [[nodiscard]] static auto GetTimeDelta() noexcept -> double;


        /**
         * @brief Register the given system.
         * @tparam Type Type of system.
         * @tparam Args Type of args.
         * @param args Arguments to use to initialize the system.
         */
        template <typename Type, typename... Args>
        static void RegisterSystem([[maybe_unused]] Args&&... args);

        /**
         * @brief Checks if a given system exists.
         * @tparam Type Type of system.
         * @return True if exist otherwise false.
         */
        template <typename Type>
        [[nodiscard]] static bool HasSystem() noexcept;

        /**
         * @brief Returns a reference to the given system.
         *
         * @warning Attempting to use an invalid system results in undefined behavior.
         *
         * @tparam Type Type of system.
         * @return Reference to the system.
         */
        template <typename Type>
        [[nodiscard]] static auto GetSystem() noexcept -> Type&;

        /**
         * @brief Remove the given system instance..
         * @tparam Type Type of sytem.
         */
        template <typename Type>
        static void RemoveSystem() noexcept;

        /**
         * @brief Subscribe to an event.
         * @tparam Event Type of event.
         * @tparam Method A valid function object.
         */
        template <typename Event, auto Method>
        static void RegisterEvent();

        /**
         * @brief Subscribe to an event.
         * @tparam Event Type of event.
         * @tparam Method A valid function object.
         * @tparam Type Type of instance.
         * @param instance A valid object.
         */
        template <typename Event, auto Method, typename Type>
        static void RegisterEvent(Type&& instance);

        /**
         * @brief Emit an event.
         *
         * @tparam Event Type of event.
         * @tparam Args Type of args.
         * @param args Arguments to use to initialize the system.
         */
        template <typename Event, typename... Args>
        static void TriggerEvent([[maybe_unused]] Args&&... args);

        /**
         * @brief Add an event to the queue.
         *
         * @note Events are stored aside until the UpdateEvent member function is invoked,
         * then all the messages that are still pending are sent to the listeners at once.
         *
         * @tparam Event Type of event.
         * @tparam Args Type of args.
         * @param args Arguments to use to initialize the system.
         */
        template <typename Event, typename... Args>
        static void EnqueueEvent([[maybe_unused]] Args&&... args);

        /**
         * @brief Emit all events of the type at once.
         *
         * @note All queued events for event type will be triggered
         *
         * @tparam Event Type of event.
         */
        template <typename Event>
        static void UpdateEvent();

        /**
         * @brief Emit all events at once.
         * @note All queued events will be triggered
         */
        static void UpdateEvent();

        /**
         * @brief Unsubscribe to an event.
         * @tparam Event Type of event.
         * @tparam Method A valid function object.
         */
        template <typename Event, auto Method>
        static void RemoveEvent();

        /**
         * @brief Unsubscribe to an event.
         * @tparam Event Type of event.
         * @tparam Method A valid function object.
         * @tparam Type Type of instance.
         * @param instance A valid object.
         */
        template <typename Event, auto Method, typename Type>
        static void RemoveEvent(Type&& instance);
    };
}

#include <Helena/Core.ipp>

#endif // HELENA_CORE_CORE_HPP