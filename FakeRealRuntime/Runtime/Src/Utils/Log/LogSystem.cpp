#include "Utils/Log/LogSystem.h"
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Platform/Memory.h"

namespace FakeReal
{
    LogSystem* LogSystem::s_Instance = nullptr;

    void LogSystem::Initialize()
    {
        void* ptr = calloc(1, sizeof(LogSystem));
        LogSystem::s_Instance = new (ptr) LogSystem();
    }

    void LogSystem::Finalize()
    {
        if (LogSystem::s_Instance)
        {
            LogSystem::s_Instance->~LogSystem();
            free(LogSystem::s_Instance);
            LogSystem::s_Instance = nullptr;
        }
    }

    LogSystem::LogSystem()
    {
        spdlog::init_thread_pool(8192, 1);

        auto stdout_sink = MakeShared<spdlog::sinks::stdout_color_sink_mt>();
        stdout_sink->set_level(spdlog::level::trace);
        stdout_sink->set_pattern("[%^%l%$] %v");

        std::vector<spdlog::sink_ptr> sinks{ stdout_sink };
        mLogger = MakeShared<spdlog::async_logger>("muggle_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

        mLogger->set_level(spdlog::level::trace);

        spdlog::register_logger(mLogger);
    }

    LogSystem::~LogSystem()
    {
        mLogger->flush();
        spdlog::drop_all();
    }
} // namespace FakeReal