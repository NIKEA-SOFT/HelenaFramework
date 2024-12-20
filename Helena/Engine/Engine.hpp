#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Platform.hpp>
#include <Helena/Logging/FileLogger.hpp>
#include <Helena/Traits/Conditional.hpp>
#include <Helena/Traits/Constructible.hpp>
#include <Helena/Traits/Function.hpp>
#include <Helena/Types/Any.hpp>
#include <Helena/Types/CompressedPair.hpp>
#include <Helena/Types/Function.hpp>
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorUnique.hpp>
#include <Helena/Types/LocationString.hpp>
#include <Helena/Util/Process.hpp>

#include <atomic>
#include <cstring>
#include <exception>
#include <functional>
#include <string>
#include <tuple>

namespace Helena
{
    class Engine final
    {
        //! Container key generator
        template <std::size_t Value> 
        struct IUniqueKey {};

        //! Unique key for storage systems type index
        using UKSystems     = IUniqueKey<0>;

        //! Unique key for storage components type index
        using UKComponents  = IUniqueKey<1>;

        //! Unique key for storage signals type index
        using UKSignals     = IUniqueKey<2>;

        //! Unique key for storage messages type index
        using UKMessages    = IUniqueKey<3>;

        template <typename T>
        using EventsPool    = std::vector<T>;

        using DeferredCtx   = Types::Any<46>;
        using DeferredPool  = EventsPool<Types::CompressedPair<DeferredCtx, void(*)(DeferredCtx&)>>;


        using CustomLogger  = std::unique_ptr<void, void (*)(const void*)>;

        template <auto Fn>
        static constexpr bool NotTemplateFunction = requires {
            typename Traits::Function<decltype(Fn)>::Class;
        };

        template <typename Event, auto Fn, bool Member>
        static constexpr auto RequiresCallback = []() {
            if constexpr(std::is_empty_v<Event>) {
                if constexpr(NotTemplateFunction<Fn>) {
                    return Traits::Function<decltype(Fn)>::Orphan;
                } return false;
            } else if constexpr(!Member && std::is_member_function_pointer_v<decltype(Fn)> == Member) {
                return std::is_invocable_v<decltype(Fn), Event>;
            } else if constexpr(Member && std::is_member_function_pointer_v<decltype(Fn)> == Member) {
                if constexpr(NotTemplateFunction<Fn>) {
                    return std::is_invocable_v<decltype(Fn), typename Traits::Function<decltype(Fn)>::Class&, Event&>;
                } return false;
            } return false;
        }() && Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
            && (std::is_member_function_pointer_v<decltype(Fn)> == Member);

        template <typename T>
        static constexpr auto RequiresConfig =
            std::invocable<decltype(T::Sleep)> &&
            std::convertible_to<decltype(T::Accumulate), std::uint32_t>;

        class Delegate
        {
            template <typename Event, auto Callback>
            static void Caller([[maybe_unused]] void* instance, void* ev)
            {
                if constexpr(std::is_member_function_pointer_v<decltype(Callback)>) {
                    HELENA_ASSERT(instance, "Instance is nullptr");
                    using Class = typename Traits::Function<decltype(Callback)>::Class;
                    if constexpr(std::is_empty_v<Event>) {
                        ((*static_cast<Class*>(instance)).*Callback)();
                    } else {
                        ((*static_cast<Class*>(instance)).*Callback)(*static_cast<Event*>(ev));
                    }
                } else {
                    if constexpr(std::is_empty_v<Event>) {
                        Callback();
                    } else {
                        Callback(*static_cast<Event*>(ev));
                    }
                }
            }

            using Callback = void (void*, void*);

        public:
            template<typename Event, auto Callback>
            struct Args {};

        public:
            template<typename Event, auto Fn>
            Delegate(Args<Event, Fn>, void* instance)
                : m_Callback{Caller<Event, Fn>}
                , m_Instance{instance} {}
            ~Delegate() = default;
            Delegate(const Delegate&) = default;
            Delegate(Delegate&&) noexcept = default;
            Delegate& operator=(const Delegate&) = default;
            Delegate& operator=(Delegate&&) noexcept = default;

            template <typename... Args>
            void operator()(Args&&... args) const {
                m_Callback(m_Instance, std::forward<Args>(args)...);
            }

            template <typename Event, auto Fn>
            [[nodiscard]] bool Compare(void* instance = nullptr) const noexcept {
                return m_Instance == instance && m_Callback == Caller<Event, Fn>;
            }

        private:
            Callback* m_Callback;
            void* m_Instance;
        };

        template <typename...>
        struct Signals {};

    public:
        //! Engine states
        enum class EState : std::uint8_t
        {
            Undefined,
            Init,
            Shutdown
        };

        //! Default Heartbeat configuration
        struct DefaultConfig {
            static constexpr auto Sleep = Types::Function::BindFront(
                static_cast<void (*)(const std::uint64_t)>(Util::Process::Sleep), 1 /* msec */);
            static constexpr auto Accumulate = 5;
        };

        //! Structure used to do something without throw a signal (event)
        static constexpr struct {} NoSignal{};

        //! Context for storage framework data
        class Context
        {
            friend class Engine;
            struct ShutdownMessage {
                std::string m_Message;
                Types::SourceLocation m_Location;
            };

            static constexpr auto m_DefaultTickRate = 1. / 30.;

        public:
            Context() noexcept
                : m_Systems{}
                , m_Components{}
                , m_Signals{}
                , m_DeferredSignals{}
                , m_ShutdownMessage{std::make_unique<ShutdownMessage>()}
                , m_Logger{new Logging::FileLogger(), +[](const void* ptr) {
                        delete static_cast<const Logging::FileLogger*>(ptr);
                    }}
                , m_TimeStart{GetTickTime()}
                , m_TimeNow{}
                , m_TimePrev{}
                , m_TickRate{m_DefaultTickRate}
                , m_TimeDelta{}
                , m_TimeElapsed{}
                , m_State{EState::Undefined} {}

            virtual ~Context() {
                m_Signals.Clear();
                m_Systems.Clear();
                m_Components.Clear();
            }

            Context(const Context&) = delete;
            Context(Context&&) noexcept = delete;
            Context& operator=(const Context&) = delete;
            Context& operator=(Context&&) noexcept = delete;

        private:
            virtual void Main() {}

        private:
            // Systems and Components
            Types::VectorAny<UKSystems> m_Systems;
            Types::VectorAny<UKComponents> m_Components;

            // Signals
            Types::VectorUnique<UKSignals, EventsPool<Delegate>> m_Signals;
            DeferredPool m_DeferredSignals;

            // Reason
            std::unique_ptr<ShutdownMessage> m_ShutdownMessage;

            // Logger
            CustomLogger m_Logger;

            // Timers for Heartbeat
            std::uint64_t m_TimeStart;
            std::uint64_t m_TimeNow;
            std::uint64_t m_TimePrev;

            double m_TickRate;
            double m_TimeDelta;
            double m_TimeElapsed;

            // Engine state
            std::atomic<EState> m_State;

        #if defined(HELENA_PLATFORM_LINUX)
            // Used on Linux for signal handling;
            std::unique_ptr<std::byte[]> m_StackMemory;
        #endif // HELENA_PLATFORM_LINUX
        };

    private:
        using ContextDeleter = void (*)(const Context*);
        using ContextStorage = std::unique_ptr<Context, ContextDeleter>;
        inline static ContextStorage m_Context{nullptr, nullptr};

        static void InitContext(ContextStorage context) noexcept;
        [[nodiscard]] static Context& MainContext() noexcept;

    private:
    #if defined(HELENA_PLATFORM_WIN)
        static BOOL WINAPI CtrlHandler([[maybe_unused]] DWORD dwCtrlType);
        static LONG WINAPI MiniDumpSEH(EXCEPTION_POINTERS* pException);

    #elif defined(HELENA_PLATFORM_LINUX)
        static void SigHandler([[maybe_unused]] int signal);
    #endif

        static void RegisterHandlers();
        [[nodiscard]] static std::uint64_t GetTickTime() noexcept;

    public:
        /**
        * @brief Initialize context of Engine
        * @tparam T Context type
        * @tparam Args Types of arguments used to construct
        * @param args Arguments for context initialization
        * @note The context can be inherited
        */
        template <std::derived_from<Engine::Context> T = Context, typename... Args>
        requires Traits::ConstructibleAggregateFrom<T, Args...>
        static void Initialize([[maybe_unused]] Args&&... args);

        /**
        * @brief Initialize the engine context for sharing between the executable and plugins
        * @param ctx Context object for support shared memory and across boundary
        * @note This overload is used to share the context object between the executable and plugins
        */
        static void Initialize(Context& ctx) noexcept;

        /**
        * @brief Has context of Engine
        * @note This method can be used to check the initialization of the framework.
        * @return Return a reference to context
        */
        [[nodiscard]] static bool HasContext() noexcept;

        /**
        * @brief Get context of Engine
        * @return Return a reference to context
        */
        template <std::derived_from<Engine::Context> T>
        [[nodiscard]] static T& GetContext() noexcept;

        /**
        * @brief Return the current state of the engine
        * @return EState state flag
        */
        [[nodiscard]] static EState GetState() noexcept;

        /**
        * @brief Set the update tickrate for the engine "Update" event
        * @param tickrate Update frequency
        * @note By default, 30 frames per second
        */
        static void SetTickrate(double tickrate) noexcept;

        /**
        * @brief Returns the current engine tickrate
        * @return Tickrate in float
        */
        [[nodiscard]] static double GetTickrate() noexcept;

        /**
        * @brief Get time elapsed since Initialize
        * @return Return a time elapsed since Initialize
        */
        [[nodiscard]] static std::uint64_t GetTimeElapsed() noexcept;

        /**
        * @brief Heartbeat of the engine
        * @tparam HeartbeatConfig Structure with fields: "Sleep" and "Accumulate" for Heartbeat control
        * @code{.cpp}
        * while(Helena::Engine::Heartbeat()) {}
        * @endcode
        * 
        * @return True if successful or false if an error is detected or called shutdown
        * @note 
        * - Heartbeat: You have to call heartbeat in a loop to keep the framework running
        * The thread will not sleep if your operations consume a lot of CPU time
        * Field: Accumulate -> if your loop is too busy, then there may be an accumulation of delta time
        * that cannot be repaid by a single Update call, which will cause more Update calls to
        * follow immediately to reduce the accumulated time.
        * It is not recommended to use a large value for Accumulate, your thread may get stuck in a loop.
        * The correct solution is to offload the thread by finding a performance bottleneck.
        * Field: Sleep -> your own sleep function.
        */
        template <typename HeartbeatConfig = DefaultConfig>
        requires Engine::RequiresConfig<HeartbeatConfig>
        [[nodiscard]] static bool Heartbeat();

        /**
        * @brief Check if the engine is currently running
        * @return True if running, false if shutdown or not initialized
        * @note This function is similar to calling GetState() == EState::Init;
        */
        [[nodiscard]] static bool Running() noexcept;

        /**
        * @brief Shutdown the engine (thread safe)
        * 
        * @code{.cpp}
        * Helena::Engine::Shutdown(); // no error
        * Helena::Engine::Shutdown("Unknown error");
        * Helena::Engine::Shutdown("Error code: {}, text: {}", 55, "example");
        * @endcode
        * 
        * @tparam Args Types of arguments
        * @param msg Reason message
        * @param args Arguments for formatting the message 
        * @note If the error message is not empty, an error message will be displayed in the console
        * @warning If there is no console, the message will not be displayed
        */
        template <typename... Args>
        static void Shutdown(const Types::LocationString& msg = {}, [[maybe_unused]] Args&&... args);

        /**
        * @brief Returns the last shutdown reason
        *
        * @return Reason string if the engine State == EState::Shutdown and has a reason, empty otherwise
        * @warning No point in calling this function if State != EState::Shutdown
        */
        [[nodiscard]] static std::string ShutdownReason() noexcept;

        /**
        * @brief Register the custom logger in the engine
        * 
        * @param logger Custom logger pointer
        * @param deleter Custom deleter (by default: delete logger)
        * @note The framework has an independent logging system (see: Helena/Logging/Logging.hpp).
        * Registering a logger does not redirect the logging system, to redirect the output
        * see Helena/Logging/CustomPrint.hpp.
        * The problem is that logging in a multi-threaded environment requires synchronizations,
        * I wondered what would be the best solution if we want to be able to log to a file.
        * Components and systems? No, for this you need to enable thread safety for them,
        * and this is unreasonable, since you pay more for synchronize, this option was
        * immediately eliminated for me, what other options did I have?
        * Forcing the developer to create his own context? This is possible,
        * but this solution is not very convenient for developers.
        * Therefore, the best way out with minimal costs is to provide the ability to
        * register custom loggers inside the default context so that the developer can retrieve it
        * at any time, and he can store all the synchronization primitives inside his logging class.
        * This explains why registering a logger has nothing to do with redirecting logs to that registered logger.
        * You have to redirect output manually by getting your logger instance from the Context.
        * There are reasons for that:
        *   1) You don't pay for synchronization.
        *      I mean, this approach allows you to avoid the need to make systems and components thread-safe.
        *   2) Logging is independent of the Engine.
        *      You can use logging even if you haven't registered the Engine.
        *   3) Shared context.
        *      Your logger should be able to share states between exe/elf and dll/so,
        *      this is the main reason why I chose this solution.
        */
        template <typename Logger>
        static void RegisterLogger(Logger* logger, void (*deleter)(const void*) =
            +[](const void* ptr) {
                delete static_cast<Logger*>(ptr);
            }
        );

        /*
        * @brief Check the exist of logger
        * @return True if logger exist, or false
        */
        [[nodiscard]] static bool HasLogger() noexcept;

        /*
        * @brief Check the exist of logger
        * 
        * @tparam T Type of logger derived from IFileLogger.
        * @return Instance of logger if exist or exception/crash.
        */
        template <typename Logger = Logging::FileLogger>
        [[nodiscard]] static Logger& GetLogger();

        /**
        * @brief Register the system in the engine
        *
        * @code{.cpp}
        * struct MySystem {};
        * Helena::Engine::RegisterSystem<MySystem>(Helena::Engine::NoSignal);
        * @endcode
        *
        * @tparam T Type of system
        * @tparam Args Types of arguments
        * @param NoSignal Use Engine::NoSignal to not notify listeners
        * @param args Arguments for system initialization
        */
        template <typename T, typename... Args>
        requires Traits::ConstructibleAggregateFrom<T, Args...>
        static void RegisterSystem(decltype(NoSignal), Args&&... args);

        /**
        * @brief Register the system in the engine
        * This overload notifies listeners of a system registration event.
        *
        * @code{.cpp}
        * struct MySystem {};
        * Helena::Engine::RegisterSystem<MySystem>();
        * @endcode
        *
        * @tparam T Type of system
        * @tparam Args Types of arguments
        * @param args Arguments for system initialization
        */
        template <typename T, typename... Args>
        requires Traits::ConstructibleAggregateFrom<T, Args...>
        static void RegisterSystem(Args&&... args);

        /**
        * @brief Check the exist of system
        * 
        * @code{.cpp}
        * struct MySystemA{};
        * struct MySystemB{};
        *
        * Helena::Engine::RegisterSystem<MySystemA, MySystemB>();
        * if(Helena::Engine::HasSystem<MySystemA, MySystemB>()) {
        *   // ok
        * }
        * @endcode
        * 
        * @tparam T Types of systems
        * @return True if all types of systems exist, or false
        */
        template <typename... T>
        [[nodiscard]] static bool HasSystem();

        /**
        * @brief Check the exist of any system
        *
        * @code{.cpp}
        * struct MySystemA{};
        * struct MySystemB{};
        * 
        * Helena::Engine::RegisterSystem<MySystemA>();
        * if(Helena::Engine::AnySystem<MySystemA, MySystemB>()) {
        *   // ok
        * }
        * @endcode
        *
        * @tparam T Types of systems
        * @return True if any types of systems exist, or false
        */
        template <typename... T>
        [[nodiscard]] static bool AnySystem();

        /**
        * @brief Get a reference to the system
        * 
        * @code{.cpp}
        * struct MySystemA{};
        * struct MySystemB{};
        *
        * Helena::Engine::RegisterSystem<MySystemA>();
        * Helena::Engine::RegisterSystem<MySystemB>();
        * 
        * const auto& [systemA, systemB] = Helena::Engine::GetSystem<MySystemA, MySystemB>();
        * @endcode
        * 
        * @tparam T Types of systems
        * @return Reference to a system or tuple if multiple types of systems are passed
        */
        template <typename... T>
        [[nodiscard]] static decltype(auto) GetSystem();

        /**
        * @brief Remove the system from engine
        *
        * @code{.cpp}
        * struct MySystemA{};
        * struct MySystemB{};
        *
        * Helena::Engine::RegisterSystem<MySystemA>();
        * Helena::Engine::RegisterSystem<MySystemB>();
        *
        * Helena::Engine::RemoveSystem<MySystemA, MySystemB>(Helena::Engine::NoSignal);
        * @endcode
        * 
        * @tparam T Types of systems
        * @param NoSignal Use Engine::NoSignal to not notify listeners
        */
        template <typename... T>
        static void RemoveSystem(decltype(NoSignal));

        /**
        * @brief Remove the system from engine
        * This overload notifies listeners of a system registration event.
        *
        * @code{.cpp}
        * struct MySystemA{};
        * struct MySystemB{};
        *
        * Helena::Engine::RegisterSystem<MySystemA>();
        * Helena::Engine::RegisterSystem<MySystemB>();
        *
        * Helena::Engine::RemoveSystem<MySystemA, MySystemB>();
        * @endcode
        * 
        * @tparam T Types of systems
        */
        template <typename... T>
        static void RemoveSystem();

        /**
        * @brief Register the component in the engine
        *
        * @code{.cpp}
        * struct MyComponent {};
        * Helena::Engine::RegisterComponent<MyComponent>(Helena::Engine::NoSignal);
        * @endcode
        *
        * @tparam T Type of component
        * @tparam Args Types of arguments
        * @param NoSignal Use Engine::NoSignal to not notify listeners
        * @param args Arguments for component initialization
        */
        template <typename T, typename... Args>
        requires Traits::ConstructibleAggregateFrom<T, Args...>
        static void RegisterComponent(decltype(NoSignal), Args&&... args);

        /**
        * @brief Register the component in the engine
        * This overload notifies listeners of a component registration event.
        *
        * @code{.cpp}
        * struct MyComponent {};
        * Helena::Engine::RegisterComponent<MyComponent>();
        * @endcode
        *
        * @tparam T Type of component
        * @tparam Args Types of arguments
        * @param args Arguments for component initialization
        */
        template <typename T, typename... Args>
        requires Traits::ConstructibleAggregateFrom<T, Args...>
        static void RegisterComponent(Args&&... args);

        /**
        * @brief Check the exist of component
        *
        * @code{.cpp}
        * struct MyComponentA{};
        * struct MyComponentB{};
        *
        * Helena::Engine::RegisterComponent<MyComponentA, MyComponentB>();
        * if(Helena::Engine::HasComponent<MyComponentA, MyComponentB>()) {
        *   // ok
        * }
        * @endcode
        *
        * @tparam T Types of components
        * @return True if all types of components exist, or false
        */
        template <typename... T>
        [[nodiscard]] static bool HasComponent();

        /**
        * @brief Check the exist of any component
        *
        * @code{.cpp}
        * struct MyComponentA{};
        * struct MyComponentB{};
        *
        * Helena::Engine::RegisterComponent<MyComponentA>();
        * if(Helena::Engine::AnyComponent<MyComponentA, MyComponentB>()) {
        *   // ok
        * }
        * @endcode
        *
        * @tparam T Types of components
        * @return True if any types of components exist, or false
        */
        template <typename... T>
        [[nodiscard]] static bool AnyComponent();

        /**
        * @brief Get a reference to the component
        *
        * @code{.cpp}
        * struct MyComponentA{};
        * struct MyComponentB{};
        *
        * Helena::Engine::RegisterComponent<MyComponentA>();
        * Helena::Engine::RegisterComponent<MyComponentB>();
        *
        * const auto& [componentA, componentB] = Helena::Engine::GetComponent<MyComponentA, MyComponentB>();
        * @endcode
        *
        * @tparam T Types of components
        * @return Reference to a component or tuple if multiple types of components are passed
        */
        template <typename... T>
        [[nodiscard]] static decltype(auto) GetComponent();

        /**
        * @brief Remove the component from engine
        *
        * @code{.cpp}
        * struct MyComponentA{};
        * struct MyComponentB{};
        *
        * Helena::Engine::RegisterComponent<MyComponentA>();
        * Helena::Engine::RegisterComponent<MyComponentB>();
        *
        * Helena::Engine::RemoveComponent<MyComponentA, MyComponentB>(Helena::Engine::NoSignal);
        * @endcode
        *
        * @tparam T Types of components
        * @param NoSignal Use Engine::NoSignal to not notify listeners
        */
        template <typename... T>
        static void RemoveComponent(decltype(NoSignal));

        /**
        * @brief Remove the component from engine
        * This overload notifies listeners of a component registration event.
        *
        * @code{.cpp}
        * struct MyComponentA{};
        * struct MyComponentB{};
        *
        * Helena::Engine::RegisterComponent<MyComponentA>();
        * Helena::Engine::RegisterComponent<MyComponentB>();
        *
        * Helena::Engine::RemoveComponent<MyComponentA, MyComponentB>();
        * @endcode
        *
        * @tparam T Types of components
        */
        template <typename... T>
        static void RemoveComponent();

        /**
        * @brief Listening to the event
        *
        * @code{.cpp}
        * void OnInit() {
        *   // The event is called when the engine is initialized
        * }
        *
        * Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init, &OnInit>();
        * @endcode
        *
        * @tparam Event Type of event
        * @tparam Callback Function
        */
        template <typename Event, auto Callback>
        requires Engine::RequiresCallback<Event, Callback, /* Member function */ false>
        static void SubscribeEvent();

        /**
        * @brief Listening to the event
        *
        * @code{.cpp}
        * struct MySystem {
        *   void OnInit() {
        *       // The event is called when the engine is initialized
        *   }
        * };
        * Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init, &MySystem::OnInit>(this);
        * @endcode
        *
        * @tparam Event Type of event
        * @tparam Callback Member function
        * @param instance Instance of object
        */
        template <typename Event, auto Callback>
        requires Engine::RequiresCallback<Event, Callback, /* Member function */ true>
        static void SubscribeEvent(typename Traits::Function<decltype(Callback)>::Class* instance);

        /**
        * @brief Returns the count or tuple with counts of listeners subscribed to Event
        *
        * @tparam Event Types of event
        * @return Count or tuple with counts of listeners subscribed to Event
        */
        template <typename... Event>
        requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
        [[nodiscard]]
        static auto Subscribers();

        /**
        * @brief Check has listeners subscribed to Events
        *
        * @tparam Event Type of event
        * @return True if listeners are subscribed to all of the listed Events
        */
        template <typename... Event>
        requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
        [[nodiscard]]
        static auto HasSubscribers();

        /**
        * @brief Check has any listeners are subscribed to any of the Events.
        *
        * @tparam Event Type of event
        * @return True if any listeners are subscribed to any of the listed Events
        */
        template <typename... Event>
        requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
        [[nodiscard]]
        static auto AnySubscribers();

        /**
        * @brief Trigger an event for all listeners
        * 
        * @code{.cpp}
        * struct MySystem {
        *   void OnInit() {
        *       // The event is called when the engine is initialized
        *   }
        * };
        * Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(&MySystem::OnInit);
        * @endcode
        * 
        * @tparam Event Type of event
        * @tparam Args Types of arguments
        * @param args Arguments for construct the event
        */
        template <typename Event, typename... Args>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void SignalEvent([[maybe_unused]] Args&&... args);

        /**
        * @brief Trigger an event for all listeners
        *
        * @tparam Event Type of event
        * @param event Event of signal (by reference)
        */
        template <typename Event>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void SignalEvent(Event& event);

        /**
        * @brief Push signal event in queue for call in next Engine tick
        *
        * @tparam Event Type of event
        * @tparam Args Types of arguments
        * @param args Arguments for construct the event or lvalue of event
        */
        template <typename Event, typename... Args>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void EnqueueSignal(Args&&... args);

        /**
        * @brief Stop listening to the event
        *
        * void OnInit() {
        *     // The event is called when the engine is initialized
        * }
        * 
        * Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Init, &OnInit>();
        * @endcode
        * 
        * @tparam Event Type of event
        * @tparam Callback Function
        */

        template <typename Event, auto Callback>
        requires Engine::RequiresCallback<Event, Callback, /* Member function */ false>
        static void UnsubscribeEvent();

        /**
        * @brief Stop listening to the event
        *
        * @code{.cpp}
        * struct MySystem {
        *   void OnInit() {
        *       // The event is called when the engine is initialized
        *   }
        * };
        * 
        * Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Init, &MySystem::OnInit>(this);
        * @endcode
        * 
        * @tparam Event Type of event
        * @tparam Callback Member function
        * @param instance Instance of object
        */
        template <typename Event, auto Callback>
        requires Engine::RequiresCallback<Event, Callback, /* Member function */ true>
        static void UnsubscribeEvent(typename Traits::Function<decltype(Callback)>::Class* instance);

    private:
        template <typename Event>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void SignalEvent(EventsPool<Delegate>& pool, Event& event);

        template <typename Event, auto Callback>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void SubscribeEvent(Delegate::Args<Event, Callback>, void* instance);

        template <typename Event, auto Callback>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void UnsubscribeEvent(Delegate::Args<Event, Callback>, void* instance);
    };
}

#include <Helena/Engine/Engine.ipp>

#endif // HELENA_ENGINE_ENGINE_HPP
