#ifndef DYN_H
#define DYN_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*          *
 *  VECTOR  *
 *          */
// CONFIG DEFINES:
//  - VEC_GROWTH
//  - VEC_START_CAP
//
// CONST DEFINES:
//  - VEC_FOREACH
//
// TYPES AND TYPE MACROS:
//  - VecDef(T)
//
// FUNCTIONS/MACROS:
//  - vec_owned_slice(vec)
//  - vec_slice_range(arr, start, end)
//  - vec_from(*arr, size)
//  - vec_push(*vec, item)
//  - vec_unshift(*vec, item)
//  - vec_append(*vec, *arr, size)
//  - vec_prepend(*vec, *arr, size)
//  - vec_pop(*vec)
//  - vec_shift(*vec, *out)
//  - vec_clone_items(vec)
//      - todo: vec_drain(*vec)
//      - todo: vec_drain_range(*str, start, end)
//      - todo: vec_remove(*vec, idx)
//      - todo: vec_remove_range(*vec, start, end)
//  - vec_empty(*vec)
//      - todo: vec_split_at(vec, idx)
//  - vec_free(vec)
//  - vec_ensure_cap(vec, new_cap)
//
//
// USAGE:
//  # DEFINING
//  Define a vector type with the `VecDef(T)` macro, e.g:
//  ```c
//  typedef VecDef(int) IntVec;
//  ```
//
//  # INITIALIZING
//  To create such a vector, you can zero intialize it:
//  ```c
//  IntVec ivec = {0};
//  ```
//
//  # USING
//  After you have a vector initialized, you can manipulate it with macros. Such as `vec_push()`
//  for pushing a new element
//  ```c
//  IntVec ivec = {0};
//
//  vec_push(&ivec, 1); // -> ivec = { 1 }
//  vec_push(&ivec, 2); // -> ivec = { 1, 2 }
//  vec_push(&ivec, 3); // -> ivec = { 1, 2, 3 }
//  ```
//
//  # FREEING
//  Remember to free the vector after use with `vec_free()`, to avoid memory leaks
//  ```c
//  vec_free(al);
//  ```

#ifndef VEC_GROWTH_RATE
/// The rate of vectors growth
#define VEC_GROWTH_RATE 1.5
#endif

#ifndef VEC_START_CAP
/// The starting capacity of the vector
#define VEC_START_CAP 10
#endif

/// Define a Vector struct with the given inner type T
#define VecDef(T)   \
    struct          \
    {               \
        T* items;   \
        size_t len; \
        size_t cap; \
    }

/// Iterate over a vector or slice
#define VEC_FOREACH(T, vec, elem) \
    for (T* elem = (vec).items; (elem) < (vec).items + (vec).len; (elem)++)

/// Create an owned slice from a vector.
/// The caller is responsible for freeing the new slice with `slice_free()` after use
#define vec_owned_slice(vec) slice_from((vec_clone_items((vec))), (vec).len)

// Slice a vector in a range from start to end.
// This does not clone or copy any of the elements of the vector, and the vector is still
// responsible for the memory
#define vec_slice_range(T, arr, start, end)                              \
    (((start) < (arr).len || (end) < (arr).len)                          \
         ? ((T){.items = (arr).items + (start), .len = (end) - (start)}) \
         : ((T){0}))

/// Create a new vector from a pointer or array. This will clone the contents of the pointer or
/// array.
/// The caller is responsible for freeing the new vector with `vec_free()`
#define vec_from(arr, size)                                   \
    {                                                         \
        .items = __clone_arr((arr), (size) * sizeof(*(arr))), \
        .len = (size),                                        \
        .cap = (size),                                        \
    }

/// Push an item to a vector
#define vec_push(vec, item)                    \
    {                                          \
        vec_ensure_cap((vec), (vec)->len + 1); \
        (vec)->items[(vec)->len] = (item);     \
        (vec)->len += 1;                       \
    }


/// Add an element to the front of the vector
#define vec_unshift(vec, item)                                                        \
    {                                                                                 \
        vec_ensure_cap((vec), (vec)->len + 1);                                        \
        memcpy((vec)->items + 1, (vec)->items, (vec)->len * sizeof((vec)->items[0])); \
        (vec)->items[0] = (item);                                                     \
        (vec)->len++;                                                                 \
    }


/// Append a list of items to a vector
#define vec_append(vec, arr, size)                   \
    {                                                \
        vec_ensure_cap((vec), (vec)->len + (size));  \
        for (size_t i = 0; i < (size); i++)          \
            (vec)->items[(vec)->len + i] = (arr)[i]; \
        (vec)->len += (size);                        \
    }

/// Prepend a list of items to the front of the vector
#define vec_prepend(vec, arr, size)                                                \
    {                                                                              \
        vec_ensure_cap((vec), (vec)->len + (size));                                \
        /* Move all current items to make space in front */                        \
        memmove((vec)->items + (size), (vec)->items, (vec)->len * sizeof(*(arr))); \
        /* Add the items from the arr */                                           \
        memcpy((vec)->items, (arr), (size) * sizeof(*(arr)));                      \
        (vec)->len += (size);                                                      \
    }


/// Pops and returns the last element in the vector.
/// This asserts that the vector has an element that can be popped
#define vec_pop(vec)                                                               \
    (assert((vec)->len >= 1 &&                                                     \
            "Vector failed to pop, expected length of >= 1, but got length of 0"), \
     (vec)->len -= 1, ((vec)->items[(vec)->len]))

// Remove the first element in the vector, and puts it into out.
/// This asserts that the vector has an element that can be shifted
#define vec_shift(vec, out)                                                                 \
    {                                                                                       \
        assert((vec)->len >= 1 &&                                                           \
               "Vector failed to shift, expected length of >= 1, but got length of 0");     \
        *(out) = (vec)->items[0];                                                           \
        memcpy((vec)->items, (vec)->items + 1, ((vec)->len - 1) * sizeof((vec)->items[0])); \
        (vec)->len -= 1;                                                                    \
    }

/// Clone the items of a vector, returning them as an allocated pointer.
/// The caller is responsible for freeing the new pointer
#define vec_clone_items(vec) \
    ((vec).cap > 0 ? __clone_arr((vec).items, (vec).len * sizeof((vec).items[0])) : 0)

/// Empty the given vector. This does not free or remove any memory
#define vec_empty(vec) (vec)->len = 0

/// Free the given vector and its contents
#define vec_free(vec)      \
    {                      \
        free((vec).items); \
        (vec).items = 0;   \
        (vec).len = 0;     \
        (vec).cap = 0;     \
    }


/// Ensure that a vector has atleast the given capacity.
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
            fprintf(stderr, "err: Failed to reallocate vector\n");                  \
            exit(EXIT_FAILURE);                                                     \
        }                                                                           \
                                                                                    \
        (vec)->cap = new_cap;                                                       \
    }

void* __clone_arr(void* ptr, size_t size);

/* END OF VECTOR DECL */

/*          *
 *  SLICES  *
 *          */
// CONFIG DEFINES:
//  -
//
// CONST DEFINES:
//  -
//
// TYPES AND TYPE MACROS:
//  - SliceDef(T)
//
// FUNCTIONS AND MACROS:
//  - slice_from(arr, size)
//  - slice_free(slice)
//      - todo: slice_clone(slice)
//
// USAGE:
//  # DEFINING
//  Define a slice type with the `VecDef(T)` macro, e.g:
//  ```c
//  typedef SliceDef(int) IntSlice;
//  ```
//
//  # INITIALIZING
//  To create such a vector, you can zero intialize it:
//  ```c
//  IntSlice islice = {0};
//  ```

/// Define a Slice struct with the given inner type T
#define SliceDef(T) \
    struct          \
    {               \
        T* items;   \
        size_t len; \
    }


/// Create a new slice from a pointer, and a len.
/// This will not clone or copy the items in the array, so it will not own the contents.
/// Do not use `slice_free()` on a slice from this, unless you know what you are doing, as that
/// would free the original contents.
#define slice_from(arr, size) \
    {                         \
        .items = arr,         \
        .len = size,          \
    }


/// Free a slice. Be sure that the slice owns its contents before calling this, as it otherwise can
/// create memory issues
#define slice_free(slice)     \
    {                         \
        free((slice)->items); \
        (slice)->len = 0;     \
    }

/* END OF SLICE DECL */



/*          *
 *  STRING  *
 *          */
// CONFIG DEFINES:
//  -
//
// CONST DEFINES
//  - STR_FMT
//  - STR_ARG
//
// TYPES AND TYPE MACROS:
//  - String
//  - StrSlice
//  - StrVec
//
// FUNCTIONS AND MACROS:
//  - str_slice_with_len(len)
//  - str_slice_from(chs)
//  - str_owned_slice(str)
//  - str_slice_clone(slice)
//  - str_slice_range(str, start, end)
//  - str_slice_free(*slice)
//
//  - str_with_capacity(capacity)
//  - str_from(*chs)
//  - str_push(*str, ch)
//  - str_unshift(*str, ch)
//  - str_append(*str, *chs)
//  - str_appendf(*str, *fmt, ...)
//  - str_prepend(*str, *chs)
//  - str_prependf(*str, *chs)
//  - str_pop(*str)
//  - str_shift(*str)
//  - str_clone_chars(str)
//  - str_drain(*str);
//  - str_drain_range(*str, start, end);
//  - str_remove(*str, idx)
//  - str_remove_range(*str, start, end)
//  - str_empty(*str)
//  - str_free(*str)
//  - str_ensure_cap(*str, new_cap)
//  - str_trim_front(*str)
//  - str_trim_back(*str)
//  - str_trim(*str)
//  - str_split_by(str, delim)
//  - str_split_by_once(str, delim)
//  - str_split_by_many(str, *delims)
//  - str_split_at(str, idx)
//  - str_read_file(*buffer, *file)
//  - str_read_lines(*linebuffer, *file)
//
// USAGE:
//  # DEFINING
//  Define a slice type with the `VecDef(T)` macro, e.g:
//  ```c
//  typedef SliceDef(int) IntVec;
//  ```
//
//  # INITIALIZING
//  To create such a vector, you can zero intialize it:
//  ```c
//  IntVec ivec = {0};
//  ```
//
//  # USING
//  After you have a vector initialized, you can manipulate it with macros. Such as `vec_push()`
//  for pushing a new element
//  ```c
//  IntVec ivec = {0};
//
//  vec_push(&ivec, 1); // -> ivec = { 1 }
//  vec_push(&ivec, 2); // -> ivec = { 1, 2 }
//  vec_push(&ivec, 3); // -> ivec = { 1, 2, 3 }
//  ```
//
//  # FREEING
//  Remember to free the vector after use with `vec_free()`, to avoid memory leaks
//  ```c
//  vec_free(al);
//  ```

/// A dynamic and owned string
typedef struct String
{
    /// The characters of the string. Can be used as a cstr
    char* chars;
    /// The length of the string
    size_t len;
    /// The capacity of the string
    size_t cap;
} String;


/// A string slice, which holds a cstr and a length
typedef struct StrSlice
{
    /// The characters of the string slice. Can be used as a cstr
    char* chars;
    /// The length of the string slice
    size_t len;
} StrSlice;

/// A format string used to print strings and string slices properly. Remember to use `STR_ARG()`
/// as the format argument.
#define STR_FMT "%.*s"
/// The arg used
#define STR_ARG(str) (int)(str).len, (str).chars

/// A vector containing string slices.
/// To avoid leaking memory, remember to free each individual string slice by themselves, as
/// `vec_free` will not do this for you.
typedef VecDef(StrSlice) StrVec;

/// Allocate a new empty string slice with a specific length. The caller is responsible for freeing
/// the memory with `slice_free()`
StrSlice str_slice_with_len(size_t len);

/// Get a slice to a c string, this will not copy or clone the chars, just point to them.
StrSlice str_slice_from(char* chs);

/// Get an owned slice to the contents of a string. The caller is responsible for freeing the new
/// slice with `str_slice_free(slice)`
StrSlice str_owned_slice(String str);

/// Clone a string slice and its chars.
/// The caller i responsible for freeing the slice with `str_slice_free()`
StrSlice str_slice_clone(StrSlice slice);

/// Get a slice from a string in a specific range. This does not copy or clone the chars in the
/// range.
StrSlice str_slice_range(String str, size_t start, size_t end);

/// Free a string slice and its memory. This should *only* be called if the slice owns its own
/// characters
void str_slice_free(StrSlice* slice);

/// Allocates a new string with a specific capacity. The caller is responsible for freeing the
/// string with `str_free()`
String str_with_capacity(size_t capacity);

/// Create a new string from a cstr. This will clone the contents of the cstr.
/// The caller is responsible for freeing the new string with `str_free()`
String str_from(char* chs);

/// Push a char unto the end of a string
void str_push(String* str, char ch);

/// Adds a char to the start of a string
void str_unshift(String* str, char ch);

/// Append a cstr unto the end of a string
void str_append(String* str, char* chs);

/// Append a formatted cstr unto the end of a string
void str_appendf(String* str, const char* fmt, ...);

/// Prepend a cstr to the front of a string
void str_prepend(String* str, char* chs);

/// Prepend a formatted cstr to the front of a string
void str_prependf(String* str, const char* fmt, ...);

/// Pop and return the last char of a string.
/// This asserts that the string has a char that can be popped
char str_pop(String* str);

/// Remove and return the first char of a string.
/// This asserts that the string has a char that can be popped
char str_shift(String* str);

/// Clone the chars behind a string, and return them as an owned string slice.
/// The caller is responsible for freeing the allocated cstr
char* str_clone_chars(String str);

/// Puts all the contents of a string into a slice, removing all of the contents of the original
/// string.
/// The slice is now the owner of the memory.
/// The caller is responsible for freeing the string slice with `str_slice_free()`
StrSlice str_drain(String* str);

/// Puts all the contents of a string in a given range into a slice, removing the elements from the
/// original string. The slice is now the owner of the removed memory. The caller is responsible for
/// freeing the string slice with `str_slice_free()`
StrSlice str_drain_range(String* str, size_t start, size_t end);

/// Remove a char at a given index from a string
void str_remove(String* str, size_t idx);

/// Remove a set of chars in a given range, from a string
void str_remove_range(String* str, size_t start, size_t end);

/// Empty the given string. This does not free or remove any memory
void str_empty(String* str);

/// Free the given vector and its contents
void str_free(String* str);

/// Ensure that a string has atleast the given capacity.
void str_ensure_cap(String* str, size_t new_cap);

/// Trim all whitespace from the front of a string
void str_trim_front(String* str);

/// Trim all whitespace from the back of the string
void str_trim_back(String* str);

/// Trim all whitespace from the front and back of a string
void str_trim(String* str);

/// Split a string by a delimeter
StrVec str_split_by(String str, char delim);

/// Split a string by a set of delimeters
StrVec str_split_by_many(String str, char* delims);

/// Split a string by a delimeter once
StrVec str_split_by_once(String str, char delim);

/// Split a string at an index
StrVec str_split_at(String str, size_t idx);

/// Reads a file into a string buffer, and returns the amount of bytes read.
size_t str_read_file(String* buffer, FILE* file);

// Reads all the lines in a file into a line buffer, and returns the amount of bytes read.
size_t str_read_lines(StrVec* linebuffer, FILE* file);

/* END OF STRING DECL */



/*         *
 *  ASCII  *
 *         */
// CONFIG DEFINES:
//  -
//
// CONST DEFINES:
//  -
//
// TYPES AND TYPE MACROS:
//  -
//
// FUNCTIONS AND MACROS:
//  - ascii_is_alphabetic(ch)
//  - ascii_is_numeric(ch)
//  - ascii_is_alphanumeric(ch)
//  - ascii_is_whitespace(ch)
//  - ascii_is_uppercase(ch)
//  - ascii_is_lowercase(ch)
//  - ascii_to_uppercase(ch)
//  - ascii_to_lowercase(ch)
//
// USAGE:

/// Checks if an ascii char is alphabetic, i.e. [a-zA-Z]
char ascii_is_alphabetic(char ch);
/// Checks if an ascii char is numeric, i.e. [0-9]
bool ascii_is_numeric(char ch);
/// Checks if an ascii char is alphanumeric, i.e. [a-zA-Z0-9]
bool ascii_is_alphanumeric(char ch);
/// Checks if an ascii char is whitespace, e.g. '\n', '\t', ' '
bool ascii_is_whitespace(char ch);
/// Checks if an ascii char is uppercase
bool ascii_is_uppercase(char ch);
/// Checks if an ascii char is lowercase
bool ascii_is_lowercase(char ch);
/// Returns the uppercase version of an ascii char
char ascii_to_uppercase(char ch);
/// Returns the lowercase version of an ascii char
char ascii_to_lowercase(char ch);

/* END OF ASCII DECL */

#endif // DYN_H
