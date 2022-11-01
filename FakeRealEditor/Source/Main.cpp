#include "Engine.h"

int main()
{
	FakeReal::Engine* pEngine = new FakeReal::Engine();
	pEngine->Start();
	pEngine->Initialize();
	pEngine->Run();
	pEngine->Shutdown();
	delete pEngine;
	return 0;
}