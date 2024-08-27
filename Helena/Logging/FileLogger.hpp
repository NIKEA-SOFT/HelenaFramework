#ifndef HELENA_LOGGING_FILELOGGER_HPP
#define HELENA_LOGGING_FILELOGGER_HPP

#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Platform.hpp>
#include <Helena/Platform/Defines.hpp>
#include <Helena/Logging/ColorStyle.hpp>
#include <Helena/Logging/Print.hpp>
#include <Helena/Traits/Cacheline.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/VectorUnique.hpp>
#include <Helena/Types/Spinlock.hpp>
#include <Helena/Util/Process.hpp>
#include <Helena/Util/String.hpp>

#include <bit>
#include <cstdio>
#include <semaphore>
#include <list>
#include <queue>
#include <thread>
#include <memory>
#include <atomic>
#include <functional>

namespace Helena::Logging
{
    class FileLogger
    {
    protected:
        using FileCreateFn = FILE* (FileLogger::*)(std::size_t size);
        using FileHandle = std::unique_ptr<FILE, void (*)(FILE*)>;
        using Buffer = std::vector<std::byte>;
        using BufferView = std::basic_string_view<std::byte>;

        struct UniqueKey {};

        struct FileInfo {
            FileHandle m_Handle{nullptr, nullptr};
            std::size_t m_Size{};
            std::unique_ptr<char[]> m_Buffer{};
        };

        struct Task {
            Task(FileCreateFn fn, BufferView buffer)
                : m_File{fn}
                , m_Buffer{buffer} {}

            FileCreateFn m_File;
            BufferView m_Buffer;
        };

        struct DefaultPrefix {
            static constexpr auto Prefix = CreatePrefix("LOG");
            [[maybe_unused]] static constexpr auto Style = CreateStyle();
        };

        static constexpr std::size_t MessageBufferCapacity = 1 * 1024; // 1 KB
        static constexpr std::size_t FileBufferCapacity = 1 * 1024 * 1024; // 1 MB

    public:
        FileLogger(std::size_t maxFileSizeMB = 30)
            : m_Catalog{}
            , m_Buffers{}
            , m_LastTaskCount{}
            , m_FreeBuffers{}
            , m_Stop{}
            , m_Semaphore{0}
            , m_MaxFileSize{maxFileSizeMB * 1024 * 1024} {
            m_Thread = std::jthread{&FileLogger::RunWorker, this};
        }

        ~FileLogger()
        {
            bool isEmpty{};

        repeat:
            m_LockTasks.Lock();
            isEmpty = m_Tasks.empty();
            m_LockTasks.Unlock();

            if(!isEmpty) {
                Util::Process::SchedYield();
                goto repeat;
            }

            m_Stop = true;
            m_Semaphore.release();
        }

        FileLogger(const FileLogger&) = delete;
        FileLogger(FileLogger&&) noexcept = delete;
        FileLogger& operator=(const FileLogger&) = delete;
        FileLogger& operator=(FileLogger&&) noexcept = delete;

        void Catalog(std::string folderRelativeExe) {
            m_Catalog = std::move(folderRelativeExe);
            m_Catalog.erase(std::remove_if(std::begin(m_Catalog), std::end(m_Catalog), [](const auto c) {
                return c == '/' || c == '\\';
            }), m_Catalog.end());
        }
        
        template <DefinitionLogger Logger = DefaultPrefix, typename Char, typename Traits = std::char_traits<Char>>
        void Write(const std::basic_string_view<Char, Traits> message)
        {
            auto file = &FileLogger::CreateHandleIfNotExist<Logger>;
            auto buffer = CreateBuffer(message);

            {
                std::lock_guard lock{m_LockTasks};
                m_Tasks.emplace(file, buffer);
            }

            m_Semaphore.release();
        }

        template <typename Char, typename Traits = std::char_traits<Char>>
        void Write(const Char* message) {
            Write(std::basic_string_view<Char, Traits>{message});
        }

    protected:
        HELENA_NOINLINE
        static FileInfo CreateLogFile(std::string_view catalog, std::string_view prefix, std::size_t initSize)
        {
            using Prefix = decltype(prefix);
            typename Prefix::value_type buffer[64]{};

            HELENA_ASSERT(prefix.size() < std::size(buffer), "Prefix is too long");
            std::transform(prefix.begin(), prefix.end(), std::begin(buffer), [](const auto c) {
                return static_cast<Prefix::value_type>(tolower(c));
            });

            const auto exePath = Util::Process::ExecutablePath();
            const auto logsDir = Util::String::FormatView("{}{}{}{}", exePath, HELENA_SEPARATOR_QUOTED,
                catalog, catalog.empty() ? "" : HELENA_SEPARATOR_QUOTED);
            const auto dateTime = Types::DateTime::FromLocalTime();
            const auto fileName = Util::String::FormatView(
                "{}{}_{:04}{:02}{:02}_{:02}_{:02}_{:02}_{:02}.log", logsDir, buffer,
                dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
                dateTime.GetHours(), dateTime.GetMinutes(), dateTime.GetSeconds(),
                dateTime.GetMilliseconds());

        #if defined(HELENA_PLATFORM_WIN) && !defined(HELENA_COMPILER_MINGW)
            const auto lastError = ::GetLastError();
            (void)::CreateDirectoryA(logsDir.data(), nullptr);
            ::SetLastError(lastError);
            const auto file = _fsopen(fileName.data(), "wb", _SH_DENYWR);
        #else
            auto lastErrno = errno;
            (void)mkdir(logsDir.data(), 0755);
            errno = lastErrno;
            const auto file = std::fopen(fileName.data(), "wb");
        #endif // HELENA_PLATFORM_WINDOWS
            if(!file) {
                HELENA_MSG_ERROR("Failed to create log file: {}", fileName);
                return {};
            }

            return FileInfo {
                .m_Handle = FileHandle{file, +[](FILE* file) {
                    if(file) std::fclose(file);
                }},
                .m_Size = file ? initSize : 0,
                .m_Buffer = std::make_unique<char[]>(FileBufferCapacity)
            };
        }

        template <DefinitionLogger Logger>
        auto CreateHandleIfNotExist(std::size_t newSize) -> typename FileHandle::pointer
        {
            const auto fnCreateHandle = [this](std::size_t initSize) HELENA_NOINLINE {
                auto&& fileInfo = CreateLogFile(m_Catalog, Logger::Prefix, initSize);
                if(fileInfo.m_Handle) {
                    std::setvbuf(fileInfo.m_Handle.get(), fileInfo.m_Buffer.get(), _IOFBF, FileBufferCapacity);
                    m_Files.template Create<Logger>(std::forward<decltype(fileInfo)>(fileInfo));
                    return m_Files.template Get<Logger>().m_Handle.get();
                }
                
                return fileInfo.m_Handle.get();
            };

            const auto fnCloseHandle = [this]() HELENA_NOINLINE {
                m_Files.template Remove<Logger>();
            };

            // FileInfo already exist?
            if(const auto fileInfo = m_Files.template Ptr<Logger>()) [[likely]]
            {
                fileInfo->m_Size += newSize;
                if(fileInfo->m_Size < m_MaxFileSize) [[likely]] {
                    return fileInfo->m_Handle.get();
                }
                
                fnCloseHandle();
            }

            return fnCreateHandle(newSize);
        }

        template <typename Char, typename Traits = std::char_traits<Char>>
        BufferView CreateBuffer(const std::basic_string_view<Char, Traits> message)
        {
            m_LockTasks.Lock();
            auto tasks = m_Tasks.size();
            m_LockTasks.Unlock();
            
            // Calculate free buffers
            m_FreeBuffers += m_LastTaskCount - tasks;
            m_LastTaskCount = tasks + 1;

            // Take buffer from cache list or allocate new node
            auto bufferPtr = std::add_pointer_t<Buffer>{};
            if(m_FreeBuffers) [[unlikely]] {
                bufferPtr = std::addressof(m_Buffers.front());
                m_Buffers.splice(m_Buffers.end(), m_Buffers, m_Buffers.begin());
                --m_FreeBuffers;
            } else {
                bufferPtr = std::addressof(m_Buffers.emplace_back());
            }

            // Calculate size's
            const auto hasEndline = message.back() == Print<Char>::Endline;
            const auto inBytesSize = message.size() * sizeof(Char) + (!hasEndline * sizeof(Char));
            const auto inCharsSize = message.size() + !hasEndline;

            // Copy message to buffer of std::byte's
            const auto beginBytes = std::bit_cast<typename BufferView::const_pointer>(message.data());
            bufferPtr->reserve((std::max)(MessageBufferCapacity, inBytesSize));
            bufferPtr->assign(beginBytes, beginBytes + inBytesSize);

            // Append endline if message not contain them
            if(!hasEndline) {
                new (std::addressof(bufferPtr->back())) Char{Print<Char>::Endline};
            }

            // Remove color if contain
            std::remove_const_t<decltype(message)> tmp{std::bit_cast<Char*>(bufferPtr->data()), inCharsSize};
            ColorStyle::RemoveColor(tmp);
            return {std::bit_cast<typename BufferView::const_pointer>(tmp.data()), tmp.size() * sizeof(Char)};
        }

        void RunWorker()
        {
            while(!m_Stop)
            {
                m_Semaphore.acquire();
                m_LockTasks.Lock();

                // m_Tasks is empty when m_Stop = true
                const auto size = m_Tasks.size();
                if(!size) [[unlikely]] {
                    m_LockTasks.Unlock();
                    continue;
                }

                const auto [fileFn, buffer] = m_Tasks.front();
                m_LockTasks.Unlock();

                if(auto file = std::invoke(fileFn, this, buffer.size()))
                {
                    std::fwrite(buffer.data(), 1, buffer.size(), file);
                    if(size == 1) {
                        std::fflush(file);
                    }
                }

                m_LockTasks.Lock();
                if(!m_Tasks.empty()) [[likely]] {
                    m_Tasks.pop();
                }
                m_LockTasks.Unlock();
            }

            return;
        }

    protected:
        std::string m_Catalog;
        std::list<Buffer> m_Buffers;
        std::size_t m_LastTaskCount;
        std::size_t m_FreeBuffers;
        std::jthread m_Thread;
        bool m_Stop;

        alignas(Traits::Cacheline) Types::Spinlock m_LockTasks;
        std::counting_semaphore<> m_Semaphore;
        std::queue<Task> m_Tasks;

        alignas(Traits::Cacheline) Types::VectorUnique<UniqueKey, FileInfo> m_Files;
        std::size_t m_MaxFileSize;
    };
}

#endif // HELENA_LOGGING_FILELOGGER_HPP
