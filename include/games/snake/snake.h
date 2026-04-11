#ifndef SNAKE_H
#define SNAKE_H

#include <raylib.h>
#include <stdbool.h>

#define SNAKE_SIZE 20
#define INIT_SNAKE_LENGTH 3
#define MAX_SNAKE_LENGHT 100
#define MAX_TRIES_FOOD 60
#define GRID_SIZE 20
#define SPEED 8

#define VICTORY_SCREEN_DURATION 600 // 5 seconds at 120fps
#define GAMEOVER_SCREEN_DURATION 600

typedef enum {
    SNAKE_PAUSE_RESUME,
    SNAKE_PAUSE_RESTART,
    SNAKE_PAUSE_QUIT,
    SNAKE_PAUSE_OPTION_COUNT
} SnakePauseOptions;

typedef enum {
    SNAKE_MENU,
    SNAKE_GAMEPLAY,
    SNAKE_PAUSE,
    SNAKE_VICTORY,
    SNAKE_GAME_OVER
} SnakeScreen;

typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
} SnakeDirections;

typedef struct {
    SnakeScreen current_screen;
    SnakeScreen prev_screen;

    int cell_size;
    int grid_width;
    int grid_height;

    Rectangle snake_body[MAX_SNAKE_LENGHT];
    int lenght;
    int speed;
    int move_counter;
    SnakeDirections direction;
    SnakeDirections next_direction;

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

    SnakePauseOptions selected_pause;
} SnakeGameState;

void SnakeInit(SnakeGameState *state);
bool SnakeUpdate(SnakeGameState *state);
void SnakeDraw(SnakeGameState *state);
void SnakeClose(SnakeGameState *state);

#endif
