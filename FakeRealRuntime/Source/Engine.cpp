#include "FRPch.h"
#include "Engine.h"
#include "Core/Base/Macro.h"
#include "Function/Render/WindowSystem.h"

namespace FakeReal
{
	Engine::Engine()
		: mFPS(0), mAverageDuration(0.0f), mFrameCount(0)
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
		LOG_DEBUG("Engine::Initialize");
	}

	void Engine::Start()
	{
		LOG_DEBUG("Engine::Start");
	}

	void Engine::Run()
	{
		LOG_DEBUG("Engine::Run");

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
		CalculateFPS(deltaTime);

		g_global_runtime_context.m_pWindowSystem->PollEvents();
		//g_global_runtime_context.m_pWindowSystem->SetTitle(std::string("FR Engine - " + std::to_string(mFPS) + "FPS").c_str());
	}

	void Engine::Shutdown()
	{
		LOG_DEBUG("Engine::Shutdown");
		g_global_runtime_context.ShutdownSystems();
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

	const double Engine::s_fps_alpha = 1.0 / 100.0;
	void Engine::CalculateFPS(double deltaTime)
	{
		mFrameCount++;

		if (mFrameCount == 1)
		{
			mAverageDuration = deltaTime;
		}
		else
		{
			mAverageDuration = mAverageDuration * (1 - s_fps_alpha) + deltaTime * s_fps_alpha;
		}

		mFPS = static_cast<int>(1.f / mAverageDuration);
	}

}