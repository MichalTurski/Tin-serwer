//
// Created by michal on 25.05.18.
//

#include <string>
#include <cstdarg>
#include <unistd.h>
#include "log.h"

int verbosity;
FILE *logfile = nullptr;

void log(int level, const char *fmt, ...) {
    time_t rawtime;
    struct tm *timeinfo;
    char timeBuf[80];
    va_list fargs, args;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeBuf, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

    va_start(args, fmt);
    va_copy(fargs, args);
    if (logfile != nullptr) {
        fprintf(logfile, "%s ", timeBuf);
        fprintf(logfile, "[LOG_LEVEL = %d] ", level);
        vfprintf(logfile, fmt, fargs);
        fprintf(logfile, "\n");
        fflush(logfile);
    }
    if (level <= verbosity) {
        printf("%s ", timeBuf);
        printf("[PID = %d] ", getpid());
        printf("[LOG_LEVEL = %d] ", level);
        vprintf(fmt, args);
        printf("\n");
        fflush(stdout);
    }
    va_end(args);
    va_end(fargs);
}

void initLog(std::string &logfileName, int verbo) {
    verbosity = verbo;
    logfile = fopen(logfileName.c_str(), "w");
    if (logfile == nullptr) {
        log(1, "Failed to open file %s.", logfileName.c_str());
        exit(-1);
    } else {
        log(3, "Opening logfile %s succeed.", logfileName.c_str());
    }
}

void logClose() {
    fclose(logfile);
}