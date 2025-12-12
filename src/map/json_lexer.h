#ifndef JSON_LEXER_H
#define JSON_LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef double json_number_t;

/// The type of token that this token is
typedef enum
{
    /// A json key: A string with ':' after
    JSON_KEY,
    /// A value of type string
    JSON_STRING_VAL,
    /// A value of type bool
    JSON_BOOL_VAL,
    /// A value of type double
    JSON_NUMBER_VAL,
    /// An opening list bracket [
    JSON_OPEN_LIST,
    /// A closing list bracket ]
    JSON_CLOSE_LIST,
    /// An opening object bracket {
    JSON_OPEN_OBJ,
    /// A closing object bracket }
    JSON_CLOSE_OBJ,
    /// The delimiter between items ,
    JSON_ITEM_DELIM,
    /// End of file
    JSON_EOF,
} JsonTokenType;

typedef struct
{
    JsonTokenType tag;
    union
    {
        char* key;
        char* string_val;
        bool bool_val;
        json_number_t number_val;
    };
} JsonToken;

typedef struct
{
    char* buf;
    size_t idx;
} JsonLexer;


void json_token_free(JsonToken tok);

#define expect_token(got, expect)                                        \
    {                                                                    \
        assert((got).tag == (expect) && "Encountered unexpected token"); \
    }

#define expect_token_and_free(got, expect)                                        \
    {                                                                             \
        JsonToken __expect_token = (got);                                         \
        assert(__expect_token.tag == (expect) && "Encountered unexpected token"); \
        json_token_free(__expect_token);                                          \
    }


JsonToken json_lexer_next(JsonLexer* lex);
JsonToken json_lexer_peek(JsonLexer lex);

#endif // JSON_LEXER_H
