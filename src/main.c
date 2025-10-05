#include "game.h"

int main(int argc, char **argv) {
    Game game;
    if (game_init(&game) != 0) {
        fprintf(stderr, "Game initialization failed\n");
        return 1;
    }
    game_run(&game);
    game_close(&game);
    return 0;
}

