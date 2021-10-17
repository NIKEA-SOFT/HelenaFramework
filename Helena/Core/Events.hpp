#ifndef HELENA_CORE_CONTEXT_HPP
#define HELENA_CORE_CONTEXT_HPP

#include <array>
#include <vector>

namespace Helena
{
    enum class EStateCore : std::uint8_t 
    {
        Initialation,
        Execution,
        Finalizing,
        Shutdown
    };

	struct Context 
	{
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

	    EState m_State {};

	    std::string m_ShutdownReason;
	#if defined(HF_PLATFORM_WIN)
	    std::mutex m_ShutdownMutex {};
	    std::condition_variable m_ShutdownCondition {};
	#endif
	};
}

#endif // HELENA_CORE_CONTEXT_HPP