#include "visual.h"
#include "../dyn.h"

#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#endif

//https://stackoverflow.com/questions/6486289/how-to-clear-the-console-in-c
//https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c

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

typedef struct
{
    void (*triggerAction)();
    char* descriptionText;
} ConsoleCommands;

StrSlice textBoxText;
VecDef(ConsoleCommands) commands;

void clear();
void printf_color(char* string, int colorId);

void init_console()
{
#ifdef _WIN32
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hInput, ENABLE_VIRTUAL_TERMINAL_INPUT);
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hOutput, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void make_white_space(String* string, int amount)
{
    char whiteSpace[amount];
    for (int x = 0; x < sizeof(whiteSpace); ++x)
        whiteSpace[x] = x < (sizeof(whiteSpace) - 1) ? ' ' : '\0';
    str_append(string, whiteSpace);
}

const char tlCorner[2] = {(char)201, '\0'}; //╔
const char verticalLine[2] = {(char)186, '\0'}; //║
const char trCorner[2] = {(char)187, '\0'}; //╗
const char* linebreak = "\n"; //line break

const int vHeight = VIEWPORT_HEIGHT;
const int tHeight = TEXTBOX_HEIGHT;
const int vWidth = VIEWPORT_WIDTH;
const int tWidth = TEXTBOX_WIDTH;
const int height = (tHeight > vHeight) ? tHeight : vHeight;

unsigned int selectedCmd = 0;

void draw_cmd()
{
    printf("\e[?25l");
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

    String TUIBody = str_from(ANSI_NORMAL);
    for (int y = 0; y < height; y++)
    {
        if (y < vHeight + 1)
        {
            str_append(&TUIBody, ANSI_NORMAL);
            if (y == 0)
                str_append(&TUIBody, tlCorner);
            else
                str_append(&TUIBody, verticalLine);

            char* gridMapSlice = calloc(VIEWPORT_WIDTH * 2 + 1, sizeof(char));
            if (y == 0)
            {
                str_append(&TUIBody, ANSI_NORMAL);
                char headerText[] = "MAP";
                int length = (VIEWPORT_WIDTH * 2 - 2) / 2 - ((sizeof(headerText) - 1) / 2);
                for (int x = 0; x < VIEWPORT_WIDTH * 2; ++x)
                {
                    if (x >= length && x < length + sizeof(headerText) - 1)
                        gridMapSlice[x] = headerText[x - length];
                    else
                        gridMapSlice[x] = (char)205;
                }
                // printf_color(gridMapSlice, COLOR_WHITE);
            }
            else
            {
                str_append(&TUIBody, ANSI_GREEN);
                for (int x = 0; x < VIEWPORT_WIDTH; ++x)
                {
                    gridMapSlice[x * 2 + 1] = (char)219;
                    gridMapSlice[x * 2] = (char)219;
                }
                // printf_color(gridMapSlice, COLOR_GREEN);
            }
            str_append(&TUIBody, gridMapSlice);

            free(gridMapSlice);
            str_append(&TUIBody, ANSI_NORMAL);
            if (y == 0)
                str_append(&TUIBody, trCorner);
            else
                str_append(&TUIBody, verticalLine);
        }
        else
        {
            make_white_space(&TUIBody, VIEWPORT_WIDTH + 5 + 1);
        }

        make_white_space(&TUIBody, TEXTBOX_OFFSET_X);
        if (y - TEXTBOX_OFFSET_Y < tHeight && y >= TEXTBOX_OFFSET_Y)
        {
            char* textBoxSlice = calloc(tWidth, sizeof(char));
            for (int x = 0; x < tWidth; x++)
            {
                if (y - TEXTBOX_OFFSET_Y == 0)
                {
                    char headerText[] = "MESSAGE BOX";
                    int length = tWidth / 2 - ((sizeof(headerText) - 1) / 2);
                    if (x == 0 || x >= tWidth - 1)
                        textBoxSlice[x] = x < tWidth - 1 ? (char)201 : (char)187;
                    else if (x >= length && x < length + sizeof(headerText) - 1)
                        textBoxSlice[x] = headerText[x - length];
                    else
                        textBoxSlice[x] = (char)205;
                }
                else if (y - TEXTBOX_OFFSET_Y == tHeight - 1)
                {
                    if (x == 0 || x >= tWidth - 1)
                        textBoxSlice[x] = x < tWidth - 1 ? (char)200 : (char)188;
                    else
                        textBoxSlice[x] = (char)205;
                }
                else if (x == 0 || x >= tWidth - 1)
                    textBoxSlice[x] = (char)186;
                else
                {
                    //ACTUALLY WRITE
                    if (textRead < textBoxText.len)
                    {
                        textBoxSlice[x] = textBoxText.chars[textRead];
                        textRead++;
                    }
                    else
                        textBoxSlice[x] = ' ';
                }
            }
            //printf_color(textBoxSlice, COLOR_WHITE);
            str_append(&TUIBody, textBoxSlice);
            free(textBoxSlice);
        }

        str_append(&TUIBody, linebreak);
    }
    printf(TUIBody.chars);
    printf(ANSI_NORMAL);

    for (int i = 0; i < tWidth + (vWidth + 1) * 2 + 2 + TEXTBOX_OFFSET_X * 2; i++)
    {
        if (i == 0 || i == (vWidth) * 2 + 1)
            printf("%c", 202);
        else
            printf("%c", 205);
    }
    printf("\n");
    draw_cmd();
    str_free(&TUIBody);
    printf("\e[?25h");
}

void write_to_textbox(char* text)
{
    str_slice_free(&textBoxText);
    textBoxText = str_slice_from(text);
    draw_console();
}

void append_console_command(void (*action), char* description)
{
    const ConsoleCommands cmd = {action, description};
    vec_append(&commands, &cmd, 1);
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