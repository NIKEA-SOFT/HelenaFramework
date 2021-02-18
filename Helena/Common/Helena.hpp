#pragma once

#ifndef COMMON_HELENA_HPP
#define COMMON_HELENA_HPP

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <functional>
#include <chrono>
#include <type_traits>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/color.h>

//#include <safe_ptr/safe_ptr.h>
#include <entt/entt.hpp>
#include <pugixml/pugixml.hpp>
#include <lua/lua.hpp>

#include <Common/Platform.hpp>
#include <Common/Hash.hpp>
#include <Common/Spinlock.hpp>
#include <Common/Util.hpp>
//#include <Common/ContextManager.hpp>
#include <Common/Core.hpp>
#include <Common/ResourceManager.hpp>
//#include <Common/PluginManager.hpp>

#endif // COMMON_HELENA_HPP