/*
 * Knight Engine 2D - Main Entry Point
 */

#include "core/engine.h"
#include "core/game_state.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    game_state_t game = {0};

    if (!engine_init(&game)) {
        engine_cleanup(&game);
        return 1;
    }

    engine_run(&game);
    engine_cleanup(&game);

    return 0;
}
