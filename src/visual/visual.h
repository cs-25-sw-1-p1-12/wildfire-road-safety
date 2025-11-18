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

///Adds ads a function to a list of functions the user can execute with an index input into the console,
///the description is what is displayed in the text
void append_console_command(void* action, char* description);

///configure the console to accept ANSI codes if it's windows based machine (does nothing if it's other types of OS)
void init_console();

//This will execute on of the commands added with append_console_command depending on input,
//this runs on a separate thread so scan doesn't block the program.
void execute_command();
#endif
