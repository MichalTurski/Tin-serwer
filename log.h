#ifndef _L_LOG_IOT_
#define _L_LOG_IOT_

#include <string>

void initLog(std::string &logfileName, int verbo);
void logClose();
//#define log(level, ...) printf("[LOG LEVEL = %d] ", level); printf(__VA_ARGS__); printf("\n"); fflush(stdout)
void log(int level, const char *fmt, ...);

#endif //_L_LOG_IOT_
