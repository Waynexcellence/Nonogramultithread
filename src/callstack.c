#include "callstack.h"

char call_stack[CALL_STACK_SIZE] = CALL_STACK_HEADER;
size_t call_stack_len[MAX_CALL_DEPTH] = {0};
size_t call_stack_depth = 0;
size_t call_stack_pos = sizeof(CALL_STACK_HEADER)-1;