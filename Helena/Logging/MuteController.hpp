#ifndef HELENA_LOGGING_MUTECONTROLLER_HPP
#define HELENA_LOGGING_MUTECONTROLLER_HPP

namespace Helena::Logging
{
    template <DefinitionLogger>
    struct MuteController {
        // NOTE: Don't declare the given using in your own specializations (used for optimization)
        using DefaultFingerprint = void;

        static bool Muted() {
            return false;
        }
    };
}

#endif // HELENA_LOGGING_MUTECONTROLLER_HPP
