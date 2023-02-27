#include "Engine.h"
#include <filesystem>

int main(int argc, char** argv)
{
	std::filesystem::path executePath = argv[0];
	std::filesystem::path configFilePath = executePath.parent_path() / "FakeRealEditorConfig.ini";
	FakeReal::Engine* pEngine = new FakeReal::Engine();
	pEngine->Start(configFilePath.generic_string());
	pEngine->Initialize();
	pEngine->Run();
	pEngine->Shutdown();
	delete pEngine;
	return 0;
}