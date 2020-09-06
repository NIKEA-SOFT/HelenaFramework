#ifndef COMMON_HASH_HPP__
#define COMMON_HASH_HPP__

#include <string>
#include <type_traits>

namespace Helena
{
    /*! @brief Heterogeneous lookup hasher */
    struct StringHash {
        using is_transparent = void;
        using hash_type = std::hash<std::string_view>;
        std::size_t operator()(const std::string& key) const { return hash_type{}(key); }
        std::size_t operator()(std::string_view key) const { return hash_type{}(key); }
        std::size_t operator()(const char* key) const { return hash_type{}(key); }
    };
}

#endif // COMMON_HASH_HPP__