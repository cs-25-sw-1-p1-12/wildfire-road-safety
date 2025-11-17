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


#define TEXTBOX_WIDTH 30;
#define TEXTBOX_HEIGHT 10;


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
            printf("| ");
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
                        gridMapSlice[x] = ' ';
                }
                printf_color(gridMapSlice, COLOR_WHITE);
            }
            else
            {
                for (int x = 0; x < VIEWPORT_WIDTH; ++x)
                {
                    gridMapSlice[x * 2 + 1] = ' ';
                    gridMapSlice[x * 2] = 'X';
                }
                printf_color(gridMapSlice, COLOR_GREEN);
            }


            free(gridMapSlice);
            printf("|  ");
        }
        else
        {
            printf("     ");
            for (int x = 0; x < VIEWPORT_WIDTH; ++x)
                printf("  ");
        }

        if (y < tHeight)
        {
            char* textBoxSlice = calloc(tWidth, sizeof(char));
            for (int x = 0; x < tWidth; x++)
            {
                if (x == 0 || x >= tWidth - 1)
                    textBoxSlice[x] = '|';
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

    for (int i = 0; i < tWidth + (vWidth + 1) * 2 + 3 + vWidth*2; i++)
        printf_color("-", COLOR_WHITE);
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
    ConsoleCommands cmd = {action,description};
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
