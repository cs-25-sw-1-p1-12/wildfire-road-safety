#include "json_lexer.h"

#include "../dyn.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool parse_string(JsonLexer* lex, JsonToken* token);
bool is_numerical(char ch);
bool parse_numerical(JsonLexer* lex, JsonToken* token);
bool is_whitespace(char ch);
bool check_prefix(const char* str, const char* pre);

JsonToken json_lexer_next(JsonLexer* lex)
{
    JsonToken token = (JsonToken){.tag = JSON_EOF};

    size_t old_idx = 0;
    while (lex->idx < strlen(lex->buf))
    {
        if (lex->idx != 0 && lex->idx == old_idx)
            printf("INFI LOOP IN LEXER!!\n");

        old_idx = lex->idx;

        char ch = lex->buf[lex->idx];

        if (is_whitespace(ch))
        {
            lex->idx++;
            continue;
        }

        if (ch == '[')
        {
            lex->idx++;
            token = (JsonToken){.tag = JSON_OPEN_LIST};
            break;
        }
        if (ch == ']')
        {
            lex->idx++;
            token = (JsonToken){.tag = JSON_CLOSE_LIST};
            break;
        }

        if (ch == '{')
        {
            lex->idx++;
            token = (JsonToken){.tag = JSON_OPEN_OBJ};
            break;
        }
        if (ch == '}')
        {
            lex->idx++;
            token = (JsonToken){.tag = JSON_CLOSE_OBJ};
            break;
        }

        if (ch == '"')
        {
            lex->idx++;
            if (!parse_string(lex, &token))
            {
                token = (JsonToken){.tag = JSON_EOF};
                break;
            }
            break;
        }

        if (check_prefix(lex->buf + lex->idx, "true"))
        {
            lex->idx += strlen("true");
            token = (JsonToken){
                .tag = JSON_BOOL_VAL,
                .bool_val = true,
            };
            break;
        }

        if (check_prefix(lex->buf + lex->idx, "false"))
        {
            lex->idx += strlen("false");
            token = (JsonToken){
                .tag = JSON_BOOL_VAL,
                .bool_val = false,
            };
            break;
        }

        if (is_numerical(ch))
        {
            if (!parse_numerical(lex, &token))
            {
                token = (JsonToken){.tag = JSON_EOF};
            }
            break;
        }
        lex->idx++;
    }

    return token;
}

// TODO: Finish
bool parse_string(JsonLexer* lex, JsonToken* token)
{
    String buf = {0};

    size_t idx = lex->idx;

    bool reached_closing = false;
    while (idx < strlen(lex->buf))
    {
        char ch = lex->buf[idx];
        if (ch == '\\')
        {
            // Skip the '\'
            idx++;
            str_push(&buf, lex->buf[idx]);
            // skip next char
            idx++;
            continue;
        }

        if (ch == '"')
        {
            idx++;
            reached_closing = true;
            break;
        }

        str_push(&buf, lex->buf[idx]);
        idx++;
    }

    if (!reached_closing)
        return false;

    char* str = str_clone_chars(buf);
    str_free(&buf);

    // Chceck if string is key or value
    if (lex->buf[idx] == ':')
    {
        idx++;
        *token = (JsonToken){
            .tag = JSON_KEY,
            .key = str,
        };
    }
    else
    {
        *token = (JsonToken){
            .tag = JSON_STRING_VAL,
            .key = str,
        };
    }

    lex->idx = idx;
    return true;
}

bool is_numerical(char ch)
{
    return ch >= '0' && ch <= '9';
}

bool parse_numerical(JsonLexer* lex, JsonToken* token)
{
    String buf = {0};

    size_t old_idx = 0;

    size_t idx = lex->idx;
    while (idx < strlen(lex->buf))
    {
        if (idx == old_idx)
            printf("INFI LOOP IN NUMBER PARSER!!!\n");
        old_idx = idx;

        char ch = lex->buf[idx];

        if (is_whitespace(ch) || ch == ',')
            break;

        if (is_numerical(ch) || ch == '_' || ch == '.' ||
            (ch == 'x' && buf.len == 1 && buf.chars[0] == '0'))
        {
            str_push(&buf, ch);
            idx++;
        }
        else
        {
            str_free(&buf);
            return false;
        }
    }

    bool ret_val = false;

    char* endptr = NULL;
    double number_val = strtod(buf.chars, &endptr);
    if (*endptr == '\0')
    {
        *token = (JsonToken){
            .tag = JSON_NUMBER_VAL,
            .number_val = number_val,
        };
        ret_val = true;
    }
    else
    {
        ret_val = false;
    }

    lex->idx = idx;
    str_free(&buf);
    return ret_val;
}

void json_token_free(JsonToken tok)
{
    switch (tok.tag)
    {
        case JSON_KEY:
            free(tok.key);
            break;
        case JSON_STRING_VAL:
            free(tok.string_val);
            break;
        case JSON_BOOL_VAL:
        case JSON_NUMBER_VAL:
        case JSON_OPEN_LIST:
        case JSON_CLOSE_LIST:
        case JSON_OPEN_OBJ:
        case JSON_CLOSE_OBJ:
        case JSON_ITEM_DELIM:
        case JSON_EOF:
            break;
    }
}

void expect_token(JsonToken got, JsonTokenType expect)
{
    assert(got.tag == expect && "Encountered unexpected token");
}

void expect_token_and_free(JsonToken got, JsonTokenType expect)
{
    expect_token(got, expect);
    json_token_free(got);
}

bool is_whitespace(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

size_t skip_whitespace(char* input, size_t idx)
{
    size_t len = strlen(input);
    size_t i = idx;
    while (i < len && is_whitespace(input[i]))
        i++;

    return i;
}

bool check_prefix(const char* str, const char* pre)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}
