#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Platform/Platform.hpp>
#include <Helena/Traits/Conditional.hpp>
#if defined(HELENA_THREADSAFE_SYSTEMS)
    #include <Helena/Types/Spinlock.hpp>
#endif
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorUnique.hpp>
#include <Helena/Types/LocationString.hpp>
#include <Helena/Util/Sleep.hpp>
#include <Helena/Util/Function.hpp>

#include <atomic>
#include <cstring>
#include <functional>
#include <string>

namespace Helena
{
    class Engine final
    {
        //! Container key generator
        template <std::size_t Value> 
        struct IUniqueKey {};

        //! Unique key for storage systems type index
        using UKSystems = IUniqueKey<0>;

        //! Unique key for storage signals type index
        using UKSignals = IUniqueKey<1>;

        //! Unique key for storage messages type index
        using UKMessages = IUniqueKey<2>;

        template <typename Event, typename... Args>
        static constexpr auto RequiresCallback = Traits::Conditional<std::is_empty_v<Event>,
            Traits::Conditional<Traits::Arguments<Args...>::Orphan, std::true_type, std::false_type>,
            Traits::Conditional<Traits::Arguments<Args...>::Single && (Traits::SameAs<Event, Traits::RemoveCVRP<Args>> && ...),
                std::true_type, std::false_type>
        >::value && Traits::SameAs<Event, Traits::RemoveCVRP<Event>>;

        template <typename T>
        static constexpr auto RequiresConfig = requires {
            std::invocable<decltype(T::Sleep)>;
            std::convertible_to<decltype(T::Accumulate), std::uint32_t>;
        };

        //! Event callback storage with type erasure
        struct CallbackStorage
        {
            using Function = void (*)();
            using MemberFunction = void (CallbackStorage::*)();
            struct Storage
            {
                union alignas(16) {
                    Function m_Callback;
                    MemberFunction m_CallbackMember;
                };

                template <typename T>
                [[nodiscard]] T As() const noexcept
                {
                    T fn{};
                    if constexpr(std::is_member_function_pointer_v<T>) {
                        new (std::addressof(fn)) decltype(m_CallbackMember){m_CallbackMember};
                    } else {
                        new (std::addressof(fn)) decltype(m_Callback){m_Callback};
                    }
                    return fn;
                }
            };
            using Callback = void (*)(Storage, void*);

            template <typename Ret, typename... Args>
            CallbackStorage(Ret (*callback)(Args...), const Callback cb) : m_Callback{cb} {
                new (std::addressof(m_Storage)) decltype(callback){callback};
            }

            template <typename Ret, typename T, typename... Args>
            CallbackStorage(Ret (T::*callback)(Args...), const Callback cb) : m_Callback{cb} {
                static_assert(sizeof(callback) <= sizeof(Storage),
                    "The sizeof of member function exceeds the storage size.");
                new (std::addressof(m_Storage)) decltype(callback){callback};
            }

            CallbackStorage(const CallbackStorage& rhs) = default;
            CallbackStorage(CallbackStorage&& rhs) noexcept = default;
            CallbackStorage& operator=(const CallbackStorage& rhs) = default;
            CallbackStorage& operator=(CallbackStorage&& rhs) noexcept = default;

            template <typename Ret, typename... Args>
            CallbackStorage& operator=(Ret (*callback)(Args...)) noexcept {
                new (std::addressof(m_Storage)) decltype(callback){callback};
                return *this;
            }

            template <typename Ret, typename T, typename... Args>
            CallbackStorage& operator=(Ret (T::*callback)(Args...)) noexcept {
                static_assert(sizeof(callback) <= sizeof(Storage),
                    "The sizeof of member function exceeds the storage size.");
                new (std::addressof(m_Storage)) decltype(callback){callback};
                return *this;
            }

            template <typename Ret, typename... Args>
            [[nodiscard]] bool operator==(Ret (*callback)(Args...)) const noexcept {
                decltype(callback) fn{}; std::memcpy(std::addressof(fn), &m_Storage, sizeof(callback));
                return fn == callback;
            }

            template <typename Ret, typename T, typename... Args>
            [[nodiscard]] bool operator==(Ret (T::*callback)(Args...)) const noexcept {
                static_assert(sizeof(callback) <= sizeof(Storage),
                    "The sizeof of member function exceeds the storage size.");
                decltype(callback) fn{}; std::memcpy(std::addressof(fn), &m_Storage, sizeof(callback));
                return fn == callback;
            }

            template <typename Ret, typename... Args>
            [[nodiscard]] bool operator!=(Ret (*callback)(Args...)) const noexcept {
                return !(*this == callback);
            }

            template <typename Ret, typename T, typename... Args>
            [[nodiscard]] bool operator!=(Ret (T::*callback)(Args...)) const noexcept {
                return !(*this == callback);
            }

            Callback m_Callback;
            Storage m_Storage;
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
            static constexpr auto Sleep = Util::Function::BindFront(
                static_cast<void (*)(const std::uint64_t)>(Util::Sleep), 1 /* msec */);
            static constexpr auto Accumulate = 5;
        };

        //! Structure for control engine heartbeat behaviour
        static constexpr struct {} NoSignal{};

        //! Context for storage framework data
        class Context
        {
            template <typename T>
            using Pool = std::vector<T>;
            using SignalsPool = Pool<std::function<void ()>>;

            friend class Engine;
            struct ShutdownMessage {
                std::string m_Message;
                Types::SourceLocation m_Location;
            };

            static constexpr auto m_DefaultTickRate = 1. / 30.;

        public:
            Context() noexcept
                : m_Systems{}
                , m_Signals{}
                , m_DeferredSignals{}
                , m_ShutdownMessage{std::make_unique<ShutdownMessage>()}
                , m_TimeStart{GetTickTime()}
                , m_TimeNow{}
                , m_TimePrev{}
                , m_TickRate{m_DefaultTickRate}
                , m_TimeDelta{}
                , m_TimeElapsed{}
            #if defined(HELENA_THREADSAFE_SYSTEMS)
                , m_LockSystems{}
            #endif
                , m_State{EState::Undefined} {}

            virtual ~Context() {
                m_Signals.Clear();
                m_Systems.Clear();
            }

            Context(const Context&) = delete;
            Context(Context&&) noexcept = delete;
            Context& operator=(const Context&) = delete;
            Context& operator=(Context&&) noexcept = delete;

        private:
            virtual void Main() {}

        private:
            // Systems
            Types::VectorAny<UKSystems> m_Systems;

            // Signals
            Types::VectorUnique<UKSignals, Pool<CallbackStorage>> m_Signals;
            SignalsPool m_DeferredSignals;

            // Reason
            std::unique_ptr<ShutdownMessage> m_ShutdownMessage;

            // Timers for Heartbeat
            std::uint64_t m_TimeStart;
            std::uint64_t m_TimeNow;
            std::uint64_t m_TimePrev;

            double m_TickRate;
            double m_TimeDelta;
            double m_TimeElapsed;

        #if defined(HELENA_THREADSAFE_SYSTEMS)
            // Thread safe systems
            Types::Spinlock m_LockSystems;
        #endif

            // Engine state
            std::atomic<EState> m_State;
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
        requires std::constructible_from<T, Args...>
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
        [[nodiscard]] static auto ShutdownReason() noexcept;

        /**
        * @brief Register the system in the engine
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
        requires std::constructible_from<T, Args...>
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
        * Helena::Engine::RemoveSystem<MySystemA, MySystemB>();
        * @endcode
        * 
        * @tparam T Types of systems
        */
        template <typename... T>
        static void RemoveSystem();

        /**
        * @brief Listening to the event
        * 
        * @code{.cpp}
        * void OnInit() {
        *   // The event is called when the engine is initialized
        * }
        * 
        * Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(&OnInit);
        * @endcode
        * 
        * @tparam Event Type of event
        * @tparam Args Types of arguments
        * @param callback Callback function
        */
        template <typename Event, typename... Args>
        requires Engine::RequiresCallback<Event, Args...>
        static void SubscribeEvent(void (*callback)(Args...));

        /**
        * @brief Listening to the event
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
        * @tparam System Type of system
        * @tparam Args Types of events
        * @param callback Callback function
        */
        template <typename Event, typename System, typename... Args>
        requires Engine::RequiresCallback<Event, Args...>
        static void SubscribeEvent(void (System::*callback)(Args...));

        /**
        * @brief Returns the count or tuple with counts of listeners subscribed to Event
        *
        * @tparam Event Types of event
        * @return Count or tuple with counts of listeners subscribed to Event
        */
        template <typename... Event>
        requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
        [[nodiscard]]
        static decltype(auto) SubscribersEvent();

        /**
        * @brief Check has listeners subscribed to Events
        *
        * @tparam Event Type of event
        * @return True if listeners are subscribed to all of the listed Events
        */
        template <typename... Event>
        requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
        [[nodiscard]]
        static decltype(auto) HasSubscribersEvent();

        /**
        * @brief Check has any listeners are subscribed to any of the Events.
        *
        * @tparam Event Type of event
        * @return True if any listeners are subscribed to any of the listed Events
        */
        template <typename... Event>
        requires (Traits::SameAs<Event, Traits::RemoveCVRP<Event>> && ...)
        [[nodiscard]]
        static decltype(auto) AnySubscribersEvent();

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
        * @code{.cpp}
        * struct MySystem {
        *   void OnInit() {
        *       // The event is called when the engine is initialized
        *   }
        * };
        * 
        * Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Init>(&OnInit);
        * @endcode
        * 
        * @tparam Event Type of event
        * @tparam Args Types of arguments
        * @param callback Callback function
        */
        template <typename Event, typename... Args>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void UnsubscribeEvent(void (*callback)(Args...));

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
        * Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Init>(&MySystem::OnInit);
        * @endcode
        * 
        * @tparam Event Type of event
        * @tparam System Type of system
        * @tparam Args Types of arguments
        * @param callback Callback function
        */
        template <typename Event, typename System, typename... Args>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void UnsubscribeEvent(void (System::*callback)(Args...));

    private:
        template <typename Event, typename Callback, typename SignalFunctor>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void SubscribeEvent(Callback&& callback, SignalFunctor&& fn);

        template <typename Event, typename Comparator>
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void UnsubscribeEvent(Comparator&& comparator);
    };
}

#include <Helena/Engine/Engine.ipp>

#endif // HELENA_ENGINE_ENGINE_HPP
