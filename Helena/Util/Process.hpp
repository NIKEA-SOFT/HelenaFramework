#ifndef HELENA_UTIL_PROCESS_HPP
#define HELENA_UTIL_PROCESS_HPP

#include <Helena/Platform/Defines.hpp>
#include <Helena/Traits/Function.hpp>
#include <Helena/Types/Allocators.hpp>
#include <Helena/Util/String.hpp>

#include <cstddef>
#include <chrono>
#include <thread>

namespace Helena::Util
{
    class Process
    {
        static inline auto m_ExecutablePath = []{
        #if defined(HELENA_PLATFORM_LINUX)
            static char path[PATH_MAX]{};
            auto length = ::readlink("/proc/self/exe", path, sizeof(path));
            length = length * !(length == -1 || length == sizeof(path));
        #elif defined(HELENA_PLATFORM_WIN)
            static char path[MAX_PATH]{};
            auto length = ::GetModuleFileNameA(nullptr, path, MAX_PATH);
        #else
            #error Unsupported platform
        #endif
            while(length--) {
                const auto notFound = !(path[length] == HELENA_SEPARATOR);
                if(path[length] = path[length] * notFound; !notFound) break;
            }
            return path;
        }();

    public:
        static const char* ExecutablePath() {
            return m_ExecutablePath;
        }

        static void Sleep(const std::uint64_t milliseconds) {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }

        template <typename Rep, typename Period>
        static void Sleep(const std::chrono::duration<Rep, Period>& time) {
            std::this_thread::sleep_for(time);
        }

        HELENA_NOINLINE
        static auto Stacktrace(std::size_t maxFrames = 64)
        {
            Types::StackAllocator<2048> memoryResource;
            Types::MemoryAllocator allocator{&memoryResource};
            auto stack = allocator.AllocateObjects<void*>(maxFrames);
            auto result = typename Traits::Function<decltype(&Util::String::FormatView<char>)>::Return{};

            static constexpr const char* header = "--- Stacktrace [frames: {}] ---\n";
            static constexpr const char* endchar[]{"\n", ""};

        #if defined(HELENA_PLATFORM_WIN)
            const auto frames = ::CaptureStackBackTrace(0, static_cast<DWORD>(maxFrames), stack, nullptr);
            constexpr auto symbolSize = sizeof(SYMBOL_INFO) + 256 * sizeof(TCHAR);
            const auto symbolMemory = allocator.AllocateBytes(symbolSize);
            auto symbol = new (symbolMemory) SYMBOL_INFO{
                .SizeOfStruct = sizeof(SYMBOL_INFO),
                .MaxNameLen = 255
            };

            const auto process = ::GetCurrentProcess();
            if(frames && ::SymInitialize(process, nullptr, TRUE))
            {
                result = Util::String::FormatView(header, frames - 1);
                for(std::remove_const_t<decltype(frames)> i = 0; i < frames; ++i)
                {
                    auto moduleHandle = HMODULE{};
                    char modulePath[MAX_PATH];

                    (void)::SymFromAddr(process, reinterpret_cast<DWORD64>(stack[i]), nullptr, symbol);
                    (void)::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        reinterpret_cast<LPCTSTR>(stack[i]), &moduleHandle);
                    auto length = ::GetModuleFileName(moduleHandle, modulePath, sizeof(modulePath));

                    // Extract module name
                    // Example: C:\HelenaTest\Helena.exe
                    // Result: Helena.exe
                    std::string_view moduleNameView{modulePath, length};
                    static constexpr std::string_view Separator = "\\/";
                    const auto position = moduleNameView.find_last_of(Separator);
                    const auto found = position != moduleNameView.npos;
                    const auto moduleName = moduleNameView.data() + position * found + found;

                    const auto isFinish = frames == i + 1;
                    result = Util::String::FormatView(
                    #if defined(HELENA_PROCESSOR_X86)
                        "{}{:#10x} | Module: {} | {}{}",
                    #else
                        "{}{:#018x} | Module: {} | {}{}",
                    #endif // HELENA_PROCESSOR_X86
                        result, reinterpret_cast<std::uintptr_t>(stack[i]),
                        moduleName, symbol->Name, endchar[isFinish]);
                }

                (void)::SymCleanup(process);
                std::destroy_at(symbol);
                allocator.FreeBytes(symbolMemory, symbolSize);
            }
        #elif defined(HELENA_PLATFORM_LINUX)
            const auto frames = ::backtrace(stack, static_cast<int>(maxFrames));
            const auto symbols = ::backtrace_symbols(stack, frames);

            result = Util::String::FormatView(header, frames);
            if(frames)
            {
                for(std::remove_const_t<decltype(frames)> i = 0; i < frames; ++i)
                {
                    std::string_view name{symbols[i]};
                    auto begin = name.find_first_of('(');
                    if(begin != name.npos) {
                        auto end = name.find_first_of(')', begin);
                        auto plus = name.find_first_of('+', begin);

                        if(plus != name.npos && plus < end) {
                            end = plus;
                            name = name.substr(begin + 1, end - begin - 1);
                            const_cast<char*>(name.data())[name.size()] = '\0';
                            if(name.empty()) {
                                name = "Unknown";
                            }
                        } else {
                            name = "Unknown";
                        }
                    }

                    const auto isFinish = frames == i + 1;
                    result = Util::String::FormatView(
                    #if defined(HELENA_PROCESSOR_X86)
                        "{}{:#10x} | {}{}",
                    #else
                        "{}{:#018x} | {}{}",
                    #endif // HELENA_PROCESSOR_X86
                        result, reinterpret_cast<std::uintptr_t>(stack[i]),
                        name.data(), endchar[isFinish]);
                }
            }
        #endif
            allocator.FreeObjects(stack, maxFrames);
            return result;
        }
    };
}

#endif // HELENA_UTIL_PROCESS_HPP