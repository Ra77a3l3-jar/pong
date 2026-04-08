#ifndef SNAKE_H
#define SNAKE_H

#include <raylib.h>
#include <stdbool.h>

#define SNAKE_WIDTH 7
#define SNAKE_HEIGHT 7

typedef enum {
    SNAKE_MENU,
    SNAKE_GAMEPLAY,
    SNAKE_VICTORY,
    SNAKE_GAME_OVER
} SnakeScreen;

typedef struct {
    SnakeScreen screen;
    SnakeScreen prev_screen;

    int grid_size;
    int grid_width;
    int grid_height;

    Rectangle snake_head;
    Rectangle snake_body[100];
    int snake_speed;
    int snake_direction; // 0: up 1: right 2: down 3: left

    Vector2 food;
    int food_timer;

    int level;
    int score;
    bool game_over;
    bool victory;
    int victory_timer;
    int game_over_timer;

    int key_left;
    int key_right;
    int key_up;
    int key_down;

} SnakeGameState;

void SnakeInit(SnakeGameState *state);
bool SnakeUpdate(SnakeGameState *state);
void SnakeDraw(SnakeGameState *state);
void SnakeClose(SnakeGameState *state);

#endif
