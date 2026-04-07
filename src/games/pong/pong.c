#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "pong.h"
#include "utils.h"

#define DIFFICULTY_LEVELS 6
float difficulty_speeds[DIFFICULTY_LEVELS] = {2.3, 2.6, 2.9, 3.2, 3.5, 4.5};

#define VICTORY_POINTS_OPTIONS 4
int victory_points_options[VICTORY_POINTS_OPTIONS] = {5, 10, 15, 20};

void PongInit(void *state) {
    PongGameState *pong_state = (PongGameState*)state;

    pong_state->current_screen = PONG_MENU;
    pong_state->prev_screen = PONG_MENU;

    pong_state->vs_computer = false;

    pong_state->player_paddle = (Rectangle){20, GetScreenHeight()/2 - 50, 20, 100};
    pong_state->oponent_paddle = (Rectangle){GetScreenWidth() - 40, GetScreenHeight()/2 - 50, 20, 100};

    pong_state->ball_position = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};
    pong_state->ball_speed = (Vector2){5, 3};
    pong_state->ball_rad = 10;
    pong_state->trail_index = 0;
    pong_state->trail_enabled = false;
    for(int i = 0; i < BALL_TRAIL_LENGHT; i++) {
        pong_state->ball_trail[i] = (Vector2){0, 0};
    }

    pong_state->player_score = 0;
    pong_state->oponent_score = 0;
    pong_state->game_over = false;

    pong_state->difficulty_level = 2;
    pong_state->victory_points = 10;
    pong_state->computer_movement_speed = difficulty_speeds[pong_state->difficulty_level];

    pong_state->paddle_hit_sound = LoadSound("assets/audio/pong/pong_paddle_hit.wav");
    pong_state->score_sound = LoadSound("assets/audio/pong/pong_score.wav");
    pong_state->wall_hit_sound = LoadSound("assets/audio/pong/pong_wall_hit.wav");
    pong_state->victory_sound = LoadSound("assets/audio/pong/pong_victory.wav");

    pong_state->selected_pause = PAUSE_RESUME;

    pong_state->key_player_up = KEY_W;
    pong_state->key_player_down = KEY_S;
    pong_state->key_opponent_up = KEY_UP;
    pong_state->key_opponent_down = KEY_DOWN;
    pong_state->waiting_for_key = false;
    pong_state->rebind_key = 0;
    pong_state->selected_key_index = 0;

    pong_state->selected_settings_section = SETTINGS_VICTORY_POINTS;
}

bool PongUpdate(void *state) {
    PongGameState *pong_state = (PongGameState*)state;

    if(IsWindowResized()) {
        // Keep player paddle on left side (x stays at 20)
        // Update opponent paddle to stay on right side
        pong_state->oponent_paddle.x = GetScreenWidth() - 40;

        // Adjust paddle Y position to suite new screen bounds
        if(pong_state->player_paddle.y + pong_state->player_paddle.height > GetScreenHeight()) {
            pong_state->player_paddle.y = GetScreenHeight() - pong_state->player_paddle.height;
        }
        if(pong_state->oponent_paddle.y + pong_state->oponent_paddle.height > GetScreenHeight()) {
            pong_state->oponent_paddle.y = GetScreenHeight() - pong_state->oponent_paddle.height;
        }

        // Adjust ball position to new screen bounds
        if(pong_state->ball_position.x > GetScreenWidth()) {
            pong_state->ball_position.x = GetScreenWidth() - 20;
        }
        if(pong_state->ball_position.y > GetScreenHeight()) {
            pong_state->ball_position.y = GetScreenHeight() - 20;
        }
    }

    if(pong_state->waiting_for_key) {
        for(int i = FIRST_KEY; i < LAST_KEY; i++) {
            if(IsKeyPressed(i)) {
                switch(pong_state->rebind_key) {
                    case 1: pong_state->key_player_up = i; break;
                    case 2: pong_state->key_player_down = i; break;
                    case 3: pong_state->key_opponent_up = i; break;
                    case 4: pong_state->key_opponent_down = i; break;
                }
                pong_state->waiting_for_key = false;
                break;
            }
        }
        // To cancle rebinding
        if(IsKeyPressed(KEY_ESCAPE)) {
            pong_state->waiting_for_key = false;
        }
    }

    if(IsKeyPressed(KEY_ESCAPE)) {
        if(pong_state->current_screen == PONG_GAMEPLAY) {
            pong_state->current_screen = PONG_PAUSED;
        } else if(pong_state->current_screen == PONG_PAUSED) {
            pong_state->current_screen = PONG_GAMEPLAY;
        } else if(pong_state->current_screen == PONG_SETTINGS) {
            pong_state->current_screen = pong_state->prev_screen;
        } else {
            // Menu and Victory screen
            return false;
        }
    }

    if(pong_state->current_screen == PONG_MENU) {
        if (IsKeyPressed(KEY_ONE)) {
            pong_state->vs_computer = false;
            pong_state->current_screen = PONG_GAMEPLAY;
        } else if (IsKeyPressed(KEY_TWO)) {
            pong_state->vs_computer = true;
            pong_state->current_screen = PONG_GAMEPLAY;
        } else if (IsKeyPressed(KEY_S)) {
            pong_state->current_screen = PONG_SETTINGS;
        }
    }

    if(pong_state->current_screen == PONG_PAUSED) {
        if(IsKeyPressed(KEY_UP)) {
            pong_state->selected_pause =(pong_state->selected_pause + PAUSE_OPTION_COUT - 1) % PAUSE_OPTION_COUT;
        } else if(IsKeyPressed(KEY_DOWN)) {
            pong_state->selected_pause = (pong_state->selected_pause + 1) % PAUSE_OPTION_COUT;
        } else if(IsKeyPressed(KEY_ENTER)) {
            switch(pong_state->selected_pause) {
                case PAUSE_RESUME: {
                        pong_state->current_screen = PONG_GAMEPLAY;
                        break;
                }
                case PAUSE_RESTART: {
                        bool vs_computer = pong_state->vs_computer;
                        PongInit(state);
                        pong_state->vs_computer = vs_computer;
                        pong_state->current_screen = PONG_GAMEPLAY;
                        break;
                }
                case PAUSE_SETTINGS: {
                        pong_state->prev_screen = PONG_PAUSED;
                        pong_state->current_screen = PONG_SETTINGS;
                        break;
                }
                case PAUSE_QUIT: {
                        pong_state->current_screen = PONG_MENU;
                        PongInit(state);
                        break;
                }
            }
        }
    }

    if(pong_state->current_screen == PONG_GAMEPLAY) {
        // Player movement
        float prev_player_y = pong_state->player_paddle.y;
        float prev_oponent_y = pong_state->oponent_paddle.y;

        if(IsKeyDown(pong_state->key_player_up) && pong_state->player_paddle.y > 0) {
            pong_state->player_paddle.y -= 7;
        }
        if(IsKeyDown(pong_state->key_player_down) && pong_state->player_paddle.y + pong_state->player_paddle.height < GetScreenHeight()) {
            pong_state->player_paddle.y += 7;
        }
        pong_state->player_paddle_speed = pong_state->player_paddle.y - prev_player_y;

        // Computer movement
        if(pong_state->vs_computer) {
            if(pong_state->ball_speed.x > 0) { // React only if ball going towords oponent paddle
                float error = (float)GetRandomValue(-10, 10);
                float target = pong_state->ball_position.y - pong_state->oponent_paddle.height/2 + error;
                // Position where the paddle wants to get (aligns paddle with ball centre plus the offset)

                // Paddle movement towwords target
                if(pong_state->oponent_paddle.y < target) {
                    pong_state->oponent_paddle.y += pong_state->computer_movement_speed;
                } else if(pong_state->oponent_paddle.y > target) {
                    pong_state->oponent_paddle.y -= pong_state->computer_movement_speed;
                }
            }
        } else {
            if (IsKeyDown(pong_state->key_opponent_up) && pong_state->oponent_paddle.y > 0) {
                pong_state->oponent_paddle.y -= 7;
            }
            if (IsKeyDown(pong_state->key_opponent_down) && pong_state->oponent_paddle.y + pong_state->oponent_paddle.height < GetScreenHeight()) {
                pong_state->oponent_paddle.y += 7;
            }
        }
        pong_state->oponent_paddle_speed = pong_state->oponent_paddle.y - prev_oponent_y;

        // Ball movement
        pong_state->ball_position.x += pong_state->ball_speed.x;
        pong_state->ball_position.y += pong_state->ball_speed.y;

        // Ball collision with Top and Botom
        if (pong_state->ball_position.y <= 0 || pong_state->ball_position.y >= GetScreenHeight()) {
            pong_state->ball_speed.y *= -1;

            // Randomize a bit the bouncing from colliding with the wall
            float randomFactor = ((float)GetRandomValue(-30, 30) / 100.0f);
            pong_state->ball_position.x += randomFactor;

            PlaySound(pong_state->wall_hit_sound);
        }

        // Ball collision with Paddle
        if (CheckCollisionCircleRec(pong_state->ball_position, pong_state->ball_rad, pong_state->player_paddle) ||
            CheckCollisionCircleRec(pong_state->ball_position, pong_state->ball_rad, pong_state->oponent_paddle)) {

            // Calculate relative intersection point (0.0 1.0)
            Rectangle *paddle = CheckCollisionCircleRec(pong_state->ball_position, pong_state->ball_rad, pong_state->player_paddle) ? &pong_state->player_paddle : &pong_state->oponent_paddle;
            float relative_y_intersection = (pong_state->ball_position.y - paddle->y) / paddle->height;

            float paddle_speed = (paddle == &pong_state->player_paddle) ? pong_state->player_paddle_speed : pong_state->oponent_paddle_speed;

            /* Adjust ball speed based on where it hit
               The middle of the paddle does not change the y of balls trajectory
               Top and bottom of paddle increase y speed */

            pong_state->ball_speed.y = (relative_y_intersection - 0.5) * 10 + paddle_speed * 0.2; // Scale factor of the bounce angle

            pong_state->ball_speed.x *= -1.0; // Just reverse movement

            if(fabsf(paddle_speed) > 0.1) { // If paddle is moving
                pong_state->ball_speed.x *= 1.1; // Increse by 10% speed
            } else { // If paddle is static
                pong_state->ball_speed.x *= 0.90; // Decrease by 10% speed
            }

            PlaySound(pong_state->paddle_hit_sound);
        }

        if(pong_state->trail_enabled) {
            for(int i = BALL_TRAIL_LENGHT - 1; i > 0; i--) {
                pong_state->ball_trail[i] = pong_state->ball_trail[i - 1];
            }
            // Current ball position at begining
            pong_state->ball_trail[0] = pong_state->ball_position;
        }

        // Scoring
        if (pong_state->ball_position.x <= 0) {
            pong_state->oponent_score++;
            pong_state->ball_position = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};
            pong_state->ball_speed = (Vector2){5, 3};
            PlaySound(pong_state->score_sound);
        } else if (pong_state->ball_position.x >= GetScreenWidth()) {
            pong_state->player_score++;
            pong_state->ball_position = (Vector2){GetScreenWidth()/2, GetScreenHeight()/2};
            pong_state->ball_speed = (Vector2){-5, 3};
            PlaySound(pong_state->score_sound);
        }

        // Check for Victory
        if(pong_state->player_score >= pong_state->victory_points) {
            pong_state->victory_player = 0;
            pong_state->victory_timer = VICTORY_SCREEN_DURATION;
            pong_state->current_screen = PONG_VICTORY;
            PlaySound(pong_state->victory_sound);
        } else if(pong_state->oponent_score >= pong_state->victory_points) {
            pong_state->victory_player = 1;
            pong_state->victory_timer = VICTORY_SCREEN_DURATION;
            pong_state->current_screen = PONG_VICTORY;
            PlaySound(pong_state->victory_sound);
        }
    }
    return true;
}

void PongDraw(void *state) {
    PongGameState *pong_state = (PongGameState*)state;

    switch(pong_state->current_screen) {
        case PONG_MENU: {
                DrawText("PONG", GetScreenWidth()/2 - MeasureText("PONG", 60)/2, GetScreenHeight()/4, 60, WHITE);

                // Draw options
                DrawText("Press 1 for PvP", GetScreenWidth()/2 - MeasureText("Press 1 for PvP", 30)/2, GetScreenHeight()/2, 30, WHITE);
                DrawText("Press 2 for PvE", GetScreenWidth()/2 - MeasureText("Press 2 for PvE", 30)/2, GetScreenHeight()/2 + 40, 30, WHITE);
                DrawText("Press S for Settings", GetScreenWidth()/2 - MeasureText("Press S for Settings", 30)/2, GetScreenHeight()/2 + 80, 30, GRAY);
                DrawText("Press ESC to Exit", GetScreenWidth()/2 - MeasureText("Press ESC to Exit", 30)/2, GetScreenHeight()/2 + 120, 30, GRAY);
                break;
        }
        case PONG_GAMEPLAY: {
                // Player paddle texture
                DrawRectangleRec(pong_state->player_paddle, WHITE);
                DrawRectangleRec(pong_state->oponent_paddle, WHITE);

                // Draw ball
                DrawCircleV(pong_state->ball_position, pong_state->ball_rad, WHITE);

                if(pong_state->trail_enabled) {
                    for(int i = 0; i < BALL_TRAIL_LENGHT; i++) {
                        if(i == 0) continue; // Skip current ball position

                        Vector2 position = pong_state->ball_trail[i];

                        if(position.x <= 0 || position.y <= 0) continue; // Skip invalid position

                        float alpha = 1 - (i - 10);

                        float size = pong_state->ball_rad * 0.7 * (1 - i/10);

                        Color trail = ColorAlpha(WHITE, alpha);
                        DrawCircleV(position, size, trail);
                    }
                }

                // Draw points
                DrawText(TextFormat("%d", pong_state->player_score), GetScreenWidth()/4, 50, 30, WHITE);
                DrawText(TextFormat("%d", pong_state->oponent_score), 3*GetScreenWidth()/4, 50, 30, WHITE);
                DrawText(TextFormat("%d", GetFPS()), 2*GetScreenWidth()/4, 50, 30, WHITE);
                break;
        }
        case PONG_PAUSED: {
                DrawRectangleRec(pong_state->player_paddle, WHITE);
                DrawRectangleRec(pong_state->oponent_paddle, WHITE);
                DrawCircleV(pong_state->ball_position, pong_state->ball_rad, WHITE);
                DrawText(TextFormat("%d", pong_state->player_score), GetScreenWidth()/4, 50, 30, WHITE);
                DrawText(TextFormat("%d", pong_state->oponent_score), 3*GetScreenWidth()/4, 50, 30, WHITE);

                DrawText("PAUSED", GetScreenWidth()/2 - MeasureText("PAUSED", 40)/2, GetScreenHeight()/2 - 100, 40, WHITE);

                // Draw pause menu options
                char *pause_options[PAUSE_OPTION_COUT] = {"Resume", "Restart", "Settings", "Quit to Menu"};
                for (int i = 0; i < PAUSE_OPTION_COUT; i++) {
                    Color color = (i == pong_state->selected_pause) ? RED : WHITE;
                    DrawText(pause_options[i], GetScreenWidth()/2 - MeasureText(pause_options[i], 30)/2, GetScreenHeight()/2 - 20 + i * 40, 30, color);
                }
                break;
        }
        case PONG_SETTINGS: {
                DrawText("SETTINGS", 100, 50, 40, WHITE);

                int left_x = 100;
                int start_y = 120;
                int item_spacing = 48;

                if(!pong_state->waiting_for_key) {
                    // UP/DOWN: Navigate between sections
                    if(IsKeyPressed(KEY_UP)) {
                        pong_state->selected_settings_section = (pong_state->selected_settings_section + SETTINGS_SECTION_COUNT - 1) % SETTINGS_SECTION_COUNT;
                        if(pong_state->selected_settings_section == SETTINGS_CONTROLS) {
                            pong_state->selected_key_index = 0; // Reset to first key when entering controls
                        }
                    }
                    if(IsKeyPressed(KEY_DOWN)) {
                        pong_state->selected_settings_section = (pong_state->selected_settings_section + 1) % SETTINGS_SECTION_COUNT;
                        if(pong_state->selected_settings_section == SETTINGS_CONTROLS) {
                            pong_state->selected_key_index = 0; // Reset to first key when entering controls
                        }
                    }

                    // LEFT/RIGHT: Modify values or navigate keys
                    if(pong_state->selected_settings_section == SETTINGS_VICTORY_POINTS) {
                        if(IsKeyPressed(KEY_LEFT)) {
                            for(int i = 0; i < VICTORY_POINTS_OPTIONS; i++) {
                                if(victory_points_options[i] == pong_state->victory_points) {
                                    pong_state->victory_points = victory_points_options[(i + VICTORY_POINTS_OPTIONS - 1) % VICTORY_POINTS_OPTIONS];
                                    break;
                                }
                            }
                        }
                        if(IsKeyPressed(KEY_RIGHT)) {
                            for(int i = 0; i < VICTORY_POINTS_OPTIONS; i++) {
                                if(victory_points_options[i] == pong_state->victory_points) {
                                    pong_state->victory_points = victory_points_options[(i + 1) % VICTORY_POINTS_OPTIONS];
                                    break;
                                }
                            }
                        }
                    } else if(pong_state->selected_settings_section == SETTINGS_TRAIL_EFFECT) {
                        if(IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                            pong_state->trail_enabled = !pong_state->trail_enabled;
                        }
                    } else if(pong_state->selected_settings_section == SETTINGS_DIFFICULTY) {
                        if(IsKeyPressed(KEY_LEFT) && pong_state->difficulty_level > 0) {
                            pong_state->difficulty_level--;
                            pong_state->computer_movement_speed = difficulty_speeds[pong_state->difficulty_level];
                        }
                        if(IsKeyPressed(KEY_RIGHT) && pong_state->difficulty_level < DIFFICULTY_LEVELS - 1) {
                            pong_state->difficulty_level++;
                            pong_state->computer_movement_speed = difficulty_speeds[pong_state->difficulty_level];
                        }
                    } else if(pong_state->selected_settings_section == SETTINGS_CONTROLS) {
                        if(IsKeyPressed(KEY_LEFT)) {
                            pong_state->selected_key_index = (pong_state->selected_key_index + 3) % 4;
                        }
                        if(IsKeyPressed(KEY_RIGHT)) {
                            pong_state->selected_key_index = (pong_state->selected_key_index + 1) % 4;
                        }
                        // ENTER: Start rebinding selected key
                        if(IsKeyPressed(KEY_ENTER)) {
                            pong_state->waiting_for_key = true;
                            pong_state->rebind_key = pong_state->selected_key_index + 1;
                        }
                    }
                }

                // Points section
                Color vp_color = (pong_state->selected_settings_section == SETTINGS_VICTORY_POINTS) ? RED : WHITE;
                DrawText(TextFormat("Victory Points:    %d", pong_state->victory_points), left_x, start_y, 24, vp_color);

                // Difficulty section
                Color diff_color = (pong_state->selected_settings_section == SETTINGS_DIFFICULTY) ? RED : WHITE;
                DrawText(TextFormat("Difficulty:        %d", pong_state->difficulty_level + 1), left_x, start_y + item_spacing, 24, diff_color);

                Color trail_color = (pong_state->selected_settings_section == SETTINGS_TRAIL_EFFECT) ? RED : WHITE;
                DrawText(TextFormat("Trail Effect:      %s", pong_state->trail_enabled ? "ON" : "OFF"), left_x, start_y + item_spacing * 2, 24, trail_color);

                DrawText("CONTROLS", 100, start_y + item_spacing * 3 + 20, 28, GRAY);

                const char *controls[4] = {"Player Up", "Player Down", "Opponent Up", "Opponent Down"};
                int keys[4] = {
                    pong_state->key_player_up,
                    pong_state->key_player_down,
                    pong_state->key_opponent_up,
                    pong_state->key_opponent_down
                };

                int controls_start_y = start_y + item_spacing * 4 + 20;
                for(int i = 0; i < 4; i++) {
                    const char *key_name = GetKeyName(keys[i]);
                    char buff[64];
                    snprintf(buff, sizeof(buff), "%s:     %s", controls[i], key_name);

                    Color color = WHITE;
                    if(pong_state->waiting_for_key && pong_state->rebind_key == i + 1) {
                        color = RED;
                    } else if(pong_state->selected_settings_section == SETTINGS_CONTROLS && pong_state->selected_key_index == i) {
                        color = RED;
                    }

                    DrawText(buff, left_x, controls_start_y + i * item_spacing, 24, color);

                    // Draw highlight box for selected key
                    if(pong_state->selected_settings_section == SETTINGS_CONTROLS && pong_state->selected_key_index == i && !pong_state->waiting_for_key) {
                        int text_width = MeasureText(buff, 24);
                        DrawRectangleLines(left_x - 5, controls_start_y + i * item_spacing - 5, text_width + 10, 35, RED);
                    }

                    // Draw highlight for rebinding
                    if(pong_state->waiting_for_key && pong_state->rebind_key == i + 1) {
                        int text_width = MeasureText(buff, 24);
                        DrawRectangle(left_x - 10, controls_start_y + i * item_spacing - 5, text_width + 20, 35, ColorAlpha(RED, 0.3));
                    }
                }

                int instructions_y = GetScreenHeight() - 120;
                if(!pong_state->waiting_for_key) {
                    DrawText("UP/DOWN: Navigate   LEFT/RIGHT: Change values or select key   ENTER: Rebind key", left_x, instructions_y, 22, GRAY);
                } else {
                    DrawText("PRESS A KEY TO BIND (ESC TO CANCEL)", GetScreenWidth()/2 - MeasureText("PRESS A KEY TO BIND (ESC TO CANCEL)", 25)/2, instructions_y + 20, 25, RED);
                }

                DrawText("Press ESC to return", left_x, GetScreenHeight() - 30, 20, GRAY);
                break;
        }
        case PONG_VICTORY: {
                const char *winner = (pong_state->victory_player == 0) ? "PLAYER 1 WINS!" : "PLAYER 2 WINS!";

                DrawText(winner, GetScreenWidth()/2 - MeasureText(winner, 50)/2, GetScreenHeight()/2 - 50, 50, GREEN);

                DrawText("Press M to return to menu", GetScreenWidth()/2 - MeasureText("Press M to return to menu", 30)/2, GetScreenHeight()/2 + 50, 30, WHITE);

                pong_state->victory_timer--;
                if(pong_state->victory_timer <= 0 || IsKeyPressed(KEY_M)) {
                    pong_state->current_screen = PONG_MENU;
                    PongInit(state);
                }
                break;
        }
    }
}

void PongClose(void *state) {
    PongGameState *pong_state = (PongGameState*)state;
    UnloadSound(pong_state->paddle_hit_sound);
    UnloadSound(pong_state->score_sound);
    UnloadSound(pong_state->wall_hit_sound);
    UnloadSound(pong_state->victory_sound);
}
