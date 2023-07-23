#include "game.h"
#include "Utils/Macro.h"

Game::Game()
{

}

Game::~Game()
{

}

void Game::Initialize()
{
    LogSystem::Initialize();
    LOG_INFO("Game::Initialize");
    mWindowSystem = new WindowSystem();
    WindowCreateInfo winCreateInfo {};
    winCreateInfo.width       = 1280;
    winCreateInfo.height      = 720;
    winCreateInfo.full_screen = false;
    mWindowSystem->Initialize(winCreateInfo);
}

void Game::Finalize()
{
    LOG_INFO("Game::Finalize");
    if (mWindowSystem)
    {
        mWindowSystem->ShouldClose();
        delete mWindowSystem;
        mWindowSystem = nullptr;
    }
    LogSystem::Finalize();
}

void Game::Run()
{
    while (!mWindowSystem->ShouldClose())
    {
        mWindowSystem->PollEvents();
    }
}