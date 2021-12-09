#include <windows.h>
#include <time.h>

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

int board[SIZE_X][SIZE_Y];
int turn;

RECT boardRect;

// temporal variables
RECT rect;
TCHAR buf[128];

void init(HWND hWnd) {
	turn = 0;
	for (int i = 0; i < SIZE_Y; i++)
		for (int j = 0; j < SIZE_X; j++)
			board[j][i] = 0;
	InvalidateRect(hWnd, &boardRect, TRUE);
}

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

int getHeight(int col) {	// 0 ~ SIZE_Y - 1
	int i;
	for (i = 0; i < SIZE_Y; i++)
		if (board[col][i]) break;
	i--;

	return i;	// cf. if column is full, return -1
}

int dropStone(HWND hWnd, int col) {
	int i = getHeight(col);
	if (i == -1)
		return -1;

	turn++;
	board[col][i] = (turn % 2) ? RED : YELLOW;

	rect = getSquare(col, i);
	InvalidateRect(hWnd, &rect, FALSE);

	return i;
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

int evaluate(int depth, int lastX, int lastY) {
	switch (getWinner(lastX, lastY)) {
	case RED:
		return -100 - depth;
	case YELLOW:
		return 100 + depth;
	default:
		return 0;
	}
}

int minimax(int* pos, int depth, int turn, int lastX, int lastY)
{
	int score; // 현재 점수
	int best; // 최고 점수
	int position; // 최적 위치
	int i;

	if (turn == RED)
		best = 9999; // 양의 무한대 (최소값을 찾아야 하므로)
	else
		best = -9999; // 음의 무한대 (최대값을 찾아야 하므로)

	if (depth == 0 || getWinner(lastX, lastY) != 0)
		return evaluate(depth, lastX, lastY);

	for (int j = 0; j < SIZE_X; j++) {// 모든 열을 조사함
		i = getHeight(j);
		if (i == -1) continue;

		board[j][i] = turn; // 현재 열에 돌을 놓는다
		if (turn == RED)
			score = minimax(pos, depth - 1, YELLOW, i, j);
		else
			score = minimax(pos, depth - 1, RED, i, j);
		board[j][i] = 0; // 원래대로 되돌린다

		if (turn == RED) // 사람의 차례라면
		{
			if (score < best) // 최적값은 최소가 되는것을 찾아 기록한다
			{
				best = score;
				position = j; // 최적위치 기록
			}
		}
		else // 컴퓨터의 차례라면
		{
			if (score > best) // 최적값은 최대가 되는것을 찾아 기록한다
			{
				best = score;
				position = j; // 최적위치 기록
			}
		}
	}
	*pos = position; // 최종적으로 결정된 위치를 선택한다.
	return best; // 최적값을 반환한다.
}

int compute(int lastX, int lastY) { // 컴퓨터 인공지능
	int pos;
	int weight = minimax(&pos, 8, YELLOW, lastX, lastY); // 이후 depth 수만큼 예상함
	if (weight != 0)
		return pos;
	else {	// lastX 근처에 random
		do {
			pos = lastX + (rand() % 5) - 2;
		} while (pos < 0 || pos >= SIZE_X);
	}
	return pos;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	static int width, height;
	static HBRUSH hbWhite, hbBlue, hbRed, hbYellow;

	int i, j, tmp;

	switch (iMessage) {
	case WM_CREATE:
		srand((unsigned)time(NULL));
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
		j = (LOWORD(lParam) - boardRect.left) * SIZE_X / (boardRect.right - boardRect.left);	// x-index
		if (j < 0 || j >= SIZE_X)	// non-hit check
			break;

		i = dropStone(hWnd, j);
		if (i == -1)	// column is full
			break;
		tmp = getWinner(j, i);
		if (tmp != 0) {
			if (tmp == RED)
				wsprintf(buf, TEXT("Red Won, Continue?"));
			else if (tmp == YELLOW)
				wsprintf(buf, TEXT("Yellow Won, Continue?"));
			if (MessageBox(hWnd, buf, TEXT("Connect Four"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				init(hWnd);
			else
				SendMessage(hWnd, WM_CLOSE, 0L, 0L);
			break;
		}
		/*
		j = compute(j, i);
		i = dropStone(hWnd, j);
		tmp = getWinner(j, i);
		if (tmp != 0) {
			if (tmp == RED)
				wsprintf(buf, TEXT("Red Won, Continue?"));
			else if (tmp == YELLOW)
				wsprintf(buf, TEXT("Yellow Won, Continue?"));
			if (MessageBox(hWnd, buf, TEXT("Connect Four"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				init(hWnd);
			else
				SendMessage(hWnd, WM_CLOSE, 0L, 0L);
			break;
		}
		//*/
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		for (i = 0; i < SIZE_Y; i++) {
			for (j = 0; j < SIZE_X; j++) {
				rect = getSquare(j, i);
				SelectObject(hdc, hbBlue);
				Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
				switch (board[j][i]) {
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
				rect = getInnerSqaure(j, i);
				Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);
			}
		}
		EndPaint(hWnd, &ps);
		break;


	case WM_DESTROY:
		DeleteObject(hbYellow);
		DeleteObject(hbRed);
		DeleteObject(hbBlue);
		DeleteObject(hbWhite);
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}