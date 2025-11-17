#ifndef VISUAL_H
#define VISUAL_H

#include "../models/fire.h"
#include "../models/road.h"

#define VIEWPORT_HEIGHT 10
#define VIEWPORT_WIDTH 10

void draw_current_state(RoadSegSlice roads, FireSlice fires);
///Refreshes the console, and redraws everything
void draw_console();
///This changes the content of the textbox that is displayed besides the gridmap
void write_to_textbox(char*);

void append_console_command(void* action, char* description);

#endif
