#ifndef VISUAL_H
#define VISUAL_H

#include "../models/fire.h"
#include "../models/road.h"
#include "../models/geo.h"
#include "../models/vegetation.h"

#define VIEWPORT_HEIGHT 50
#define VIEWPORT_WIDTH 50
#define TEXTBOX_WIDTH 65
#define TEXTBOX_HEIGHT 30
#define TEXTBOX_OFFSET_X 15
#define TEXTBOX_OFFSET_Y 5

#define CONSOLE_TARGET_HEIGHT 63
#define CONSOLE_TARGET_WIDTH 237

//Escape codes
#define ENABLE_MOUSE_INPUT_ANSI "\e[?1000;1006;1015h"
#define DISABLE_MOUSE_INPUT_ANSI "\e[?1000;1006;1015l"

#define ENABLE_ALTERNATIVE_BUFFER_ANSI "\033[?1049h"
#define DISABLE_ALTERNATIVE_BUFFER_ANSI "\033[?1049l"


#define ANSI_GREEN "\033[38;5;28m"
#define ANSI_GREEN_LIGHT "\033[38;5;76m"
#define ANSI_GRAY "\033[38;5;238m"
#define ANSI_GRAY_LIGHT "\033[38;5;242m"
#define ANSI_ORANGE "\033[38;5;202m"
#define ANSI_PINK "\033[38;5;201m"
#define ANSI_RED "\033[38;5;196m"
#define ANSI_SAND "\033[38;5;228m"
#define ANSI_WHITE "\033[38;5;255m"


typedef struct
{
    char* topLeft;
    char* topRight;
    char* bottomLeft;
    char* bottomRight;
} OutlineCorners;

void draw_current_state(RoadSegSlice roads, FireSlice fires, VegSlice vegetation);
///Refreshes the console, and redraws everything
void draw_console();
///This changes the content of the textbox that is displayed besides the gridmap
void write_to_textbox(const char*, ...);

///Adds a function to a list of functions, the user can execute with an index input, into the console,
///the description is what is displayed in the text
void prepend_console_command(void* action, char* description);

///configure the console to accept ANSI codes if it's windows based machine (does nothing if it's other types of OS)
void init_console();
///Resets any changes the console could have potentially experienced
void close_console();

///This will execute on of the commands added with append_console_command depending on input,
///this runs on a separate thread so scan doesn't block the program.
void execute_command();

void set_bounding_box(BoundBox box);

///Clears the console of all characters
void clear();
#endif
