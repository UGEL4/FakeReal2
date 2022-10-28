#pragma once

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
		void Tick();
		void Shutdown();
	};
}
