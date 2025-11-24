#include "visual.h"

#include "../Debug/Logger.h"
#include "../dyn.h"

#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else // Linux & MacOS
#include <termios.h>
#include <unistd.h>
#endif

// https://stackoverflow.com/questions/6486289/how-to-clear-the-console-in-c
// https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
// https://stackoverflow.com/questions/917783/how-do-i-work-with-dynamic-multi-dimensional-arrays-in-c
// https://www.cs.uleth.ca/~holzmann/C/system/ttyraw.c

#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
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

/// ╩ default(win32: 202)
#define UP_T_JUNC "╩"
#define LINEBREAK "\n"

// These const exist due to some issue with Clion not interpreting them as actual numbers.
const int vHeight = VIEWPORT_HEIGHT;
const int tHeight = TEXTBOX_HEIGHT;
const int vWidth = VIEWPORT_WIDTH;
const int tWidth = TEXTBOX_WIDTH;
const int height = (tHeight > vHeight) ? tHeight : vHeight;

unsigned int selectedCmd = 0;
RoadSegSlice current_roads;
FireSlice current_fires;

typedef struct
{
    void (*triggerAction)();
    char* descriptionText;
} ConsoleCommands;

String textBoxText;
VecDef(ConsoleCommands) commands;

void clear();
void draw_grid();
void draw_outline(int line, int column, int height, int width, char* title, OutlineCorners corners);
LCoord get_terminal_size();


#ifdef _WIN32
DWORD defaultConsoleSettingsInput;
DWORD defaultConsoleSettingsOutput;
UINT defaultConsoleOutputType;
#else // Linux & MacOS
struct termios orig_termios;
#endif
LCoord fontSize;

void init_console()
{
    // setbuf(stdout, NULL);
    debug_log(MESSAGE, "start initiating console...");
    textBoxText = str_from("");
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hInput, &defaultConsoleSettingsInput);
    SetConsoleMode(hInput, ENABLE_VIRTUAL_TERMINAL_INPUT);

    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hOutput, &defaultConsoleSettingsOutput);
    SetConsoleMode(hOutput, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    defaultConsoleOutputType = GetConsoleOutputCP();
    SetConsoleOutputCP(65001);

    HWND windowHandle = GetConsoleWindow();
    ShowWindow(windowHandle, SW_SHOWMAXIMIZED);
#else // Linux & MacOS
    tcgetattr(STDIN_FILENO, &orig_termios);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

#endif
    printf("\e[?25h");
    debug_log(MESSAGE, "Done!");
    printf("\033[?1049h");
}

void close_console()
{
    printf("\033[?1049l");
    debug_log(MESSAGE, "start closing console...");
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


void fast_print(const char* format)
{
    fwrite(format, strlen(format), 1, stdout);
}

void fast_print_args(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char textBuffer[strlen(format) + 100];
    int amountWritten = vsprintf(textBuffer, format, args);
    char text[amountWritten + 1];
    mempcpy(text, textBuffer, sizeof(text));
    va_end(args);
    fast_print(text);
}

BoundBox globalBounds = (BoundBox){
    .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
    .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
};

LCoord get_terminal_size()
{
    // https://stackoverflow.com/questions/74431114/get-terminal-size-using-ansi-escape-sequences/74432616#74432616
    fast_print("\033[s\033[9999;9999H");
    fast_print("\033[6n");
    const int c = getchar();
    if (c == 27)
    {
        char response[100];
        int line;
        int column;
        scanf("[%d;%d", &line, &column);
        return (LCoord){.x = column, .y = line};
    }
    fast_print("\033[u");
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

            Node node1 = nodes.items[j];
            LCoord node1LCoord = global_to_local(node1.coords, globalBounds, vHeight, vWidth);
            Node node2 = nodes.items[j + 1];
            LCoord node2LCoord = global_to_local(node2.coords, globalBounds, vHeight, vWidth);


            double dist = get_point_dist_to_road(node1LCoord, node2LCoord, point, tolerance);

            if (dist >= -tolerance && dist <= tolerance)
                return true;
        }
    }

    return false;
}

bool local_pos_is_on_road(LCoord coord)
{
    for (int i = 0; i < current_roads.len; i++)
    {
        NodeSlice nodes = current_roads.items[i].nodes;

        for (int nodeI = 1; nodeI < nodes.len; nodeI++)
        {
            LCoord coord1 =
                global_to_local(nodes.items[nodeI - 1].coords, globalBounds, vHeight, vWidth);
            LCoord coord2 =
                global_to_local(nodes.items[nodeI].coords, globalBounds, vHeight, vWidth);

            Vec2 v1_1 = {.x = (double)coord.x - (double)coord1.x,
                         .y = (double)coord.y - (double)coord1.y};
            Vec2 v1_2 = {.x = (double)coord2.x - (double)coord1.x,
                         .y = (double)coord2.y - (double)coord1.y};
            double v1_1len = sqrt(v1_1.x * v1_1.x + v1_1.y * v1_1.y);
            double v1_2len = sqrt(v1_2.x * v1_2.x + v1_2.y * v1_2.y);
            double angle1 = acos((v1_1.x * v1_2.x + v1_1.y * v1_2.y) / (v1_1len * v1_2len));

            const double stepPerLen = 0.5f;
            const double totalSteps = v1_2len / stepPerLen;
            double stepSize = v1_2len / totalSteps;
            for (int step = 0; step <= totalSteps; step++)
            {
                double x = (v1_2.x / v1_2len) * stepSize * step + coord1.x;
                double y = (v1_2.y / v1_2len) * stepSize * step + coord1.y;

                double dst = sqrt(pow(x - coord.x, 2) + pow(y - coord.y, 2));
                if (dst <= 0.5f)
                    return true;
            }
        }
    }
    return false;
}


void draw_grid()
{
    fast_print("\e[?25l");
    // printf("\e[s");
    String gridContent = str_from("");
    fast_print("\033[2;2H");

    int greenCount = 0;
    int blueCount = 0;
    const char* ANSI_CODE = ANSI_NORMAL;
    for (int y = 0; y < vHeight; y++)
    {
        for (int x = 0; x < vWidth; x++)
        {
            const LCoord lCoord = (LCoord){.x = x, .y = y};
            const bool isRoad = road_has_road_at(current_roads, lCoord, 0.5);
            if (isRoad == false && strcmp(ANSI_CODE, ANSI_GREEN) != 0)
            {
                greenCount++;
                str_append(&gridContent, ANSI_GREEN);
                ANSI_CODE = ANSI_GREEN;
            }
            if (isRoad == true && strcmp(ANSI_CODE, ANSI_BLUE) != 0)
            {
                blueCount++;
                str_append(&gridContent, ANSI_BLUE);
                ANSI_CODE = ANSI_BLUE;
            }
            str_append(&gridContent, GRID_BLOCK);
            str_append(&gridContent, GRID_BLOCK);
        }
        str_append(&gridContent, "\033[1E");
        str_append(&gridContent, "\033[1C");
    }
    str_append(&gridContent, ANSI_NORMAL);
    fast_print(gridContent.chars);
    debug_log(MESSAGE, "green: %d, blue: %d, total grid size: %d", greenCount, blueCount,
              vWidth * 2 * vHeight);
    str_free(&gridContent);
    // printf("\e[u");
    fast_print("\e[?25h");
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

        // DEBUG LINE NUMBER DISPLAY
        //  char num[1000];
        //  sprintf(num, " - %d", x + 1);
        //  str_append(&gridOutline, num);
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

void draw_console()
{
    fast_print_args(" ");
    // fast_print_args(
    //     "%sIF YOU SEE THIS SOMETHING HAS GONE WRONG WITH THE CLEARING OF THE TUI! (OR IT'S JUST
    //     SLOW)", ANSI_NORMAL);
    clear();
    fast_print("\e[?25l");
    const OutlineCorners gridCorners = {TL_CORNER, TR_CORNER, BL_CORNER, UP_T_JUNC};
    draw_outline(1, 1, vHeight, vWidth * 2, "MAP", gridCorners);
    const OutlineCorners textboxCorners = {TL_CORNER, TR_CORNER, BL_CORNER, BR_CORNER};
    draw_outline(TEXTBOX_OFFSET_Y, vWidth * 2 + TEXTBOX_OFFSET_X, tHeight, tWidth, "MESSAGE BOX",
                 textboxCorners);
    draw_text(textBoxText.chars, TEXTBOX_OFFSET_Y + 1, vWidth * 2 + TEXTBOX_OFFSET_X + 3, tHeight,
              tWidth);
    String horiLine = str_from("");
    LCoord size = get_terminal_size();
    draw_horizontal_outline(&horiLine, height + 2, vWidth * 2 + 3, size.x - (vWidth * 2 + 3), "");
    fast_print(horiLine.chars);
    str_free(&horiLine);
    draw_grid();
    draw_cmd();
    fast_print("\e[?25h");
}

void draw_current_state(RoadSegSlice roads, FireSlice fires)
{
    current_roads = roads;
    current_fires = fires;
    draw_console();
}

void write_to_textbox(char* text)
{
    textBoxText = str_from(text);
    draw_text(textBoxText.chars, TEXTBOX_OFFSET_Y + 1, vWidth * 2 + TEXTBOX_OFFSET_X + 3, tHeight,
              tWidth);
}

void append_console_command(void(*action), char* description)
{
    const ConsoleCommands cmd = {action, description};
    vec_push(&commands, cmd);
}

void execute_command()
{
    fast_print("\e[?25l");

    fast_print("\e[?1000;1006;1015h");
    const int c = getchar();
    fast_print("\e[?1000;1006;1015l");
    if (c == 27)
    {
        // printf("\e[?1000;1006;1015l");
        getchar();
        const int nCode = getchar();
        fast_print("\33[2K\r");

        if (nCode == 65)
        {
            if (selectedCmd == 0)
                selectedCmd = commands.len - 1;
            else
                selectedCmd -= 1;
        }
        // Up
        else if (nCode == 66)
        {
            selectedCmd += 1;
            if (selectedCmd >= commands.len)
                selectedCmd = 0;
        }
        else if (nCode == '<')
        {
            // https://stackoverflow.com/questions/5966903/how-can-i-get-mousemove-and-mouseclick-in-bash/55437976#55437976
            int mouseX = -1;
            int mouseY = -1;

            String readCmd = str_from("");
            for (int i = 0; i < 100; i++)
            {
                char c = getchar();
                str_push(&readCmd, c);
                if (c == 'm' || c == 'M')
                    break;
            };
            sscanf(readCmd.chars, "0;%d;%d", &mouseX, &mouseY);
            // printf(readCmd.chars);
            // printf("(x: %d, y: %d)", mouseX, mouseY);
            int potIndex = mouseY - (height + 3);
            if (potIndex >= 0 && potIndex < commands.len)
            {
                char indexStr[10];
                sprintf(indexStr, "[%d]", potIndex);
                if (strlen(commands.items[potIndex].descriptionText) + strlen(indexStr) + 1 >=
                    mouseX)
                    if (readCmd.chars[readCmd.len - 1] == 'M')
                    {
                        selectedCmd = potIndex;
                        draw_cmd();
                    }
                    else if (readCmd.chars[readCmd.len - 1] == 'm')
                    {
                        selectedCmd = potIndex;
                        commands.items[selectedCmd].triggerAction();
                    }
            }
            else if (mouseX > 1 && mouseX < vWidth * 2 + 2 && mouseY > 1 && mouseY < vHeight + 2)
            {
                mouseY = MIN(mouseY, vHeight);
                mouseX = MIN(mouseX + 2, vWidth * 2) / 2;
                // fast_print_args("that's inside the grid! (x: %d, y: %d)", mouseX, mouseY);
            }
            str_free(&readCmd);
        }

        for (int i = 0; i < commands.len; i++)
            fast_print("\033[A\33[2K\r");
        draw_cmd();
    }
    else if (c == 32)
    {
        fast_print("\e[?1000;1006;1015l");
        commands.items[selectedCmd].triggerAction();
    }
    fast_print("\e[?25h");
}

void clear()
{
    fast_print("\033c");
}
