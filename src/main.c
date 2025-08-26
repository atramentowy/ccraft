#include "game.h"

int main(int argc, char **argv) {
	Game game;
	game_init(&game);
	game_run(&game);
	game_close(&game);

	return 0;
}
