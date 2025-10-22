#ifndef Combination_H
#define Combination_H

#include <stdint.h>

void printBinary_impl(uint64_t value);
void fprintBinary_impl(uint64_t value, FILE* file);
uint64_t combination(uint32_t m, uint32_t n);
uint64_t repetition_combination(uint32_t m, uint32_t n);
uint32_t parseUint32(char* forWhat);
char parseChar(char* forWhat);
/**
 * 返回一個數的數字個數
 * DIGITS(12) = 2
 * DIGITS(9) = 1
 * DIGITS(0) = 0
 * */ 
#define DIGITS(x) ({						\
    int __n = (x);							\
    int __d = 0;							\
    if (__n == 0) __d = 0;					\
    else if (__n < 10) __d = 1;				\
    else if (__n < 100) __d = 2;			\
    else if (__n < 1000) __d = 3;			\
    else while (__n) { __d++; __n /= 10;}	\
    __d; /* ← block expression*/			\
})

static inline uint32_t digits(uint32_t n) {
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    uint32_t d = 0;
    while (n) { d++; n /= 10; }
    return d;
}

#define printBinary(value) do {		\
	printf("%s = ", #value);		\
	printBinary_impl((value));		\
	printf("\n");					\
} while(0)

#define fprintBinary(value, file) do {	\
	fprintf((file), "%s = ", #value);	\
	fprintBinary_impl((value), (file));	\
	fprintf((file), "\n");				\
} while(0)

#endif