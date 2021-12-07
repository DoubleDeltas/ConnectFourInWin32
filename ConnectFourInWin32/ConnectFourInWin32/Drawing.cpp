#include <Windows.h>

RECT getSquare(PRECT pboard, int xSize, int ySize, int i, int j) {
	int width = (pboard->right - pboard->left) / xSize;
	int height = (pboard->bottom - pboard->top) / ySize;
	RECT rect = {
			pboard->left + j * width,
			pboard->top + i * height,
			pboard->left + (j + 1) * width,
			pboard->top + (i + 1) * height
	};
	return rect;
}

RECT getInnerSqaure(PRECT pboard, int xSize, int ySize, int i, int j) {
	int width = (pboard->right - pboard->left) / xSize;
	int height = (pboard->bottom - pboard->top) / ySize;
	RECT rect = {
		pboard->left + j * width + width / 10,
		pboard->top + i * height + height / 10,
		pboard->left + (j + 1) * width - width / 10,
		pboard->top + (i + 1) * height - height / 10
	};
	return rect;
}