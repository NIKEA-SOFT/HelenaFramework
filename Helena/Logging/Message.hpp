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

        std::size_t offset{};
        auto& buffer = Internal::GetCachedBuffer<Char>();
        const auto fnFormatTo = [](auto inserter, const std::basic_string_view<Char> format, auto&&... args) {
            std::vformat_to(inserter, format, std::make_format_args<typename Print<Char>::Context>(args...));
        };

        try {
            Logger::Style.template BeginColor<Char>(buffer);

            const auto fnFormatStyle = [&](const Char* file, const Char* prefix) {
                const auto dateTime = Types::DateTime::FromLocalTime();
                fnFormatTo(std::back_inserter(buffer), Print<Char>::FormatStyle, dateTime, prefix, file, format.Line());
            };

            // Convert Prefix and Location from const char* to wchar_t* using stack memory
            // The complexity of the conversion is approximately equal to the
            // length of the file name (where the log was called from) + the length of the prefix.
            // Average: 30 loop iterations + overhead of use_facet (mbrtowc)
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

                std::array<Char, 512> data;
                offset = fnConvert(data.data(), data.size(), format.File());
                fnConvert(data.data() + offset, data.size() - offset, Logger::Prefix.data());
                fnFormatStyle(data.data(), data.data() + offset);
            } else {
                fnFormatStyle(format.File(), Logger::Prefix.data());
            }

            offset = buffer.bytes();
            fnFormatTo(std::back_inserter(buffer), format.Message(), std::forward<Args>(args)...);
        } catch(const std::format_error&) {
            buffer.resize(offset);
            Exception::Style.template BeginColor<Char>(buffer);
            fnFormatTo(std::back_inserter(buffer), Print<Char>::FormatError, format.Message());
        } catch(const std::bad_alloc&) {
            buffer.resize(offset);
            Exception::Style.template BeginColor<Char>(buffer);
            fnFormatTo(std::back_inserter(buffer), Print<Char>::AllocateError, format.Message());
        }

        Logger::Style.template EndColor<Char>(buffer);
        if constexpr(Endline) {
            buffer.push_back(Print<Char>::Endline);
        }

        CustomPrint<Logger>::Message(buffer.template View<Char>());
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
