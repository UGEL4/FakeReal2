#include "FRPch.h"
#include "LogSystem.h"
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace FakeReal
{
	LogSystem::LogSystem()
	{
		spdlog::init_thread_pool(8192, 1);

		auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
		stdout_sink->set_level(spdlog::level::trace);
		stdout_sink->set_pattern("[%^%l%$] %v");

		std::vector<spdlog::sink_ptr> sinks{ stdout_sink };
		m_pLogger = std::make_shared<spdlog::async_logger>("muggle_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

		m_pLogger->set_level(spdlog::level::trace);

		spdlog::register_logger(m_pLogger);
	}

	LogSystem::~LogSystem()
	{
		m_pLogger->flush();
		spdlog::drop_all();
	}

}
