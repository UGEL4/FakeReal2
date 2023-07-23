#pragma once
// #include "Function/Global/GlobalRuntimeContext.h"
#include "Log/LogSystem.h"

#define LOG_HELPER(LEVEL, ...) \
	PrintLog(LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__);
    //g_logSystem.Log(LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_INFO(...) LOG_HELPER(LogLevel::info, __VA_ARGS__);
#define LOG_DEBUG(...) LOG_HELPER(LogLevel::debug, __VA_ARGS__);
#define LOG_WARNING(...) LOG_HELPER(LogLevel::warning, __VA_ARGS__);
#define LOG_ERROR(...) LOG_HELPER(LogLevel::error, __VA_ARGS__);
#define LOG_FATAL(...) LOG_HELPER(LogLevel::fatal, __VA_ARGS__);

#ifdef NDEBUG
    #define ASSERT(statement)
#else
    #define ASSERT(statement) assert(statement)
#endif

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }