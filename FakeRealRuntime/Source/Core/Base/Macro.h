#pragma once
#include "Function/Global/GlobalRuntimeContext.h"
#include "Core/Log/LogSystem.h"

#define LOG_HELPER(LEVEL, ...)\
	g_global_runtime_context.m_pLogSystem->Log(LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_INFO(...)		LOG_HELPER(LogSystem::LogLevel::info, __VA_ARGS__);
#define LOG_DEBUG(...)		LOG_HELPER(LogSystem::LogLevel::debug, __VA_ARGS__);
#define LOG_WARNING(...)	LOG_HELPER(LogSystem::LogLevel::warning, __VA_ARGS__);
#define LOG_ERROR(...)		LOG_HELPER(LogSystem::LogLevel::error, __VA_ARGS__);
#define LOG_FATAL(...)		LOG_HELPER(LogSystem::LogLevel::fatal, __VA_ARGS__);

#ifdef NDEBUG
#define ASSERT(statement)
#else
#define ASSERT(statement) assert(statement)
#endif

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }