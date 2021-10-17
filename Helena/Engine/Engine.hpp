#ifndef HELENA_ENGINE_ENGINE_HPP
#define HELENA_ENGINE_ENGINE_HPP

#include <Helena/Debug/Assert.hpp>
#include <Helena/Types/Function.hpp>
#include <Helena/Types/Delegate.hpp>
#include <Helena/Util/Format.hpp>

#include <memory>
#include <atomic>
#include <algorithm>
#include <mutex>

namespace Helena
{
    enum class EStateEngine : std::uint8_t 
    {
        Undefined,
        Init,
        Shutdown
    };

    class EngineContext 
    {
        friend class Engine;

        EngineContext() : m_Tickrate{1.f / 30.f}, m_DeltaTime{}, m_State{EStateEngine::Undefined} {}

        [[nodiscard]] static EngineContext* GetInstance() noexcept {
            HELENA_ASSERT(m_Context, "EngineContext not initilized");
            return m_Context.get();
        }

        void SetState(EStateEngine state) noexcept {
            m_State = state;
        }

        [[nodiscard]] EStateEngine GetState() noexcept {
            return m_State;
        }

        [[nodiscard]] std::string_view GetShutdownReason() noexcept {
            return std::string_view{m_ShutdownReason};
        }

    public:
        template <typename T = EngineContext, typename... Args>
        [[nodiscard]] static std::shared_ptr<EngineContext> CreateContext([[maybe_unused]] Args&&... args) {
            HELENA_ASSERT(!m_Context, "EngineContext already initialized!");
            m_Context = std::make_shared<T>(std::forward<Args>(args)...);
        }

        [[nodiscard]] static std::shared_ptr<EngineContext> CreateContext(const std::shared_ptr<EngineContext>& ctx) noexcept {
            HELENA_ASSERT(!m_Context, "EngineContext already initialized!");
            m_Context = ctx;
        }

        [[nodiscard]] static std::shared_ptr<EngineContext> GetContext() noexcept {
            HELENA_ASSERT(!m_Context, "EngineContext already initialized!");
            return m_Context;
        }

        virtual ~EngineContext() = default;
        EngineContext(const EngineContext&) = delete;
        EngineContext(EngineContext&&) noexcept = delete;
        EngineContext& operator=(const EngineContext&) = delete;
        EngineContext& operator=(EngineContext&&) noexcept = delete;

        void SetTickrate(float tickrate) noexcept {
            m_Tickrate = std::min(tickrate, 1.f);
        }

        // Engine callback's
        Types::Function<32, void ()> m_CallbackInit;        // Called when initialization engine
        Types::Function<32, void ()> m_CallbackTick;        // Called every tick
        Types::Function<32, void ()> m_CallbackUpdate;      // Called every fixed time tick time
        Types::Function<32, void ()> m_CallbackShutdown;    // Called after cbFinish

    private:
        std::mutex  m_ShutdownMutex;
        std::string m_ShutdownReason;

        float m_Tickrate;
        float m_DeltaTime;

        std::chrono::steady_clock::time_point m_TimeStart;
        std::chrono::steady_clock::time_point m_TimeNow;
        std::chrono::steady_clock::time_point m_TimePrev;

        EStateEngine m_State;

        inline static std::shared_ptr<EngineContext> m_Context;     // Global context
    };

    class Engine final
    {

    public:
        [[nodiscard]] static bool Heartbeat() 
        {
            const auto instance = EngineContext::GetInstance();
            const auto state = instance->GetState();

            switch(state) 
            {
                case EStateEngine::Undefined:  
                {
                    instance->SetState(EStateEngine::Init);
                    if(instance->m_CallbackInit) {
                        instance->m_CallbackInit();
                    }

                } break;

                case EStateEngine::Init: {
                    HELENA_ASSERT(instance->m_CallbackTick, "Tick callback is empty in EngineContext");
                    HELENA_ASSERT(instance->m_CallbackUpdate, "Update callback is empty in EngineContext");

                    instance->m_CallbackTick();

                    instance->m_CallbackUpdate();

                } break;

                case EStateEngine::Shutdown: 
                {
                    instance->SetState(EStateEngine::Undefined);

                    if(const auto reason = instance->GetShutdownReason(); !reason.empty()) {
                        HELENA_MSG_FATAL("Shutdown HelenaFramework Engine, reason: {}", reason);
                    } else {
                        HELENA_MSG_DEFAULT("Shutdown HelenaFramework Engine");
                    }

                } return false;
            }

            return true;
        }

        [[nodiscard]] static EStateEngine GetState() noexcept {
            return EngineContext::GetInstance()->GetState();
        }

        [[nodiscard]] static bool Running() noexcept {
            return EngineContext::GetInstance()->GetState() == EStateEngine::Init;
        }

        template <typename... Args>
        static void Shutdown(const std::string_view format, Args&&... args) 
        {
            const auto instance = EngineContext::GetInstance();
            const auto state = instance->GetState();
            const std::lock_guard lock{instance->m_ShutdownMutex};

            if(state != EStateEngine::Shutdown) {
                instance->m_ShutdownReason = Util::Format(format, std::forward<Args>(args)...);
                instance->SetState(EStateEngine::Shutdown);
            }
        }

    private:
    };
}

#include <Helena/Engine/Engine.ipp>

#endif // HELENA_ENGINE_ENGINE_HPP
