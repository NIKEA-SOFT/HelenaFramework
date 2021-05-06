#pragma once

#ifndef COMMON_HELENA_HPP
#define COMMON_HELENA_HPP

#define HELENA_VERSION_MAJOR 1
#define HELENA_VERSION_MINOR 0
#define HELENA_VERSION_PATCH 0

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <ctime>
#include <clocale>
#include <filesystem>
#include <vector>
#include <array>
#include <unordered_map>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <queue>
#include <optional>
#include <shared_mutex>
#include <functional>
#include <chrono>
#include <any>
#include <variant>
#include <ctime>
#include <type_traits>

//#define SOL_ALL_SAFETIES_ON 1
//#include <sol/sol.hpp>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/color.h>

#include <safe_ptr/safe_ptr.h>
#define ENTT_ID_TYPE std::size_t
#include <entt/entt.hpp>
#include <pugixml/pugixml.hpp>
#include <robin_hood/robin_hood.h>

#include <Common/Platform.hpp>
#include <Common/Hash.hpp>
#include <Common/Util.hpp>
#include <Common/Spinlock.hpp>
#include <Common/Core.hpp>

// Systems
#include <Common/Systems/EntityComponent.hpp>
#include <Common/Systems/ConfigManager.hpp>

#endif // COMMON_HELENA_HPP