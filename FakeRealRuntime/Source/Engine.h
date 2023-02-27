#pragma once

#include <chrono>
#include <string>

namespace FakeReal
{
	class Engine
	{
		static const double s_fps_alpha;
	public:
		Engine();
		~Engine();

		void Start(const std::string& configFilePath);
		void Initialize();
		void Run();
		void Tick(double deltaTime);
		void Shutdown();
		int GetFPS() const { return mFPS; }
	private:
		double CalculateDeltaTime();
		void CalculateFPS(double deltaTime);

	private:
		std::chrono::steady_clock::time_point m_last_tick_time_point;
		int mFPS;
		double mAverageDuration;
		unsigned int mFrameCount;
	};
}
