#pragma once

#ifndef HELENA_HPP
#define HELENA_HPP

#define HELENA_VERSION_MAJOR 1
#define HELENA_VERSION_MINOR 0
#define HELENA_VERSION_PATCH 0

// Helena Platform
#include <Helena/Platform.hpp>

// STL
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <ctime>
#include <clocale>
#include <optional>
#include <variant>
#include <functional>
#include <string>
#include <any>
#include <filesystem>
#include <vector>
#include <queue>
#include <array>
#include <unordered_map>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <ctime>
#include <type_traits>

// Dependencies
//#define SOL_ALL_SAFETIES_ON 1
//#include <sol/sol.hpp>

#include <safe_ptr/safe_ptr.h>
#define ENTT_ID_TYPE std::size_t
#include <entt/entt.hpp>
#include <pugixml/pugixml.hpp>
#include <robin_hood/robin_hood.h>

// Helena
#include <Helena/Format.hpp>
#include <Helena/Internal.hpp>
#include <Helena/Log.hpp>
#include <Helena/Assert.hpp>
#include <Helena/Hash.hpp>
#include <Helena/HashComparator.hpp>
#include <Helena/Util.hpp>
#include <Helena/Core.hpp>


#endif // HELENA_HPP
