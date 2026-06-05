#include <windows.h>
#include "games.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(BG);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "BrickGameClass";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    RECT rect = { 0, 0, WIN_W, WIN_H };
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE, 0);

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "BrickGameClass",
        "BRICK GAME - CLASSIC HANDHELD",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL
    );

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 20, NULL);

    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_TIMER: {
            if (!inMenu && gameRunning) {
                UpdateCurrentGame();
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_KEYDOWN: {
            int key = (int)wParam;

            if (key == VK_ESCAPE) {
                inMenu = true;
                gameRunning = false;
                break;
            }

            if (inMenu) {
                if (key == VK_UP) {
                    selectedGameType = (selectedGameType - 1 + GAME_COUNT) % GAME_COUNT;
                }
                if (key == VK_DOWN) {
                    selectedGameType = (selectedGameType + 1) % GAME_COUNT;
                }
                if (key == VK_RETURN || key == VK_SPACE) {
                    inMenu = false;
                    gameRunning = true;
                    InitCurrentGame();
                }
            } else {
                if (gameOver || gameYouWin) {
                    if (key == VK_RETURN || key == VK_SPACE) {
                        inMenu = true;
                    }
                } else {
                    HandleCurrentGameKey(key);
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Double Buffering
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, WIN_W, WIN_H);
            HBITMAP hOldBm = (HBITMAP)SelectObject(hdcMem, hbmMem);

            HBRUSH hBgBrush = CreateSolidBrush(BG);
            RECT winRect = { 0, 0, WIN_W, WIN_H };
            FillRect(hdcMem, &winRect, hBgBrush);
            DeleteObject(hBgBrush);

            if (inMenu) {
                // --- DESIGN MENIU CLASIC ---
                SetTextColor(hdcMem, PLAYER);
                SetBkMode(hdcMem, TRANSPARENT);

                HFONT hTitleFont = CreateFont(34, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
                HFONT hOldFont = (HFONT)SelectObject(hdcMem, hTitleFont);
                TextOut(hdcMem, 40, 25, "BRICK GAME", 10);

                HFONT hSubTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
                SelectObject(hdcMem, hSubTitleFont);
                TextOut(hdcMem, 40, 65, "9999 IN 1", 9);

                HFONT hAuthorFont = CreateFont(16, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
                SelectObject(hdcMem, hAuthorFont);
                SetTextColor(hdcMem, ON);
                TextOut(hdcMem, WIN_W - 160, 25, "by EugenGtt", 12);

                HFONT hMenuFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
                SelectObject(hdcMem, hMenuFont);

                for (int i = 0; i < GAME_COUNT; i++) {
                    if (i == selectedGameType) {
                        SetTextColor(hdcMem, PLAYER);
                        char buf[64];
                        wsprintf(buf, ">> %s", GameNames[i]);
                        TextOut(hdcMem, 40, 140 + i * 35, buf, lstrlen(buf));
                    } else {
                        SetTextColor(hdcMem, ON);
                        TextOut(hdcMem, 60, 140 + i * 35, GameNames[i], lstrlen(GameNames[i]));
                    }
                }

                SetTextColor(hdcMem, PLAYER);
                TextOut(hdcMem, 40, 400, "PRESS START (ENTER)", 19);

                SelectObject(hdcMem, hOldFont);
                DeleteObject(hTitleFont);
                DeleteObject(hSubTitleFont);
                DeleteObject(hAuthorFont);
                DeleteObject(hMenuFont);
            }
            else {
                // --- DESIGN INTERFAȚĂ JOC ---
                HBRUSH hBrushOn = CreateSolidBrush(ON);
                HBRUSH hBrushPlayer = CreateSolidBrush(PLAYER);
                HBRUSH hBrushOff = CreateSolidBrush(OFF);

                int offsetX = 30;
                int offsetY = 30;

                for (int y = 0; y < 20; y++) {
                    for (int x = 0; x < 10; x++) {
                        RECT cellRect = {
                            offsetX + x * CELL,
                            offsetY + y * CELL,
                            offsetX + x * CELL + CELL - 2,
                            offsetY + y * CELL + CELL - 2
                        };

                        if (screen[y][x] == 1) {
                            FillRect(hdcMem, &cellRect, hBrushOn);
                        } else if (screen[y][x] == 2) {
                            FillRect(hdcMem, &cellRect, hBrushPlayer);
                        } else {
                            FillRect(hdcMem, &cellRect, hBrushOff);
                        }
                    }
                }

                // Panou lateral (Dreapta)
                SetTextColor(hdcMem, PLAYER);
                SetBkMode(hdcMem, TRANSPARENT);
                HFONT hSideFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
                HFONT hOldFont = (HFONT)SelectObject(hdcMem, hSideFont);

                int panelX = offsetX + 10 * CELL + 30;

                // Afișare SCOR
                TextOut(hdcMem, panelX, 50, "SCORE", 5);
                char scoreBuf[16];
                wsprintf(scoreBuf, "%06d", gameScore);
                TextOut(hdcMem, panelX, 75, scoreBuf, lstrlen(scoreBuf));

                // Afișare LEVEL (doar pentru Tank)
                if (selectedGameType == GAME_TANK) {
                    TextOut(hdcMem, panelX, 130, "LEVEL", 5);
                    char lvlBuf[16];
                    wsprintf(lvlBuf, "%02d", currentLevel);
                    TextOut(hdcMem, panelX, 155, lvlBuf, lstrlen(lvlBuf));
                }

                if (gameOver) {
                    SetTextColor(hdcMem, RGB(255, 50, 50));
                    TextOut(hdcMem, panelX, 230, "GAME OVER", 9);
                    SetTextColor(hdcMem, ON);
                    TextOut(hdcMem, panelX, 260, "PRESS ENTER", 11);
                } else if (gameYouWin) {
                    SetTextColor(hdcMem, RGB(50, 255, 50));
                    TextOut(hdcMem, panelX, 230, "YOU WIN!", 8);
                    SetTextColor(hdcMem, ON);
                    TextOut(hdcMem, panelX, 260, "PRESS ENTER", 11);
                } else {
                    SetTextColor(hdcMem, ON);
                    TextOut(hdcMem, panelX, 350, "ESC: MENU", 9);
                }

                // --- SEMNĂTURA ÎN TIMPUL JOCULUI (JOS DREAPTA) ---
                HFONT hGameAuthorFont = CreateFont(16, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
                SelectObject(hdcMem, hGameAuthorFont);
                SetTextColor(hdcMem, ON); // Culoarea verde clasică
                TextOut(hdcMem, WIN_W - 145, WIN_H - 75, "by EugeneGtt", 12);

                SelectObject(hdcMem, hOldFont);
                DeleteObject(hSideFont);
                DeleteObject(hGameAuthorFont);
                DeleteObject(hBrushOn);
                DeleteObject(hBrushPlayer);
                DeleteObject(hBrushOff);
            }

            BitBlt(hdc, 0, 0, WIN_W, WIN_H, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hOldBm);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
