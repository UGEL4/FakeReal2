#pragma once
#define FMT_CONSTEVAL //clang in c++2020 bug
#include <cstdint>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include "Core/Base/BaseDefine.h"

namespace FakeReal
{
	class LogSystem
	{
	public:
		LogSystem();
		~LogSystem();

		enum class LogLevel : uint8_t
		{
			debug,
			info,
			warning,
			error,
			fatal
		};

	public:
		template<typename ... Args>
		void Log(LogLevel level, Args&&... args)
		{
			switch (level)
			{
			case FakeReal::LogSystem::LogLevel::debug:
				m_pLogger->debug(std::forward<Args>(args)...);
				break;
			case FakeReal::LogSystem::LogLevel::info:
				m_pLogger->info(std::forward<Args>(args)...);
				break;
			case FakeReal::LogSystem::LogLevel::warning:
				m_pLogger->warn(std::forward<Args>(args)...);
				break;
			case FakeReal::LogSystem::LogLevel::error:
				m_pLogger->error(std::forward<Args>(args)...);
				break;
			case LogLevel::fatal:
				m_pLogger->critical(std::forward<Args>(args)...);
				FatalCallback(std::forward<Args>(args)...);
				break;
			default:
				break;
			}
		}

		template<typename... Args>
		//void FatalCallback(Args&&... args)
		void FatalCallback(fmt::format_string<Args...> s, Args&&... args)
		{
			const std::string format_str = fmt::format(s, std::forward<Args>(args)...);
			throw std::runtime_error(format_str);
		}

	private:
		SharedPtr<spdlog::logger> m_pLogger;
	};
}