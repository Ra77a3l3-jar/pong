#include <raylib.h>
#include <stdbool.h>
#include "snake.h"

static void SnakeFood(SnakeGameState *state) {
    bool val_position = false;
    int tries = 0;
    int max_tries = MAX_TRIES_FOOD;

    while(!val_position && tries <= max_tries) {
        tries++;
        state->food.x = GetRandomValue(0, state->grid_width - 1);
        state->food.y = GetRandomValue(0, state->grid_height - 1);

        val_position = true;
        // Check if food spawns on snake body
        for(int i = 0; i < state->lenght; i++) {
            if(state->food.x == state->snake_body[i].x && state->food.y == state->snake_body[i].y) {
                val_position = false; // Food spawns on snake, try again
            }
        }
    }
}

static void SnakeReset(SnakeGameState *state) {
    // Middle of grid
    int start_x = state->grid_width / 2;
    int start_y = state->grid_height / 2;

    // Draw inital segments
    for(int i = 0; i < INIT_SNAKE_LENGTH; i++) {
        state->snake_body[i].x = start_x - i;
        state->snake_body[i].y = start_y;
    }
    state->lenght = INIT_SNAKE_LENGTH;
    state->direction = RIGHT;
    state->next_direction = RIGHT;

    SnakeFood(state);
}

void SnakeInit(SnakeGameState *state) {
    SnakeGameState *snake_state = (SnakeGameState*)state;

    snake_state->screen = SNAKE_MENU;
    snake_state->prev_screen = SNAKE_MENU;

    snake_state->cell_size = GRID_SIZE;
    snake_state->grid_width = GetScreenWidth() / snake_state->cell_size;
    snake_state->grid_height = GetScreenHeight() / snake_state->cell_size;

    snake_state->speed = SPEED;
    snake_state->move_counter = 0;

    snake_state->level = 1;
    snake_state->score = 0;
    snake_state->game_over = false;
    snake_state->victory = false;

    snake_state->key_up = KEY_W;
    snake_state->key_down = KEY_S;
    snake_state->key_left = KEY_A;
    snake_state->key_right = KEY_D;

    SnakeReset(snake_state);
}

bool SnakeUpdate(SnakeGameState *state) {
    SnakeGameState *snake_state = (SnakeGameState*)state;

    if(IsKeyPressed(KEY_ESCAPE)) {
        switch(snake_state->screen) {
            case SNAKE_GAMEPLAY:
                state->prev_screen = snake_state->screen;
                state->screen = SNAKE_PAUSE;
                break;
            case SNAKE_PAUSE:
                state->prev_screen = snake_state->screen;
                state->screen = SNAKE_GAMEPLAY;
                break;
            case SNAKE_MENU:
                return false; // Exit to gamebox
        }
    }
}

void SnakeDraw(SnakeGameState *state) {
    SnakeGameState *snake_state = (SnakeGameState*)state;
}

void SnakeClose(SnakeGameState *state) {

}
