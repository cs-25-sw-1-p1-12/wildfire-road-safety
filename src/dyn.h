#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef VEC_GROWTH_RATE
#define VEC_GROWTH_RATE 1.5
#endif

#ifndef VEC_START_CAP
#define VEC_START_CAP 10
#endif

#define MAX(a, b) (a >= b ? a : b)
#define MIN(a, b) (a <= b ? a : b)

#define VecDef(T)   \
    struct          \
    {               \
        T* items;   \
        size_t len; \
        size_t cap; \
    }

#define SliceDef(T) \
    struct          \
    {               \
        T* items;   \
        size_t len; \
    }

typedef struct String
{
    char* chars;
    size_t len;
    size_t cap;
} String;


typedef struct StrSlice
{
    char* chars;
    size_t len;
} StrSlice;

/// A vector containing string slices.
/// To avoid leaking memory, remember to free each individual string slice by themselves, as
/// `vec_free` will not do this for you.
typedef VecDef(StrSlice) StringVec;

/// Get a slice to a c string, this will not copy or clone the chars, just point to them.
StrSlice str_slice_from(char* chs);
/// Get an owned slice to the contents of a string. The caller is responsible for freeing the new
/// slice with `str_slice_free(slice)`
StrSlice str_owned_slice(String str);
/// Free a string slice and its memory. This should *only* be called if the slice owns its own
/// characters
void str_slice_free(StrSlice* slice);

String str_from(char* chs);
void str_push(String* str, char ch);
void str_append(String* str, char* chs);
void str_appendf(String* str, const char* fmt, ...);
char str_pop(String* str);
char* str_clone_chars(String str);
void str_empty(String* str);
void str_free(String* str);
void str_ensure_cap(String* str, size_t new_cap);

void str_trim_front(String* str);
void str_trim_back(String* str);
StringVec str_split_by(String str, char delim);
StringVec str_split_by_many(String str, char* delims);
StringVec str_split_by_once(String str, char delim);
StringVec str_split_at(String str, size_t idx);

/// Reads a file into a string buffer, and returns the amount of bytes read.
size_t read_file_to_str(String* buffer, FILE* file);
size_t read_lines(StringVec* linebuffer, FILE* file);

void* clone_arr(void* ptr, size_t size);

#define slice_from(arr, size) \
    {                         \
        .items = arr,         \
        .len = size,          \
    }

#define vec_owned_slice(vec) slice_from((vec_clone_items((vec))), (vec).len)

#define slice_free(slice)     \
    {                         \
        free((slice)->items); \
        (slice)->len = 0;     \
    }

#define vec_push(vec, item)                    \
    {                                          \
        vec_ensure_cap((vec), (vec)->len + 1); \
        (vec)->items[(vec)->len] = (item);     \
        (vec)->len += 1;                       \
    }

#define vec_append(vec, item_arr, amount)                 \
    {                                                     \
        vec_ensure_cap((vec), (vec)->len + (amount));     \
        for (size_t i = 0; i < (amount); i++)             \
            (vec)->items[(vec)->len + i] = (item_arr)[i]; \
        (vec)->len += (amount);                           \
    }

#define vec_pop(vec)                                                                   \
    (assert((vec)->len >= 1 && "Vector expected length of >= 1, but got length of 0"), \
     (vec)->len -= 1, ((vec)->items[(vec)->len]))

#define vec_clone_items(vec) \
    ((vec).cap > 0 ? clone_arr((vec).items, (vec).len * sizeof((vec).items[0])) : 0)

#define vec_empty(vec) (vec)->len = 0

#define vec_free(vec)      \
    {                      \
        free((vec).items); \
        (vec).len = 0;     \
        (vec).cap = 0;     \
    }


#define vec_ensure_cap(vec, new_cap)                                                \
    if ((vec)->cap < new_cap)                                                       \
    {                                                                               \
        size_t calced_cap;                                                          \
        if ((vec)->cap == 0)                                                        \
        {                                                                           \
            calced_cap = VEC_START_CAP;                                             \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            calced_cap = (new_cap * VEC_GROWTH_RATE);                               \
        }                                                                           \
        (vec)->items = realloc((vec)->items, calced_cap * sizeof((vec)->items[0])); \
                                                                                    \
        if ((vec)->items == NULL)                                                   \
        {                                                                           \
            exit(EXIT_FAILURE);                                                     \
        }                                                                           \
                                                                                    \
        (vec)->cap = new_cap;                                                       \
    }
