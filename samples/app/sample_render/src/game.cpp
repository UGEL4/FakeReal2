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
    m_last_tick_time_point = std::chrono::steady_clock::now();
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
        double deltaTime = CalculateDeltaTime();
        CalculateFPS(deltaTime);
        mWindowSystem->PollEvents();
        mWindowSystem->SetTitle(std::string("FR Engine - " + std::to_string(mFPS) + "FPS").c_str());
    }
}

double Game::CalculateDeltaTime()
{
    double delta;
    std::chrono::steady_clock::time_point now_tick_time_point = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span                   = std::chrono::duration_cast<std::chrono::duration<double>>(now_tick_time_point - m_last_tick_time_point);
    delta                                                     = time_span.count();
    m_last_tick_time_point                                    = now_tick_time_point;

    return delta;
}

const double Game::s_fps_alpha = 1.0 / 100.0;
void Game::CalculateFPS(double deltaTime)
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