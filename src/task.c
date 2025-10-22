#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <direct.h>
#include "combination.h"
#include "board.h"
#include "task.h"
#include "callstack.h"

/**
 * 此 Line 幫助其他 Line
 * 對 mutex 上鎖
 * 放入確定值到 receive->sure
 * 對 mutex 解鎖
 * 呼叫 pthread_cond_signal()
 */
void send(Solver* solver, Shared* shared, Receive* receive, Sure sure) {
	fprintf(solver->log, "%s[%u]:%s\n", solver->name, solver->index, __func__);
	fprintBinary(sure.count, solver->log);
	fprintBinary(sure.value, solver->log);
	pthread_mutex_lock(&receive->mutex);			// 得到接受幫助的 Line 的 Receive 的鎖
	fprintBinary(receive->sure.count, solver->log);
	fprintBinary(receive->sure.value, solver->log);
	receive->sure.count |= sure.count;
	receive->sure.value |= sure.value;
	fprintBinary(receive->sure.count, solver->log);
	fprintBinary(receive->sure.value, solver->log);
	pthread_mutex_unlock(&receive->mutex);			// 釋放接受幫助的 Line 的 Receive 的鎖
	pthread_cond_signal(&receive->cond);			// 叫醒被幫助的 Line
}

/**
 * 根據 possible 中前 possibility 個可能解
 * 算出進一步的確定值
 * 將確定值放入到自己 Line 的 Sure 中
 * 返回進一步的確定值，可以檢查 count 是否為 0 來判斷是否計算出進一步的確定值
 */
Sure calculating(Solver* solver, Shared* shared) {
	Line* line = solver->line;
	uint64_t field = (line->size==64)?(~0ULL):((1ULL<<line->size)-1);
	uint64_t unfound = (~field)|line->sure.count;						// 此 Line 尚未確定的那些位置 bit 是 0，已確定的 bit 是 1 
	fprintf(solver->log, "%s[%u]:before %s\n", solver->name, solver->index, __func__);
	fprintBinary(unfound, solver->log);
	fprintBinary(line->sure.count, solver->log);
	fprintBinary(line->sure.value, solver->log);
	fprintf(solver->log, "possibility = %llu\n", line->possibility);
	for(uint64_t x=0;x<line->possibility;x++) {
		fprintBinary(line->possible[x], solver->log);
	}
	uint64_t sample = line->possible[0];
	for(uint64_t x=1;x<line->possibility&&(unfound!=ULLONG_MAX);x++) {	// 如果 unfound 是 ULLONG_MAX 代表此次 calculating 無法求出新確定值
		uint64_t diff = sample ^ line->possible[x];						// bit = 1 代表衝突，無法求出解
		unfound |= diff;												// 00-0 01-X 10-1 11-0
	}
	Sure result = {0};
	uint64_t found = ~unfound;
	if( found ) {
		uint64_t sure = found & sample;									// 新確定值來自於 sample 中 found bit 是 1 的值
		line->sure.count |= found;										// 放入到 Line 自己的記憶體裡面
		line->sure.value |= sure;										// 放入到 Line 自己的記憶體裡面
		result.count = found;
		result.value = sure;
	}
	fprintf(solver->log, "%s[%u]:after  %s\n", solver->name, solver->index, __func__);
	fprintBinary(found, solver->log);
	fprintBinary(line->sure.count, solver->log);
	fprintBinary(line->sure.value, solver->log);
	return result;
}

/**
 * 因為此 Line 算不出進一步確定值，需其他 Line 將確定值送給此 Line
 * 對 mutex 上鎖
 * while() pthread_cond_wait() ...
 * 檢查新放入的確定值，是否能增加自己已有的確定值
 * 不能	->
 * 		清空 receive 中的確定值，繼續等待
 * 能	->
 * 		將 receive 中新放入的確定值，放到 line 中
 * 		清空 receive 中的確定值
 * 		對 mutex 解鎖
 * 		回傳其他 Line 的有效幫助 Sure
 */
Sure waiting(Solver* solver, Shared* shared) {
	Line* line = solver->line;
	Receive* receive = &line->receive;
	Board* board = shared->board;
	pthread_mutex_lock(&receive->mutex);					// 得到 receive 的鎖
	fprintf(solver->log, "%s[%u]:before %s, receive = %p\n", solver->name, solver->index, __func__, &line->receive);
	fprintBinary(receive->sure.count, solver->log);
	fprintBinary(receive->sure.value, solver->log);
	pthread_mutex_lock(&board->mutex);						// 得到 board 的鎖
	board->waitingLine += 1;
	if( board->waitingLine == board->height+board->width ) {
		printf("^C ^C ^C ^C ^C ^C ^C\n");
	}
	pthread_mutex_unlock(&board->mutex);					// 釋放 board 的鎖
	while(true) {
		while( !receive->sure.count ) {
			pthread_cond_wait(&receive->cond, &receive->mutex);// 釋放 receive 的鎖
		}
															// 得到 receive 的鎖
		uint64_t diff = receive->sure.count^line->sure.count;// Receive 與 Line 差異的 bit
		diff |= receive->sure.count;						// 補上 Receive 原本的值
		if( diff != line->sure.count ) {					// 不一樣代表其他 Line 提供的是有用的幫助
			break;
		}
		fprintf(solver->log, "%s[%u]:in %s, no help\n", solver->name, solver->index, __func__);
		fprintBinary(receive->sure.count, solver->log);
		receive->sure.count = 0LLU;
	}
	Sure result = {.count=receive->sure.count, .value=receive->sure.value};
	result.value &= result.count;
	fprintf(solver->log, "%s[%u]:after  %s\n", solver->name, solver->index, __func__);
	fprintBinary(receive->sure.count, solver->log);
	fprintBinary(receive->sure.value, solver->log);
	receive->sure.count = 0ULL;
	pthread_mutex_unlock(&receive->mutex);					// 釋放 receive 的鎖
	pthread_mutex_lock(&board->mutex);						// 得到 board 的鎖
	board->waitingLine -= 1;
	pthread_mutex_unlock(&board->mutex);					// 釋放 board 的鎖
	return result;
}

/**
 * 針對從其他 Line 得到的確定值
 * 篩選自己的 possible 中，無法成為解的值，放到 possible 後面
 * C 	011011011
 * S 	010010010
 * p[0]	
 * p[1]	
 * p[2] 
 */
void filtering(Solver* solver, Sure sure) {
	Line* line = solver->line;
	fprintf(solver->log, "%s[%u]:before %s\n", solver->name, solver->index, __func__);
	fprintBinary(line->sure.count, solver->log);
	fprintBinary(line->sure.value, solver->log);
	fprintf(solver->log, "%s[%u]->possibility = %llu\n", solver->name, solver->index, line->possibility);
	uint64_t remaining = 0;
	for(uint64_t x=0;x<line->possibility;x++) {
		uint64_t bechecked = line->possible[x] & sure.count;
		uint64_t diff = bechecked ^ sure.value;
		if( !diff ) {
			fprintf(solver->log, "possible[%llu] = %llu pass\n", remaining, line->possible[x]);
			line->possible[remaining++] = line->possible[x];
		}
	}
	line->possibility = remaining;
	fprintf(solver->log, "%s[%u]:after  %s\n", solver->name, solver->index, __func__);
	fprintf(solver->log, "%s[%u]->possibility = %llu\n", solver->name, solver->index, line->possibility);
}

/**
 * 各個 thread 之間共用的變數
 * 相當於全域變數
 */
Shared* allocate_Shared(Board* board) {
	PUSH_CALL("%s(Board*)", __func__);
	Shared* shared = (Shared*) malloc(sizeof(Shared));
	shared->board = board;
	pthread_mutex_init(&shared->lock, NULL);
	POP_CALL();
	return shared;
}

/**
 * calloc solver
 * strncpy(solver->name)
 * solver->index = index
 * solver->line = line
 */
Solver* allocate_Solver(char* name, uint32_t index, Line* line) {
	PUSH_CALL("%s(name=%s, index=%u, Line*)", __func__, name, index);
	Solver* solver = (Solver*) calloc(1, sizeof(Solver));
	if( !solver ) {
		CallStackExit("solver calloc failed", "");
	}
	strncpy(solver->name, name, sizeof(solver->name));
	solver->index = index;
	if( !line ) {
		CallStackExit("line shouldn't be NULL", "");
	}
	solver->line = line;
	_mkdir("log");
	char filename[16] = {0};
	sprintf(filename, "log\\%s[%u]", name, index);
	solver->log = fopen(filename, "w");
	POP_CALL();
	return solver;
}

/**
 * malloc arg
 * assign arg->solver
 * assign arg->shared
 */
Arg* allocate_Arg(Solver* solver, Shared* shared) {
	PUSH_CALL("%s(Solver*, Shared*)", __func__);
	Arg* arg = (Arg*) malloc(sizeof(Arg));
	if( !arg ) {
		CallStackExit("arg malloc failed", "");
	}
	if( !solver ) {
		CallStackExit("solver shouldn't be NULL", "");
	}
	if( !shared ) {
		CallStackExit("shared shouldn't be NULL", "");
	}
	arg->solver = solver;
	arg->shared = shared;
	POP_CALL();
	return arg;
}


void* funcSolve(void* argument) {
	Arg* arg = (Arg*) argument;
	if (!arg) {
		printf("funcSolve got NULL arg!\n");
		pthread_exit(NULL);
	}
	Solver* solver = arg->solver;
	if (!solver) {
		printf("funcSolve got NULL solver!\n");
		pthread_exit(NULL);
	}
	Shared* shared = arg->shared;
	if (!shared) {
		printf("funcSolve got NULL shared!\n");
		pthread_exit(NULL);
	}
	Board* board = shared->board;
	if (!board) {
		printf("funcSolve got NULL board!\n");
		pthread_exit(NULL);
	}
	Line* line = solver->line;
	if (!line) {
		printf("funcSolve got NULL line!\n");
		pthread_exit(NULL);
	}
	FILE* log = solver->log;
	if (!log) {
		printf("funcSolve got NULL log!\n");
		pthread_exit(NULL);
	}
	uint32_t size = line->size;									// solver 這條線的長度
	uint64_t need = (size==64)? (~0ULL) : ((1ULL<<size)-1);
	fprintf(solver->log, "%s[%u] start\n", solver->name, solver->index);
	Line** opposite = (!strncmp(solver->name, "row", 3))? (board->col) : (board->row);
	while( line->sure.count != need ) {
		Sure sure = calculating(solver, shared);
		if( sure.count == 0ULL ) {								// calculating 計算不出確定值，waiting 等待其他 Line 幫忙
			Sure help = waiting(solver, shared);
			filtering(solver, help);
		} else {												// 計算出確定值，幫忙其他 Line
			uint32_t shift = opposite[0]->size-solver->index-1;	// 左移多少個 bit 是此 Line 幫助其他 Line 的位置
			Sure help = {.count = 1ULL<<shift, .value=0ULL};	// 幫助其他 Line 所放的值
			for(uint32_t x=0;x<size;x++) {
				uint64_t mask = 1ULL<<x;
				if( sure.count&mask ) {
					uint64_t temp = (sure.value&mask)? 1ULL : 0ULL;
					help.value = temp<<shift;
					fprintf(solver->log, "send to %u th receive\n", size-1-x);
					send(solver, shared, &opposite[size-1-x]->receive, help);
				}
			}
		}
	}
	fprintf(solver->log, "%s[%u] will pthread_exit later\n", solver->name, solver->index);
	fprintBinary(line->sure.count, solver->log);
	fprintBinary(line->sure.value, solver->log);
	pthread_exit(NULL);
}



















