#include <windows.h>
#include <math.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("Connect Four");

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
const RECT rectTurnText = { 500, 200, 500, 220 };

int board[SIZE_X][SIZE_Y];
int turn;

RECT boardRect;

// temporal variable
RECT rect;

void init(HWND hWnd) {
	turn = 0;
	for (int i = 0; i < SIZE_X; i++)
		for (int j = 0; j < SIZE_Y; j++)
			board[i][j] = 0;
	InvalidateRect(hWnd, &boardRect, TRUE);
}


/*--------------- game ----------------*/
RECT getSquare(int x, int y);

int getHeight(int x) {
	int y;
	for (y = 0; y < SIZE_Y; y++) {
		if (board[x][y] != EMPTY)
			break;
	}
	return y - 1;	// cf. if y == -1, column 'x' is full
}

int dropStone(HWND hWnd, int x) {
	int y = getHeight(x);

	if (y == -1) return -1;

	turn++;
	board[x][y] = (turn % 2) ? 1 : 2;

	rect = getSquare(x, y);
	InvalidateRect(hWnd, &rect, FALSE);
//	InvalidateRect(hWnd, &rectTurnText, TRUE);

	return y;
}


int getWinner(int lastX, int lastY) {
	int cnt;
	int player;
	int d;
	for (player = RED; player <= YELLOW; player++) {
		// is there player stone?
		if (board[lastX][lastY] != player)
			continue;
		// left-right check
		cnt = 1;
		d = 0;		// left
		while (TRUE) {
			d++;
			if (lastX - d < 0 || board[lastX - d][lastY] != player)
				break;
			cnt++;
		}
		d = 0;
		while (TRUE) {
			d++;
			if (lastX + d >= SIZE_X || board[lastX + d][lastY] != player)
				break;
			cnt++;
		}
		if (cnt >= 4)
			return player;

		// up-down check
		cnt = 1;
		d = 0;		// up
		while (TRUE) {
			d++;
			if (lastY - d < 0 || board[lastX][lastY - d] != player)
				break;
			cnt++;
		}
		d = 0;		// down
		while (TRUE) {
			d++;
			if (lastY + d >= SIZE_Y || board[lastX][lastY + d] != player)
				break;
			cnt++;
		}
		if (cnt >= 4)
			return player;

		// / direction check
		cnt = 1;
		d = 0;		// upright
		while (TRUE) {
			d++;
			if (lastX + d >= SIZE_X || lastY - d < 0 || board[lastX + d][lastY - d] != player)
				break;
			cnt++;
		}
		d = 0;		// downright
		while (TRUE) {
			d++;
			if (lastX - d < 0 || lastY - d >= SIZE_Y || board[lastX - d][lastY + d] != player)
				break;
			cnt++;
		}
		if (cnt >= 4)
			return player;

		// \ direction check
		cnt = 1;
		d = 0;		// upleft
		while (TRUE) {
			d++;
			if (lastX - d < 0 || lastY - d < 0 || board[lastX - d][lastY - d] != player)
				break;
			cnt++;
		}
		d = 0;		// downright
		while (TRUE) {
			d++;
			if (lastX + d >= SIZE_X || lastY - d >= SIZE_Y || board[lastX + d][lastY + d] != player)
				break;
			cnt++;
		}
		if (cnt >= 4)
			return player;

	}
	return 0;	// no one won
}

BOOL isFull() {
	for (int j = 0; j < SIZE_X; j++) {
		if (getHeight(j) != -1)
			return FALSE;
	}
	return TRUE;
}

/*-------------- computer ----------------*/
int evaluate(int lastX, int lastY, int depth) {
	int winner = getWinner(lastX, lastY);
	if (winner == RED)
		return -10 - depth;
	else if (winner == YELLOW)
		return 10 + depth;
	else
		return 0;
}

int minimax(int *ppos, int lastX, int lastY, int depth, int turn) {
	int score;
	int best;
	int pos;

	if (turn == RED)
		best = 9999;	// to find point that evaluated value is worst
	else
		best = -9999;	// to find ~ best

	if (depth == 0 || getWinner(lastX, lastY) != 0) {	// termination condition
		return evaluate(lastX, lastY, depth);
	}

	int y;
	for (int x = 0; x < SIZE_X; x++) {
		y = getHeight(x);
		if (y == -1) continue;	// skip full-filled column

		board[x][y] = turn;		// drop stone

		if (turn == RED)
			score = minimax(ppos, x, y, depth - 1, YELLOW);
		else // turn == YELLOW(computer)
			score = minimax(ppos, x, y, depth - 1, RED);

		board[x][y] = EMPTY;	// undrop stone

		if (score == abs(10 + depth - 1)) {	// alpha-beta pruning, when immidiate child evaluated
			best = score;
			pos = x;
		}
		else {
			if (turn == RED) {
				if (score < best) {
					best = score;
					pos = x;
				}
			}
			else {	//turn == YELLOW(computer)
				if (score > best) {
					best = score;
					pos = x;
				}
			}
		}
		
	}
	*ppos = pos;
	return best;
}

int compute(int lastX, int lastY) {
	int depth = 0;
	int pos;
	int score;
	
	score = minimax(&pos, lastX, lastY, 7, YELLOW);

	/*
	if (score == 0) // couldn't predict with given depth
		pos = SIZE_X / 2;
	//*/
	return pos;
}

/*--------------- drawing ----------------*/

RECT getSquare(int x, int y) {
	int width = (boardRect.right - boardRect.left) / SIZE_X;
	int height = (boardRect.bottom - boardRect.top) / SIZE_Y;
	RECT rect = {
			boardRect.left + x * width,
			boardRect.top + y * height,
			boardRect.left + (x + 1) * width,
			boardRect.top + (y + 1) * height
	};
	return rect;
}

RECT getInnerSqaure(int x, int y) {
	int width = (boardRect.right - boardRect.left) / SIZE_X;
	int height = (boardRect.bottom - boardRect.top) / SIZE_Y;
	RECT rect = {
		boardRect.left + x * width + width / 10,
		boardRect.top + y * height + height / 10,
		boardRect.left + (x + 1) * width - width / 10,
		boardRect.top + (y + 1) * height - height / 10
	};
	return rect;
}

/*----------- window procedure and game -----------*/

BOOL processWin(int lastX, int lastY);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	static int width, height;
	static HBRUSH hbWhite, hbBlue, hbRed, hbYellow;

	HBRUSH oldBrush;
	int i, j;

	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;

		GetClientRect(hWnd, &rect);
		boardRect.top = rect.top + 10;
		boardRect.bottom = rect.bottom - 10;
		boardRect.left = rect.left + 10;
		boardRect.right = boardRect.left + (boardRect.bottom - boardRect.top) * SIZE_X / SIZE_Y;

		hbWhite = CreateSolidBrush(RGB(255, 255, 255));
		hbBlue = CreateSolidBrush(RGB(0, 0, 255));
		hbRed = CreateSolidBrush(RGB(255, 0, 0));
		hbYellow = CreateSolidBrush(RGB(255, 255, 0));

		break;


	case WM_LBUTTONDOWN:
		i = (LOWORD(lParam) - boardRect.left) * SIZE_X / (boardRect.right - boardRect.left);	// x-index
		if (i < 0 || i >= SIZE_X)	// non-hit check
			break;

		// RED: player
		j = dropStone(hWnd, i);
		if (j == -1)	// column is full
			break;
		if (processWin(i, j)) break;
		
		// YELLOW: computer
		i = compute(i, j);
		j = dropStone(hWnd, i);
		if (j == -1)	// column is full
			break;
		processWin(i, j);
		break;


	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		oldBrush = (HBRUSH)SelectObject(hdc, NULL);
		for (i = 0; i < SIZE_X; i++) {
			for (j = 0; j < SIZE_Y; j++) {
				rect = getSquare(i, j);
				SelectObject(hdc, hbBlue);
				Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
				switch (board[i][j]) {
				case 0:
					SelectObject(hdc, hbWhite);
					break;
				case 1:
					SelectObject(hdc, hbRed);
					break;
				case 2:
					SelectObject(hdc, hbYellow);
					break;
				}
				rect = getInnerSqaure(i, j);
				Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);
			}
		}

		SelectObject(hdc, oldBrush);
		EndPaint(hWnd, &ps);
		break;


	case WM_DESTROY:
		DeleteObject(hbYellow);
		DeleteObject(hbRed);
		DeleteObject(hbBlue);
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

BOOL processWin(int lastX, int lastY) {
	TCHAR buf[128];
	int winner = getWinner(lastX, lastY);
	if (winner != 0) {
		if (winner == RED)
			wsprintf(buf, TEXT("Red Won, Continue?"));
		else if (winner == YELLOW)
			wsprintf(buf, TEXT("Yellow Won, Continue?"));
		if (MessageBox(hWndMain, buf, TEXT("Connect Four"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			init(hWndMain);
		else
			SendMessage(hWndMain, WM_CLOSE, 0L, 0L);
		return TRUE;
	}
	return FALSE;
}
