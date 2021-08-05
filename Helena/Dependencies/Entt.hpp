#pragma once

#include <Helena/Assert.hpp>

#ifdef ENTT_ASSERT
    #undef ENTT_ASSERT
    #define ENTT_ASSERT HF_ASSERT
#endif

#define ENTT_ID_TYPE std::uint32_t

#include <entt/entt.hpp>
