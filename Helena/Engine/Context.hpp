#ifndef HELENA_ENGINE_CONTEXT_HPP
#define HELENA_ENGINE_CONTEXT_HPP

#include <Helena/Engine/EState.hpp>
#include <Helena/Types/Delegate.hpp>
#include <Helena/Types/VectorAny.hpp>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace Helena {
    class Engine;
}

namespace Helena::Core
{
    class Context 
    {
        friend class Engine;

        [[nodiscard]] static Context& GetInstance() noexcept {
            HELENA_ASSERT(m_Context, "Context not initilized");
            return *m_Context.get();
        }

    public:
        Context() : m_Tickrate{1.f / 30.f}, m_DeltaTime{}, m_TimeElapsed{}, m_TimeLeftFPS{}, m_CountFPS{}, m_State{EState::Undefined} {}
        virtual ~Context() = default;
        Context(const Context&) = delete;
        Context(Context&&) noexcept = delete;
        Context& operator=(const Context&) = delete;
        Context& operator=(Context&&) noexcept = delete;

        template <typename T = Context, typename... Args>
        requires std::is_base_of_v<Context, T> && std::is_constructible_v<T, Args...>
        [[nodiscard]] static std::shared_ptr<T> Initialize([[maybe_unused]] Args&&... args) {
            HELENA_ASSERT(!m_Context, "EngineContext already initialized!");
            m_Context = std::make_shared<T>(std::forward<Args>(args)...);
            return std::static_pointer_cast<T>(m_Context);
        }

        template <typename T = Context>
        requires std::is_base_of_v<Context, T>
        [[nodiscard]] static void Initialize(const std::shared_ptr<T>& ctx) noexcept {
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
        void SetAppName(std::string name) {
            m_ApplicationName = std::move(name);
        }

        std::string_view GetAppName() const noexcept {
            return m_ApplicationName;
        }

        void SetTickrate(float tickrate) noexcept {
            m_Tickrate = 1.f / std::max(tickrate, 1.f);
        }

        // Engine callback's
        Types::Delegate<void ()> m_CallbackInit;        // Called when initialization engine
        Types::Delegate<void ()> m_CallbackTick;        // Called every tick
        Types::Delegate<void ()> m_CallbackUpdate;      // Called every fixed time
        Types::Delegate<void ()> m_CallbackShutdown;    // Called when shutdown

    private:
        Types::VectorAny<64> m_Systems;
        Types::VectorAny<4> m_Events;


        std::mutex  m_ShutdownMutex;
        std::string m_ShutdownReason;
        std::string m_ApplicationName;

        float m_Tickrate;
        float m_DeltaTime;

        float m_TimeElapsed;
        float m_TimeLeftFPS;
        std::uint32_t m_CountFPS;

        std::chrono::steady_clock::time_point m_TimeStart;
        std::chrono::steady_clock::time_point m_TimeNow;
        std::chrono::steady_clock::time_point m_TimePrev;

        EState m_State;


        inline static std::shared_ptr<Context> m_Context;   // Global context
    };
}

#endif // HELENA_ENGINE_CONTEXT_HPP