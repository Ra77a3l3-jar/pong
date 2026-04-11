#include <raylib.h>
#include <stdbool.h>
#include "snake.h"

static void SnakeFood(SnakeGameState *state) {
    bool val_position = false;
    int tries = 0;
    int max_tries = MAX_TRIES_FOOD;

    while(!val_position && tries < max_tries) {
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
    snake_state->grid_width = (GetScreenWidth() - GRID_MARGIN * 2) / snake_state->cell_size;
    snake_state->grid_height = (GetScreenHeight() - GRID_MARGIN * 2) / snake_state->cell_size;

    snake_state->speed = SPEED;
    snake_state->move_counter = 0;

    snake_state->level = 1;
    snake_state->score = 0;
    snake_state->game_over = false;
    snake_state->victory = false;
    snake_state->victory_timer = 0;
    snake_state->game_over_timer = 0;

    snake_state->key_up = KEY_W;
    snake_state->key_down = KEY_S;
    snake_state->key_left = KEY_A;
    snake_state->key_right = KEY_D;

    snake_state->selected_pause = SNAKE_PAUSE_RESUME;

    SnakeReset(snake_state);
}

bool SnakeUpdate(SnakeGameState *state) {
    SnakeGameState *snake_state = (SnakeGameState*)state;

    if(IsWindowResized()) {
        snake_state->grid_width = (GetScreenWidth() - GRID_MARGIN * 2) / snake_state->cell_size;
        snake_state->grid_height = (GetScreenHeight() - GRID_MARGIN * 2) / snake_state->cell_size;
        // Respawn food if it ended up outside the new bounds
        if(snake_state->food.x >= snake_state->grid_width || snake_state->food.y >= snake_state->grid_height) {
            SnakeFood(snake_state);
        }
    }

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
            snake_state->next_direction = RIGHT;
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

            // Check collisions with walls
            if(snake_state->snake_body[0].x < 0 ||
               snake_state->snake_body[0].x >= snake_state->grid_width ||
               snake_state->snake_body[0].y < 0 ||
               snake_state->snake_body[0].y >= snake_state->grid_height) {
                snake_state->game_over = true;
                snake_state->current_screen = SNAKE_GAME_OVER;
            }

            // Check for collision with rest of the body
            for(int i = 1; i < snake_state->lenght; i++) {
                if(snake_state->snake_body[0].x == snake_state->snake_body[i].x &&
                   snake_state->snake_body[0].y == snake_state->snake_body[i].y) {
                    snake_state->game_over = true;
                    snake_state->current_screen = SNAKE_GAME_OVER;
                    snake_state->game_over_timer = GAMEOVER_SCREEN_DURATION;
                }
            }

            // Food eaten
            if(snake_state->snake_body[0].x == snake_state->food.x &&
               snake_state->snake_body[0].y == snake_state->food.y) {
                // Snake grows
                if(snake_state->lenght < MAX_SNAKE_LENGHT) {
                    snake_state->lenght++;
                    // Copy tail position for new piece
                    snake_state->snake_body[snake_state->lenght - 1] = snake_state->snake_body[snake_state->lenght - 2];
                }

                // Increase score
                snake_state->score += 1 * snake_state->level;

                // Victory if 90% of the entire
                if(snake_state->lenght >= snake_state->grid_width * snake_state->grid_height * 0.9) {
                    snake_state->victory = true;
                    snake_state->current_screen = SNAKE_VICTORY;
                    snake_state->victory_timer = VICTORY_SCREEN_DURATION;
                } else {
                    SnakeFood(snake_state);
                }
            }
        }
    }

    if(snake_state->current_screen == SNAKE_PAUSE) {
        if(IsKeyPressed(KEY_UP)) {
            snake_state->selected_pause = (snake_state->selected_pause + SNAKE_PAUSE_OPTION_COUNT - 1) % SNAKE_PAUSE_OPTION_COUNT;
        } else if(IsKeyPressed(KEY_DOWN)) {
            snake_state->selected_pause = (snake_state->selected_pause + 1) % SNAKE_PAUSE_OPTION_COUNT;
        } else if(IsKeyPressed(KEY_ENTER)) {
            switch(snake_state->selected_pause) {
                case SNAKE_PAUSE_RESUME: {
                    snake_state->current_screen = SNAKE_GAMEPLAY;
                    break;
                }
                case SNAKE_PAUSE_RESTART: {
                    SnakeInit(state);
                    snake_state->current_screen = SNAKE_GAMEPLAY;
                    break;
                }
                case SNAKE_PAUSE_QUIT: {
                    SnakeInit(state);
                    break;
                }
                default: break;
            }
        }
    }
    
    return true;
}

void SnakeDraw(SnakeGameState *state) {
    SnakeGameState *snake_state = (SnakeGameState*)state;

    // Grid ofset to centre
    int offset_x = (GetScreenWidth() - snake_state->grid_width * snake_state->cell_size) / 2;
    int offset_y = (GetScreenHeight() - snake_state->grid_height * snake_state->cell_size) / 2;

    ClearBackground(BLACK);

    switch(snake_state->current_screen) {
        case SNAKE_MENU: {
            DrawText("SNAKE", GetScreenWidth()/2 - MeasureText("SNAKE", 60)/2, GetScreenHeight()/4, 60, WHITE);
            DrawText("Press ENTER to Start", GetScreenWidth()/2 - MeasureText("Press ENTER to Start", 30)/2, GetScreenHeight()/2, 30, WHITE);
            DrawText("Press ESC to return", GetScreenWidth()/2 - MeasureText("Press ESC to return", 20)/2, GetScreenHeight()/2 + 50, 20, GRAY);
            break;
        }
        case  SNAKE_GAMEPLAY: {
            // Draw play area border
            DrawRectangleLines(
                offset_x,
                offset_y,
                snake_state->grid_width * snake_state->cell_size,
                snake_state->grid_height * snake_state->cell_size,
                ColorAlpha(WHITE, 0.25)
            );

            // Draw food (centered in cell)
            int food_pad = 3;
            DrawRectangle(
                offset_x + snake_state->food.x * snake_state->cell_size + food_pad,
                offset_y + snake_state->food.y * snake_state->cell_size + food_pad,
                snake_state->cell_size - food_pad * 2,
                snake_state->cell_size - food_pad * 2,
                RED
            );

            // Draw snake
            for(int i = 0; i < snake_state->lenght; i++) {
                Color body_color = (i == 0) ? LIME : GREEN;

                DrawRectangle(
                    offset_x + snake_state->snake_body[i].x * snake_state->cell_size + 1,
                    offset_y + snake_state->snake_body[i].y * snake_state->cell_size + 1,
                    snake_state->cell_size - 2,
                    snake_state->cell_size - 2,
                    body_color
                );
            }

            // Draw info
            DrawText(TextFormat("Score   %d", snake_state->score), 20, 20, 20, WHITE);
            DrawText(TextFormat("Level   %d", snake_state->level), GetScreenWidth() - 120, 20, 20, WHITE);
            DrawText(TextFormat("Speed   %d", snake_state->speed), GetScreenWidth()/2 - 50, 20, 20, WHITE);
            break;
        }
        case SNAKE_PAUSE: {
            // Draw play area border (dimmed while paused)
            DrawRectangleLines(
                offset_x,
                offset_y,
                snake_state->grid_width * snake_state->cell_size,
                snake_state->grid_height * snake_state->cell_size,
                ColorAlpha(WHITE, 0.15)
            );

            // Draw food (centered in cell, dimmed while paused)
            int food_pad_p = 3;
            DrawRectangle(
                offset_x + snake_state->food.x * snake_state->cell_size + food_pad_p,
                offset_y + snake_state->food.y * snake_state->cell_size + food_pad_p,
                snake_state->cell_size - food_pad_p * 2,
                snake_state->cell_size - food_pad_p * 2,
                ColorAlpha(RED, 0.5)
            );

            // Draw snake (dimmed while paused)
            for(int i = 0; i < snake_state->lenght; i++) {
                Color body_color = (i == 0) ? ColorAlpha(LIME, 0.5) : ColorAlpha(GREEN, 0.5);

                DrawRectangle(
                    offset_x + snake_state->snake_body[i].x * snake_state->cell_size + 1,
                    offset_y + snake_state->snake_body[i].y * snake_state->cell_size + 1,
                    snake_state->cell_size - 2,
                    snake_state->cell_size - 2,
                    body_color
                );
            }
            // Draw info
            DrawText(TextFormat("Score   %d", snake_state->score), 20, 20, 20, WHITE);
            DrawText(TextFormat("Level   %d", snake_state->level), GetScreenWidth() - 120, 20, 20, WHITE);
            DrawText(TextFormat("Speed   %d", snake_state->speed), GetScreenWidth()/2 - 50, 20, 20, WHITE);

            int menu_start_y = GetScreenHeight()/2 - 20;
            int menu_spacing = 50;

            Color resume_color = (snake_state->selected_pause == SNAKE_PAUSE_RESUME) ? RED : WHITE;
            Color restart_color = (snake_state->selected_pause == SNAKE_PAUSE_RESTART) ? RED : WHITE;
            Color quit_color = (snake_state->selected_pause == SNAKE_PAUSE_QUIT) ? RED : WHITE;

            DrawText("Resume", GetScreenWidth()/2 - MeasureText("Resume", 30)/2, menu_start_y, 30, resume_color);
            DrawText("Restart", GetScreenWidth()/2 - MeasureText("Restart", 30)/2, menu_start_y + menu_spacing, 30, restart_color);
            DrawText("Quit to Menu", GetScreenWidth()/2 - MeasureText("Quit to Menu", 30)/2, menu_start_y + menu_spacing * 3, 30, quit_color);

            DrawText("Use UP/DOWN to navigate, ENTER to select", GetScreenWidth()/2 - MeasureText("Use UP/DOWN to navigate, ENTER to select", 20)/2, GetScreenHeight() - 50, 20, GRAY);
            break;
        }
        case SNAKE_VICTORY: {
            DrawText("VICTORY!", GetScreenWidth()/2 - MeasureText("VICTORY!", 60)/2, GetScreenHeight()/2 - 100, 60, GREEN);
            DrawText(TextFormat("Score: %d", snake_state->score), GetScreenWidth()/2 - MeasureText("Score: 100", 40)/2, GetScreenHeight()/2, 40, WHITE);
            DrawText("Press ESC to return", GetScreenWidth()/2 - MeasureText("Press ESC to return", 30)/2, GetScreenHeight()/2 + 80, 30, WHITE);

            if(IsKeyPressed(KEY_ESCAPE)) {
                snake_state->current_screen = SNAKE_MENU;
                SnakeInit(snake_state);
            }

            snake_state->victory_timer--;
            if(snake_state->victory_timer <= 0) {
                snake_state->current_screen = SNAKE_MENU;
                SnakeInit(snake_state);
            }
            break;
        }
        case SNAKE_GAME_OVER: {
            DrawText("GAME OVER", GetScreenWidth()/2 - MeasureText("GAME OVER", 60)/2, GetScreenHeight()/2 - 100, 60, RED);
            DrawText(TextFormat("Final Score: %d", snake_state->score), GetScreenWidth()/2 - MeasureText("Final Score: 100", 40)/2, GetScreenHeight()/2, 40, WHITE);
            DrawText("Press ESC to return", GetScreenWidth()/2 - MeasureText("Press ESC to return", 30)/2, GetScreenHeight()/2 + 80, 30, WHITE);

            if(IsKeyPressed(KEY_ESCAPE)) {
                SnakeInit(snake_state);
            }

            snake_state->game_over_timer--;
            if(snake_state->game_over_timer <= 0) {
                SnakeInit(snake_state);
            }
            break;
        }
    }
}

void SnakeClose(SnakeGameState *state) {

}
