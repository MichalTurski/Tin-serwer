#ifndef _L_UTILS_IOT_
#define _L_UTILS_IOT_

#include <cstdio>

#define log(level, ...) printf("[LOG LEVEL = %d] ", level); printf(__VA_ARGS__); printf("\n"); fflush(stdout)

#endif //_L_UTILS_IOT_
