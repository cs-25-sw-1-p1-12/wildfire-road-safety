#include "fire_json_parse.h"

#include "json_lexer.h"

#include <string.h>

bool fire_json_parse(char* input, FireSlice* fire_data)
{
    JsonLexer lex = {
        .buf = input,
        .idx = 0,
    };
    JsonToken tok = {0};

    FireVec fires = {0};

    expect_token_and_free(json_lexer_next(&lex), JSON_OPEN_OBJ);

    tok = json_lexer_next(&lex);
    printf("EXPECTED: {tag: %d}\tGOT {tag: %d}\n", JSON_KEY, tok.tag);
    expect_token(tok, JSON_KEY);
    if (strcmp(tok.key, "message") != 0)
    {
        debug_log(ERROR, "Got fire data with wrong layout");
        json_token_free(tok);
        return false;
    }
    json_token_free(tok);

    tok = json_lexer_next(&lex);
    expect_token(tok, JSON_STRING_VAL);
    if (strcmp(tok.string_val, "success") != 0)
    {
        debug_log(ERROR, "Didn't get 'success' response from firedata, got: '%s'", tok.string_val);
        json_token_free(tok);
        return false;
    }
    json_token_free(tok);

    tok = json_lexer_next(&lex);
    expect_token(tok, JSON_KEY);
    if (strcmp(tok.key, "data") != 0)
    {
        debug_log(ERROR, "Got fire data with wrong layout");
        json_token_free(tok);
    }
    json_token_free(tok);
    expect_token_and_free(json_lexer_next(&lex), JSON_OPEN_LIST);

    while (true) // FIRE OBJECT LIST
    {
        tok = json_lexer_next(&lex);
        if (tok.tag == JSON_CLOSE_LIST)
            break; // since token is JSON_CLOSE_LIST there is no need to free it

        expect_token(tok, JSON_OPEN_OBJ);

        FireArea fire = {0};
        while (true) // FIRE OBJECT FIELDS
        {
            tok = json_lexer_next(&lex);
            if (tok.tag == JSON_CLOSE_OBJ)
                break;

            expect_token(tok, JSON_KEY);

            if (strcmp(tok.key, "lat") == 0)
            {
                json_token_free(tok);

                tok = json_lexer_next(&lex);
                expect_token(tok, JSON_NUMBER_VAL);
                fire.gcoord.lat = tok.number_val;
                json_token_free(tok);
                continue;
            }
            else if (strcmp(tok.key, "lng") == 0)
            {
                json_token_free(tok);

                tok = json_lexer_next(&lex);
                expect_token(tok, JSON_NUMBER_VAL);
                fire.gcoord.lon = tok.number_val;
                json_token_free(tok);
                continue;
            }
            else if (strcmp(tok.key, "frp") == 0)
            {
                json_token_free(tok);

                tok = json_lexer_next(&lex);
                expect_token(tok, JSON_NUMBER_VAL);
                fire.frp = tok.number_val;
                json_token_free(tok);
                continue;
            }
            else if (strcmp(tok.key, "fireCategory") == 0)
            {
                json_token_free(tok);

                tok = json_lexer_next(&lex);
                expect_token(tok, JSON_STRING_VAL);
                String category = str_from(tok.string_val);
                fire.category = str_clone_chars(category);
                str_free(&category);
                json_token_free(tok);
            }
            else
            {
                json_token_free(tok);
                tok = json_lexer_next(&lex); // Skip the next value
                json_token_free(tok);
            }
        }

        vec_push(&fires, fire);
    }

    *fire_data = (FireSlice)vec_owned_slice(fires);

    vec_free(fires);

    return true;
}
