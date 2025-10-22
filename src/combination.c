#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "combination.h"
#include "callstack.h"

void printBinary_impl(uint64_t value) {
	char result[65] = {0};
	for(int x=63;x>=0;x--) {
		uint64_t tmp = 1ULL << x;
		result[63-x] = (value&tmp)? '1' : '0';
	}
	printf("%s", result);
}

void fprintBinary_impl(uint64_t value, FILE* file) {
	char result[65] = {0};
	for(int x=63;x>=0;x--) {
		uint64_t tmp = 1ULL << x;
		result[63-x] = (value&tmp)? '1' : '0';
	}
	fprintf(file, "%s", result);
}

uint64_t gcd(uint64_t a, uint64_t b) {
    while (b) {
        uint64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

uint64_t combination(uint32_t m, uint32_t n) {
    if ( n > m ){
		return 0;
	}
	PUSH_CALL("%s(m=%d, n=%d)", __func__, m, n);
    if ( n > m-n ) n = m-n;

    uint64_t result = 1;
    for (uint32_t i = 1; i <= n; i++) {
        uint64_t numerator = m - n + i;
        uint64_t denominator = i;
        uint64_t g = gcd(numerator, denominator);
        numerator /= g;
        denominator /= g;

        result /= denominator;

        uint64_t temp = 0;
        if (__builtin_mul_overflow(result, numerator, &temp)) {
            CallStackExit("%s", "Overflow");
        }
        result = temp;
    }
	POP_CALL();
    return result;
}

uint64_t repetition_combination(uint32_t m, uint32_t n) {
	PUSH_CALL("%s(m=%d, n=%d)", __func__, m, n);
    uint64_t result = combination(m + n - 1, n);
	POP_CALL();
	return result;
}

uint32_t parseUint32(char* forWhat) {
	PUSH_CALL("%s(forWhat=%s)", __func__, forWhat);
	char str[16] = {0};
	while(true) {
		bool again = false;
		memset(str, 0, sizeof(str));
		printf("enter %s: ", forWhat);
		fgets(str, sizeof(str), stdin);
		for(size_t x=0,len=strlen(str);x<len;x++) {
			if( isdigit(str[x]) ) continue;
			if( isspace(str[x]) ) continue;
			fprintf(stderr, "Invalid input %s", str);
			again = true;
			break;
		}
		if( again ) continue;
		break;
	}
	POP_CALL();
	return (uint32_t) atoi(str);
}

// 返回輸入的第一個[字母/數字]
char parseChar(char* forWhat) {
	PUSH_CALL("%s(forWhat=%s)", __func__, forWhat);
	char str[16] = {0};
	char result = 0;
	bool again = true;
	while( again ) {
		memset(str, 0, sizeof(str));
		printf("enter %s: ", forWhat);
		fgets(str, sizeof(str), stdin);
		for(size_t x=0,len=strlen(str);x<len;x++) {
			if( isdigit(str[x]) || isalpha(str[x]) ) {
				result = str[x];
				again = false;
				break;
			}
		}
		if( !again ) break;
		fprintf(stderr, "Invalid input %s", str);
	}
	POP_CALL();
	return result;
}





