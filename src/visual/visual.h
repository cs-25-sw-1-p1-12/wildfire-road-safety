#ifndef VISUAL_H
#define VISUAL_H

#include "../models/fire.h"
#include "../models/road.h"

#define VIEWPORT_HEIGHT 50
#define VIEWPORT_WIDTH 50
#define TEXTBOX_WIDTH 65;
#define TEXTBOX_HEIGHT 30;
#define TEXTBOX_OFFSET_X 15
#define TEXTBOX_OFFSET_Y 5

void draw_current_state(RoadSegSlice roads, FireSlice fires);
///Refreshes the console, and redraws everything
void draw_console();
///This changes the content of the textbox that is displayed besides the gridmap
void write_to_textbox(char*);

void append_console_command(void* action, char* description);

void init_console();
#endif
