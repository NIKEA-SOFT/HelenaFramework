#ifndef __MODULE_MODULELOG_HPP__
#define __MODULE_MODULELOG_HPP__

#include <Helena/Common/HFModule.hpp>
#include <Helena/Common/HFUtil.hpp>

#include <Dependencies/spdlog/spdlog.h>

namespace Helena
{
    enum class ELogLevel : uint8_t {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

	constexpr const char* filename(std::string_view path = __FILE__) {
	    constexpr char symbols[]{'\\', '/'};
	    return path.substr(path.find_last_of(symbols, strlen(symbols)) + 1).data();
	}

    class ModuleLog final : public HFModule
    {
    public:
        template <typename... Args>
        void Log(std::string_view msg, Args... args) {
            spdlog::details::registry::instance().get_default_raw()->log(spdlog::source_loc{filename(), __LINE__, nullptr}, spdlog::level::info, msg, args...);
        }

    private:
        
    };
}
#endif // __MODULE_MODULELOG_HPP__