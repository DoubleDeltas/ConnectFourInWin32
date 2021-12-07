#include <windows.h>
#include "Drawing.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("Class");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

const int SIZE_X = 7;
const int SIZE_Y = 6;

const int EMPTY = 0, RED = 1, YELLOW = 2;

int board[SIZE_Y][SIZE_X];
int turn;



LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	static int width, height;
	static RECT boardRect;
	static HBRUSH hbBlue, hbRed, hbYellow;

	HBRUSH oldBrush;
	RECT rect;
	int i, j;

	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;

		GetClientRect(hWnd, &rect);
		boardRect.top = rect.top + 10;
		boardRect.bottom = rect.bottom - 10;
		boardRect.left = rect.left + 10;
		boardRect.right = boardRect.left + (boardRect.bottom - boardRect.top) * SIZE_X / SIZE_Y;

		hbBlue = CreateSolidBrush(RGB(0, 0, 255));
		hbRed = CreateSolidBrush(RGB(255, 0, 0));
		hbYellow = CreateSolidBrush(RGB(255, 255, 0));

		break;

	case WM_LBUTTONDOWN:
		j = (LOWORD(lParam) - boardRect.left) * SIZE_X / (boardRect.right - boardRect.left);	// x-index

		if (j < 0 || j >= SIZE_X)	// non-hit check
			break;


		break;


	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		for (i = 0; i < SIZE_Y; i++) {
			for (j = 0; j < SIZE_X; j++) {
				rect = getSquare(&boardRect, SIZE_X, SIZE_Y, i, j);
				oldBrush = (HBRUSH)SelectObject(hdc, hbBlue);
				Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
				switch (board[i][j]) {
				case 0:
					SelectObject(hdc, oldBrush);
					break;
				case 1:
					SelectObject(hdc, hbRed);
					break;
				case 2:
					SelectObject(hdc, hbYellow);
					break;
				}
				rect = getInnerSquare(&boardRect, SIZE_X, SIZE_Y, i, j);
				Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);
			}
		}

		EndPaint(hWnd, &ps);
		break;


	case WM_DESTROY:
		DeleteObject(hbYellow);
		DeleteObject(hbRed);
		DeleteObject(hbBlue);
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}