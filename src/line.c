#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "combination.h"
#include "board.h"
#include "line.h"
#include "callstack.h"

/**
 * calloc line
 * size = size
 * calloc hint
 * possible = NULL
 * possibility = 0
 * initiallize receive
 */
Line* allocate_Line(uint32_t size, uint32_t index) {
	PUSH_CALL("%s(size=%u)", __func__, size);
	Line* line = (Line*) calloc(1, sizeof(Line));
	if( !line ) {
		CallStackExit("line malloc failed", "");
	}
	line->size = size;
	line->index = index;
	line->hint = (Hint*) calloc(1, sizeof(Hint));
	if( !line->hint ) {
		CallStackExit("line->hint calloc failed", "");
	}
	line->possible = NULL;
	line->possibility = 0;
	pthread_mutex_init(&line->receive.mutex, NULL);
	pthread_cond_init(&line->receive.cond, NULL);
	POP_CALL();
	return line;
}

// 遞迴
// unused 代表 answer 中最右邊的 unused 位元未使用
// Pidx 代表放到 possible 中的哪個位置
// Hidx 代表目前處理的是在 hint->array 中的哪個 hint
// 0000000，放入2 2
// 110 XXXX
// 0110 XXX
// 00110 XX
void re(Line* line, uint64_t answer, uint32_t unused, uint32_t* Pidx, uint32_t Hidx) {
	PUSH_CALL("%s(Line*, answer, unused=%u, *Pidx=%u, Hidx=%u)", __func__, unused, *Pidx, Hidx);
	if( Hidx >= line->hint->size ) {
		CallStackExit("Hidx >= line->hint->size", "");
	}
	uint32_t len = line->hint->array[Hidx];						// 此 hint 代表的白值長度
	uint64_t white = (len==64)?(~0ULL):(1ULL<<len)-1;			// 所有 bit 中 1 的寬度為 len
	if( Hidx == line->hint->size-1 ) {							// 放完最後一個 hint，可能有多種可能
		if( unused < len ) {
			CallStackExit("unused < len", "");					// 最後的空間不足以放入白值
		}
		for(int x=unused-len;x>=0;x--) {						// 從 unused 左邊開始往右邊放白值
			uint64_t temp = answer;
			temp |= (white<<x);
			line->possible[*Pidx] = temp;
			*Pidx = *Pidx+1;
		}
		POP_CALL();
		return;
	}
	uint32_t need = (line->hint->size-Hidx-2);					// 若之後有 (n) 個 hint，need = (n-1)
	for(int x=Hidx+1;x<line->hint->size;x++) {					// 計算第 Hidx 後的所有 hint，至少要多少個
		need += line->hint->array[x];							// bit 才可剛好塞下 Hidx 之後的所有 hint
	}
	if( unused-need < len+1 ) {
		CallStackExit("unused-need < len+1", "");					// 中段的空間不足以放入 len格白值 + 1格黑值
	}
	white <<= (need+1);												// 左移 need+1 位，右邊的 bit 留給之後的遞迴
	for(int x=unused-need-len-1;x>=0;x--) {							// 這層總共可以做 unused-need-len-1 個分支
		re(line, answer|(white<<x), need+x, Pidx, Hidx+1);
	}
	POP_CALL();
	return;
}

// 給定 Hint 求出所有的 possible
void hintToPossible(Line* line, uint32_t size) {
	PUSH_CALL("%s(Line*)", __func__);
	if( line->hint->size==0 ) {
		CallStackExit("line->hint->size should not be zero", "");
	}
	if( !line->hint->array ) {
		CallStackExit("line->hint->array should not be NULL", "");
	}
	if( !line->possible ) {
		CallStackExit("line->possible should not be NULL", "");
	}
	if( line->hint->size==1 ) {
		if( line->hint->array[0]==0 ) {
			line->possible[0] = 0;
			POP_CALL();
			return;
		}
	}
	uint32_t Pidx = 0;
	re(line, 0ULL, size, &Pidx, 0);

	POP_CALL();
	return;
}











