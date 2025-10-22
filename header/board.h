#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <pthread.h>

#define HINT_LEFT UINT32_C(0)	// 印 Row Hint 的方向
#define HINT_RIGHT UINT32_C(1)	// 印 Row Hint 的方向
#define HINT_UP UINT32_C(2)		// 印 Col Hint 的方向
#define HINT_DOWN UINT32_C(3)	// 印 Col Hint 的方向

typedef struct Line Line;
typedef Line Row;
typedef Line Col;

typedef struct Board {
	// map 的高，第 1 個維度
	uint32_t height;
	
	// map 的寬，第 2 個維度
	uint32_t width;
	
	// row 的陣列，row[n] = Row*
	Row** row;
	
	// col 的陣列，col[n] = Col*
	Col** col;

	// 保護 waiting Line
	pthread_mutex_t mutex;

	// 等待的 Line 都在等最後一個 Line 觸發 main thread 去 signal 他們
	pthread_cond_t cond;

	// 目前有多少個 Line 正在等待其他 Line 放入解
	uint32_t waitingLine;
} Board;

Board* allocate_Board(const uint32_t height, const uint32_t width);
uint32_t printRowLeftHint(Board* board);
void printBoard(Board* board, uint32_t horizontal, uint32_t vertical);
Board* parseBoard();
#endif