#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <synchapi.h>
#include "combination.h"
#include "line.h"
#include "board.h"
#include "task.h"
#include "callstack.h"

int main() {
	Board* board = parseBoard();
	Shared* shared = allocate_Shared(board);
	Task* row = malloc(board->height * sizeof(Task));
	Task* col = malloc(board->width * sizeof(Task));
	for(uint32_t h=0;h<board->height;h++) {
		Solver* solver = allocate_Solver("row", h, board->row[h]);
		row[h].arg = allocate_Arg(solver, shared);
	}
	for(uint32_t w=0;w<board->width;w++) {
		Solver* solver = allocate_Solver("col", w, board->col[w]);
		col[w].arg = allocate_Arg(solver, shared);
	}

	for(int x=0;x<board->height;x++) {
		pthread_create(&row[x].pthread, NULL, funcSolve, row[x].arg);
	}
	for(int x=0;x<board->width;x++) {
		pthread_create(&col[x].pthread, NULL, funcSolve, col[x].arg);
	}
	for(int x=0;x<board->height;x++) {
		pthread_join(row[x].pthread, NULL);
	}
	for(int x=0;x<board->width;x++) {
		pthread_join(col[x].pthread, NULL);
	}
	printf("\n");
	printBoard(board, HINT_LEFT, HINT_DOWN);
	printf("\n");
	printBoard(board, HINT_LEFT, HINT_UP);
	printf("\n");
	printBoard(board, HINT_RIGHT, HINT_DOWN);
	printf("\n");
	printBoard(board, HINT_RIGHT, HINT_UP);
	printf("\n");
	return 0;
}


