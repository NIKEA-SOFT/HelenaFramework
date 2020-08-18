#ifndef __COMMON_HFHASH_HPP__
#define __COMMON_HFHASH_HPP__

#include <string_view>
#include <utility>
#include <unordered_map>

namespace Helena
{
    /*! @brief Heterogeneous lookup hasher */
    struct HFStringHash {
        using is_transparent = void;
        using hash_type = std::hash<std::string_view>;
        std::size_t operator()(const std::string& key) const { return hash_type{}(key); }
        std::size_t operator()(std::string_view key) const { return hash_type{}(key); }
        std::size_t operator()(const char* key) const { return hash_type{}(key); }
    };
}

#endif // __COMMON_HFHASH_HPP__