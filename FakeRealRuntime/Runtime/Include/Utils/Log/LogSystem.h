#pragma once
#define FMT_CONSTEVAL //clang in c++2020 bug
#include <cstdint>
#include "spdlog/spdlog.h"
#include <stdexcept>
#include "Utils/BaseDefine.h"
#include "Platform/Config.h"

namespace FakeReal
{
    enum class LogLevel : uint8_t
    {
        debug,
        info,
        warning,
        error,
        fatal
    };
    class RUNTIME_API LogSystem
    {
    public:
        LogSystem();
        ~LogSystem();
    public:
        template <typename... Args>
        void Log(LogLevel level, Args&&... args)
        {
            switch (level)
            {
                case LogLevel::debug:
                    mLogger->debug(std::forward<Args>(args)...);
                    break;
                case LogLevel::info:
                    mLogger->info(std::forward<Args>(args)...);
                    break;
                case LogLevel::warning:
                    mLogger->warn(std::forward<Args>(args)...);
                    break;
                case LogLevel::error:
                    mLogger->error(std::forward<Args>(args)...);
                    break;
                case LogLevel::fatal:
                    mLogger->critical(std::forward<Args>(args)...);
                    FatalCallback(std::forward<Args>(args)...);
                    break;
                default:
                    break;
            }
        }

        template <typename... Args>
        void FatalCallback(Args&&... args)
        // void FatalCallback(fmt::format_string<Args...> s, Args&&... args)
        {
            const std::string format_str = fmt::format(std::forward<Args>(args)...);
            throw std::runtime_error(format_str);
        }

    private:
        SharedPtr<spdlog::logger> mLogger;
    };

    template <typename... Args>
    RUNTIME_API void PrintLog(LogLevel level, Args&&... args);
} // namespace FakeReal