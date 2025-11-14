#include "json_parse.h"

#include "../dyn.h"
#include "../models/road.h"
#include "json_lexer.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef VecDef(size_t) IndexVec;
typedef VecDef(RoadSeg) RoadVec;
typedef VecDef(Node) NodeVec;

bool find_node(NodeVec* nodes, size_t id, Node* out);
bool parse_node(JsonLexer* lex, Node* node);
bool parse_road(JsonLexer* lex, NodeVec* nodes, RoadSeg* road);
void skip_object(JsonLexer* lex);

#define EXPECT_KEY_VALUE(out, lex, key, value_tag)                                             \
    {                                                                                          \
        JsonToken tok = json_lexer_next((lex));                                                \
        expect_token(tok, JSON_KEY);                                                           \
        assert(strcmp(tok.key, (key)) == 0 && "Expected first key in json-object to be \"" key \
                                              "\"");                                           \
        json_token_free(tok);                                                                  \
                                                                                               \
        tok = json_lexer_next((lex));                                                          \
        expect_token(tok, value_tag);                                                          \
        JSON_TOKEN_GET_BY_TAG((out), tok, (value_tag))                                         \
    }

bool parse(char* input, RoadSegSlice* road_buf)
{
    JsonLexer lex = (JsonLexer){
        .buf = input,
        .idx = 0,
    };

    RoadVec roads = {0};
    NodeVec nodes = {0};

    expect_token_and_free(json_lexer_next(&lex), JSON_OPEN_OBJ);

    // skip to the node list
    JsonToken tok = {0};
    while (tok.tag != JSON_OPEN_LIST)
        tok = json_lexer_next(&lex);

    while (lex.idx < strlen(lex.buf))
    {
        JsonToken tok = json_lexer_next(&lex);
        if (tok.tag == JSON_CLOSE_LIST)
            break;

        expect_token_and_free(tok, JSON_OPEN_OBJ);

        tok = json_lexer_next(&lex);
        if (tok.tag == JSON_EOF)
            break;

        expect_token(tok, JSON_KEY);
        assert(strcmp(tok.key, "type") == 0 && "Expected first key in json-object to be \"type\"");
        json_token_free(tok);


        tok = json_lexer_next(&lex);
        expect_token(tok, JSON_STRING_VAL);

        // NODE
        if (strcmp(tok.string_val, "node") == 0)
        {
            // ID
            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_KEY);
            assert(strcmp(tok.key, "id") == 0 && "Expected \"id\" key in json-object");
            json_token_free(tok);

            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_NUMBER_VAL);
            size_t id = (size_t)lround(tok.number_val);
            json_token_free(tok);

            // LAT
            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_KEY);
            assert(strcmp(tok.key, "lat") == 0 && "Expected \"lat\" key in json-object");
            json_token_free(tok);

            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_NUMBER_VAL);
            json_number_t lat = tok.number_val;
            json_token_free(tok);

            // LON
            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_KEY);
            assert(strcmp(tok.key, "lon") == 0 && "Expected \"lon\" key in json-object");
            json_token_free(tok);

            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_NUMBER_VAL);
            double lon = tok.number_val;
            json_token_free(tok);

            Node node = (Node){
                .id = id, .coords = {.lat = lat, .lon = lon}
            };

            vec_push(&nodes, node);
            skip_object(&lex); // tags and other data
            continue;
        }
        else if (strcmp(tok.string_val, "way") == 0)
        {
            // ID
            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_KEY);
            assert(strcmp(tok.key, "id") == 0 && "Expected \"id\" key in json-object");
            json_token_free(tok);

            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_NUMBER_VAL);
            size_t id = (size_t)lround(tok.number_val);
            json_token_free(tok);

            // NODES
            tok = json_lexer_next(&lex);
            expect_token(tok, JSON_KEY);
            assert(strcmp(tok.key, "nodes") == 0 && "Expected \"nodes\" key in json-object");
            json_token_free(tok);
            expect_token_and_free(json_lexer_next(&lex), JSON_OPEN_LIST);

            NodeVec inner_nodes = {0};
            while (tok.tag != JSON_CLOSE_LIST)
            {
                tok = json_lexer_next(&lex);
                if (tok.tag == JSON_CLOSE_LIST)
                    break;

                expect_token(tok, JSON_NUMBER_VAL);

                Node node = {0};
                if (!find_node(&nodes, tok.number_val, &node))
                {
                    tok = json_lexer_next(&lex); // Skip the , or reach ]
                    continue;
                }

                vec_push(&inner_nodes, node);
            }

            RoadSeg road = (RoadSeg){
                .id = id,
                .nodes = (NodeSlice)vec_owned_slice(inner_nodes),
                .name = NULL,
                .material = NULL,
            };

            vec_free(inner_nodes);
            vec_push(&roads, road);
            skip_object(&lex); // tags and other data
            continue;
        }
        else
        {
            // LogWarnf("Expected json-object to have \"type\" value of either \"node\" or \"way\",
            // "
            //          "got \"%s\"\n",
            //          tok.string_val);
            skip_object(&lex);
            continue;
        }
    }

    *road_buf = (RoadSegSlice)vec_owned_slice(roads);

    vec_free(roads);
    vec_free(nodes);

    return true;
}

// {
//   "type": "node",
//   "id": 13204742634,
//   "lat": 57.0115201,
//   "lon": 9.9977530
// },

bool parse_node(JsonLexer* lex, Node* node);

// {
//   "type": "way",
//   "id": 1442543522,
//   "nodes": [
//     12141798162,
//     12141798161,
//     12141798160,
//     12141798159,
//     6217324826
//   ],
//   "tags": {
//     "highway": "cycleway",
//     "oneway": "yes",
//     "surface": "asphalt"
//   }
// },
bool parse_road(JsonLexer* lex, NodeVec* nodes, RoadSeg* road);

bool find_node(NodeVec* nodes, size_t id, Node* out)
{
    for (size_t i = 0; i < nodes->len; i++)
    {
        Node node = nodes->items[i];
        if (node.id == id)
        {
            *out = nodes->items[i];
            return true;
        }
    }
    return false;
}

void skip_object(JsonLexer* lex)
{
    // Assuming that the first { has been consumed
    uint32_t bracket_balance = 1;

    while (bracket_balance > 0)
    {
        JsonToken tok = json_lexer_next(lex);

        if (tok.tag == JSON_OPEN_OBJ)
            bracket_balance += 1;

        if (tok.tag == JSON_CLOSE_OBJ)
            bracket_balance -= 1;

        if (tok.tag == JSON_EOF)
            break;

        json_token_free(tok);
    }
}
