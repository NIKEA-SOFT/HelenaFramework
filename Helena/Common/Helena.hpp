#pragma once

#ifndef COMMON_HELENA_HPP
#define COMMON_HELENA_HPP

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
#include <time.h>
#include <type_traits>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/color.h>

#include <safe_ptr/safe_ptr.h>
#include <entt/entt.hpp>
#include <pugixml/pugixml.hpp>

#include <Common/Platform.hpp>
#include <Common/Hash.hpp>
#include <Common/Util.hpp>
#include <Common/Spinlock.hpp>
#include <Common/Core.hpp>

// Systems
#include <Common/Systems/ECSystem.hpp>

#endif // COMMON_HELENA_HPP