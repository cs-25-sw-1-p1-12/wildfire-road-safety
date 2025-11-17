#include "visual.h"
#include "../dyn.h"
#ifdef __linux__
#include "unistd.h"
#elif _WIN32
#include <windows.h>
#endif

//https://stackoverflow.com/questions/6486289/how-to-clear-the-console-in-c
//https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c

#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_RED 4
#define COLOR_PURPLE 5
#define COLOR_BROWN 6
#define COLOR_WHITE 7
#define COLOR_GRAY 8
#define COLOR_ORANGE 12


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
    void* triggerAction;
    char* descriptionText;
} ConsoleCommands;

StrSlice textBoxText;
VecDef(ConsoleCommands) commands;

void clear();
void printf_color(char* string, int colorId);

void draw_console()
{
    clear();
    // for (int i = 0; i < 255; i++)
    // {
    //     printf("%d:%c\n", i, i);
    // }
    int textRead = 0;
    const int vHeight = VIEWPORT_HEIGHT;
    const int tHeight = TEXTBOX_HEIGHT;
    const int vWidth = VIEWPORT_WIDTH;
    const int tWidth = TEXTBOX_WIDTH;
    const int height = (tHeight > vHeight) ? tHeight : vHeight;
    for (int y = 0; y < height; y++)
    {
        if (y < vHeight + 1)
        {
            if (y == 0)
                printf("%c", 201);
            else
                printf("%c", 186);
            char* gridMapSlice = calloc(VIEWPORT_WIDTH * 2 + 1, sizeof(char));
            if (y == 0)
            {
                char headerText[] = "MAP";
                int length = (VIEWPORT_WIDTH * 2 - 2) / 2 - ((sizeof(headerText) - 1) / 2);
                for (int x = 0; x < VIEWPORT_WIDTH * 2; ++x)
                {
                    if (x >= length && x < length + sizeof(headerText) - 1)
                        gridMapSlice[x] = headerText[x - length];
                    else
                        gridMapSlice[x] = (char)205;
                }
                printf_color(gridMapSlice, COLOR_WHITE);
            }
            else
            {
                for (int x = 0; x < VIEWPORT_WIDTH; ++x)
                {
                    gridMapSlice[x * 2 + 1] = (char)219;
                    gridMapSlice[x * 2] = (char)219;
                }
                printf_color(gridMapSlice, COLOR_GREEN);
            }


            free(gridMapSlice);
            if (y == 0)
                printf("%c", 187);
            else
                printf("%c", 186);
        }
        else
        {
            printf("     ");
            for (int x = 0; x < VIEWPORT_WIDTH; ++x)
                printf("  ");
        }

        for (int i = 0; i < TEXTBOX_OFFSET_X; i++)
        {
            printf(" ");
        }
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
                    else
                        if (x >= length && x < length + sizeof(headerText) - 1)
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
            printf_color(textBoxSlice, COLOR_WHITE);
            free(textBoxSlice);
        }

        printf("\n");
    }

    for (int i = 0; i < tWidth + (vWidth + 1) * 2 + 2 + TEXTBOX_OFFSET_X*2; i++)
    {
        if (i == 0 || i == (vWidth) * 2 + 1)
            printf("%c", 202);
        else
            printf("%c", 205);
    }
    printf("\n");
    for (int i = 0; i < commands.len; i++)
    {
        printf("[%d] %s\n", i, commands.items[i].descriptionText);
    }
}

void write_to_textbox(char* text)
{
    str_slice_free(&textBoxText);
    textBoxText = str_slice_from(text);
    draw_console();
}

void append_console_command(void* action, char* description)
{
    const ConsoleCommands cmd = {action, description};
    vec_append(&commands, &cmd, 1);
}

#ifdef __linux__
void clear()
{
    // CSI[2J clears screen, CSI[H moves the cursor to top-left corner
    std::cout << "\x1B[2J\x1B[H";
}
#elif _WIN32
void clear()
{
    COORD topLeft = {0, 0};
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
        );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
        );
    SetConsoleCursorPosition(console, topLeft);
}

void printf_color(char* string, int colorId)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (colorId)
    {
        default:
            SetConsoleTextAttribute(console, colorId);
            printf(string);
            break;
    }
    SetConsoleTextAttribute(console, COLOR_WHITE);
}
#endif