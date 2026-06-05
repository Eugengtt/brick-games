#ifndef GAMES_H
#define GAMES_H

#include <windows.h>
#include <vector>

extern const int GRID_W;
extern const int GRID_H;
extern const int CELL;
extern const int WIN_W;
extern const int WIN_H;

extern const COLORREF BG;
extern const COLORREF ON;
extern const COLORREF PLAYER;
extern const COLORREF OFF;

// Space Invaders a fost eliminat de aici
enum GameType { GAME_TETRIS, GAME_SNAKE, GAME_TANK, GAME_PONG, GAME_BREAKOUT, GAME_COUNT };
extern const char* GameNames[];

extern int screen[20][10];
extern bool inMenu;
extern int selectedGameType;
extern bool gameRunning;
extern bool gameOver;
extern bool gameYouWin;
extern int gameScore;

extern int currentLevel;
extern int enemiesKilledInLevel;
extern int enemiesRequiredForLevel;

void InitCurrentGame();
void UpdateCurrentGame();
void HandleCurrentGameKey(int key);

#endif
