#include "games.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

const int GRID_W = 10;
const int GRID_H = 20;
const int CELL = 28;
const int WIN_W = GRID_W * CELL + 260;
const int WIN_H = GRID_H * CELL + 90;

const COLORREF BG = RGB(15, 25, 15);
const COLORREF ON = RGB(40, 180, 40);
const COLORREF PLAYER = RGB(240, 180, 20);
const COLORREF OFF = RGB(5, 12, 5);

// Lista de jocuri
const char* GameNames[] = {
    "1. TETRIS",
    "2. SNAKE CLASSIC",
    "3. TANK BATTALION",
    "4. PONG RETRO",
    "5. BRICK BREAKOUT"
};

int screen[20][10];
bool inMenu = true;
int selectedGameType = GAME_TETRIS;
bool gameRunning = false;
bool gameOver = false;
bool gameYouWin = false;
int gameScore = 0;

int currentLevel = 1;
int enemiesKilledInLevel = 0;
int enemiesRequiredForLevel = 10;

// --- VARIABILE PONG ---
int pongPaddleX;
int pongBotX;
float pongBallX, pongBallY;
float pongBallDX, pongBallDY;
int pongPlayerScore = 0;
int pongBotScore = 0;
int pongRound = 1;
DWORD pongTimer;
float pongSpeedMultiplier = 1.0f;
bool botDecidedError = false;
int botErrorOffset = 0;

// --- VARIABILE BREAKOUT ---
int breakoutPaddleX;
float breakoutBallX, breakoutBallY;
float breakoutBallDX, breakoutBallDY;
int breakoutGrid[20][10];
int breakoutLevel = 1;
int breakoutLives = 3;
DWORD breakoutTimer;

// --- VARIABILE SNAKE ---
std::vector<std::pair<int,int>> snake;
int sDirX, sDirY, foodX, foodY;
DWORD snakeTimer;

// --- FORME GEOMETRICE BRICK GAME (BREAKOUT) ---
const int SHAPE_HEART[20][10] = {
    {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0},
    {0,1,1,0,0,0,0,1,1,0}, {1,1,1,1,0,0,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1}, {0,1,1,1,1,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,0,0}, {0,0,0,1,1,1,1,0,0,0},
    {0,0,0,0,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}
};
const int SHAPE_NUMBER7[20][10] = {
    {0,0,0,0,0,0,0,0,0,0}, {0,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,0}, {0,0,0,0,0,0,1,1,0,0},
    {0,0,0,0,0,1,1,0,0,0}, {0,0,0,0,1,1,0,0,0,0},
    {0,0,0,1,1,0,0,0,0,0}, {0,0,1,1,0,0,0,0,0,0},
    {0,0,1,1,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}
};
const int SHAPE_DUMBBELL[20][10] = {
    {0,0,0,0,0,0,0,0,0,0}, {0,1,1,1,0,0,1,1,1,0},
    {0,1,1,1,0,0,1,1,1,0}, {0,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,0}, {0,1,1,1,0,0,1,1,1,0},
    {0,1,1,1,0,0,1,1,1,0}, {0,0,0,0,0,0,0,0,0,0}
};

// --- LOGICĂ JOC: TETRIS ---
int tetrisGrid[20][10];
int tetrisPiece[4][4];
int tetrisPX, tetrisPY;
DWORD tetrisTimer;
const int T_SHAPES[7][4][4] = {
    {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
    {{0,0,0,0},{0,1,0,0},{1,1,1,0},{0,0,0,0}}, {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
    {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}}, {{0,0,0,0},{1,0,0,0},{1,1,1,0},{0,0,0,0}},
    {{0,0,0,0},{0,0,1,0},{1,1,1,0},{0,0,0,0}}
};
void Tetris_Init() {
    for(int y=0; y<20; y++) for(int x=0; x<10; x++) tetrisGrid[y][x] = 0;
    gameScore = 0; gameOver = false; gameYouWin = false;
    int t = rand() % 7;
    for(int i=0;i<4;i++) for(int j=0;j<4;j++) tetrisPiece[i][j] = T_SHAPES[t][i][j];
    tetrisPX = 10/2 - 2; tetrisPY = 0; tetrisTimer = GetTickCount();
}
bool Tetris_Collision(int x, int y, int p[4][4]) {
    for(int i=0;i<4;i++) for(int j=0;j<4;j++) if(p[i][j]) {
        int nx = x+j, ny = y+i;
        if(nx<0 || nx>=10 || ny>=20) return true;
        if(ny>=0 && tetrisGrid[ny][nx]) return true;
    }
    return false;
}
void Tetris_Merge() {
    for(int i=0;i<4;i++) for(int j=0;j<4;j++) if(tetrisPiece[i][j] && tetrisPY+i>=0) tetrisGrid[tetrisPY+i][tetrisPX+j] = 1;
    for(int y=20-1; y>=0; y--) {
        bool full = true;
        for(int x=0; x<10; x++) if(!tetrisGrid[y][x]) { full=false; break; }
        if(full) {
            for(int k=y; k>0; k--) for(int x=0; x<10; x++) tetrisGrid[k][x] = tetrisGrid[k-1][x];
            for(int x=0; x<10; x++) tetrisGrid[0][x] = 0;
            gameScore += 100; y++;
        }
    }
    int t = rand() % 7;
    for(int i=0;i<4;i++) for(int j=0;j<4;j++) tetrisPiece[i][j] = T_SHAPES[t][i][j];
    tetrisPX = 10/2 - 2; tetrisPY = 0;
    if(Tetris_Collision(tetrisPX, tetrisPY, tetrisPiece)) gameOver = true;
}
void Tetris_Update() {
    if(gameOver) return;
    if(GetTickCount() - tetrisTimer > 450) {
        tetrisTimer = GetTickCount();
        if(!Tetris_Collision(tetrisPX, tetrisPY+1, tetrisPiece)) tetrisPY++;
        else Tetris_Merge();
    }
    for(int y=0; y<20; y++) for(int x=0; x<10; x++) screen[y][x] = tetrisGrid[y][x];
    for(int i=0;i<4;i++) for(int j=0;j<4;j++) if(tetrisPiece[i][j] && tetrisPY+i>=0) screen[tetrisPY+i][tetrisPX+j] = 1;
}
void Tetris_Key(int key) {
    if(key == VK_LEFT && !Tetris_Collision(tetrisPX-1, tetrisPY, tetrisPiece)) tetrisPX--;
    if(key == VK_RIGHT && !Tetris_Collision(tetrisPX+1, tetrisPY, tetrisPiece)) tetrisPX++;
    if(key == VK_DOWN && !Tetris_Collision(tetrisPX, tetrisPY+1, tetrisPiece)) tetrisPY++;
    if(key == VK_UP) {
        int rot[4][4];
        for(int i=0;i<4;i++) for(int j=0;j<4;j++) rot[j][3-i] = tetrisPiece[i][j];
        if(!Tetris_Collision(tetrisPX, tetrisPY, rot)) memcpy(tetrisPiece, rot, sizeof(rot));
    }
}

// --- LOGICĂ JOC: SNAKE (ACTUALIZAT CU "YOU WIN") ---
void Snake_Init() {
    snake.clear();
    snake.push_back({5, 10}); snake.push_back({5, 11}); snake.push_back({5, 12});
    sDirX = 0; sDirY = -1; gameScore = 0; gameOver = false; gameYouWin = false;
    foodX = rand() % 10; foodY = rand() % (20 - 4) + 2; snakeTimer = GetTickCount();
}
void Snake_Update() {
    if(gameOver || gameYouWin) return;

    if(GetTickCount() - snakeTimer > 180) {
        snakeTimer = GetTickCount();
        int nX = snake[0].first + sDirX; int nY = snake[0].second + sDirY;

        if(nX < 0 || nX >= 10 || nY < 0 || nY >= 20) { gameOver = true; return; }
        for(size_t i=1; i<snake.size(); i++) if(snake[i].first == nX && snake[i].second == nY) { gameOver = true; return; }

        snake.insert(snake.begin(), {nX, nY});

        if(nX == foodX && nY == foodY) {
            gameScore += 15;

            // Verificare victorie absolută (Maxim 200 de pătrățele pe ecran)
            // Sfat util: Schimbă 200 cu 10 dacă vrei doar să testezi rapid ecranul de WIN!
            if(snake.size() >= 200) {
                gameYouWin = true;
                return;
            }

            // Generare bobiță nouă doar pe spații libere
            bool onSnake;
            do {
                onSnake = false;
                foodX = rand() % 10; foodY = rand() % 20;
                for(auto& segment : snake) {
                    if(segment.first == foodX && segment.second == foodY) { onSnake = true; break; }
                }
            } while(onSnake);
        }
        else {
            snake.pop_back();
        }
    }
    for(int y=0; y<20; y++) for(int x=0; x<10; x++) screen[y][x] = 0;
    for(size_t i=0; i<snake.size(); i++) screen[snake[i].second][snake[i].first] = (i == 0) ? 2 : 1;
    if(!gameYouWin) screen[foodY][foodX] = 1;
}
void Snake_Key(int key) {
    if(key == VK_LEFT && sDirX == 0) { sDirX = -1; sDirY = 0; }
    if(key == VK_RIGHT && sDirX == 0) { sDirX = 1; sDirY = 0; }
    if(key == VK_UP && sDirY == 0)  { sDirX = 0; sDirY = -1; }
    if(key == VK_DOWN && sDirY == 0) { sDirX = 0; sDirY = 1; }
}

// --- LOGICĂ JOC: TANK ---
struct Tank { int x, y; int dirX, dirY; bool active; DWORD lastShotTime; };
struct Bullet { int x, y; int dx, dy; bool isPlayer; bool active; };
Tank pTank; std::vector<Tank> eTanks; std::vector<Bullet> bullets;
int mapObstacles[20][10]; DWORD tankSpawnTimer, tankActionTimer; DWORD playerLastShotTime = 0;

void DrawTankShape(int tx, int ty, int dx, int dy, int type) {
    if(tx < 0 || tx >= 10 || ty < 0 || ty >= 20) return;
    screen[ty][tx] = type;
    if(dx != 0) {
        if(ty > 0) { screen[ty-1][tx] = type; }
        if(ty < 20-1) { screen[ty+1][tx] = type; }
        if(tx-dx >= 0 && tx-dx < 10) {
            if(ty > 0) { screen[ty-1][tx-dx] = type; }
            if(ty < 20-1) { screen[ty+1][tx-dx] = type; }
        }
        if(tx+dx >= 0 && tx+dx < 10) { screen[ty][tx+dx] = type; }
    } else {
        if(tx > 0) { screen[ty][tx-1] = type; }
        if(tx < 10-1) { screen[ty][tx+1] = type; }
        if(ty-dy >= 0 && ty-dy < 20) {
            if(tx > 0) { screen[ty-dy][tx-1] = type; }
            if(tx < 10-1) { screen[ty-dy][tx+1] = type; }
        }
        if(ty+dy >= 0 && ty+dy < 20) { screen[ty+dy][tx] = type; }
    }
}
bool TankCollideWithWalls(int cx, int cy) {
    for (int y = cy - 1; y <= cy + 1; y++) {
        for (int x = cx - 1; x <= cx + 1; x++) {
            if (x < 0 || x >= 10 || y < 0 || y >= 20) return true;
            if (mapObstacles[y][x] == 1) return true;
        }
    }
    return false;
}
void Tank_GenerateMap() {
    for(int y=0; y<20; y++) for(int x=0; x<10; x++) mapObstacles[y][x] = 0;
    switch(currentLevel) {
        case 1:
            for(int x = 1; x < 10 - 1; x++) { if(x != 4 && x != 5) { mapObstacles[5][x] = 1; mapObstacles[11][x] = 1; } }
            break;
        case 2:
            for(int y = 4; y < 16; y++) { if(y != 9 && y != 10) { mapObstacles[y][3] = 1; mapObstacles[y][6] = 1; } }
            break;
        case 3:
            for(int x = 2; x <= 7; x++) { mapObstacles[7][x] = 1; mapObstacles[12][x] = 1; }
            for(int y = 8; y <= 11; y++) { mapObstacles[y][2] = 1; mapObstacles[y][7] = 1; }
            mapObstacles[7][4] = 0; mapObstacles[7][5] = 0;
            break;
        case 4:
            for(int i = 3; i <= 15; i++) { int x1 = (i - 1) / 2; if(x1 >= 1 && x1 < 10-1) { mapObstacles[i][x1] = 1; mapObstacles[i][10 - 1 - x1] = 1; } }
            break;
        case 5:
            for(int x = 0; x < 10 - 2; x++) mapObstacles[5][x] = 1;
            for(int x = 2; x < 10; x++) mapObstacles[10][x] = 1;
            for(int x = 0; x < 10 - 2; x++) mapObstacles[15][x] = 1;
            break;
        case 6:
            for(int y=3; y<=5; y++) { mapObstacles[y][1] = 1; mapObstacles[y][10-2] = 1; }
            for(int y=13; y<=15; y++) { mapObstacles[y][1] = 1; mapObstacles[y][10-2] = 1; }
            break;
        case 7:
            for(int y = 4; y < 20 - 4; y += 3) { for(int x = 1; x < 10; x += 2) mapObstacles[y][x] = 1; }
            break;
        case 8:
            for(int x = 0; x < 10; x++) { if(x != 2 && x != 5 && x != 7) mapObstacles[9][x] = 1; }
            for(int x = 0; x < 10; x++) { if(x != 1 && x != 4 && x != 8) mapObstacles[11][x] = 1; }
            break;
        case 9:
            for(int y = 4; y < 16; y += 2) { mapObstacles[y][0] = 1; mapObstacles[y][10-1] = 1; }
            for(int x = 2; x <= 7; x++) mapObstacles[10][x] = 1;
            break;
        default:
            for(int x = 0; x < 10; x++) { if(x != 4 && x != 5) mapObstacles[4][x] = 1; }
            mapObstacles[9][2] = 1; mapObstacles[9][7] = 1; mapObstacles[14][4] = 1; mapObstacles[14][5] = 1;
            break;
    }
    for(int y=0; y<=2; y++) for(int x=0; x<10; x++) mapObstacles[y][x] = 0;
    for(int y=20-4; y<20; y++) for(int x=0; x<10; x++) mapObstacles[y][x] = 0;
}
void Tank_Init() {
    currentLevel = 1; enemiesKilledInLevel = 0; enemiesRequiredForLevel = 10; gameScore = 0; gameOver = false; gameYouWin = false; playerLastShotTime = 0;
    Tank_GenerateMap(); pTank = { 10 / 2, 20 - 3, 0, -1, true, 0 }; eTanks.clear(); bullets.clear();
    tankSpawnTimer = tankActionTimer = GetTickCount();
}
void Tank_NextLevel() {
    currentLevel++; if(currentLevel > 10) { gameYouWin = true; return; }
    enemiesKilledInLevel = 0; enemiesRequiredForLevel = 10 + (currentLevel - 1) * 3; Tank_GenerateMap();
    pTank = { 10 / 2, 20 - 3, 0, -1, true, 0 }; eTanks.clear(); bullets.clear(); playerLastShotTime = 0;
}
void Tank_Update() {
    if(gameOver || gameYouWin) return;
    DWORD now = GetTickCount();
    int activeAndRemaining = (int)eTanks.size() + enemiesKilledInLevel;
    if(now - tankSpawnTimer > 2300 && (int)eTanks.size() < 3 && activeAndRemaining < enemiesRequiredForLevel) {
        tankSpawnTimer = now; int spawnX = (rand() % 2 == 0) ? 1 : 10 - 2;
        if (!TankCollideWithWalls(spawnX, 1)) eTanks.push_back({ spawnX, 1, 0, 1, true, now });
    }
    if(now - tankActionTimer > 70) {
        tankActionTimer = now;
        for(size_t i=0; i<bullets.size(); i++) {
            if(!bullets[i].active) continue;
            bullets[i].x += bullets[i].dx; bullets[i].y += bullets[i].dy;
            if(bullets[i].x < 0 || bullets[i].x >= 10 || bullets[i].y < 0 || bullets[i].y >= 20) { bullets[i].active = false; continue; }
            if(mapObstacles[bullets[i].y][bullets[i].x] == 1) { mapObstacles[bullets[i].y][bullets[i].x] = 0; bullets[i].active = false; continue; }
        }
        for(size_t i=0; i<bullets.size(); i++) {
            if(!bullets[i].active) continue;
            for(size_t j=i+1; j<bullets.size(); j++) {
                if(!bullets[j].active) continue;
                if(bullets[i].x == bullets[j].x && (bullets[i].y == bullets[j].y || abs(bullets[i].y - bullets[j].y) <= 1)) { bullets[i].active = false; bullets[j].active = false; }
            }
        }
        for(size_t i=0; i<bullets.size(); i++) {
            if(!bullets[i].active) continue;
            if(bullets[i].isPlayer) {
                for(size_t j=0; j<eTanks.size(); j++) {
                    if(!eTanks[j].active) continue;
                    if(abs(bullets[i].x - eTanks[j].x) <= 1 && abs(bullets[i].y - eTanks[j].y) <= 1) {
                        eTanks[j].active = false; bullets[i].active = false; gameScore += 50; enemiesKilledInLevel++;
                        if(enemiesKilledInLevel >= enemiesRequiredForLevel) { Tank_NextLevel(); return; }
                    }
                }
            } else { if(abs(bullets[i].x - pTank.x) <= 1 && abs(bullets[i].y - pTank.y) <= 1) gameOver = true; }
        }
        static int moveDelay = 0;
        if(++moveDelay % 5 == 0) {
            for(auto& et : eTanks) {
                if(!et.active) continue;
                int nextY = et.y + 1;
                if(!TankCollideWithWalls(et.x, nextY)) et.y = nextY;
                else { int randDir = (rand() % 2 == 0) ? 1 : -1; if(!TankCollideWithWalls(et.x + randDir, et.y)) et.x += randDir; }
                if(now - et.lastShotTime > 900 && rand() % 3 == 0) { bullets.push_back({et.x, et.y + 2, 0, 1, false, true}); et.lastShotTime = now; }
            }
        }
    }
    eTanks.erase(std::remove_if(eTanks.begin(), eTanks.end(), [](const Tank& t){return !t.active;}), eTanks.end());
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b){return !b.active;}), bullets.end());
    for(int y=0; y<20; y++) for(int x=0; x<10; x++) screen[y][x] = mapObstacles[y][x];
    DrawTankShape(pTank.x, pTank.y, pTank.dirX, pTank.dirY, 2);
    for(auto& et : eTanks) if(et.active) DrawTankShape(et.x, et.y, et.dirX, et.dirY, 1);
    for(auto& b : bullets) if(b.active) screen[b.y][b.x] = 1;
}
void Tank_Key(int key) {
    int nextX = pTank.x, nextY = pTank.y;
    if(key == VK_LEFT)  { nextX--; pTank.dirX = -1; pTank.dirY = 0; }
    if(key == VK_RIGHT) { nextX++; pTank.dirX = 1;  pTank.dirY = 0; }
    if(key == VK_UP)    { nextY--; pTank.dirX = 0;  pTank.dirY = -1; }
    if(key == VK_DOWN)  { nextY++; pTank.dirX = 0;  pTank.dirY = 1; }
    if (!TankCollideWithWalls(nextX, nextY)) { pTank.x = nextX; pTank.y = nextY; }
    if(key == VK_SPACE || key == VK_RETURN) {
        DWORD now = GetTickCount();
        if (now - playerLastShotTime >= 450) { bullets.push_back({ pTank.x + pTank.dirX*2, pTank.y + pTank.dirY*2, pTank.dirX, pTank.dirY, true, true }); playerLastShotTime = now; }
    }
}

// --- LOGICĂ JOC: PONG ---
void Pong_ResetBall() {
    pongBallX = 10 / 2.0f; pongBallY = 20 / 2.0f;
    pongBallDX = (rand() % 2 == 0 ? 0.08f : -0.08f) * pongSpeedMultiplier;
    pongBallDY = (rand() % 2 == 0 ? 0.14f : -0.14f) * pongSpeedMultiplier;
    botDecidedError = false;
}
void Pong_Init() {
    pongPaddleX = 10 / 2 - 1; pongBotX = 10 / 2;
    pongPlayerScore = 0; pongBotScore = 0; pongRound = 1; pongSpeedMultiplier = 1.0f;
    gameOver = false; gameYouWin = false; Pong_ResetBall(); pongTimer = GetTickCount();
}
void Pong_Update() {
    if (gameOver || gameYouWin) return;
    DWORD now = GetTickCount();
    if (now - pongTimer > 40) {
        pongTimer = now; pongBallX += pongBallDX; pongBallY += pongBallDY;
        if (pongBallX <= 0) { pongBallX = 0; pongBallDX = -pongBallDX; }
        if (pongBallX >= 10 - 1) { pongBallX = 10 - 1; pongBallDX = -pongBallDX; }

        int botWidth = 1;
        if (pongRound >= 3) botWidth = 2;
        if (pongRound >= 6) botWidth = 3;
        if (pongRound >= 10) botWidth = 4;

        if (pongBallY < 10.0f && !botDecidedError) {
            botDecidedError = true;
            if (pongRound == 1 && (rand() % 100 < 50)) botErrorOffset = (rand() % 2 == 0) ? 2 : -2;
            else if (pongRound <= 3 && (rand() % 100 < 25)) botErrorOffset = (rand() % 2 == 0) ? 1 : -1;
            else botErrorOffset = 0;
        }
        if (pongBallY >= 11.0f) botDecidedError = false;

        int targetX = (int)pongBallX + botErrorOffset;
        pongBotX = targetX - (botWidth / 2);
        if (pongBotX < 0) pongBotX = 0;
        if (pongBotX > 10 - botWidth) pongBotX = 10 - botWidth;

        int bX = (int)pongBallX, bY = (int)pongBallY;
        if (bY == 1 && pongBallDY < 0) {
            if (bX >= pongBotX && bX < pongBotX + botWidth) {
                pongBallDY = -pongBallDY;
                if (botWidth > 1 && bX == pongBotX) pongBallDX -= 0.03f;
                if (botWidth > 1 && bX == pongBotX + botWidth - 1) pongBallDX += 0.03f;
            }
        }
        if (bY == 20 - 2 && pongBallDY > 0) {
            if (bX >= pongPaddleX && bX <= pongPaddleX + 2) {
                pongBallDY = -pongBallDY;
                if (bX == pongPaddleX) pongBallDX -= 0.04f;
                if (bX == pongPaddleX + 2) pongBallDX += 0.04f;
            }
        }
        if (pongBallY >= 20) { pongBotScore++; if (pongBotScore >= 3) gameOver = true; else Pong_ResetBall(); }
        if (pongBallY < 0) {
            pongPlayerScore++;
            if (pongPlayerScore >= 3) {
                pongRound++; gameScore += 500;
                if (pongRound > 10) gameYouWin = true;
                else { pongPlayerScore = 0; pongBotScore = 0; pongSpeedMultiplier += 0.15f; Pong_ResetBall(); }
            } else Pong_ResetBall();
        }
    }
    for (int y = 0; y < 20; y++) for (int x = 0; x < 10; x++) screen[y][x] = 0;
    int curBotW = (pongRound >= 10) ? 4 : ((pongRound >= 6) ? 3 : ((pongRound >= 3) ? 2 : 1));
    for (int x = pongBotX; x < pongBotX + curBotW; x++) screen[1][x] = 1;
    for (int x = pongPaddleX; x < pongPaddleX + 3; x++) screen[20 - 2][x] = 2;
    int fbX = (int)pongBallX, fbY = (int)pongBallY;
    if (fbX >= 0 && fbX < 10 && fbY >= 0 && fbY < 20) screen[fbY][fbX] = 1;
}
void Pong_Key(int key) {
    if (key == VK_LEFT && pongPaddleX > 0) pongPaddleX--;
    if (key == VK_RIGHT && pongPaddleX < 10 - 3) pongPaddleX++;
}

// --- LOGICĂ JOC: BREAKOUT ---
void Breakout_LoadShape() {
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            if (breakoutLevel == 1)      breakoutGrid[y][x] = SHAPE_HEART[y][x];
            else if (breakoutLevel == 2) breakoutGrid[y][x] = SHAPE_NUMBER7[y][x];
            else                         breakoutGrid[y][x] = SHAPE_DUMBBELL[y][x];
        }
    }
}
void Breakout_ResetBall() {
    breakoutPaddleX = 10 / 2 - 1; breakoutBallX = 10 / 2.0f; breakoutBallY = 20 - 4.0f;
    breakoutBallDX = (rand() % 2 == 0 ? 0.07f : -0.07f); breakoutBallDY = -0.12f;
}
void Breakout_Init() {
    breakoutLevel = 1; breakoutLives = 3; gameScore = 0; gameOver = false; gameYouWin = false;
    Breakout_LoadShape(); Breakout_ResetBall(); breakoutTimer = GetTickCount();
}
bool Breakout_CheckWin() {
    for (int y = 0; y < 20; y++) for (int x = 0; x < 10; x++) if (breakoutGrid[y][x] == 1) return false;
    return true;
}
void Breakout_Update() {
    if (gameOver || gameYouWin) return;
    DWORD now = GetTickCount();
    if (now - breakoutTimer > 40) {
        breakoutTimer = now; breakoutBallX += breakoutBallDX; breakoutBallY += breakoutBallDY;
        if (breakoutBallX <= 0) { breakoutBallX = 0; breakoutBallDX = -breakoutBallDX; }
        if (breakoutBallX >= 10 - 1) { breakoutBallX = 10 - 1; breakoutBallDX = -breakoutBallDX; }
        if (breakoutBallY <= 0) { breakoutBallY = 0; breakoutBallDY = -breakoutBallDY; }

        int bX = (int)breakoutBallX, bY = (int)breakoutBallY;
        if (bX >= 0 && bX < 10 && bY >= 0 && bY < 20 && breakoutGrid[bY][bX] == 1) {
            breakoutGrid[bY][bX] = 0; breakoutBallDY = -breakoutBallDY; gameScore += 50;
            if (Breakout_CheckWin()) {
                breakoutLevel++;
                if (breakoutLevel > 3) gameYouWin = true;
                else { Breakout_LoadShape(); Breakout_ResetBall(); }
                return;
            }
        }
        if (bY == 20 - 2 && breakoutBallDY > 0 && bX >= breakoutPaddleX && bX <= breakoutPaddleX + 2) {
            breakoutBallDY = -breakoutBallDY;
            if (bX == breakoutPaddleX) breakoutBallDX -= 0.03f;
            if (bX == breakoutPaddleX + 2) breakoutBallDX += 0.04f;
        }
        if (breakoutBallY >= 20) { breakoutLives--; if (breakoutLives <= 0) gameOver = true; else Breakout_ResetBall(); }
    }
    for (int y = 0; y < 20; y++) for (int x = 0; x < 10; x++) screen[y][x] = 0;
    for (int y = 0; y < 20; y++) for (int x = 0; x < 10; x++) if (breakoutGrid[y][x] == 1) screen[y][x] = 1;
    for (int x = breakoutPaddleX; x < breakoutPaddleX + 3; x++) screen[20 - 2][x] = 2;
    int fbX = (int)breakoutBallX, fbY = (int)breakoutBallY;
    if (fbX >= 0 && fbX < 10 && fbY >= 0 && fbY < 20) screen[fbY][fbX] = 1;
}
void Breakout_Key(int key) {
    if (key == VK_LEFT && breakoutPaddleX > 0) breakoutPaddleX--;
    if (key == VK_RIGHT && breakoutPaddleX < 10 - 3) breakoutPaddleX++;
}

// --- CONECTARE MODULE PRINTRE METODELE PRINCIPALE ---
void InitCurrentGame() {
    srand(static_cast<unsigned int>(time(0)));
    switch(selectedGameType) {
        case GAME_TETRIS: Tetris_Init(); break;
        case GAME_SNAKE: Snake_Init(); break;
        case GAME_TANK: Tank_Init(); break;
        case GAME_PONG: Pong_Init(); break;
        case GAME_BREAKOUT: Breakout_Init(); break;
    }
}

void UpdateCurrentGame() {
    switch(selectedGameType) {
        case GAME_TETRIS: Tetris_Update(); break;
        case GAME_SNAKE: Snake_Update(); break;
        case GAME_TANK: Tank_Update(); break;
        case GAME_PONG: Pong_Update(); break;
        case GAME_BREAKOUT: Breakout_Update(); break;
    }
}

void HandleCurrentGameKey(int key) {
    switch(selectedGameType) {
        case GAME_TETRIS: Tetris_Key(key); break;
        case GAME_SNAKE: Snake_Key(key); break;
        case GAME_TANK: Tank_Key(key); break;
        case GAME_PONG: Pong_Key(key); break;
        case GAME_BREAKOUT: Breakout_Key(key); break;
    }
}
