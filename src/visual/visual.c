#include "visual.h"

#include "../Debug/Logger.h"
#include "../dyn.h"
#include "../Debug/Logger.h"

#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else // Linux & MacOS
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

//https://stackoverflow.com/questions/6486289/how-to-clear-the-console-in-c
//https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
//https://stackoverflow.com/questions/917783/how-do-i-work-with-dynamic-multi-dimensional-arrays-in-c
//https://www.cs.uleth.ca/~holzmann/C/system/ttyraw.c
//https://superuser.com/questions/413073/windows-console-with-ansi-colors-handling
//https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
//https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h3-Sixel-Graphics
//https://stackoverflow.com/questions/14888027/mutex-lock-threads
//https://www.geeksforgeeks.org/c/thread-functions-in-c-c/

//#define ANSI_RED "\033[31m"
//#define ANSI_GREEN "\033[32m"
#define ANSI_BLUE "\033[34m"
#define ANSI_NORMAL "\033[0m"

/*
185:╣
186:║
187:╗
188:╝
200:╚
201:╔
202:╩
203:╦
204:╠
205:═
206:╬
219:█
 */

// dyn.h lacks a function to only append char instead of a whole string/char*
// So these are here for that purpose.
/// ╔ default(win32: 201)
#define TL_CORNER "╔"
/// ╗ default(win32: 187)
#define TR_CORNER "╗"
/// ╚ default(win32: 200)
#define BL_CORNER "╚"
/// ╝ default(win32: 188)
#define BR_CORNER "╝"

/// ║ default(win32: 186)
#define VERT_LINE "║"
/// ═ default(win32: 205)
#define HORI_LINE "═"

/// █ default(win32: 219)
#define GRID_BLOCK "█"
#define GRID_BLOCK_LIGHT "░"
#define GRID_BLOCK_MEDIUM "▒"
#define GRID_BLOCK_DARK "▓"


/// ╩ default(win32: 202)
#define UP_T_JUNC "╩"
#define LINEBREAK "\n"

LCoord get_terminal_size();

// These const exist due to some issue with Clion not interpreting them as actual numbers.
const int vHeight = VIEWPORT_HEIGHT;
const int tHeight = TEXTBOX_HEIGHT;
const int vWidth = VIEWPORT_WIDTH;
const int tWidth = TEXTBOX_WIDTH;
const int height = (tHeight > vHeight) ? tHeight : vHeight;
LCoord consoleSize;

int scaled_vHeight()
{
    if (consoleSize.x <= 0 || consoleSize.y <= 0)
        for (int i = 0; i < 10; i++)
        {
            consoleSize = get_terminal_size();
            if (consoleSize.x > 0 || consoleSize.y > 0)
                break;
            sleep(1);
        }
    return (int)((double)vHeight * (consoleSize.y / CONSOLE_TARGET_HEIGHT));
}

int scaled_vWidth()
{
    if (consoleSize.x <= 0 || consoleSize.y <= 0)
        for (int i = 0; i < 10; i++)
        {
            consoleSize = get_terminal_size();
            if (consoleSize.x > 0 || consoleSize.y > 0)
                break;
            sleep(1);
        }
    return (int)((double)vWidth * (consoleSize.x / CONSOLE_TARGET_WIDTH));
}

int scaled_tHeight()
{
    if (consoleSize.x <= 0 || consoleSize.y <= 0)
        for (int i = 0; i < 10; i++)
        {
            consoleSize = get_terminal_size();
            if (consoleSize.x > 0 || consoleSize.y > 0)
                break;
            sleep(1);
        }
    return (int)((double)tHeight * (consoleSize.y / CONSOLE_TARGET_HEIGHT));
}

int scaled_tWidth()
{
    if (consoleSize.x <= 0 || consoleSize.y <= 0)
        for (int i = 0; i < 10; i++)
        {
            consoleSize = get_terminal_size();
            if (consoleSize.x > 0 || consoleSize.y > 0)
                break;
            sleep(1);
        }
    return (int)((double)tWidth * (consoleSize.x / CONSOLE_TARGET_WIDTH));
}

int scaled_height()
{
    if (consoleSize.x <= 0 || consoleSize.y <= 0)
        for (int i = 0; i < 10; i++)
        {
            consoleSize = get_terminal_size();
            if (consoleSize.x > 0 || consoleSize.y > 0)
                break;
            sleep(1);
        }
    return (int)((double)height * (consoleSize.y / CONSOLE_TARGET_HEIGHT));
}

unsigned int selectedCmd = 0;
RoadSegSlice current_roads;
FireSlice current_fires;

typedef struct
{
    void (*triggerAction)();
    char* descriptionText;
    size_t index;
} ConsoleCommands;

String textBoxText;
VecDef(ConsoleCommands) commands;

void draw_grid();
void draw_outline(int line, int column, int height, int width, char* title, OutlineCorners corners);


#ifdef _WIN32
#define UTF8CODE 65001
DWORD defaultConsoleSettingsInput;
DWORD defaultConsoleSettingsOutput;
UINT defaultConsoleOutputType;
#else // Linux & MacOS
struct termios orig_termios;
#endif
LCoord fontSize;

bool cmdIsRunning = false;
bool isMonitoring = true;

void monitor_resize_event()
{
    debug_log(MESSAGE, "RESIZE MONITOR WAS CREATED!");
    while (isMonitoring)
    {
        //https://stackoverflow.com/questions/46658472/non-blocking-readconsoleinput
        //https://stackoverflow.com/questions/10856926/sigwinch-equivalent-on-windows
        //https://learn.microsoft.com/en-us/windows/console/reading-input-buffer-events
        //https://stackoverflow.com/questions/6812224/getting-terminal-size-in-c-for-windows
        //https://stackoverflow.com/questions/23369503/get-size-of-terminal-window-rows-columns
        //https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
        double height = -1;
        double width = -1;
#if WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        width = w.ws_col;
        height = w.ws_row;
#endif
        if (!cmdIsRunning)
            if (fabs(width - consoleSize.x) > 1 || fabs(height - consoleSize.y) > 1)
            {
                debug_log(MESSAGE, "window size changed, resizing (%f, %f) -> (%f, %f)",
                          consoleSize.x, consoleSize.y, width, height);
                consoleSize = (LCoord){.x = width, .y = height};
                draw_console();
            }
        int milliSecs = 16;
        struct timespec ts = (struct timespec){
            .tv_sec = milliSecs / 1000,
            .tv_nsec = (milliSecs % 1000) * 1000000,
        };
        nanosleep(&ts, &ts);
    }
}

pthread_t reSizeMonitor;

void init_console()
{
    atexit(close_console);
    debug_log(MESSAGE, "start initiating console...");
    textBoxText = str_from("");
#ifdef _WIN32

    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    //Saves the console input settings,
    //so it can be correctly reset on shutdown
    GetConsoleMode(hInput, &defaultConsoleSettingsInput);
    //Allows the windows console to receive and handle ANSI escape codes
    SetConsoleMode(hInput, ENABLE_VIRTUAL_TERMINAL_INPUT);

    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    //Saves the console output settings,
    //so it can be correctly reset on shutdown
    GetConsoleMode(hOutput, &defaultConsoleSettingsOutput);
    //Allows the windows console to display and use ANSI escape codes
    SetConsoleMode(hOutput, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    //Saves the console encoding, so it can be correctly reset on shutdown
    defaultConsoleOutputType = GetConsoleOutputCP();
    //Force the windows console to use UTF8 encoding
    SetConsoleOutputCP(UTF8CODE);

    /* THIS IS BORKED IN WINDOWS 11
    //Maximize console window so everything gets drawn correctly.
    HWND windowHandle = GetConsoleWindow();
    ShowWindow(windowHandle, SW_MAXIMIZE);
    */
#else // Linux & MacOS
    tcgetattr(STDIN_FILENO, &orig_termios);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
#endif
    //Shows mouse cursor (in the event it was hidden before initiation).
    printf("\e[?25h");
    debug_log(MESSAGE, "Done!");
    //Enable the alternative buffer. Aka removes the ability to scroll in the console.
    printf(ENABLE_ALTERNATIVE_BUFFER_ANSI);
    consoleSize = get_terminal_size();
}

void close_console()
{
    isMonitoring = false;
    pthread_join(reSizeMonitor, NULL);
    debug_log(MESSAGE, "start closing console...");
    //Disable the alternative buffer
    printf(DISABLE_ALTERNATIVE_BUFFER_ANSI);
    //Disable the detection of mouse inputs.
    printf(DISABLE_MOUSE_INPUT_ANSI);
    str_free(&textBoxText);
    vec_free(commands);
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hInput, defaultConsoleSettingsInput);
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hOutput, defaultConsoleSettingsOutput);

    SetConsoleOutputCP(defaultConsoleOutputType);
#else // Linux & MacOS
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
#endif
    debug_log(MESSAGE, "Done!");
}

///Writes directly to the console, is much faster than printf, does not use a buffer.
void fast_print(const char* format)
{
    //fputs(format, stdout);
    fwrite(format, strlen(format), 1, stdout);
}

///Writes directly to the console, this allows for arguments but is slightly slower than fast_print
void fast_print_args(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char textBuffer[strlen(format) + 100];
    int amountWritten = vsprintf(textBuffer, format, args);
    char text[amountWritten + 1];
    memcpy(text, textBuffer, sizeof(text));
    va_end(args);
    fast_print(text);
}

BoundBox globalBounds = (BoundBox){
    .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
    .c2 = {.lat = 57.01467041792688, .lon = 9.99681826817088}
};

LCoord get_terminal_size()
{
    //https://stackoverflow.com/questions/74431114/get-terminal-size-using-ansi-escape-sequences/74432616#74432616
    fast_print("\033[s\033[9999;9999H");
    fast_print("\033[6n");
    const int c = getchar();
    if (c == 27)
    {
        int line = -1;
        int column = -1;

        scanf("[%d;%dR", &line, &column);

        fast_print("\e[u");
        debug_log(MESSAGE, "%f", (float)line);
        if (line < 0 || column < 0)
            return (LCoord){.x = 0, .y = 0};

        return (LCoord){.x = column, .y = line};
    }
    fast_print("\033[u");
    return (LCoord){.x = 0, .y = 0};
}

LCoord get_cursor_pos()
{
    //https://stackoverflow.com/questions/74431114/get-terminal-size-using-ansi-escape-sequences/74432616#74432616
    fast_print("\033[6n");
    const int c = getchar();
    if (c == 27)
    {
        int line = -1;
        int column = -1;

        scanf("[%d;%dR", &line, &column);

        debug_log(MESSAGE, "%f", (float)line);
        if (line < 0 || column < 0)
            return (LCoord){.x = 0, .y = 0};

        return (LCoord){.x = column, .y = line};
    }
    return (LCoord){.x = 0, .y = 0};
}


void set_bounding_box(BoundBox box)
{
    globalBounds = box;
}

void make_white_space(String* string, int amount)
{
    char whiteSpace[amount];
    for (int x = 0; x < sizeof(whiteSpace); ++x)
        whiteSpace[x] = x < (sizeof(whiteSpace) - 1) ? ' ' : '\0';
    str_append(string, whiteSpace);
}

void make_white_space_fast_print(int length)
{
    char whiteSpace[length];
    for (int x = 0; x < sizeof(whiteSpace); ++x)
        whiteSpace[x] = x < (sizeof(whiteSpace) - 1) ? ' ' : '\0';
    fast_print(whiteSpace);
}

void ANSI_RGB(String* string, int r, int g, int b)
{
    char rgb[256];
    sprintf(rgb, "\033[38;2;%d;%d;%dm", r, g, b);
    str_append(string, rgb);
}

void printf_color(char* text, char* color)
{
    if ((int)text[0] != 27)
    {
        fast_print_args("\033[38;5;212m%s%s", text, ANSI_NORMAL);
        return;
    }
    fast_print_args("%s%s%s", color, text, ANSI_NORMAL);
}

double get_point_dist_to_road(LCoord n1, LCoord n2, LCoord p, double tolerance)
{
    // Check if in bounding box
    if (p.x > MAX(n1.x, n2.x) + tolerance || p.x < MIN(n1.x, n2.x) - tolerance ||
        p.y > MAX(n1.y, n2.y) + tolerance || p.y < MIN(n1.y, n2.y) - tolerance)
        return INFINITY;

    // Formula from
    // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
    double numerator = fabs((n2.y - n1.y) * p.x - (n2.x - n1.x) * p.y + n2.x * n1.y - n2.y * n1.x);

    double denominator = sqrt(pow(n2.y - n1.y, 2) + pow(n2.x - n1.x, 2));

    return numerator / denominator;
}

bool road_has_road_at(RoadSegSlice road_data, LCoord point, double tolerance)
{
    RoadSegSlice roads = road_data;

    for (size_t i = 0; i < roads.len; i++)
    {
        NodeSlice nodes = roads.items[i].nodes;

        for (size_t j = 0; j < nodes.len; j++)
        {
            if (j >= nodes.len - 1)
                break;

            RoadNode node1 = nodes.items[j];
            LCoord node1LCoord = global_to_local(node1.coords, globalBounds, vHeight, vWidth);
            RoadNode node2 = nodes.items[j + 1];
            LCoord node2LCoord = global_to_local(node2.coords, globalBounds, vHeight, vWidth);


            double dist = get_point_dist_to_road(node1LCoord, node2LCoord, point, tolerance);

            if (dist >= -tolerance && dist <= tolerance)
                return true;
        }
    }

    return false;
}

bool fire_has_fire_at(FireSlice fire_data, LCoord point, double tolerance)
{
    const FireSlice fires = fire_data;
    for (int i = 0; i < fires.len; i++)
    {
        const LCoord n1 = fires.items[i].lcoord;
        double dst = sqrt(pow(point.y - n1.y, 2) + pow(point.x - n1.x, 2));
        if (dst < tolerance)
            return true;
    }
    return false;
}

int get_road_risk(RoadSegSlice road_data, LCoord point, double tolerance)
{
    RoadSegSlice roads = road_data;

    for (size_t i = 0; i < roads.len; i++)
    {
        NodeSlice nodes = roads.items[i].nodes;

        for (size_t j = 0; j < nodes.len; j++)
        {
            if (j >= nodes.len - 1)
                break;

            RoadNode node1 = nodes.items[j];
            LCoord node1LCoord = global_to_local(node1.coords, globalBounds, vHeight, vWidth);
            RoadNode node2 = nodes.items[j + 1];
            LCoord node2LCoord = global_to_local(node2.coords, globalBounds, vHeight, vWidth);


            double dist = get_point_dist_to_road(node1LCoord, node2LCoord, point, tolerance);

            if (dist >= -tolerance && dist <= tolerance)
            {
                debug_log(MESSAGE, "risk is: %d with the id: %d", roads.items[i].risk,
                          roads.items[i].id);
                return roads.items[i].risk;
            }
        }
    }

    return -1;
}

void draw_grid()
{
    fast_print("\e[s");
    fast_print("\e[?25l");
    //printf("\e[s");
    String gridContent = str_from("");
    fast_print("\033[2;2H");

    int greenCount = 0;
    int blueCount = 0;
    char* ANSI_CODE;
    const int h = scaled_vHeight();
    const int w = scaled_vWidth();
    const LCoord prctDiff = {.x = (double)w / vWidth, .y = (double)h / vHeight};
    int ansi_code = -2;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            const double tolerance = 1.0;
            const LCoord lCoord = (LCoord){.x = (x / prctDiff.x), .y = (y / prctDiff.y)};
            const bool isRoad = road_has_road_at(current_roads, lCoord, tolerance);
            const bool isFire = fire_has_fire_at(current_fires, lCoord, tolerance);

            if (isFire)
            {
                int code;
                int dummy1;
                int dummy2;

                ANSI_CODE = ANSI_RED;


                sscanf(ANSI_CODE, "[%d;%d;%d", &dummy1, &dummy2, &code);
                if (code != ansi_code)
                {
                    ansi_code = code;
                    str_append(&gridContent, ANSI_CODE);
                }

                str_append(&gridContent, GRID_BLOCK);
                str_append(&gridContent, GRID_BLOCK);
            }
            else if (isRoad)
            {
                const int risk = get_road_risk(current_roads, lCoord, tolerance);
                blueCount++;
                int code;
                int dummy1;
                int dummy2;

                if (risk > 1)
                    ANSI_CODE = ANSI_ORANGE;
                else if (risk < 0)
                    ANSI_CODE = ANSI_PINK;
                else
                    ANSI_CODE = ANSI_GRAY;


                sscanf(ANSI_CODE, "[%d;%d;%d", &dummy1, &dummy2, &code);
                if (code != ansi_code)
                {
                    ansi_code = code;
                    str_append(&gridContent, ANSI_CODE);
                }

                str_append(&gridContent, GRID_BLOCK);
                str_append(&gridContent, GRID_BLOCK);
            }
            else
            {
                if (ansi_code != -1)
                {
                    ansi_code = -1;
                    greenCount++;
                    str_append(&gridContent, ANSI_GREEN);
                }
                str_append(&gridContent, GRID_BLOCK_MEDIUM);
                str_append(&gridContent, GRID_BLOCK_MEDIUM);
            }
        }
        str_append(&gridContent, "\033[1E");
        str_append(&gridContent, "\033[1C");
    }
    str_append(&gridContent, ANSI_NORMAL);
    fast_print(gridContent.chars);
    //(MESSAGE, "green: %d, blue: %d, total grid size: %d", greenCount, blueCount, vWidth * 2 * vHeight);
    str_free(&gridContent);
    printf("\e[u");;
}

void draw_text(char* text, int line, int column, int height, int width)
{
    char offsetPos[1000];
    sprintf(offsetPos, "\033[%d;%dH", line, column);
    String textBox = str_from(offsetPos);
    String stringText = str_from(text);

    char newline[1000] = "\033[1E";
    if (column > 1)
        sprintf(newline, "\033[1E\033[%dC", column - 1);

    for (int i = 0; i < MIN(stringText.len, height * width); i++)
    {
        char c[2] = {stringText.chars[i], '\0'};
        str_append(&textBox, c);
        int currentLineWidth = i % width;
        if (currentLineWidth == width - 1)
        {
            str_append(&textBox, newline);
        }
    }
    fast_print(textBox.chars);
    str_free(&textBox);
    str_free(&stringText);
}

void draw_horizontal_outline(String* string, int line, int column, int width, char* title)
{
    String headerText = str_from(title);
    int lengthMax = (int)ceil(((double)width) / 2 - (((double)headerText.len) / 2));
    int lengthMin = (int)floor(((double)width) / 2 - (((double)headerText.len) / 2));

    char text[1000];
    sprintf(text, "\033[%d;%dH", line, column);
    str_append(string, text);

    for (int i = 0; i < lengthMax; i++)
        str_append(string, HORI_LINE);

    str_append(string, title);
    for (int i = 0; i < lengthMin; i++)
        str_append(string, HORI_LINE);
}

void draw_outline(int line, int column, int height, int width, char* title, OutlineCorners corners)
{
    char offsetPos[1000];
    sprintf(offsetPos, "\033[%d;%dH", line, column);

    String gridOutline = str_from(offsetPos);

    str_append(&gridOutline, corners.topLeft);
    draw_horizontal_outline(&gridOutline, line, column + 1, width, title);
    str_append(&gridOutline, corners.topRight);

    char text[1000];
    sprintf(text, "\033[%dC", width);

    char newline[1000] = "\033[1E";
    if (column > 1)
        sprintf(newline, "\033[1E\033[%dC", column - 1);
    for (int x = 0; x < height; x++)
    {
        str_append(&gridOutline, newline);
        str_append(&gridOutline, VERT_LINE);
        str_append(&gridOutline, text);
        str_append(&gridOutline, VERT_LINE);

        //DEBUG LINE NUMBER DISPLAY
        // char num[1000];
        // sprintf(num, " - %d", x + 1);
        // str_append(&gridOutline, num);
    }

    str_append(&gridOutline, newline);
    str_append(&gridOutline, corners.bottomLeft);
    draw_horizontal_outline(&gridOutline, line + height + 1, column + 1, width, "");
    str_append(&gridOutline, corners.bottomRight);
    fast_print(gridOutline.chars);
    str_free(&gridOutline);
}

void draw_cmd()
{
    const int height = scaled_height();
    fast_print("\e[?25l");
    fast_print_args("\033[%d;0H", height + 3);
    for (int i = 0; i < commands.len; i++)
    {
        if (selectedCmd == i)
            fast_print(ANSI_BLUE);
        fast_print_args("\033[0K[%d] %s\n", i, commands.items[i].descriptionText);
        if (selectedCmd == i)
            fast_print(ANSI_NORMAL);
    }
    fast_print("\e[?25h");
}

void clean_up_console()
{
    const int tHeight = scaled_tHeight();
    const int tWidth = scaled_tWidth();
    const int vWidth = scaled_vWidth();
    const int height = scaled_height();
    fast_print("\e[s");
    fast_print("\e[?25l");
    fast_print("\033[H");
    for (int y = 0; y <= height; y++)
    {
        if (y >= TEXTBOX_OFFSET_Y - 1 && y <= TEXTBOX_OFFSET_Y + tHeight)
        {
            fast_print_args("\033[%dC", vWidth * 2 + 2);
            make_white_space_fast_print(TEXTBOX_OFFSET_X - 2);
            if (textBoxText.len <= 0 &&
                y >= TEXTBOX_OFFSET_Y && y < TEXTBOX_OFFSET_Y + tHeight)
            {
                fast_print_args("\033[%dC", 1);
                make_white_space_fast_print(tWidth + 1);

                fast_print_args("\033[%dC\033[0K", 1);
            }
            else
            {
                fast_print_args("\033[%dC\033[0K", tWidth + 2);
            }
        }
        else
        {
            fast_print_args("\033[%dC\033[0K", vWidth * 2 + 2);
        }
        if (y < height)
            fast_print("\033[1E");
    }

    fast_print("\033[2E");
    for (int i = 0; i < commands.len; i++)
    {
        String cmdText = str_from("[%d] ");
        str_append(&cmdText, commands.items[i].descriptionText);
        char fullCmdText[strlen(cmdText.chars) + 100];
        sprintf(fullCmdText, cmdText.chars, i);
        fast_print_args("\033[%dC\033[0K\n", strlen(fullCmdText));
    }
    fast_print("\033[0J");
    fast_print("\e[u");
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void draw_console()
{
    pthread_mutex_lock(&mutex);
    if (reSizeMonitor <= 0)
        pthread_create(&reSizeMonitor, NULL, (void*)&monitor_resize_event, NULL);

    //Get draw space size adjusted to fit the screen size.
    const int vHeight = scaled_vHeight();
    const int tHeight = scaled_tHeight();
    const int tWidth = scaled_tWidth();
    const int vWidth = scaled_vWidth();
    const int height = scaled_height();

    fast_print_args(" ");
    // fast_print_args(
    //     "%sIF YOU SEE THIS SOMETHING HAS GONE WRONG WITH THE CLEARING OF THE TUI! (OR IT'S JUST SLOW)",
    //     ANSI_NORMAL);
    fast_print("\e[s");
    fast_print("\033[H");
    fast_print("\e[?25l");
    const OutlineCorners gridCorners = {TL_CORNER, TR_CORNER, BL_CORNER, UP_T_JUNC};
    draw_outline(1, 1, vHeight, vWidth * 2, "MAP", gridCorners);
    const OutlineCorners textboxCorners = {TL_CORNER, TR_CORNER, BL_CORNER, BR_CORNER};
    draw_outline(TEXTBOX_OFFSET_Y, vWidth * 2 + TEXTBOX_OFFSET_X, tHeight, tWidth,
                 "MESSAGE BOX",
                 textboxCorners);
    draw_text(textBoxText.chars, TEXTBOX_OFFSET_Y + 1, vWidth * 2 + TEXTBOX_OFFSET_X + 3, tHeight,
              tWidth);
    String horiLine = str_from("");
    draw_horizontal_outline(&horiLine, height + 2, vWidth * 2 + 3,
                            (int)consoleSize.x - (vWidth * 2 + 3),
                            "");
    fast_print(horiLine.chars);
    str_free(&horiLine);
    draw_grid();
    draw_cmd();
    clean_up_console();
    fast_print("\033[0J");
    fast_print("\e[u");

    fast_print("\033[?30l");
    pthread_mutex_unlock(&mutex);
}

void draw_current_state(RoadSegSlice roads, FireSlice fires)
{
    current_roads = roads;
    current_fires = fires;
    draw_console();
}

void write_to_textbox(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char textBuffer[strlen(format) + 100];
    int amountWritten = vsprintf(textBuffer, format, args);
    const char text[amountWritten + 1];
    memcpy(text, textBuffer, sizeof(text));
    va_end(args);

    const int tHeight = scaled_tHeight();
    const int tWidth = scaled_tWidth();
    const int vWidth = scaled_vWidth();
    textBoxText = str_from(textBuffer);
    draw_text(textBoxText.chars, TEXTBOX_OFFSET_Y + 1, vWidth * 2 + TEXTBOX_OFFSET_X + 3, tHeight,
              tWidth);
}

void prepend_console_command(void (*action), char* description)
{
    const ConsoleCommands cmd = {action, description, commands.len};
    vec_unshift(&commands, cmd);
}

///Start detecting user inputs (keyboard or mouse) from the user.
void execute_command()
{
    // const LCoord size = get_terminal_size();
    // if (fabs(size.x - consoleSize.x) > 0.01 || fabs(size.x - consoleSize.x) > 0.01)
    // {
    //     draw_console();
    //     consoleSize = size;
    // }
    fast_print("\e[s");
    fast_print("\e[?25l");

    fast_print(ENABLE_MOUSE_INPUT_ANSI);
    const int c = getchar();
    fast_print(DISABLE_MOUSE_INPUT_ANSI);
    if (c == 27)
    {
        const int bracket = getchar();
        //used to clear the "[" from the buffer (there are better ways to do this but can't be arsed)
        const int nCode = getchar();
        if (nCode == 65)
        {
            if (selectedCmd == 0)
                selectedCmd = commands.len - 1;
            else
                selectedCmd -= 1;
        }
        //Up
        else if (nCode == 66)
        {
            selectedCmd += 1;
            if (selectedCmd >= commands.len)
                selectedCmd = 0;
        }
        else if (nCode == '<')
        {
            //https://stackoverflow.com/questions/5966903/how-can-i-get-mousemove-and-mouseclick-in-bash/55437976#55437976
            int mouseX = -1;
            int mouseY = -1;

            String readCmd = str_from("");
            for (int i = 0; i < 100; i++)
            {
                char checkChar = (char)getchar();
                str_push(&readCmd, checkChar);
                if (checkChar == 'm' || checkChar == 'M')
                    break;
            };
            sscanf(readCmd.chars, "0;%d;%d", &mouseX, &mouseY);
            // printf(readCmd.chars);
            // printf("(x: %d, y: %d)", mouseX, mouseY);
            int potIndex = mouseY - (scaled_height() + 3);
            if (potIndex >= 0 && potIndex < commands.len)
            {
                char indexStr[10];
                sprintf(indexStr, "[%d]", potIndex);
                if (strlen(commands.items[potIndex].descriptionText) + strlen(indexStr) + 1 >=
                    mouseX)
                {
                    if (readCmd.chars[readCmd.len - 1] == 'M')
                    {
                        selectedCmd = potIndex;
                        draw_console();
                        // draw_cmd();
                    }
                    else if (readCmd.chars[readCmd.len - 1] == 'm')
                    {
                        selectedCmd = potIndex;
                        debug_log(MESSAGE, "EXECUTING CMD WITH INDEX: %d", selectedCmd);
                        cmdIsRunning = true;
                        commands.items[selectedCmd].triggerAction();
                        cmdIsRunning = false;
                    }
                }
            }
            else if (mouseX > 1 && mouseX < vWidth * 2 + 2 && mouseY > 1 && mouseY < vHeight + 2)
            {
                mouseY = MIN(mouseY, vHeight);
                mouseX = MIN(mouseX+2, vWidth*2) / 2;
                //fast_print_args("that's inside the grid! (x: %d, y: %d)", mouseX, mouseY);
            }
            str_free(&readCmd);
        }
        draw_console();
        // draw_cmd();
    }
    else if (c == 32)
    {
        fast_print(DISABLE_MOUSE_INPUT_ANSI);
        debug_log(MESSAGE, "EXECUTING CMD WITH INDEX: %d", selectedCmd);
        cmdIsRunning = true;
        commands.items[selectedCmd].triggerAction();
        cmdIsRunning = false;
    }
    //THIS ALLOW NUM INPUTS TO SELECT COMMANDS, CAN CURRENTLY CAUSE THE APP
    //TO SELECT THE WRONG COMMAND IF THE INPUT BUFFER  CONTAINS TO MANY CHARS
    // else if (c >= 48 && c <= 57)
    // {
    //     int index = (c - 48);
    //     if (index > 0 && index < commands.len)
    //     {
    //         debug_log(MESSAGE, "EXECUTING CMD WITH INDEX: %d", index);
    //         if (commands.items[index].index != index) debug_log(ERROR, "COMMAND INDEX MISMATCH!");
    //         commands.items[index].triggerAction();
    //     }
    // }

    fast_print("\e[l");
}

void clear()
{
    fflush(stdout);
    fast_print("\033c");
    fflush(stdout);
}