#pragma once

#include <chrono>

namespace FakeReal
{
	class Engine
	{
	public:
		Engine();
		~Engine();

		void Initialize();
		void Start();
		void Run();
		void Tick(double deltaTime);
		void Shutdown();

	private:
		double CalculateDeltaTime();

	private:
		std::chrono::steady_clock::time_point m_last_tick_time_point;
	};
}
