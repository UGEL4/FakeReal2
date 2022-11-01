#include "Engine.h"

namespace FakeReal
{
	Engine::Engine()
	{
		g_global_runtime_context.InitializeSystem();
		LOG_DEBUG("Engine::Engine");
	}

	Engine::~Engine()
	{
		g_global_runtime_context.ShutdownSystem();
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
		while (true)
		{
			Tick();
		}
	}

	void Engine::Tick()
	{

	}

	void Engine::Shutdown()
	{
		LOG_ERROR("Engine::Shutdown");
	}

}