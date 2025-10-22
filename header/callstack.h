#ifndef CALLSTACK_H
#define CALLSTACK_H

#include <stddef.h>

#define CALL_STACK_SIZE 4096
#define MAX_CALL_DEPTH 256
#define CALL_STACK_HEADER "Call Stack:\n\tMain()\n\t"

extern char call_stack[CALL_STACK_SIZE];
extern size_t call_stack_len[MAX_CALL_DEPTH];
extern size_t call_stack_depth;
extern size_t call_stack_pos;

#define PUSH_CALL(fmt, ...) do {															\
    if (call_stack_depth >= MAX_CALL_DEPTH) {												\
        fprintf(stderr, "Call stack depth exceeded!\n");									\
        exit(EXIT_FAILURE);																	\
    }																						\
    size_t _len = snprintf(call_stack + call_stack_pos, CALL_STACK_SIZE - call_stack_pos,	\
                           "[%s:%d] " fmt "\n\t", __FILE__, __LINE__, ##__VA_ARGS__);		\
    if (_len >= CALL_STACK_SIZE - call_stack_pos) {											\
        fprintf(stderr, "Call stack buffer overflow!\n");									\
        exit(EXIT_FAILURE);																	\
    }																						\
    call_stack_len[call_stack_depth++] = _len;												\
    call_stack_pos += _len;																	\
} while(0)

#define POP_CALL() do {											\
    if (call_stack_depth > 0) {									\
        call_stack_pos -= call_stack_len[--call_stack_depth];	\
        call_stack[call_stack_pos] = '\0';						\
    }															\
} while(0)

#define CallStackExit(fmt, ...)	do {									\
	char __buf[256];													\
	snprintf(__buf, sizeof(__buf), fmt, __VA_ARGS__);					\
	fprintf(stderr, "Error: %s [%s:%d]\n", __buf, __FILE__, __LINE__);	\
	fprintf(stderr, "%s\n", call_stack);								\
	exit(EXIT_FAILURE);													\
} while(0)
	
#endif