#pragma once

#include "WindowSystem.h"
#include "RenderSystem.h"
#include <chrono>

using namespace FakeReal;

class Game
{
public:
    Game();
    ~Game();

    void Initialize();
    void Finalize();
    void Run();

private:
    double CalculateDeltaTime();
    void CalculateFPS(double deltaTime);

private:
    WindowSystem* mWindowSystem {nullptr};
    std::chrono::steady_clock::time_point m_last_tick_time_point;
    int mFPS {0};
    double mAverageDuration {0.f};
    unsigned int mFrameCount {0};
    static const double s_fps_alpha;
};