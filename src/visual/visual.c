#include "visual.h"
#include "../dyn.h"

#include <math.h>
#include <pthread.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#endif

//https://stackoverflow.com/questions/6486289/how-to-clear-the-console-in-c
//https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
//https://stackoverflow.com/questions/917783/how-do-i-work-with-dynamic-multi-dimensional-arrays-in-c

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

//dyn.h lacks a function to only append char instead of a whole string/char*
//So these are here for that purpose.
const char tlCorner[2] = {(char)201, '\0'}; //╔
const char verticalLine[2] = {(char)186, '\0'}; //║
const char trCorner[2] = {(char)187, '\0'}; //╗
const char upT[2] = {(char)202, '\0'};
const char horizontalLine[2] = {(char)205, '\0'};
const char* linebreak = "\n"; //line break

//These const exist due to some issue with Clion not interpreting them as actual numbers.
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


#ifdef _WIN32
DWORD defaultConsoleSettingsInput;
DWORD defaultConsoleSettingsOutput;
#endif

const FILE* runtimeLog;

void init_console()
{
    runtimeLog = fopen("runtime.log", "w");
    fprintf(runtimeLog, "RUNTIME LOG START!\n");

    textBoxText = str_from("");
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hInput, &defaultConsoleSettingsInput);
    SetConsoleMode(hInput, ENABLE_VIRTUAL_TERMINAL_INPUT);

    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hOutput, &defaultConsoleSettingsOutput);
    SetConsoleMode(hOutput, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    SetConsoleOutputCP(65001);

    HWND windowHandle = GetConsoleWindow();
    ShowWindow(windowHandle, 3);
#endif
    printf("\e[?25h");
}

void close_console()
{
    fclose((FILE*)runtimeLog);
    str_free(&textBoxText);
    vec_free(commands);
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hInput, defaultConsoleSettingsInput);
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hOutput, defaultConsoleSettingsOutput);
#endif
}

BoundBox globalBounds = (BoundBox){
    .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
    .c2 = {.lat = 57.01467041792688, .lon = 9.99681826817088}
};

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

void printf_color(char* text, char* color)
{
    if ((int)text[0] != 27)
    {
        printf("\033[38;5;212m%s%s", text, ANSI_NORMAL);
        return;
    }
    printf("%s%s%s", color, text, ANSI_NORMAL);
}

double get_point_dist_to_road(LCoord n1, LCoord n2, LCoord p, double tolerance)
{
    // Check if in bounding box
    if (p.x > fmax(n1.x, n2.x) + tolerance || p.x < fmin(n1.x, n2.x) - tolerance ||
        p.y > fmax(n1.y, n2.y) + tolerance || p.y < fmin(n1.y, n2.y) - tolerance)
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

        if (current_roads.items[i].name != NULL)
            fprintf(runtimeLog, "NAME: %s\n", current_roads.items[i].name);
        if (current_roads.items[i].material != NULL)
            fprintf(runtimeLog, "MATE: %s\n", current_roads.items[i].material);

        for (int nodeI = 1; nodeI < nodes.len; nodeI++)
        {
            LCoord coord1 = global_to_local(nodes.items[nodeI - 1].coords, globalBounds, vHeight,
                                            vWidth);
            LCoord coord2 = global_to_local(nodes.items[nodeI].coords, globalBounds, vHeight,
                                            vWidth);

            Vec2 v1_1 = {
                .x = (double)coord.x - (double)coord1.x,
                .y = (double)coord.y - (double)coord1.y
            };
            Vec2 v1_2 = {
                .x = (double)coord2.x - (double)coord1.x,
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
            continue;
            Vec2 v2_1 = {
                (double)coord.x - (double)coord2.x,
                (double)coord.y - (double)coord2.y
            };
            Vec2 v2_2 = {
                (double)coord1.x - (double)coord2.x,
                (double)coord1.y - (double)coord2.y
            };
            double v2_1len = sqrtl(v2_1.x * v2_1.x + v2_1.y * v2_1.y);
            double v2_2len = sqrtl(v2_2.x * v2_2.x + v2_2.y * v2_2.y);
            double cosVal = (v2_1.x * v2_2.y + v2_1.y * v2_2.x) / (v2_1len * v2_2len);
            double angle2 = acos(cosVal);

            if (angle1 <= M_PI / 2 && angle2 <= M_PI / 2)
            {
                double cross = v1_2.x * v1_1.y - v1_2.y * v1_1.x;
                double dst = fabs(cross / sqrt(pow(v1_2.x, 2) * pow(v1_2.y, 2)));
                if (dst <= 0.5f)
                    return true;
            }
        }
    }
    return false;
}


void draw_grid()
{
    printf("\e[?25l");
    //printf("\e[s");
    String gridContent = str_from("");
    printf("\033[2;2H");


    char* ANSI_CODE = ANSI_NORMAL;
    for (int y = 0; y < vHeight; y++)
    {
        for (int x = 0; x < vWidth; x++)
        {
            LCoord lCoord;
            lCoord.x = x;
            lCoord.y = y;
            BOOL isRoad =  road_has_road_at(current_roads, lCoord, 0.5);;//local_pos_is_on_road(lCoord);
            if (isRoad == false)
            {
                str_append(&gridContent, ANSI_GREEN);
                ANSI_CODE = ANSI_GREEN;
            }
            else
            {
                str_append(&gridContent, ANSI_BLUE);
                ANSI_CODE = ANSI_BLUE;
            }
            char blocks[3] = {(char)219, (char)219, '\0'};
            str_append(&gridContent, blocks);
        }
        str_append(&gridContent, "\033[1E");
        str_append(&gridContent, "\033[1C");
    }
    str_append(&gridContent, ANSI_NORMAL);
    printf(gridContent.chars);
    str_free(&gridContent);
    // printf("\e[u");
    printf("\e[?25h");
    fflush(runtimeLog);
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
    for (int i = 0; i < min(stringText.len, height * width); i++)
    {
        char c[2] = {stringText.chars[i], '\0'};
        str_append(&textBox, c);
        int currentLineWidth = i % width;
        if (currentLineWidth == width - 1)
        {
            str_append(&textBox, newline);
        }
    }
    printf(textBox.chars);
    str_free(&textBox);
    str_free(&stringText);
}

void draw_horizontal_outline(String* string, int line, int column, int width, char* title)
{
    String headerText = str_from(title);
    char gridMapSlice[width + 1];
    int length = (width - 2) / 2 - ((headerText.len) / 2);

    char text[1000];
    sprintf(text, "\033[%d;%dH", line, column);
    str_append(string, text);
    for (int x = 0; x < width; ++x)
    {
        if (x >= length && x < length + headerText.len)
            gridMapSlice[x] = headerText.chars[x - length];
        else
            gridMapSlice[x] = (char)205;
    }
    gridMapSlice[width] = '\0';
    str_append(string, gridMapSlice);
}

void draw_outline(int line, int column, int height, int width, char* title, OutlineCorners corners)
{
    const char tlCorner[2] = {corners.topLeft, '\0'};
    const char trCorner[2] = {corners.topRight, '\0'};
    const char blCorner[2] = {corners.bottomLeft, '\0'};
    const char brCorner[2] = {corners.bottomRight, '\0'};
    char offsetPos[1000];
    sprintf(offsetPos, "\033[%d;%dH", line, column);

    String gridOutline = str_from(offsetPos);

    str_append(&gridOutline, tlCorner);
    draw_horizontal_outline(&gridOutline, line, column + 1, width, title);
    str_append(&gridOutline, trCorner);

    char text[1000];
    sprintf(text, "\033[%dC", width);

    char newline[1000] = "\033[1E";
    if (column > 1)
        sprintf(newline, "\033[1E\033[%dC", column - 1);
    for (int x = 0; x < height; x++)
    {
        str_append(&gridOutline, newline);
        str_append(&gridOutline, verticalLine);
        str_append(&gridOutline, text);
        str_append(&gridOutline, verticalLine);

        //DEBUG LINE NUMBER DISPLAY
        // char num[1000];
        // sprintf(num, " - %d", x + 1);
        // str_append(&gridOutline, num);
    }

    str_append(&gridOutline, newline);
    str_append(&gridOutline, blCorner);
    draw_horizontal_outline(&gridOutline, line + height + 1, column + 1, width, "");
    str_append(&gridOutline, brCorner);
    printf(gridOutline.chars);
    str_free(&gridOutline);
}

void draw_cmd()
{
    printf("\e[?25l");
    printf("\033[%d;0H", height + 3);
    for (int i = 0; i < commands.len; i++)
    {
        if (selectedCmd == i)
            printf(ANSI_BLUE);
        printf("[%d] %s\n", i, commands.items[i].descriptionText);
        if (selectedCmd == i)
            printf(ANSI_NORMAL);
    }
    printf("\e[?25h");
}

void draw_console()
{
    printf("\e[?25l");
    printf(
        "%sIF YOU SEE THIS SOMETHING HAS GONE WRONG WITH THE CLEARING OF THE TUI! (OR IT'S JUST SLOW)",
        ANSI_NORMAL);
    clear();
    int textRead = 0;

    const OutlineCorners gridCorners = {(char)201, (char)187, (char)202, (char)202};
    draw_outline(1, 1, vHeight, vWidth * 2, "MAP", gridCorners);
    draw_grid();

    const OutlineCorners textboxCorners = {(char)201, (char)187, (char)200, (char)188};
    draw_outline(TEXTBOX_OFFSET_Y, vWidth * 2 + TEXTBOX_OFFSET_X + 3, tHeight, tWidth,
                 "MESSAGE BOX",
                 textboxCorners);
    draw_text(textBoxText.chars, TEXTBOX_OFFSET_Y + 1, vWidth * 2 + TEXTBOX_OFFSET_X + 3, tHeight,
              tWidth);

    String horiLine = str_from("");
    draw_horizontal_outline(&horiLine, height + 2, vWidth * 2 + 3,
                            TEXTBOX_OFFSET_X * 2 + tWidth + 3, "");
    printf(horiLine.chars);
    str_free(&horiLine);
    // for (int i = 0; i < tWidth + (vWidth + 1) * 2 + 2 + TEXTBOX_OFFSET_X * 2; i++)
    // {
    //     if (i == 0 || i == (vWidth) * 2 + 1)
    //         printf("%c", 202);
    //     else
    //         printf("%c", 205);
    // }
    // printf("\n");
    draw_cmd();
    printf("\e[?25h");
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

void append_console_command(void (*action), char* description)
{
    const ConsoleCommands cmd = {action, description};
    vec_push(&commands, cmd);
}

void execute_command()
{
    printf("\e[?25l");
    const int c = getchar();
    if (c == 27)
    {
        getchar();
        const int nCode = getchar();
        printf("\33[2K\r");

        if (nCode == 65)
        {
            if (selectedCmd == 0)
                selectedCmd = commands.len - 1;
            else
                selectedCmd -= 1;
        }
        //Up
        if (nCode == 66)
        {
            selectedCmd += 1;
            if (selectedCmd >= commands.len)
                selectedCmd = 0;
        }

        for (int i = 0; i < commands.len; i++)
            printf("\033[A\33[2K\r");
        draw_cmd();
    }
    else if (c == 32)
    {
        commands.items[selectedCmd].triggerAction();
    }
    printf("\e[?25h");
}

void clear()
{
    printf("\033c");
}