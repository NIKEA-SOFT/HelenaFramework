#ifndef HELENA_TRAITS_CONSTNESS_HPP
#define HELENA_TRAITS_CONSTNESS_HPP

#include <type_traits>

namespace Helena::Traits
{
    /**
     * @brief Transcribes the constness of a type to another type.
     * @tparam To The type to which to transcribe the constness.
     * @tparam From The type from which to transcribe the constness.
     */
    template<typename To, typename From>
    struct Constness {
        /*! @brief The type resulting from the transcription of the constness. */
        using type = std::remove_const_t<To>;
    };


    /*! @copydoc constness_as */
    template<typename To, typename From>
    struct Constness<To, const From> {
        /*! @brief The type resulting from the transcription of the constness. */
        using type = std::add_const_t<To>;
    };
}

#endif // HELENA_TRAITS_CONSTNESS_HPP