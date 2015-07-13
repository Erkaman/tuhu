#include "log.hpp"

#include <stdio.h>

char* logBuffer;

void Log(const char* logLevel, const char* file, int line, const char* func, const char* logStr ) {
    fprintf(stderr, "%s: %s:%d:%s:%s\n", logLevel, file,
		 line, func, logStr);
}

void LogInit() {
    logBuffer = new char[1024];
}

void LogDispose() {
    delete [] logBuffer;
}