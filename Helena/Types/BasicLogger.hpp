#ifndef HELENA_TYPES_BASICLOGGER_HPP
#define HELENA_TYPES_BASICLOGGER_HPP

#include <Helena/Types/BasicLoggersDef.hpp>
#include <Helena/Platform/Assert.hpp>

namespace Helena::Types
{
    class BasicLogger
    {
        [[nodiscard]] static bool MakeColor(fmt::memory_buffer& buffer, const fmt::text_style style)
        {
            bool has_style{};
            if(style.has_emphasis()) {
                has_style = true;
                const auto emphasis = fmt::detail::make_emphasis<char>(style.get_emphasis());
                buffer.append(emphasis.begin(), emphasis.end());
            }

            if(style.has_foreground()) {
                has_style = true;
                const auto foreground = fmt::detail::make_foreground_color<char>(style.get_foreground());
                buffer.append(foreground.begin(), foreground.end());
            }

            if(style.has_background()) {
                has_style = true;
                const auto background = fmt::detail::make_background_color<char>(style.get_background());
                buffer.append(background.begin(), background.end());
            }

            return has_style;
        }

        static void EndColor(fmt::memory_buffer& buffer) noexcept {
            fmt::detail::reset_color<char>(buffer);
        }

    public:
        BasicLogger() = delete;
        ~BasicLogger() = delete;
        BasicLogger(const BasicLogger&) = delete;
        BasicLogger(BasicLogger&&) noexcept = delete;
        BasicLogger& operator=(const BasicLogger&) = delete;
        BasicLogger& operator=(BasicLogger&&) noexcept = delete;

        template <Traits::DefinitionLogger Logger, typename... Args>
        static void PrintConsole(const Log::Formater<Logger> format, [[maybe_unused]] Args&&... args)
        {
            HELENA_ASSERT(format.m_Location.GetFile());
            HELENA_ASSERT(format.m_Location.GetFunction());
            HELENA_ASSERT(format.m_Location.GetLine() && *format.m_Location.GetFile());

            constexpr auto formatex = fmt::string_view("[{:%Y.%m.%d %H:%M:%S}][{}:{}]{} ");
            const auto msg = fmt::string_view{format.m_Msg};
            const auto time = fmt::localtime(std::time(nullptr));
            bool has_style = false;

            fmt::memory_buffer buffer{};
            try {
                has_style = MakeColor(buffer, Logger::Style);
                const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Logger::Prefix);
                fmt::detail::vformat_to(buffer, formatex, args1);
                const auto args2 = fmt::make_format_args(std::forward<Args>(args)...);
                fmt::detail::vformat_to(buffer, msg, args2);
            } catch(const fmt::format_error&) {
                buffer.clear();
                has_style = MakeColor(buffer, Log::Exception::Style);
                const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Log::Exception::Prefix);
                fmt::detail::vformat_to(buffer, formatex, args1);
                fmt::detail::vformat_to(buffer, fmt::string_view{
                    "\n----------------------------------------\n"
                    "|| Error: format syntax invalid!\n"
                    "|| Format: {}"
                    "\n----------------------------------------"
                    }, fmt::make_format_args(msg));
            } catch(const std::bad_alloc&) {
                buffer.clear();
                has_style = MakeColor(buffer, Log::Exception::Style);
                const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Log::Exception::Prefix);
                fmt::detail::vformat_to(buffer, formatex, args1);
                fmt::detail::vformat_to(buffer, fmt::string_view{
                    "\n----------------------------------------\n"
                    "|| Error: alloc memory failed!\n"
                    "|| Format: {}"
                    "\n----------------------------------------"
                    }, fmt::make_format_args(msg));
            }

            if(has_style) {
                EndColor(buffer);
            }

            buffer.push_back('\n');
            buffer.push_back('\0');

            std::fputs(buffer.data(), stdout);
        }
    };
}

#endif // HELENA_TYPES_BASICLOGGER_HPP