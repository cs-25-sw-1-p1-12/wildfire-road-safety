#include "dyn.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StrSlice str_slice_from(char* chs)
{
    return (StrSlice){
        .chars = chs,
        .len = strlen(chs),
    };
}

StrSlice str_owned_slice(String str)
{
    if (str.cap == 0)
        return (StrSlice){0};

    char* buffer = str_clone_chars(str);
    return str_slice_from(buffer);
}

void str_slice_free(StrSlice* slice)
{
    free(slice->chars);
    slice->len = 0;
}

String str_from(char* chs)
{
    String str = {0};
    str_append(&str, chs);
    return str;
}

void str_push(String* str, char ch)
{
    // Make room for the `\0` as well
    str_ensure_cap(str, str->len + 1);

    str->chars[str->len] = ch;
    str->chars[str->len + 1] = '\0';
    str->len += 1;
}

void str_append(String* str, char* chs)
{
    size_t size = strlen(chs);
    str_ensure_cap(str, str->len + size);

    for (size_t i = 0; i < size; i++)
        str->chars[str->len + i] = chs[i];

    str->chars[str->len + size] = '\0';
    str->len += size;
}

void str_appendf(String* str, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t needed = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    str_ensure_cap(str, str->len + needed);

    char* dest = str->chars + str->len;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    va_end(args);

    str->len += needed;
}

char str_pop(String* str)
{
    assert(str->len >= 1 && "String expected length of >=, but got length of 0");
    str->len -= 1;
    return str->chars[str->len];
}

char* str_clone_chars(String str)
{
    char* buffer = (char*)malloc((str.len + 1) * sizeof(char));

    if (buffer == NULL)
    {
        exit(EXIT_FAILURE);
    }

    memcpy(buffer, str.chars, str.len);
    buffer[str.len] = '\0';

    return buffer;
}

void str_empty(String* str)
{
    str->len = 0;
}

void str_free(String* str)
{
    if (str->chars == NULL)
        return;

    free(str->chars);
    str->len = 0;
    str->cap = 0;
}

void str_ensure_cap(String* str, size_t new_cap)
{
    if (str->cap < new_cap + 1)
    {
        // + 1 for the '\0'
        str->chars = realloc(str->chars, ((new_cap * VEC_GROWTH_RATE) + 1) * sizeof(char));
        str->cap = new_cap;

        if (str->chars == NULL)
        {
            exit(EXIT_FAILURE);
        }
    }
}

size_t str_write(void* ptr, size_t size, size_t nmemb, String* str)
{
    size_t new_len = str->len + size * nmemb;

    str_ensure_cap(str, new_len);

    memcpy(str->chars + str->len, ptr, size * nmemb);

    str->chars[new_len] = '\0';
    str->len = new_len;

    return size * nmemb;
}

void* clone_arr(void* ptr, size_t size)
{
    void* buffer = malloc(size);
    memcpy(buffer, ptr, size);
    return buffer;
}

#define INTERNAL_BUF_SIZE 128
size_t read_file_to_str(String* buffer, FILE* file)
{
    size_t bytes_read = 0;
    char char_buf[INTERNAL_BUF_SIZE];
    while (fgets(char_buf, INTERNAL_BUF_SIZE, file))
    {
        bytes_read += strlen(char_buf);
        str_append(buffer, char_buf);
    }

    return bytes_read;
}

size_t read_lines(StringVec* linebuffer, FILE* file)
{
    size_t bytes_read = 0;

    char char_buf[INTERNAL_BUF_SIZE];
    while (fgets(char_buf, INTERNAL_BUF_SIZE, file))
    {
        bytes_read += strlen(char_buf);

        String line = str_from(char_buf);
        while (char_buf[strlen(char_buf) - 1] != '\n')
        {
            fgets(char_buf, INTERNAL_BUF_SIZE, file);
            str_append(&line, char_buf);
        }

        // pop the '\n'
        str_pop(&line);
        vec_push(linebuffer, str_owned_slice(line));
        str_free(&line);
    }

    return bytes_read;
}
