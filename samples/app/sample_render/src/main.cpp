#include "game.h"

int main(int argc, char** argv)
{
    Game* game = new Game();
    game->Initialize();
    game->Run();
    game->Finalize();
    delete game;
    return 0;
}