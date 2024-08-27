#ifndef HELENA_LOGGING_PRINT_HPP
#define HELENA_LOGGING_PRINT_HPP

#include <Helena/Logging/ColorStyle.hpp>
#include <cstdio>
#include <cwchar>
#include <format>

namespace Helena::Logging
{
    template <>
    struct Print<char>
    {
        using Context = std::format_context;

        static constexpr auto FormatStyle = "[{}][{}][{}:{}] ";
        static constexpr auto AllocateError =
            "\n----------------------------------------\n"
            "|| Error: alloc memory failed!\n"
            "|| Format: {}"
            "\n----------------------------------------";
        static constexpr auto FormatError =
            "\n----------------------------------------\n"
            "|| Error: format syntax invalid!\n"
            "|| Format: {}"
            "\n----------------------------------------";
        static constexpr auto Endline = '\n';

        static void Message(const std::string_view message) {
            (void)std::fputs(message.data(), stdout);
        }
    };

    template <>
    struct Print<wchar_t>
    {
        using Context = std::wformat_context;

        static constexpr auto FormatStyle = L"[{}][{}][{}:{}] ";
        static constexpr auto AllocateError =
            L"\n----------------------------------------\n"
            L"|| Error: alloc memory failed!\n"
            L"|| Format: {}"
            L"\n----------------------------------------";
        static constexpr auto FormatError =
            L"\n----------------------------------------\n"
            L"|| Error: format syntax invalid!\n"
            L"|| Format: {}"
            L"\n----------------------------------------";
        static constexpr auto Endline = L'\n';

        static void Message(const std::wstring_view message) {
            (void)std::fputws(message.data(), stdout);
        }
    };
}

#endif // HELENA_LOGGING_PRINT_HPP
