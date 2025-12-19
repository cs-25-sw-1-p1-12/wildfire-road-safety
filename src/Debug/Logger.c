//
// Created by andre on 20/11/2025.
//

#include "Logger.h"

#include "../dyn.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

FILE* logStream;

char* now();


char* LOG_MSG_TYPE_TO_STRING(LOG_MESSAGE_TYPE msg_type)
{
    switch (msg_type)
    {
        case MESSAGE:
            return "MESSAGE";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            return "NOT STRING REP FOR MSG_TYPE";
    }
}

void init_debug()
{
    logStream = fopen("debug.log", "w+");
    debug_log(MESSAGE, "===========================================", 123);
    debug_log(MESSAGE, "Log is running!", 123);
}

void debug_log(const LOG_MESSAGE_TYPE messageType, char* format, ...)
{
    if (logStream == NULL)
        init_debug();
    va_list arg;
    String msg = str_from("");

    char messageInfo[100];
    sprintf(messageInfo, "[%s][%s]: ", now(), LOG_MSG_TYPE_TO_STRING(messageType));
    str_append(&msg, messageInfo);

    va_start(arg, format);
    char message[100];
    vsprintf(message, format, arg);
    str_append(&msg, message);
    va_end(arg);

    str_push(&msg, '\n');
    fprintf(logStream, "%s", msg.chars);
    fflush(logStream);
    str_free(&msg);
}

char* now()
{
    // https://stackoverflow.com/questions/5141960/get-the-current-time-in-c
    time_t rawTime;
    time(&rawTime);
    struct tm* timeInfo = localtime(&rawTime);
    char* t = asctime(timeInfo);
    size_t len = strlen(t);
    memmove(&t[len - 1], &t[len], 1);
    return t;
}
