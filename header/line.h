#ifndef LINE_H
#define LINE_H

#include <stdint.h>
#include <pthread.h>

typedef struct Hint {
	uint32_t size;
	uint32_t* array;
} Hint;

typedef struct Sure {
	// 有效的 bit 位置
	uint64_t count;
	
	// 放確定值的地方
	uint64_t value;
} Sure;

typedef struct Receive {
	// 要把確定值放到此 Line 的 mutex
	pthread_mutex_t mutex;

	// 此 Line 等待其他 Line 幫助自己提供確定值
	pthread_cond_t cond;

	// 放確定值的地方
	Sure sure;
} Receive;

typedef struct Line {
	// 此 Line 的長度
	uint32_t size;

	// 此 Line 是第幾個順序
	uint32_t index;

	// 儲存提示
	Hint* hint;

	// 所有可能的組合，uint64_t 版本搭配位元運算
	uint64_t* possible;

	// possible 的長度，0~possibility-1是有機會當正解的
	uint64_t possibility;
	
	// 放確定值的地方
	Sure sure;

	// 其他 Line 要幫助此 Line 放資訊的集中處
	Receive receive;
} Line;
typedef Line Row;
typedef Line Col;

Line* allocate_Line(uint32_t size, uint32_t index);
void hintToPossible(Line* line, uint32_t size);
#endif