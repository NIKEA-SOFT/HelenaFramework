#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Engine/Events.hpp>
#include <Helena/Engine/Log.hpp>
#include <Helena/Platform/Platform.hpp>
#include <Helena/Platform/Defines.hpp>
#include <Helena/Platform/Assert.hpp>
#include <Helena/Traits/Cacheline.hpp>
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorUnique.hpp>
#include <Helena/Types/LocationString.hpp>
#include <Helena/Types/Mutex.hpp>

#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
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
            };
            using Callback = void (*)(Storage&, void*);

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

            Storage m_Storage;
            Callback m_Callback;
        };
        
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

        protected:
            using Callback = std::function<void ()>;

            template <typename T = Context>
            static T& GetInstance() noexcept {
                HELENA_ASSERT(m_Context, "Context not initilized");
                return *static_cast<T*>(m_Context.get());
            }

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
                , m_State{EState::Undefined} {}

            ~Context() {
                m_Events.Clear();
                m_Systems.Clear();
            }

            Context(const Context&) = delete;
            Context(Context&&) noexcept = delete;
            Context& operator=(const Context&) = delete;
            Context& operator=(Context&&) noexcept = delete;

            /**
            * @brief Initialize context of Engine
            * @tparam T Context type
            * @tparam Args Types of arguments used to construct
            * @param args Arguments for context initialization
            * @note The context can be inherited
            */
            template <typename T = Context, typename... Args>
            requires std::is_base_of_v<Context, T> && std::is_constructible_v<T, Args...>
            static void Initialize([[maybe_unused]] Args&&... args) 
            {
                HELENA_ASSERT(!m_Context, "Context already initialized!");
                m_Context = std::make_shared<T>(std::forward<Args>(args)...);
                HELENA_ASSERT(m_Context, "Initialize Context failed!");

                if(!m_Context->Main())
                {
                    constexpr const auto message = "Initialize Main of Context: {} failed!";
                    if(m_Context->m_State.load(std::memory_order_acquire) != EState::Shutdown) {
                        Shutdown(message, Traits::NameOf<T>{});
                        return;
                    }

                    HELENA_MSG_FATAL(message, Traits::NameOf<T>{});
                }
            }

            /**
            * @brief Initialize the engine context for sharing between the executable and plugins
            * @tparam T Context type
            * @param ctx Context object for support shared memory and across boundary
            * @note This overload is used to share the context object between the executable and plugins
            */
            template <typename T = Context>
            requires std::is_base_of_v<Context, T>
            static void Initialize(const std::shared_ptr<T>& ctx) noexcept {
                HELENA_ASSERT(ctx, "Context is empty!");
                HELENA_ASSERT(!m_Context || (ctx && m_Context == ctx), "Context already initialized!");
                m_Context = ctx;
            }

            /**
            * @brief Return a context object
            * @tparam T Context type
            * @return Return a shared pointer to a context object
            */
            template <typename T = Context>
            requires std::is_base_of_v<Context, T>
            [[nodiscard]] static std::shared_ptr<T> Get() noexcept {
                HELENA_ASSERT(m_Context, "Context not initialized");
                return std::static_pointer_cast<T>(m_Context);
            }

            /**
            * @brief Set the update tickrate for the engine "Update" event
            * @param tickrate Update frequency
            * @note By default, 30 frames per second
            */
            static void SetTickrate(float tickrate) noexcept {
                auto& ctx = GetInstance();
                ctx.m_Tickrate = 1.f / (std::max)(tickrate, 1.f);
            }

            /**
            * @brief Returns the current engine tickrate
            * @return Tickrate in float
            */
            [[nodiscard]] static float GetTickrate() noexcept {
                const auto& ctx = GetInstance();
                return ctx.m_Tickrate;
            }

        private:
            virtual bool Main() { return true; }

        private:
            Types::VectorAny<UKSystems, Traits::Cacheline> m_Systems;
            Types::VectorUnique<UKEventStorage, std::vector<CallbackStorage>> m_Events;

            std::unique_ptr<ShutdownMessage> m_ShutdownMessage;

            std::uint64_t m_TimeStart;
            std::uint64_t m_TimeNow;
            std::uint64_t m_TimePrev;

            float m_Tickrate;
            float m_TimeDelta;
            float m_TimeElapsed;

            std::atomic<Engine::EState> m_State;

            inline static std::shared_ptr<Context> m_Context;
        };

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
        * @brief Heartbeat of the engine
        * 
        * @code{.cpp}
        * while(Helena::Engine::Heartbeat()) {}
        * @endcode
        * 
        * @return True if successful or false if an error is detected or called shutdown
        * @note 
        * You have to call heartbeat in a loop to keep the framework running
        * Use the definition of HELENA_ENGINE_NO SLEEP to prevent sleep by 1 ms
        * The thread will not sleep if your operations consume a lot of CPU time
        */
        [[nodiscard]] static bool Heartbeat();

        /**
        * @brief Check if the engine is currently running
        * @return True if running, false if shutdown or not initialized
        * @note This function is similar to calling GetState() == EState::Init;
        */
        [[nodiscard]] static bool Running() noexcept;

        /**
        * @brief Return the current state of the engine
        * @return EState state flag
        */
        [[nodiscard]] static EState GetState() noexcept;

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
        static void SignalEvent([[maybe_unused]] Args&&... args);

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
        static void UnsubscribeEvent(void (System::*callback)([[maybe_unused]] Args...));
    };
}

#include <Helena/Engine/Engine.ipp>

#endif // HELENA_ENGINE_ENGINE_HPP
