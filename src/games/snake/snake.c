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

    snake_state->current_screen = SNAKE_MENU;
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
        switch(snake_state->current_screen) {
            case SNAKE_GAMEPLAY:
                snake_state->prev_screen = snake_state->current_screen;
                snake_state->current_screen = SNAKE_PAUSE;
                break;
            case SNAKE_PAUSE:
                snake_state->prev_screen = snake_state->current_screen;
                snake_state->current_screen = SNAKE_GAMEPLAY;
                break;
            case SNAKE_MENU:
                return false; // Exit to gamebox
        }
    }

    if(snake_state->current_screen == SNAKE_MENU) {
        if(IsKeyPressed(KEY_ENTER)) {
            snake_state->prev_screen = snake_state->current_screen;
            snake_state->current_screen = SNAKE_GAMEPLAY;
            SnakeReset(snake_state);
        }
    }

    if(snake_state->current_screen == SNAKE_GAMEPLAY) {
        if(IsKeyDown(snake_state->key_up) && snake_state->direction != DOWN) {
            snake_state->next_direction = UP;
        } else if(IsKeyDown(snake_state->key_down) && snake_state->direction != UP) {
            snake_state->next_direction = DOWN;
        } else if(IsKeyDown(snake_state->key_right) && snake_state->direction != LEFT) {
            snake_state->next_direction = LEFT;
        } else if(IsKeyDown(snake_state->key_left) && snake_state->direction != RIGHT) {
            snake_state->next_direction = LEFT;
        }

        snake_state->move_counter++;
        if(snake_state->move_counter >= snake_state->speed) {
            snake_state->move_counter = 0;
            snake_state->direction = snake_state->next_direction;

            // Move body head to tail
            for(int i = snake_state->lenght - 1; i > 0; i--) {
                snake_state->snake_body[i] = snake_state->snake_body[i - 1];
            }

            // Move head based on current direction
            switch(snake_state->direction) {
                case UP:
                    snake_state->snake_body[0].y--;
                    break;
                case DOWN:
                    snake_state->snake_body[0].y++;
                    break;
                case RIGHT:
                    snake_state->snake_body[0].x++;
                    break;
                case LEFT:
                    snake_state->snake_body[0].x--;
                    break;
            }


        }
    }
    return true;
}

void SnakeDraw(SnakeGameState *state) {
    SnakeGameState *snake_state = (SnakeGameState*)state;

    ClearBackground(BLACK);

    switch(snake_state->current_screen) {
        case SNAKE_MENU: {
            DrawText("SNAKE", GetScreenWidth()/2 - MeasureText("SNAKE", 60)/2, GetScreenHeight()/4, 60, WHITE);
            DrawText("Press ENTER to Start", GetScreenWidth()/2 - MeasureText("Press ENTER to Start", 30)/2, GetScreenHeight()/2, 30, WHITE);
            DrawText("Press ESC to return", GetScreenWidth()/2 - MeasureText("Press ESC to return", 20)/2, GetScreenHeight()/2 + 50, 20, GRAY);
            break;
        }
    }
}

void SnakeClose(SnakeGameState *state) {

}
