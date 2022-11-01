#pragma once
#include "Core/Base/Macro.h"

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

	private:
	};
}
