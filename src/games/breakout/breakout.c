#include <raylib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "breakout.h"
#include "utils.h"

void BreakoutInit(BreakoutGameState *state) {
    BreakoutGameState *breakout_state = (BreakoutGameState*)state;

    breakout_state->current_screen = BREAKOUT_MENU;
    breakout_state->prev_screen = BREAKOUT_MENU;

    breakout_state->score = 0;
    breakout_state->level = 1;
    breakout_state->lives = 3;
    breakout_state->game_over = false;
    breakout_state->victory = false;

    breakout_state->paddle = (Rectangle) {GetScreenWidth()/2 - breakout_state->paddle_width/2, GetScreenHeight() - 50, breakout_state->paddle_width, PADDLE_HEIGHT};
    breakout_state->paddle_width = PADDLE_WIDTH;
    breakout_state->paddle_speed = PADDLE_SPEED;
    breakout_state->paddle_velocity = 0.0;

    breakout_state->ball_rad = BALL_RADIUS;
    breakout_state->ball_speed_multiplier = 1.0;
    breakout_state->ball_position = (Vector2){0, 0};
    breakout_state->ball_speed = (Vector2){0, 0};
    breakout_state->ball_active = false;

    breakout_state->brick_rows = BRICK_ROWS;
    breakout_state->brick_cols = BRICK_COLS;
    breakout_state->brick_width = (GetScreenWidth() - (BRICK_COLS + 1) * BRICK_PADD) / BRICK_COLS;
    breakout_state->brick_height = BRICK_HEIGHT;
    breakout_state->brick_count = 0;

    breakout_state->key_left = KEY_LEFT;
    breakout_state->key_right = KEY_RIGHT;
    breakout_state->waiting_for_key = false;
    breakout_state->rebiding_key = 0;
    breakout_state->selected_key_index = 0;

    breakout_state->paddle_sound = LoadSound("assets/audio/breakout/breakout_paddle.wav");
    breakout_state->brick_sound = LoadSound("assets/audio/breakout/breakout_brick.wav");
    breakout_state->wall_hit_sound = LoadSound("assets/audio/breakout/breakout_wall_hit.wav");
    breakout_state->lose_life_sound = LoadSound("assets/audio/breakout/breakout_lose_life.wav");
    breakout_state->game_over_sound = LoadSound("assets/audio/breakout/breakout_game_over.wav");
    breakout_state->victory_sound = LoadSound("assets/audio/breakout/breakout_victory.wav");

    breakout_state->victory_timer = 0;
    breakout_state->game_over_timer = 0;

    breakout_state->selected_pause = BREAKOUT_PAUSE_RESUME;
    breakout_state->selected_settings_section = BREAKOUT_SETTINGS_PADDLE_WIDTH;

    BreakoutSetupLevel(breakout_state);
}

void BreakoutSetupLevel(BreakoutGameState *state) {
    state->brick_count = 0;
    float startX = BRICK_PADD;
    float startY = 50;

    for(int row = 0; row < state->brick_rows; row++) {
        for(int col = 0; col < state->brick_cols; col++) {
            int indx = row * state->brick_cols + col;
            // Bounds check to prevent array overflow
            if(indx >= BRICK_NUM) {
                state->brick_count = BRICK_NUM;
                return;
            }
            state->bricks[indx] = (Rectangle) {
                startX + col * (state->brick_width + BRICK_PADD),
                startY + row * (state->brick_height + BRICK_PADD),
                state->brick_width,
                state->brick_height
            };
            state->bricks_active[indx] = true;
            state->brick_count++;
        }
    }
}

bool BreakoutUpdate(BreakoutGameState *state) {
    BreakoutGameState *breakout_state = (BreakoutGameState*)state;

    if(IsKeyPressed(KEY_ESCAPE)) {
        if(breakout_state->current_screen == BREAKOUT_GAMEPLAY) {
            breakout_state->prev_screen = breakout_state->current_screen;
            breakout_state->current_screen = BREAKOUT_PAUSED;
        } else if(breakout_state->current_screen == BREAKOUT_PAUSED) {
            breakout_state->current_screen = BREAKOUT_GAMEPLAY;
            // Update paddle rectangle with current width
            breakout_state->paddle.width = breakout_state->paddle_width;
            breakout_state->paddle.x = GetScreenWidth()/2 - breakout_state->paddle_width/2;
        } else if(breakout_state->current_screen == BREAKOUT_SETTINGS) {
            breakout_state->current_screen = breakout_state->prev_screen;
            // Update paddle rectangle if returning to gameplay
            if(breakout_state->prev_screen == BREAKOUT_GAMEPLAY || breakout_state->prev_screen == BREAKOUT_PAUSED) {
                breakout_state->paddle.width = breakout_state->paddle_width;
                breakout_state->paddle.x = GetScreenWidth()/2 - breakout_state->paddle_width/2;
            }
        } else if(breakout_state->current_screen == BREAKOUT_MENU) {
            return false;
        }
    }

    if(breakout_state->waiting_for_key) {
        for(int i = FIRST_KEY; i < LAST_KEY; i++) {
            if(IsKeyPressed(i)) {
                switch(breakout_state->rebiding_key) {
                    case 1: breakout_state->key_left = i; break;
                    case 2: breakout_state->key_right = i; break;
                }
                breakout_state->waiting_for_key = false;
                break;
            }
        }
        // To cancel rebinding
        if(IsKeyPressed(KEY_ESCAPE)) {
            breakout_state->waiting_for_key = false;
        }
    }

    if(breakout_state->current_screen == BREAKOUT_MENU) {
        if(IsKeyPressed(KEY_ENTER)) {
            breakout_state->current_screen = BREAKOUT_GAMEPLAY;
            breakout_state->paddle = (Rectangle) {GetScreenWidth()/2 - breakout_state->paddle_width/2, GetScreenHeight() - 50, breakout_state->paddle_width, PADDLE_HEIGHT};
            breakout_state->paddle_width = PADDLE_WIDTH;
            breakout_state->paddle_speed = PADDLE_SPEED * (1 + (breakout_state->level - 1) * 0.5);
        } else if(IsKeyPressed(KEY_S)) {
            breakout_state->prev_screen = BREAKOUT_MENU;
            breakout_state->current_screen = BREAKOUT_SETTINGS;
        }
    }

    if(breakout_state->current_screen == BREAKOUT_SETTINGS) {
        if(!breakout_state->waiting_for_key) {
            if(IsKeyPressed(KEY_UP)) {
                breakout_state->selected_settings_section = (breakout_state->selected_settings_section - 1 + BREAKOUT_SETTINGS_SECTION_COUNT) % BREAKOUT_SETTINGS_SECTION_COUNT;
                if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_CONTROLS) {
                    breakout_state->selected_key_index = 0; // Reset to first key when entering controls
                }
            } else if(IsKeyPressed(KEY_DOWN)) {
                breakout_state->selected_settings_section = (breakout_state->selected_settings_section + 1) % BREAKOUT_SETTINGS_SECTION_COUNT;
                if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_CONTROLS) {
                    breakout_state->selected_key_index = 0; // Reset to first key when entering controls
                }
            }
        }

        if(!breakout_state->waiting_for_key) {
            if(IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                int direction = IsKeyPressed(KEY_LEFT) ? -1 : 1; // -1 left 1 right

                if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_PADDLE_WIDTH) {
                    breakout_state->paddle_width += direction * 10;
                    if(breakout_state->paddle_width < 30) breakout_state->paddle_width = 30;
                    if(breakout_state->paddle_width > 200) breakout_state->paddle_width = 200;
                } else if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_ROWS) {
                    breakout_state->brick_rows += direction;
                    if(breakout_state->brick_rows < 2) breakout_state->brick_rows = 2;
                    if(breakout_state->brick_rows > BRICK_MAX_ROWS) breakout_state->brick_rows = BRICK_MAX_ROWS;
                    // Check if bricks don't exced array size
                    while(breakout_state->brick_rows * breakout_state->brick_cols > BRICK_NUM) {
                        breakout_state->brick_rows--;
                    }
                    breakout_state->brick_width = (GetScreenWidth() - (breakout_state->brick_cols + 1) * BRICK_PADD) / breakout_state->brick_cols;
                    BreakoutSetupLevel(breakout_state);
                } else if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_COLS) {
                    breakout_state->brick_cols += direction;
                    if(breakout_state->brick_cols < 5) breakout_state->brick_cols = 5;
                    if(breakout_state->brick_cols > BRICK_MAX_COLS) breakout_state->brick_cols = BRICK_MAX_COLS;
                    // Check bricks don't exceed array size
                    while(breakout_state->brick_rows * breakout_state->brick_cols > BRICK_NUM) {
                        breakout_state->brick_cols--;
                    }
                    breakout_state->brick_width = (GetScreenWidth() - (breakout_state->brick_cols + 1) * BRICK_PADD) / breakout_state->brick_cols;
                    BreakoutSetupLevel(breakout_state);
                } else if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_LIVES) {
                    breakout_state->lives += direction;
                    if(breakout_state->lives < 1) breakout_state->lives = 1;
                    if(breakout_state->lives > LIVES_MAX) breakout_state->lives = LIVES_MAX;
                } else if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_LEVEL) {
                    breakout_state->level += direction;
                    if(breakout_state->level < 1) breakout_state->level = 1;
                    if(breakout_state->level > 5) breakout_state->level = 5;
                    breakout_state->ball_speed_multiplier = 0.5f + (breakout_state->level - 1) * 0.5;
                } else if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_BALL_SIZE) {
                    breakout_state->ball_rad += direction;
                    if(breakout_state->ball_rad < 3) breakout_state->ball_rad = 3;
                    if(breakout_state->ball_rad > 15) breakout_state->ball_rad = 15;
                } else if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_CONTROLS) {
                    if(IsKeyPressed(KEY_LEFT)) {
                        breakout_state->selected_key_index = (breakout_state->selected_key_index - 1 + 2) % 2;
                    }
                    if(IsKeyPressed(KEY_RIGHT)) {
                        breakout_state->selected_key_index = (breakout_state->selected_key_index + 1) % 2;
                    }
                }
            }

            // Start rebinding selected key in CONTROLS section
            if(IsKeyPressed(KEY_ENTER) && breakout_state->selected_settings_section == BREAKOUT_SETTINGS_CONTROLS) {
                breakout_state->waiting_for_key = true;
                breakout_state->rebiding_key = breakout_state->selected_key_index + 1;
            }
        }
    }

    if(breakout_state->current_screen == BREAKOUT_PAUSED) {
        if(IsKeyPressed(KEY_UP)) {
            breakout_state->selected_pause = (breakout_state->selected_pause + BREAKOUT_PAUSE_OPTION_COUNT - 1) % BREAKOUT_PAUSE_OPTION_COUNT;
        } else if(IsKeyPressed(KEY_DOWN)) {
            breakout_state->selected_pause = (breakout_state->selected_pause + 1) % BREAKOUT_PAUSE_OPTION_COUNT;
        } else if(IsKeyPressed(KEY_ENTER)) {
            switch(breakout_state->selected_pause) {
                case BREAKOUT_PAUSE_RESUME: {
                    breakout_state->current_screen = BREAKOUT_GAMEPLAY;
                    break;
                }
                case BREAKOUT_PAUSE_RESTART: {
                    BreakoutInit(state);
                    breakout_state->current_screen = BREAKOUT_GAMEPLAY;
                    break;
                }
                case BREAKOUT_PAUSE_SETTINGS: {
                    breakout_state->prev_screen = BREAKOUT_PAUSED;
                    breakout_state->current_screen = BREAKOUT_SETTINGS;
                    break;
                }
                case BREAKOUT_PAUSE_QUIT: {
                    BreakoutInit(state);
                    break;
                }
                default: break;
            }
        }
    }

    if(breakout_state->current_screen == BREAKOUT_GAMEPLAY) {
        // Paddle movement
        breakout_state->paddle_velocity = 0.0;
        if(IsKeyDown(breakout_state->key_left) && breakout_state->paddle.x > 0) {
            breakout_state->paddle.x -= breakout_state->paddle_speed;
            breakout_state->paddle_velocity = -breakout_state->paddle_speed;
        }
        if(IsKeyDown(breakout_state->key_right) && breakout_state->paddle.x + breakout_state->paddle_width < GetScreenWidth()) {
            breakout_state->paddle.x += breakout_state->paddle_speed;
            breakout_state->paddle_velocity = breakout_state->paddle_speed;
        }

        // Ball movement
        if(breakout_state->ball_active) {
            breakout_state->ball_position.x += breakout_state->ball_speed.x;
            breakout_state->ball_position.y += breakout_state->ball_speed.y;

            // Ball collision with walls
            if(breakout_state->ball_position.x - breakout_state->ball_rad <= 0 ||
                breakout_state->ball_position.x + breakout_state->ball_rad >= GetScreenWidth()) {
                breakout_state->ball_speed.x *= -1;
                PlaySound(breakout_state->wall_hit_sound);
            }

            if(breakout_state->ball_position.y - breakout_state->ball_rad <= 0) {
                breakout_state->ball_speed.y *= -1;
            }

            // Ball collision with bottom
            if(breakout_state->ball_position.y + breakout_state->ball_rad >= GetScreenHeight()) {
                breakout_state->lives--;
                breakout_state->ball_active = false;
                PlaySound(breakout_state->wall_hit_sound);

                if(breakout_state->lives <= 0) {
                    breakout_state->game_over = true;
                    breakout_state->current_screen = BREAKOUT_GAME_OVER;
                    breakout_state->game_over_timer = GAMEOVER_SCREEN_DURATION;
                    PlaySound(breakout_state->game_over_sound);
                }
                PlaySound(breakout_state->lose_life_sound);
            }

            // Ball collision with paddle
            if(CheckCollisionCircleRec(breakout_state->ball_position, breakout_state->ball_rad, breakout_state->paddle)) {
                breakout_state->ball_speed.y *= -1;

                // Direction based on hit position
                float relative_intersection = (breakout_state->ball_position.x - breakout_state->paddle.x) / breakout_state->paddle_width;
                breakout_state->ball_speed.x = (relative_intersection - 0.5) * 10 * breakout_state->ball_speed_multiplier;

                // Apply speed gain/loss based on paddle velocity
                float speed_magnitude = sqrt(breakout_state->ball_speed.x * breakout_state->ball_speed.x +
                                            breakout_state->ball_speed.y * breakout_state->ball_speed.y);
                speed_magnitude += breakout_state->paddle_velocity * BALL_SPEED_GAIN_FACTOR;

                // Cap maximum speed
                if(speed_magnitude > MAX_BALL_SPEED) {
                    speed_magnitude = MAX_BALL_SPEED;
                }

                // Normalize and apply new speed
                if(speed_magnitude > 0) {
                    float scale = speed_magnitude / sqrt(breakout_state->ball_speed.x * breakout_state->ball_speed.x +
                                                         breakout_state->ball_speed.y * breakout_state->ball_speed.y);
                    breakout_state->ball_speed.x *= scale;
                    breakout_state->ball_speed.y *= scale;
                }

                PlaySound(breakout_state->paddle_sound);
            }

            // Ball collision with bricks
            for(int i = 0; i < breakout_state->brick_count; i++) {
                if(breakout_state->bricks_active[i] &&
                    CheckCollisionCircleRec(breakout_state->ball_position, breakout_state->ball_rad, breakout_state->bricks[i])) {
                    breakout_state->bricks_active[i] = false;
                    breakout_state->score += 1;
                    breakout_state->ball_speed.y *= -1;

                    // Apply speed loss on brick collision
                    float speed_magnitude = sqrt(breakout_state->ball_speed.x * breakout_state->ball_speed.x +
                                                breakout_state->ball_speed.y * breakout_state->ball_speed.y);
                    speed_magnitude *= BALL_SPEED_LOSS_FACTOR;

                    // Normalize and apply new speed
                    if(speed_magnitude > 0) {
                        float current_speed = sqrt(breakout_state->ball_speed.x * breakout_state->ball_speed.x +
                                                  breakout_state->ball_speed.y * breakout_state->ball_speed.y);
                        if(current_speed > 0) {
                            float scale = speed_magnitude / current_speed;
                            breakout_state->ball_speed.x *= scale;
                            breakout_state->ball_speed.y *= scale;
                        }
                    }

                    PlaySound(breakout_state->brick_sound);

                    // Check if all are destroyed
                    bool all_destroyed = true;
                    for(int j = 0; j < breakout_state->brick_count; j++) {
                        if(breakout_state->bricks_active[j]) {
                            all_destroyed = false;
                            break;
                        }
                    }

                    if(all_destroyed) {
                        breakout_state->victory = true;
                        breakout_state->current_screen = BREAKOUT_VICTORY;
                        breakout_state->victory_timer = VICTORY_SCREEN_DURATION;
                        PlaySound(breakout_state->victory_sound);
                    }
                    break;
                }
            }
        }

        // Launch ball
        if(!breakout_state->ball_active && IsKeyPressed(KEY_SPACE)) {
            breakout_state->ball_active = true;
            breakout_state->ball_position = (Vector2) {
                breakout_state->paddle.x + breakout_state->paddle_width/2,
                breakout_state->paddle.y - breakout_state->ball_rad
            };
            breakout_state->ball_speed = (Vector2){0, -5 * breakout_state->ball_speed_multiplier};
        }
    }

    return true;
}

void BreakoutDraw(BreakoutGameState *state) {
    BreakoutGameState *breakout_state = (BreakoutGameState*)state;

    ClearBackground(BLACK);

    switch(breakout_state->current_screen) {
        case BREAKOUT_MENU: {
            DrawText("BREAKOUT", GetScreenWidth()/2 - MeasureText("BREAKOUT", 60)/2, GetScreenHeight()/4, 60, WHITE);
            DrawText("Press ENTER to Start", GetScreenWidth()/2 - MeasureText("Press ENTER to Start", 30)/2, GetScreenHeight()/2, 30, WHITE);
            DrawText("Press S for Settings", GetScreenWidth()/2 - MeasureText("Press S for Settings", 30)/2, GetScreenHeight()/2 + 50, 30, GRAY);
            DrawText("Press ESC to return", GetScreenWidth()/2 - MeasureText("Press ESC to return", 20)/2, GetScreenHeight()/2 + 100, 20, GRAY);
            break;
        }
        case BREAKOUT_SETTINGS: {
            DrawText("SETTINGS", 100, 50, 40, WHITE);

            int left_x = 100;
            int start_y = 120;
            int item_spacing = 48;

            Color pw_color = (breakout_state->selected_settings_section == BREAKOUT_SETTINGS_PADDLE_WIDTH) ? RED : WHITE;
            DrawText(TextFormat("Paddle Width:       %d", breakout_state->paddle_width), left_x, start_y, 24, pw_color);

            Color rows_color = (breakout_state->selected_settings_section == BREAKOUT_SETTINGS_ROWS) ? RED : WHITE;
            DrawText(TextFormat("Rows:               %d", breakout_state->brick_rows), left_x, start_y + item_spacing, 24, rows_color);

            Color cols_color = (breakout_state->selected_settings_section == BREAKOUT_SETTINGS_COLS) ? RED : WHITE;
            DrawText(TextFormat("Columns:            %d", breakout_state->brick_cols), left_x, start_y + item_spacing * 2, 24, cols_color);

            Color lives_color = (breakout_state->selected_settings_section == BREAKOUT_SETTINGS_LIVES) ? RED : WHITE;
            DrawText(TextFormat("Lives:              %d", breakout_state->lives), left_x, start_y + item_spacing * 3, 24, lives_color);

            Color level_color = (breakout_state->selected_settings_section == BREAKOUT_SETTINGS_LEVEL) ? RED : WHITE;
            DrawText(TextFormat("Level:              %d", breakout_state->level), left_x, start_y + item_spacing * 4, 24, level_color);

            Color ball_color = (breakout_state->selected_settings_section == BREAKOUT_SETTINGS_BALL_SIZE) ? RED : WHITE;
            DrawText(TextFormat("Ball Size:          %.1f", breakout_state->ball_rad), left_x, start_y + item_spacing * 5, 24, ball_color);

            DrawText("CONTROLS", 100, start_y + item_spacing * 6 + 20, 28, GRAY);

            const char *controls[2] = {"Key Left", "Key Right"};
            int keys[2] = {
                breakout_state->key_left,
                breakout_state->key_right
            };

            int controls_start_y = start_y + item_spacing * 7 + 20;
            for(int i = 0; i < 2; i++) {
                const char *key_name = GetKeyName(keys[i]);
                char buff[64];
                snprintf(buff, sizeof(buff), "%s:          %s", controls[i], key_name);

                Color color = WHITE;
                if(breakout_state->waiting_for_key && breakout_state->rebiding_key == i + 1) {
                    color = RED;
                } else if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_CONTROLS && breakout_state->selected_key_index == i) {
                    color = RED;
                }

                DrawText(buff, left_x, controls_start_y + i * item_spacing, 24, color);

                // Draw highlight box for selected key
                if(breakout_state->selected_settings_section == BREAKOUT_SETTINGS_CONTROLS && breakout_state->selected_key_index == i && !breakout_state->waiting_for_key) {
                    int text_width = MeasureText(buff, 24);
                    DrawRectangleLines(left_x - 5, controls_start_y + i * item_spacing - 5, text_width + 10, 35, RED);
                }

                // Draw highlight for rebinding
                if(breakout_state->waiting_for_key && breakout_state->rebiding_key == i + 1) {
                    int text_width = MeasureText(buff, 24);
                    DrawRectangle(left_x - 10, controls_start_y + i * item_spacing - 5, text_width + 20, 35, ColorAlpha(RED, 0.3));
                }
            }

            int instructions_y = GetScreenHeight() - 120;
            if(!breakout_state->waiting_for_key) {
                DrawText("UP/DOWN: Navigate   LEFT/RIGHT: Change values or select key   ENTER: Rebind key", left_x, instructions_y, 22, GRAY);
            } else {
                DrawText("PRESS A KEY TO BIND (ESC TO CANCEL)", GetScreenWidth()/2 - MeasureText("PRESS A KEY TO BIND (ESC TO CANCEL)", 25)/2, instructions_y + 20, 25, RED);
            }

            DrawText("Press ESC to return", left_x, GetScreenHeight() - 30, 20, GRAY);
            break;
        }
        case BREAKOUT_VICTORY: {
            DrawText("VICTORY!", GetScreenWidth()/2 - MeasureText("VICTORY!", 60)/2, GetScreenHeight()/2 - 100, 60, GREEN);
            DrawText(TextFormat("Score: %d", breakout_state->score), GetScreenWidth()/2 - MeasureText("Score: 100", 40)/2, GetScreenHeight()/2, 40, WHITE);
            DrawText("Press ESC to return", GetScreenWidth()/2 - MeasureText("Press ESC to return", 30)/2, GetScreenHeight()/2 + 80, 30, WHITE);

            if(IsKeyPressed(KEY_ESCAPE)) {
                breakout_state->current_screen = BREAKOUT_MENU;
                BreakoutInit(breakout_state);
            }

            breakout_state->victory_timer--;
            if(breakout_state->victory_timer <= 0) {
                breakout_state->current_screen = BREAKOUT_MENU;
                BreakoutInit(breakout_state);
            }
            break;
        }
        case BREAKOUT_GAME_OVER: {
            DrawText("GAME OVER", GetScreenWidth()/2 - MeasureText("GAME OVER", 60)/2, GetScreenHeight()/2 - 100, 60, RED);
            DrawText(TextFormat("Final Score: %d", breakout_state->score), GetScreenWidth()/2 - MeasureText("Final Score: 100", 40)/2, GetScreenHeight()/2, 40, WHITE);
            DrawText("Press ESC to return", GetScreenWidth()/2 - MeasureText("Press ESC to return", 30)/2, GetScreenHeight()/2 + 80, 30, WHITE);

            if(IsKeyPressed(KEY_ESCAPE)) {
                BreakoutInit(breakout_state);
            }

            breakout_state->game_over_timer--;
            if(breakout_state->game_over_timer <= 0) {
                BreakoutInit(breakout_state);
            }
            break;
        }
        case BREAKOUT_GAMEPLAY: {
            for(int i = 0; i < breakout_state->brick_count; i++) {
                if(breakout_state->bricks_active[i]) {
                    Color color;

                    switch(i / breakout_state->brick_cols) {
                        case 0: color = RED; break;
                        case 1: color = ORANGE; break;
                        case 2: color = YELLOW; break;
                        case 3: color = GREEN; break;
                        case 4: color = BLUE; break;
                        case 5: color = PURPLE; break;
                    }
                    DrawRectangleRec(breakout_state->bricks[i], color);
                }
            }

            // Draw paddle
            DrawRectangleRec(breakout_state->paddle, WHITE);

            // Draw ball
            DrawCircleV(breakout_state->ball_position, breakout_state->ball_rad, WHITE);

            // Instructions
            if(!breakout_state->ball_active) {
                DrawText("Press SPACE to Launch Ball", GetScreenWidth()/2 - MeasureText("Press SPACE to Launch Ball", 20)/2, GetScreenHeight()/2, 20, GRAY);
            }

            DrawText(TextFormat("Score: %d", breakout_state->score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Lives: %d", breakout_state->lives), GetScreenWidth() - 150, 10, 20, WHITE);
            DrawText(TextFormat("Level: %d", breakout_state->level), GetScreenWidth()/2 - 50, 10, 20, WHITE);
            break;
        }
        case BREAKOUT_PAUSED: {
            for(int i = 0; i < breakout_state->brick_count; i++) {
                if(breakout_state->bricks_active[i]) {
                    Color color;

                    switch(i / breakout_state->brick_cols) {
                        case 0: color = RED; break;
                        case 1: color = ORANGE; break;
                        case 2: color = YELLOW; break;
                        case 3: color = GREEN; break;
                        case 4: color = BLUE; break;
                        case 5: color = PURPLE; break;
                    }
                    DrawRectangleRec(breakout_state->bricks[i], color);
                }
            }

            DrawRectangleRec(breakout_state->paddle, WHITE);
            DrawCircleV(breakout_state->ball_position, breakout_state->ball_rad, WHITE);

            DrawText(TextFormat("Score: %d", breakout_state->score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Lives: %d", breakout_state->lives), GetScreenWidth() - 150, 10, 20, WHITE);
            DrawText(TextFormat("Level: %d", breakout_state->level), GetScreenWidth()/2 - 50, 10, 20, WHITE);

            DrawText("PAUSED", GetScreenWidth()/2 - MeasureText("PAUSED", 40)/2, GetScreenHeight()/2 - 100, 40, WHITE);

            int menu_start_y = GetScreenHeight()/2 - 20;
            int menu_spacing = 50;

            Color resume_color = (breakout_state->selected_pause == BREAKOUT_PAUSE_RESUME) ? RED : WHITE;
            Color restart_color = (breakout_state->selected_pause == BREAKOUT_PAUSE_RESTART) ? RED : WHITE;
            Color settings_color = (breakout_state->selected_pause == BREAKOUT_PAUSE_SETTINGS) ? RED : WHITE;
            Color quit_color = (breakout_state->selected_pause == BREAKOUT_PAUSE_QUIT) ? RED : WHITE;

            DrawText("Resume", GetScreenWidth()/2 - MeasureText("Resume", 30)/2, menu_start_y, 30, resume_color);
            DrawText("Restart", GetScreenWidth()/2 - MeasureText("Restart", 30)/2, menu_start_y + menu_spacing, 30, restart_color);
            DrawText("Settings", GetScreenWidth()/2 - MeasureText("Settings", 30)/2, menu_start_y + menu_spacing * 2, 30, settings_color);
            DrawText("Quit to Menu", GetScreenWidth()/2 - MeasureText("Quit to Menu", 30)/2, menu_start_y + menu_spacing * 3, 30, quit_color);

            DrawText("Use UP/DOWN to navigate, ENTER to select", GetScreenWidth()/2 - MeasureText("Use UP/DOWN to navigate, ENTER to select", 20)/2, GetScreenHeight() - 50, 20, GRAY);
            break;
        }
    }
}

void BreakoutClose(BreakoutGameState *state) {
    BreakoutGameState *breakout_state = (BreakoutGameState*)state;
    UnloadSound(breakout_state->paddle_sound);
    UnloadSound(breakout_state->brick_sound);
    UnloadSound(breakout_state->wall_hit_sound);
    UnloadSound(breakout_state->lose_life_sound);
    UnloadSound(breakout_state->game_over_sound);
    UnloadSound(breakout_state->victory_sound);
}
