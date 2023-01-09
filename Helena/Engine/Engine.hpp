#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Engine/Events.hpp>
#include <Helena/Platform/Platform.hpp>
#include <Helena/Platform/Defines.hpp>
#include <Helena/Platform/Assert.hpp>
#include <Helena/Traits/Cacheline.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/SameAs.hpp>
#if defined(HELENA_THREADSAFE_SYSTEMS)
    #include <Helena/Types/Spinlock.hpp>
#endif
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorUnique.hpp>
#include <Helena/Types/LocationString.hpp>

#include <algorithm>
#include <atomic>
#include <memory>
#include <functional>
#include <string>
#include <utility>

namespace Helena
{
    class Engine final
    {
        //! Container key generator
        template <std::size_t Value> 
        struct IUniqueKey {};

        //! Unique key for storage events type index
        using UKEventStorage = IUniqueKey<0>;

        //! Unique key for storage systems type index
        using UKSystems = IUniqueKey<1>;

        //! Event callback storage with type erasure
        struct CallbackStorage
        {
            using Function = void (*)();
            using MemberFunction = void (CallbackStorage::*)();
            union alignas(16) Storage {
                Function m_Callback;
                MemberFunction m_CallbackMember;

                template <typename T>
                [[nodiscard]] T As() const noexcept
                {
                    T fn{};
                    if constexpr(std::is_member_function_pointer_v<T>) {
                        new (&fn) decltype(m_CallbackMember){m_CallbackMember};
                    } else {
                        new (&fn) decltype(m_Callback){m_Callback};
                    }
                    return fn;
                }
            };
            using Callback = void (*)(const Storage&, void* const);

            template <typename Ret, typename... Args>
            CallbackStorage(Ret (*callback)(Args...), Callback cb) : m_Callback{cb} {
                new (&m_Storage) decltype(callback){callback};
            }

            template <typename Ret, typename T, typename... Args>
            CallbackStorage(Ret (T::*callback)(Args...), Callback cb) : m_Callback{cb} {
                new (&m_Storage) decltype(callback){callback};
            }

            CallbackStorage(const CallbackStorage& rhs) = default;
            CallbackStorage(CallbackStorage&& rhs) noexcept = default;
            CallbackStorage& operator=(const CallbackStorage& rhs) = default;
            CallbackStorage& operator=(CallbackStorage&& rhs) noexcept = default;

            template <typename Ret, typename... Args>
            CallbackStorage& operator=(Ret (*callback)(Args...)) noexcept {
                new (&m_Storage) decltype(callback){callback};
                return *this;
            }

            template <typename Ret, typename T, typename... Args>
            CallbackStorage& operator=(Ret (T::*callback)(Args...)) noexcept {
                new (&m_Storage) decltype(callback){callback};
                return *this;
            }

            template <typename Ret, typename... Args>
            [[nodiscard]] bool operator==(Ret (*callback)(Args...)) const noexcept {
                decltype(callback) fn{}; std::memcpy(&fn, &m_Storage, sizeof(callback));
                return fn == callback;
            }

            template <typename Ret, typename T, typename... Args>
            [[nodiscard]] bool operator==(Ret (T::*callback)(Args...)) const noexcept {
                decltype(callback) fn{}; std::memcpy(&fn, &m_Storage, sizeof(callback));
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

        //! Context for storage framework data
        class Context
        {
            friend class Engine;
            struct ShutdownMessage {
                std::string m_Message;
                Types::SourceLocation m_Location;
            };

            static constexpr auto DefaultTickrate = 1.f / 30.f;

        public:
            Context() noexcept
                : m_Systems{}
                , m_Events{}
                , m_ShutdownMessage{std::make_unique<ShutdownMessage>()}
                , m_TimeStart{GetTickTime()}
                , m_TimeNow{}
                , m_TimePrev{}
                , m_Tickrate{DefaultTickrate}
                , m_TimeDelta{}
                , m_TimeElapsed{}
            #if defined(HELENA_THREADSAFE_SYSTEMS)
                , m_LockSystems{}
            #endif
                , m_State{EState::Undefined} {}

            virtual ~Context() {
                m_Events.Clear();
                m_Systems.Clear();
            }

            Context(const Context&) = delete;
            Context(Context&&) noexcept = delete;
            Context& operator=(const Context&) = delete;
            Context& operator=(Context&&) noexcept = delete;

        private:
            virtual bool Main() { return true; }

        private:
            Types::VectorAny<UKSystems, Traits::Cacheline> m_Systems;
            Types::VectorUnique<UKEventStorage, std::vector<CallbackStorage>> m_Events;

            std::unique_ptr<ShutdownMessage> m_ShutdownMessage;

            std::uint64_t m_TimeStart;
            std::uint64_t m_TimeNow;
            std::uint64_t m_TimePrev;

            double m_Tickrate;
            double m_TimeDelta;
            double m_TimeElapsed;

        #if defined(HELENA_THREADSAFE_SYSTEMS)
            Types::Spinlock m_LockSystems;
        #endif
            std::atomic<Engine::EState> m_State;
        };

    private:
        using ContextDeleter = void (*)(Context*);
        using ContextStorage = std::unique_ptr<Context, ContextDeleter>;
        inline static ContextStorage m_Context{nullptr, nullptr};

        static void InitContext(ContextStorage context) noexcept;
        [[nodiscard]] static bool HasContext() noexcept;
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
        * @param sleepMS Sleep time in milliseconds
        * @param accumulator Count of steps to reduce accumulated time in Update events
        * @code{.cpp}
        * while(Helena::Engine::Heartbeat()) {}
        * @endcode
        * 
        * @return True if successful or false if an error is detected or called shutdown
        * @note 
        * - Heartbeat: You have to call heartbeat in a loop to keep the framework running
        * Use the definition of HELENA_ENGINE_NO SLEEP to prevent sleep.
        * The thread will not sleep if your operations consume a lot of CPU time
        * - Accumulator: if your loop is too busy, then there may be an accumulation of delta time
        * that cannot be repaid by a single Update call, which will cause more Update calls to
        * follow immediately to reduce the accumulated time.
        * It is not recommended to use a large value, your thread may get stuck in a loop.
        * The correct solution is to offload the thread by finding a performance bottleneck.
        */
        [[nodiscard]] static bool Heartbeat(std::size_t sleepMS = 1, std::uint8_t accumulator = 5);

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
        static void RegisterSystem([[maybe_unused]] Args&&... args);

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
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void SubscribeEvent(void (*callback)([[maybe_unused]] Args...));

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
        requires Traits::SameAs<Event, Traits::RemoveCVRP<Event>>
        static void SubscribeEvent(void (System::*callback)([[maybe_unused]] Args...));

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
        static void UnsubscribeEvent(void (*callback)([[maybe_unused]] Args...));

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
        static void UnsubscribeEvent(void (System::*callback)([[maybe_unused]] Args...));

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
