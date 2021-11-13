#ifndef HELENA_ENGINE_CONTEXT_HPP
#define HELENA_ENGINE_CONTEXT_HPP

#include <Helena/Core/State.hpp>
#include <Helena/Core/Events.hpp>
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorUnique.hpp>

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
        friend class Helena::Engine;

        // EnTT type_seq need overload for using our shared memory indexer
        template <typename, typename>
        friend struct ENTT_API entt::type_seq;

        [[nodiscard]] static Context& GetInstance() noexcept {
            HELENA_ASSERT(m_Context, "Context not initilized");
            return *m_Context.get();
        }

        entt::id_type GetSequenceIndex(std::uint64_t index) {
            const auto [it, result] = m_TypeSequence.try_emplace(index, m_TypeSequence.size());
            return static_cast<entt::id_type>(it->second);
        }

    public:
        using Callback = std::function<void ()>;

        Context() noexcept : m_Tickrate{1.f / 30.f}, m_DeltaTime{}, m_TimeElapsed{}, m_TimeLeftFPS{}, m_CountFPS{}, m_State{EState::Undefined} {}
        virtual ~Context() = default;
        Context(const Context&) = delete;
        Context(Context&&) noexcept = delete;
        Context& operator=(const Context&) = delete;
        Context& operator=(Context&&) noexcept = delete;

        template <typename T = Context, typename... Args>
        requires std::is_base_of_v<Context, T> && std::is_constructible_v<T, Args...>
        [[nodiscard]] static void Initialize([[maybe_unused]] Args&&... args) {
            HELENA_ASSERT(!m_Context, "EngineContext already initialized!");
            m_Context = std::make_shared<T>(std::forward<Args>(args)...);
        }

        template <typename T = Context>
        requires std::is_base_of_v<Context, T>
        static void Initialize(const std::shared_ptr<T>& ctx) noexcept {
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
        static void SetAppName(std::string name) noexcept {
            auto& ctx = GetInstance();
            ctx.m_ApplicationName = std::move(name);
        }

        static std::string_view GetAppName() noexcept {
            const auto& ctx = GetInstance();
            return ctx.m_ApplicationName;
        }

        static void SetTickrate(float tickrate) noexcept {
            auto& ctx = GetInstance();
            ctx.m_Tickrate = 1.f / std::max(tickrate, 1.f);
        }

        static void SetCallback(Callback callback) noexcept {
            auto& ctx = GetInstance();
            ctx.m_Callback = std::move(callback);
        }

        [[nodiscard]] float GetTickrate() const noexcept {
            const auto& ctx = GetInstance();
            return ctx.m_Tickrate;
        }

        // Engine callback's
    private:
        std::unordered_map<std::uint64_t, entt::id_type> m_TypeSequence;     // Support EnTT type_seq across boundary

        Types::VectorAny<64> m_Systems;
        Types::VectorUnique<std::unique_ptr<Core::IEventPool>> m_Events;
        //std::vector<Core::IEventPool> m_Events;
        //Types::VectorAny<4> m_Events;

        Callback m_Callback;

        std::mutex  m_ShutdownMutex;
        std::string m_ShutdownReason;
        std::string m_ApplicationName;

        float m_Tickrate;
        float m_DeltaTime;

        float m_TimeElapsed;
        float m_TimeLeftFPS;

        std::chrono::steady_clock::time_point m_TimeStart;
        std::chrono::steady_clock::time_point m_TimeNow;
        std::chrono::steady_clock::time_point m_TimePrev;

        std::uint32_t m_CountFPS;

        EState m_State;


        inline static std::shared_ptr<Context> m_Context;   // Global context
    };
}

namespace entt 
{
    template <typename Type>
    struct ENTT_API type_seq<Type> 
    {
        [[nodiscard]] inline static id_type value() {
            static const auto value = Helena::Core::Context::GetInstance().GetSequenceIndex(Helena::Hash::Get<Type>());
            return value;
        }
    };
}

#endif // HELENA_ENGINE_CONTEXT_HPP