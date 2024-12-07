#ifndef HELENA_LOGGING_MESSAGE_HPP
#define HELENA_LOGGING_MESSAGE_HPP

#include <Helena/Platform/Platform.hpp>
#include <Helena/Logging/Internal/LoggerBuffer.hpp>
#include <Helena/Logging/ColorStyle.hpp>
#include <Helena/Logging/CustomPrint.hpp>
#include <Helena/Logging/Formatter.hpp>
#include <Helena/Logging/MuteController.hpp>
#include <Helena/Logging/Print.hpp>
#include <Helena/Logging/Prefix.hpp>
#include <Helena/Types/DateTime.hpp>
#include <locale>
#include <utility>

namespace Helena::Logging
{
    template <DefinitionLogger Logger, bool Endline, typename Char, typename... Args>
    void Message(const Formatter<Char> format, Args&&... args)
    {
        // No need to waste CPU time for formatting if the console is not available
        [[maybe_unused]] const auto hasConsole = HELENA_PLATFORM_HAS_CONSOLE();
        if constexpr(requires { typename CustomPrint<Logger>::DefaultFingerprint; }) {
            if(!hasConsole) {
                return;
            }
        }

        // Ignore messages from logger when `static inline bool Muted = true`
        if constexpr(!requires { typename MuteController<Logger>::DefaultFingerprint; }) {
            if(MuteController<Logger>::Muted())
                return;
        }

        std::size_t resultOffset{};
        auto& buffer = Internal::GetCachedBuffer<Char>();
        const auto fnFormatTo =
        [&out = buffer](const std::string_view file, const auto style, const std::basic_string_view<Char> fmt, auto&&... args)
        {
            static constexpr auto formatLength = std::char_traits<Char>::length(Print<Char>::FormatStyle);
            [[maybe_unused]] struct Uninitialized {
                Uninitialized() = default;
                Char data[512];
            } tmp;
            const Char* formatFile{};
            const Char* formatPrefix{};
            if constexpr(!std::is_same_v<Char, char>)
            {
                const auto fnConvert = [&facet = std::use_facet<std::ctype<Char>>(std::locale())]
                (Char* data, std::size_t size, const char* src) {
                    std::size_t i{};
                    while(i < size && src[i] != '\0') {
                        data[i] = facet.widen(src[i]); ++i;
                    }
                    data[i] = facet.widen('\0');
                    return ++i;
                };

                static constexpr auto tmpCapacity = sizeof(Uninitialized::data) / sizeof(Uninitialized::data[0]);
                const auto offset = fnConvert(tmp.data, tmpCapacity, file.data());
                fnConvert(tmp.data + offset, tmpCapacity - offset, Logger::Prefix.data());
                formatFile = tmp.data;
                formatPrefix = tmp.data + offset;
            } else {
                formatFile = file.data();
                formatPrefix = Logger::Prefix.data();
            }

            out.append(Print<Char>::FormatStyle, formatLength);
            out.append(fmt.data(), fmt.size());

            std::size_t result = out.template size<Char>();
            const auto formatView = out.template View<Char>();
            const auto formatDateTime = Types::DateTime::FromLocalTime();
            out.push_back(Char{});
            style.template BeginColor<Char>(out);
            std::vformat_to(std::back_inserter(out), formatView,
                std::make_format_args<typename Print<Char>::Context>(formatDateTime, formatPrefix,
                    formatFile, args...));
            style.template EndColor<Char>(out);

            return result + 1;
        };

        try {
            resultOffset = fnFormatTo(format.File(), Logger::Style, format.Message(),
                format.Line(), std::forward<Args>(args)...);
        } catch(const std::format_error&) {
            resultOffset = fnFormatTo(format.File(), Exception::Style, Print<Char>::FormatError,
                format.Line(), format.Message());
        } catch(const std::bad_alloc&) {
            resultOffset = fnFormatTo(format.File(), Exception::Style, Print<Char>::AllocateError,
                format.Line(), format.Message());
        }

        if constexpr(Endline) {
            buffer.push_back(Print<Char>::Endline);
        }

        auto view = std::basic_string_view<Char>(
            buffer.template data<Char>() + resultOffset,
            buffer.template size<Char>() - resultOffset);
        buffer.push_back(Char{});
        CustomPrint<Logger>::Message(view);
    }

    template <DefinitionLogger Logger, bool Endline = true, typename... Args>
    void Message(const Formatter<char> format, Args&&... args) {
        Message<Logger, Endline, char>(format, std::forward<Args>(args)...);
    }

    template <DefinitionLogger Logger, bool Endline = true, typename... Args>
    void Message(const Formatter<wchar_t> format, Args&&... args) {
        Message<Logger, Endline, wchar_t>(format, std::forward<Args>(args)...);
    }
}

#endif // HELENA_LOGGING_MESSAGE_HPP
