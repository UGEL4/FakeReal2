#pragma once

#include "WindowSystem.h"
#include "RenderSystem.h"

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
    WindowSystem* mWindowSystem {nullptr};
};