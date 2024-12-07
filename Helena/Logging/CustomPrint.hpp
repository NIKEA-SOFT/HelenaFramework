#ifndef HELENA_LOGGING_CUSTOMPRINT_HPP
#define HELENA_LOGGING_CUSTOMPRINT_HPP

#include <Helena/Logging/Print.hpp>

namespace Helena::Logging
{
    // Structure to override `Print<Char>::Show` behavior of specific logger using specialization
    template <DefinitionLogger>
    struct CustomPrint {
        // NOTE: Don't declare the given using in your own specializations (used for optimization)
        using DefaultFingerprint = void;

        template <typename Char>
        static void Message(std::basic_string_view<Char> message) {
            Print<Char>::Message(message);
        }
    };
}

#endif // HELENA_LOGGING_CUSTOMPRINT_HPP
