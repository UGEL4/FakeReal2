#include "FRPch.h"
#include "Engine.h"
#include "Core/Base/Macro.h"
#include "Function/Render/WindowSystem.h"

namespace FakeReal
{
	Engine::Engine()
	{
		g_global_runtime_context.InitializeSystems();
		m_last_tick_time_point = std::chrono::steady_clock::now();
		LOG_DEBUG("Engine::Engine");
	}

	Engine::~Engine()
	{
	
	}

	void Engine::Initialize()
	{
		LOG_INFO("Engine::Initialize");
	}

	void Engine::Start()
	{
		LOG_WARNING("Engine::Start");
	}

	void Engine::Run()
	{
		LOG_INFO("Engine::Run");

		std::shared_ptr<WindowSystem> pWindow = g_global_runtime_context.m_pWindowSystem;
		ASSERT(pWindow);
		while (!pWindow->ShouldClose())
		{
			double deltaTime = CalculateDeltaTime();
			Tick(deltaTime);
		}
	}

	void Engine::Tick(double deltaTime)
	{

	}

	void Engine::Shutdown()
	{
		g_global_runtime_context.ShutdownSystems();
		LOG_ERROR("Engine::Shutdown");
	}

	double Engine::CalculateDeltaTime()
	{
		double delta;
		std::chrono::steady_clock::time_point now_tick_time_point = std::chrono::steady_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now_tick_time_point - m_last_tick_time_point);
		delta = time_span.count();
		m_last_tick_time_point = now_tick_time_point;

		return delta;
	}

}