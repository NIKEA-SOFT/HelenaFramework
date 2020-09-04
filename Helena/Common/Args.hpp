#ifndef COMMON_HFARGSWRAPPER_HPP
#define COMMON_HFARGSWRAPPER_HPP

#include <Dependencies/args/args.hxx>

namespace Helena
{
    struct Args final {
        using Parser    = typename args::ArgumentParser;
        using Group     = typename args::Group;
        using Flag      = typename args::Flag;
        template <typename Type, typename Reader = args::ValueReader>
        using ValueFlag = typename args::ValueFlag<Type, Reader>;
        using HelpFlag  = typename args::HelpFlag;
        using Help      = typename args::Help;
        using Error     = typename args::Error;
    };
}

#endif