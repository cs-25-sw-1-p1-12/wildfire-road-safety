//
// Created by andre on 20/11/2025.
//

#ifndef WILDFIRE_ROAD_SAFETY_LOGGER_H
#define WILDFIRE_ROAD_SAFETY_LOGGER_H

typedef enum
{
    MESSAGE,
    WARNING,
    ERROR,
} LOG_MESSAGE_TYPE;

char* LOG_MSG_TYPE_TO_STRING(LOG_MESSAGE_TYPE msg_type);


void init_debug();
void debug_log(LOG_MESSAGE_TYPE, char* format, ...);

#endif // WILDFIRE_ROAD_SAFETY_LOGGER_H
