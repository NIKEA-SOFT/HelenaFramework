#ifndef HELENA_TYPES_BASICLOGGER_HPP
#define HELENA_TYPES_BASICLOGGER_HPP

#include <Helena/Types/BasicLoggerDefines.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/ReferencePointer.hpp>

#include <array>
#include <locale>
#include <string>
#include <new>

namespace Helena::Log
{
    namespace Internal
    {
        template <typename Char>
        inline thread_local auto UniqueBuffer = Types::ReferencePointer<std::basic_string<Char>>::Create(1024, 0);

        template <typename Char>
        [[nodiscard]] static auto BufferSwitch() noexcept
        {
            if(UniqueBuffer<Char>) [[likely]] {
                return UniqueBuffer<Char>;
            }

            return decltype(UniqueBuffer<Char>)::Create(1024, 0);
        }
    }

    template <DefinitionLogger Logger, typename Char, typename... Args>
    void MessagePrint(const Formatter<Char> format, Args&&... args)
    {
        // No need to waste CPU time for formatting if the console is not available
        if(!HELENA_PLATFORM_HAS_CONSOLE()) {
            return;
        }

        // Ignore messages from logger when `static inline bool Muted = true`
        if constexpr(!requires { typename MuteController<Logger>::DefaultFingerprint; }) {
            if(MuteController<Logger>::Muted())
                return;
        }

        std::size_t offset{};
        auto& buffer = *Internal::BufferSwitch<Char>();
        buffer.resize(0);

        const auto fnFormatTo = [](auto inserter, const std::basic_string_view<Char> format, auto&&... args) {
            std::vformat_to(inserter, format, std::make_format_args<typename Print<Char>::Context>(args...));
        };

        Logger::Style.BeginColor(buffer);

        try {
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

            offset = buffer.size();
            fnFormatTo(std::back_inserter(buffer), format.Message(), std::forward<Args>(args)...);
        } catch(const std::format_error&) {
            buffer.resize(offset);
            fnFormatTo(std::back_inserter(buffer), Print<Char>::FormatError, format.Message());
        } catch(const std::bad_alloc&) {
            buffer.resize(offset);
            fnFormatTo(std::back_inserter(buffer), Print<Char>::AllocateError, format.Message());
        }

        Logger::Style.EndColor(buffer);
        buffer.push_back(Print<Char>::Endline);
        CustomPrint<Logger>::Message(buffer);
    }

    template <DefinitionLogger Logger, typename... Args>
    void Message(const Formatter<char> format, Args&&... args) {
        MessagePrint<Logger>(format, std::forward<Args>(args)...);
    }

    template <DefinitionLogger Logger, typename... Args>
    void Message(const Formatter<wchar_t> format, Args&&... args) {
        MessagePrint<Logger>(format, std::forward<Args>(args)...);
    }
}

#endif // HELENA_TYPES_BASICLOGGER_HPP