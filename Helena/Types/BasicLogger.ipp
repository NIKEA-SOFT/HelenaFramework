#ifndef HELENA_TYPES_BASICLOGGER_IPP
#define HELENA_TYPES_BASICLOGGER_IPP

#include <Helena/Platform/Assert.hpp>

namespace Helena::Types
{
    [[nodiscard]] inline bool BasicLogger::MakeColor(fmt::memory_buffer& buffer, const fmt::text_style style)
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

    inline void BasicLogger::EndColor(fmt::memory_buffer& buffer) noexcept {
        fmt::detail::reset_color<char>(buffer);
    }

    template <Helena::Traits::DefinitionLogger Logger, typename... Args>
    void BasicLogger::PrintConsole(const Log::Formater<Logger> format, [[maybe_unused]] Args&&... args)
    {
        HELENA_ASSERT(format.m_Location.m_File);
        HELENA_ASSERT(format.m_Location.m_Function);
        HELENA_ASSERT(format.m_Location.m_Line && format.m_Location.m_File[0] != '\0');

        constexpr auto formatex = fmt::string_view("[{:%Y.%m.%d %H:%M:%S}][{}:{}]{} ");
        const auto msg = fmt::string_view{format.m_Msg};
        const auto time = fmt::localtime(std::time(nullptr));
        bool has_style = false;

        fmt::memory_buffer buffer{};
        try {
            constexpr auto style = Logger::GetStyle();
            has_style = MakeColor(buffer, style);
            const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Logger::GetPrefix());
            fmt::detail::vformat_to(buffer, formatex, args1);
            const auto args2 = fmt::make_format_args(std::forward<Args>(args)...);
            fmt::detail::vformat_to(buffer, msg, args2);
        } catch(const fmt::format_error&) {
            buffer.clear();

            constexpr auto style = Log::Exception::GetStyle();
            has_style = MakeColor(buffer, style);
            const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Log::Exception::GetPrefix());
            fmt::detail::vformat_to(buffer, formatex, args1);
            fmt::detail::vformat_to(buffer, fmt::string_view{
                "\n----------------------------------------\n"
                "|| Error: format syntax invalid!\n"
                "|| Format: {}"
                "\n----------------------------------------"
                }, fmt::make_format_args(msg));
        } catch(const std::bad_alloc&) {
            buffer.clear();

            constexpr auto style = Log::Exception::GetStyle();
            has_style = MakeColor(buffer, style);
            const auto args1 = fmt::make_format_args(time, format.m_Location.GetFile(), format.m_Location.GetLine(), Log::Exception::GetPrefix());
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
}

#endif // HELENA_TYPES_BASICLOGGER_IPP