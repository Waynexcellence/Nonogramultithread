#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "combination.h"
#include "line.h"
#include "board.h"
#include "callstack.h"

/**
 * 用來儲存輸出 Row 跟 Col 提示的緩存區
 * 理論上只會有 Row 與 Col 兩種
 */
typedef struct PrintInfo {
	// 所有 Line 中 Hint 數量最多的 Hint 數量
	uint32_t maxHint;

	// 所有 Line 中 Hint 最多位數的 Hint 位數
	uint32_t maxDigitLength;
} PrintInfo;

// 單純把 rowInfo + colInfo
// horizontal + vertical 放在一起
typedef struct RCInfo {
	PrintInfo rInfo;
	PrintInfo cInfo;
	uint32_t horizontal;
	uint32_t vertical;
} RCInfo;

/**
 * malloc board
 * malloc row, allocate_Line(row[x])
 * malloc col, allocate_Line(col[x])
 * initialize mutex, cond, waitingLine
 * assign height, width
 */
Board* allocate_Board(const uint32_t height, const uint32_t width) {
	PUSH_CALL("%s(height=%u, width=%u)", __func__, height, width);
	Board* board = (Board*) malloc(sizeof(Board));
	if( !board ) {
		CallStackExit("board malloc failed", "");
	}
	board->waitingLine = 0;
	board->height = height;
	board->width = width;
	board->row = (Row**) malloc(height * sizeof(Row*));
	if( !board->row ) {
		CallStackExit("board->row malloc failed", "");
	}
	for(uint32_t x=0;x<board->height;x++) {
		board->row[x] = allocate_Line(board->width, x);
	}
	board->col = (Col**) malloc(width * sizeof(Col*));
	if( !board->col ) {
		CallStackExit("board->col malloc failed", "");
	}
	for(uint32_t x=0;x<board->width;x++) {
		board->col[x] = allocate_Line(board->width, x);
	}
	pthread_mutex_init(&board->mutex, NULL);
	pthread_cond_init(&board->cond, NULL);
	POP_CALL();
	return board;
}

PrintInfo get_PrintInfo(Line** line, uint32_t length) {
	PUSH_CALL("%s(Board*)", __func__);
	PrintInfo result = {0};
	for(uint32_t x=0;x<length;x++) {
		Hint* hint = line[x]->hint;
		uint32_t a = result.maxHint;
		uint32_t b = hint->size;
		result.maxHint = (a>b)? a : b;
		for(uint32_t y=0;y<hint->size;y++) {
			uint32_t c = result.maxDigitLength;
			uint32_t d = digits(hint->array[y]);
			result.maxDigitLength = (c>d)? c : d;
		}
	}
	POP_CALL();
	return result;
}

void printCol(Board* board, RCInfo rc) {
	PUSH_CALL("%s(Board*, RCInfo)", __func__);
	uint32_t prefix = (rc.horizontal==HINT_LEFT)? (rc.rInfo.maxHint*(rc.rInfo.maxDigitLength+1)):0;
	// 從 0 ~ rc.cInfo.maxHint-1
	uint32_t maxHint = rc.cInfo.maxHint;
	uint32_t maxDigitLength = rc.cInfo.maxDigitLength;
	for(uint32_t x=0;x<maxHint;x++) {
		printf("%*s", prefix, "");
		for(uint32_t y=0;y<board->width;y++) {
			Hint* hint = board->col[y]->hint;
			if( rc.vertical==HINT_UP && x+hint->size>=maxHint ) {		// 印在上方，有數字要印
				printf("%*u ", maxDigitLength, hint->array[x+hint->size-maxHint]);
			} else if( rc.vertical==HINT_DOWN && x<hint->size ) {		// 印在下方，有數字要印
				printf("%*u ", maxDigitLength, hint->array[x]);
			} else {													// 沒有數字要印
				printf("%*s ", maxDigitLength, "");
			}
		}
		printf("\n");
	}
	POP_CALL();
	return;
}

void printRow(Board* board, RCInfo rc) {
	PUSH_CALL("%s(Board*, RCInfo)", __func__);
	uint32_t maxHint = rc.rInfo.maxHint;
	uint32_t maxDigitLength = rc.rInfo.maxDigitLength;
	uint32_t maxAnswerLength = rc.cInfo.maxDigitLength;
	for(uint32_t x=0;x<board->height;x++) {
		Hint* hint = board->row[x]->hint;
		if( rc.horizontal==HINT_LEFT ) {
			printf("%*s", (maxHint-hint->size)*(maxDigitLength+1), "");
			for(uint32_t y=0;y<hint->size;y++) {
				printf("%*u ", maxDigitLength, hint->array[y]);
			}
		}
		for(uint32_t y=0;y<board->width;y++) {
			uint64_t bit = 1ULL<<(board->width-1-y);
			if( bit&board->row[x]->sure.count ) {
				printf("%*c ", maxAnswerLength, (bit&board->row[x]->sure.value)?'O':'X');
			} else {
				printf("%*c ", maxAnswerLength, '.');
			}
		}
		if( rc.horizontal==HINT_RIGHT ) {
			for(uint32_t y=0;y<hint->size;y++) {
				printf("%*u ", maxDigitLength, hint->array[y]);
			}
		}
		printf("\n");
	}
	POP_CALL();
}

void printBoard(Board* board, uint32_t horizontal, uint32_t vertical) {
	PUSH_CALL("%s(Board*, horizontal=%u, vertical=%u)", __func__, horizontal, vertical);
	if( horizontal!=HINT_LEFT && horizontal!=HINT_RIGHT ) {
		CallStackExit("horizontal(%u) is invalid", horizontal);
	}
	if( vertical!=HINT_UP && vertical!=HINT_DOWN ) {
		CallStackExit("vertical(%u) is invalid", vertical);
	}
	RCInfo rc = {.horizontal=horizontal, .vertical=vertical};
	rc.rInfo = get_PrintInfo(board->row, board->height);
	rc.cInfo = get_PrintInfo(board->col, board->width);
	if( vertical==HINT_UP ) {
		printCol(board, rc);
	}
	printRow(board, rc);
	if( vertical==HINT_DOWN ) {
		printCol(board, rc);
	}
	POP_CALL();
	return;
}
/**
 * calloc hint->array
 * 要求使用者輸入每一個 Line 的 Hint
 * 回傳此 Line 有多少種可能
 */
static uint64_t parseHint(Hint* restrict hint, uint32_t length, char* forWhat) {
	PUSH_CALL("%s(Hint*, length=%u, forWhat=%s)", __func__, length, forWhat);
	hint->array = calloc(length, sizeof(uint32_t));
	if( !hint->array ) {
		CallStackExit("hint->array calloc failed", "");
	}
	uint32_t limit = (length>>1)+1;
	uint64_t answer = 0;
	char str[256] = {0};											// 存放使用者的輸入 raw data
	while( true ) {
		bool again = false;
		memset(str, 0, sizeof(str));
		printf("enter %s: ", forWhat);
		fgets(str, sizeof(str), stdin);
		uint32_t ball = length;										// 籃子中放球的 球，H(m,n) 的 n
		for(size_t x=0,len=strlen(str);x<len;x++) {					// 針對 row data 檢查字元
			if( isdigit(str[x]) ) continue;
			if( isspace(str[x]) ) continue;
			fprintf(stderr, "Invalid input %s", str);
			again = true;
			break;
		}
		if( again ) continue;
		char *token = strtok(str, " \n");
		while( token!=NULL ) {										// 針對 Hint 檢查是否有解
			uint32_t number = (uint32_t) atoi(token);
			if( ball < number ) {
				fprintf(stderr, "you can't enter a hint without any solution, length = %u\n", length);
				again = true;
				break;
			}
			ball -= number;
			hint->array[hint->size++] = number;
			if( hint->size>limit ) {
				fprintf(stderr, "you can't enter more than %u number\n", limit);
				hint->size = 0;
				again = true;
				break;
			}
			token = strtok(NULL, " \n");
		}
		if( again ) continue;
		if( hint->size==0 ) {
			fprintf(stderr, "you can't enter a hint without anything, length = %u\n", length);
			continue;
		}
		// answer = repetition_combination(hint->size+1, ball-hint->size+1);
		// H(m,n) 等值於 C(m+n-1,n)
		// answer = combination(ball+1, ball-hint->size+1);
		if( hint->size==1 && hint->array[0]==0 ) {
			answer = 1;
		} else {
			answer = combination(ball+1, ball-hint->size+1);			// 針對 Hint 檢查是否有解
		}
		if( answer==0 ) {
			fprintf(stderr, "you can't enter a hint without any solution, length = %u\n", length);
			continue;
		}
		break;
	}
	POP_CALL();
	return answer;
}

/**
 * 先輸入 board 的 height + width
 * 再輸入每個 row + col 的 hint 存到每個 Line 中
 */
Board* parseBoard() {
	PUSH_CALL("%s()", __func__);
	uint32_t height, width;
	do {
		height = parseUint32("board's height");
		if (height < 2) {
			printf("board's height must be >= 2\n");
		} else if (height > 64) {
			printf("board's height must be <= 64\n");
		}
	} while (height < 2 || height > 64);
	do {
		width = parseUint32("board's width");
		if (width < 2) {
			printf("board's width must be >= 2\n");
		} else if (width > 64) {
			printf("board's width must be <= 64\n");
		}
	} while (width < 2 || width > 64);
	Board* board = allocate_Board(height, width);
	char forWhat[64] = {0};
	for(int x=0;x<board->height;x++) {
		sprintf(forWhat, "%d'th row hint", x);
		board->row[x]->possibility = parseHint(board->row[x]->hint, board->width, forWhat);
		board->row[x]->possible = malloc(board->row[x]->possibility * sizeof(uint64_t));
		if( !board->row[x]->possible ) {
			CallStackExit("board->row[%d]->possible malloc failed", x);
		}
		hintToPossible(board->row[x], board->width);
	}
	for(int x=0;x<board->width;x++) {
		sprintf(forWhat, "%d'th col hint", x);
		board->col[x]->possibility = parseHint(board->col[x]->hint, board->height, forWhat);
		board->col[x]->possible = malloc(board->col[x]->possibility * sizeof(uint64_t));
		if( !board->col[x]->possible ) {
			CallStackExit("board->col[%d]->possible malloc failed", x);
		}
		hintToPossible(board->col[x], board->height);
	}
	POP_CALL();
	return board;
}


















