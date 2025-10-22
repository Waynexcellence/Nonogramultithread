#ifndef TASK_H
#define TASK_H

#include <synchapi.h>
#include <stdio.h>
#include "line.h"

typedef struct Shared {
	// 公共 print 的鎖
	pthread_mutex_t lock;
	
	// 遊戲地圖
	Board* board;
} Shared;

typedef struct Solver {
	// 儲存自己是負責 "row" 還是 "col"
	char name[16];

	// 順序是第幾個 Line
	uint32_t index;

	// 每個 Solver 各自負責一個 Line
	Line* line;

	// 每個 Solver 各自有 debug 文件
	FILE* log;
} Solver;

typedef struct Arg {
	Solver* solver;
	Shared* shared;
} Arg;

typedef struct Task {
	Arg* arg;
	pthread_t pthread;
} Task;

void* funcSolve(void* arg);
void send(Solver* solver, Shared* shared, Receive* receive, Sure sure);
Sure calculating(Solver* solver, Shared* shared);
Sure waiting(Solver* solver, Shared* shared);
void filtering(Solver* solver, Sure sure);

Shared* allocate_Shared(Board* board);
Solver* allocate_Solver(char* name, uint32_t index, Line* line);
Arg* allocate_Arg(Solver* solver, Shared* shared);
#endif