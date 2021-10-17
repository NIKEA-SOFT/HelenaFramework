#ifndef HELENA_HPP
#define HELENA_HPP

// Version
#define HELENA_VERSION_MAJOR 1
#define HELENA_VERSION_MINOR 0
#define HELENA_VERSION_PATCH 0

// Helena Platform
#include <Helena/Platform/Platform.hpp>

#ifdef HELENA_STANDARD_CPP20
    //// STL
    //#include <iostream>
    //#include <cstdint>
    //#include <algorithm>
    //#include <ctime>
    //#include <clocale>
    //#include <optional>
    //#include <variant>
    //#include <functional>
    //#include <string>
    //#include <any>
    //#include <filesystem>
    //#include <vector>
    //#include <queue>
    //#include <array>
    //#include <unordered_map>
    //#include <map>
    //#include <memory>
    //#include <mutex>
    //#include <shared_mutex>
    //#include <chrono>
    //#include <ctime>
    //#include <type_traits>
    //#include <charconv>

    // Helena
    //#include <Helena/Engine/Engine.hpp>
#else 
    #error HelenaFramework does not support cpp standard below 20
#endif

#endif // HELENA_HPP
