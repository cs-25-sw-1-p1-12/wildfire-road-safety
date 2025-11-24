#include "dyn.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/*                         *
 *  VECTOR IMPLEMENTATION  *
 *                         */

void* __clone_arr(void* ptr, size_t size)
{
    void* buffer = malloc(size);
    memcpy(buffer, ptr, size);
    return buffer;
}

/* END OF VECTOR IMPLEMENTATION */



/*                         *
 *  STRING IMPLEMENTATION  *
 *                         */

StrSlice str_slice_with_len(size_t len)
{
    return (StrSlice){
        .chars = (char*)calloc(len + 1, sizeof(char)),
        .len = len,
    };
}

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

StrSlice str_slice_clone(StrSlice slice)
{
    char* chars = (char*)malloc((slice.len + 1) * sizeof(char));
    memcpy(chars, slice.chars, slice.len * sizeof(char));

    chars[slice.len] = '\0';
    return (StrSlice){.chars = chars, .len = slice.len};
}

StrSlice str_slice_range(String str, size_t start, size_t end)
{
    if (start >= str.len || end > str.len || start >= end)
        return (StrSlice){0};

    return (StrSlice){
        .chars = str.chars + start,
        .len = end - start,
    };
}

void str_slice_free(StrSlice* slice)
{
    free(slice->chars);
    slice->len = 0;
}

String str_with_capacity(size_t capacity)
{
    return (String){
        .chars = (char*)calloc(capacity + 1, sizeof(char)),
        .len = 0,
        .cap = capacity,
    };
}

String str_from(char* chs)
{
    String str = {0};
    str_append(&str, chs);
    return str;
}

void str_push(String* str, char ch)
{
    str_ensure_cap(str, str->len + 1);

    str->chars[str->len] = ch;
    str->chars[str->len + 1] = '\0';
    str->len += 1;
}

void str_unshift(String* str, char ch)
{
    str_ensure_cap(str, str->len + 1);
    memmove(str->chars + 1, str->chars, str->len * sizeof(char));
    str->chars[0] = ch;
    str->len += 1;
    str->chars[str->len] = '\0';
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

    // sprintf automatically adds '\0', so take account for it
    str->len += needed - 1;
}

void str_prepend(String* str, char* chs)
{
    size_t size = strlen(chs);
    str_ensure_cap(str, str->len + size);

    memcpy(str->chars + size, str->chars, str->len * sizeof(char));
    memcpy(str->chars, chs, size * sizeof(char));

    str->chars[str->len + size] = '\0';
    str->len += size;
}

void str_prependf(String* str, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t needed = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    str_ensure_cap(str, str->len + needed);

    memcpy(str->chars + needed - 1, str->chars, str->len * sizeof(char));

    StrSlice format_buf = str_slice_with_len(needed - 1);
    va_start(args, fmt);
    vsprintf(format_buf.chars, fmt, args);
    va_end(args);

    memcpy(str->chars, format_buf.chars, format_buf.len * sizeof(char));

    str_slice_free(&format_buf);

    // sprintf automatically adds '\0', so take account for it
    str->len += needed - 1;
}

char str_pop(String* str)
{
    assert(str->len >= 1 && "String failed to pop, expected length of >= 1, but got length of 0");
    str->len -= 1;
    char popped = str->chars[str->len];
    str->chars[str->len] = '\0';
    return popped;
}


char str_shift(String* str)
{
    assert(str->len >= 1 && "String failed to shift, expected length of >= 1, but got length of 0");

    char shifted = str->chars[0];

    memcpy(str->chars, str->chars + 1, (str->len - 1) * sizeof(char));
    str->len -= 1;
    str->chars[str->len] = '\0';

    return shifted;
}

char* str_clone_chars(String str)
{
    char* buffer = (char*)malloc((str.len + 1) * sizeof(char));

    if (buffer == NULL)
    {
        fprintf(stderr, "err: Failed to clone string chars\n");
        exit(EXIT_FAILURE);
    }

    memcpy(buffer, str.chars, str.len * sizeof(char));
    buffer[str.len] = '\0';

    return buffer;
}

StrSlice str_drain(String* str)
{
    StrSlice slice = str_slice_from(str->chars);

    *str = (String){0};

    return slice;
}

StrSlice str_drain_range(String* str, size_t start, size_t end)
{
    StrSlice slice = str_slice_range(*str, start, end);
    slice = str_slice_clone(slice);

    str_remove_range(str, start, end);

    return slice;
}

void str_remove(String* str, size_t idx)
{
    if (idx >= str->len)
        return;

    str_remove_range(str, idx, idx + 1);
}

void str_remove_range(String* str, size_t start, size_t end)
{
    if (start >= str->len || end > str->len || start >= end)
        return;

    if (end == str->len)
    {
        str->len = start;
        str->chars[str->len] = '\0';
        return;
    }

    memmove(str->chars + start, str->chars + end, (str->len - end) * sizeof(char));
    str->len -= (end - start);
    str->chars[str->len] = '\0';
}

void str_empty(String* str)
{
    str->len = 0;
    if (str->chars)
        str->chars[0] = '\0';
}

void str_free(String* str)
{
    if (str->chars == NULL)
        return;

    free(str->chars);
    str->chars = (char*)0;
    str->len = 0;
    str->cap = 0;
}

void str_ensure_cap(String* str, size_t new_cap)
{
    if (str->cap < new_cap + 1)
    {
        // + 1 for the '\0'
        str->chars = (char*)realloc(str->chars, ((new_cap * VEC_GROWTH_RATE) + 1) * sizeof(char));
        str->cap = new_cap;

        if (str->chars == NULL)
        {
            fprintf(stderr, "err: Failed to reallocate string\n");
            exit(EXIT_FAILURE);
        }

        str->chars[str->cap] = '\0';
    }
}


#define INTERNAC_BUF_SIZE 256
size_t str_read_file(String* buffer, FILE* file)
{
    size_t bytes_read = 0;
    char char_buf[INTERNAC_BUF_SIZE];
    while (fgets(char_buf, INTERNAC_BUF_SIZE, file))
    {
        bytes_read += strlen(char_buf);
        str_append(buffer, char_buf);
    }

    return bytes_read;
}

size_t str_read_lines(StrVec* linebuffer, FILE* file)
{
    size_t bytes_read = 0;

    char char_buf[INTERNAC_BUF_SIZE];
    while (fgets(char_buf, INTERNAC_BUF_SIZE, file))
    {
        bytes_read += strlen(char_buf);

        String line = str_from(char_buf);
        size_t old_size = strlen(char_buf);
        while (char_buf[strlen(char_buf) - 1] != '\n')
        {
            fgets(char_buf, INTERNAC_BUF_SIZE, file);
            if (strlen(char_buf) == old_size)
                break;
            str_append(&line, char_buf);
        }

        // pop the '\n'
        if (line.chars[line.len - 1] == '\n')
            str_pop(&line);

        vec_push(linebuffer, str_owned_slice(line));
        str_free(&line);
    }

    return bytes_read;
}
#undef INTERNAC_BUF_SIZE

void str_trim_front(String* str)
{
    if (str->len == 0)
        return;

    size_t end = 0;
    while (end < str->len && ascii_is_whitespace(str->chars[end]))
        end++;

    str_remove_range(str, 0, end);
}

/// Trim all whitespace from the back of the string
void str_trim_back(String* str)
{
    if (str->len == 0)
        return;

    size_t start = str->len;
    while (start > 0 && ascii_is_whitespace(str->chars[start - 1]))
        start--;

    str_remove_range(str, start, str->len);
}

/// Trim all whitespace from the front and back of a string
void str_trim(String* str)
{
    str_trim_back(str);
    str_trim_front(str);
}

/// Split a string by a delimeter
StrVec str_split_by(String str, char delim)
{
    size_t start = 0;
    size_t end = 0;

    StrVec parts = {0};

    while (end <= str.len)
    {
        if (str.chars[end] == delim)
        {
            StrSlice slice = str_slice_range(str, start, end);
            slice = str_slice_clone(slice);

            vec_push(&parts, slice);
            end++;
            start = end;
        }
        end++;
    }

    StrSlice slice = str_slice_range(str, start, str.len);
    slice = str_slice_clone(slice);

    vec_push(&parts, slice);

    return parts;
}

/// Split a string by a set of delimeters
StrVec str_split_by_many(String str, char* delims)
{
    size_t start = 0;
    size_t end = 0;

    StrVec parts = {0};

    while (end < str.len)
    {
        bool is_delim = false;
        for (size_t i = 0; i < strlen(delims); i++)
        {
            if (str.chars[end] == delims[i])
            {
                is_delim = true;
                break;
            }
        }

        if (is_delim)
        {
            StrSlice slice = str_slice_range(str, start, end);
            slice = str_slice_clone(slice);

            vec_push(&parts, slice);
            end++;
            start = end;
        }
        end++;
    }

    StrSlice slice = str_slice_range(str, start, str.len);
    slice = str_slice_clone(slice);

    vec_push(&parts, slice);

    return parts;
}

/// Split a string by a delimeter once
StrVec str_split_by_once(String str, char delim)
{
    size_t end = 0;

    StrVec parts = {0};

    while (end < str.len)
    {
        if (str.chars[end] == delim)
        {
            StrSlice slice = str_slice_range(str, 0, end);
            slice = str_slice_clone(slice);

            vec_push(&parts, slice);
            break;
        }
        end++;
    }

    StrSlice rest = str_slice_range(str, end + 1, str.len);
    rest = str_slice_clone(rest);
    vec_push(&parts, rest);

    return parts;
}

/// Split a string at an index
StrVec str_split_at(String str, size_t idx)
{
    StrVec parts = {0};

    if (idx >= str.len)
    {
        vec_push(&parts, str_owned_slice(str));
        return parts;
    }

    StrSlice first_part = str_slice_range(str, 0, idx);
    first_part = str_slice_clone(first_part);
    vec_push(&parts, first_part);

    StrSlice second_part = str_slice_range(str, idx, str.len);
    second_part = str_slice_clone(second_part);
    vec_push(&parts, second_part);

    return parts;
}

/* END OF STRING IMPLEMENTATION */



/*                        *
 *  ASCII IMPLEMENTATION  *
 *                        */

char ascii_is_alphabetic(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool ascii_is_numeric(char ch)
{
    return ch >= '0' && ch <= '9';
}

bool ascii_is_alphanumeric(char ch)
{
    return ascii_is_alphabetic(ch) || ascii_is_numeric(ch);
}

bool ascii_is_whitespace(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f';
}

bool ascii_is_uppercase(char ch)
{
    return ch >= 'A' && ch <= 'Z';
}

bool ascii_is_lowercase(char ch)
{
    return ch >= 'a' && ch <= 'z';
}

char ascii_to_uppercase(char ch)
{
    if (!ascii_is_alphabetic(ch))
        return ch;

    // e.g. convert 1000010 (B) to 1100010 (b)
    return ch & ~0x20;
}

char ascii_to_lowercase(char ch)
{
    if (!ascii_is_alphabetic(ch))
        return ch;

    // e.g. convert 1000010 (B) to 1100010 (b)
    return ch | 0x20;
}

/* END OF ASCII IMPLEMENTATION */
