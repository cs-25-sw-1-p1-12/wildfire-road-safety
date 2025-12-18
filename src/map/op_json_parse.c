#include "op_json_parse.h"

#include "../dyn.h"
#include "../models/road.h"
#include "json_lexer.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KMH_TO_MS 0.2777778
#define MPH_TO_MS 0.44704

typedef VecDef(size_t) IdxVec;
typedef SliceDef(size_t) IdxSlice;

typedef VecDef(RoadSeg) RoadVec;
typedef VecDef(RoadNode) RoadNodeVec;

typedef struct
{
    const char* key;
    const char* val;
} OpTag;

typedef SliceDef(OpTag) OpTagSlice;
typedef VecDef(OpTag) OpTagVec;

typedef struct
{
    size_t id;
    GCoord coords;
    OpTagSlice tags;
} OpNode;

typedef SliceDef(OpNode) OpNodeSlice;
typedef VecDef(OpNode) OpNodeVec;

typedef struct
{
    size_t id;
    IdxSlice nodes;
    OpTagSlice tags;
} OpWay;

typedef SliceDef(OpWay) OpWaySlice;
typedef VecDef(OpWay) OpWayVec;

void free_op_node(OpNode node);
void free_op_way(OpWay way);
void free_op_tag(OpTag tag);

bool generic_json_parse(char* input, OpNodeSlice* node_buf, OpWaySlice* way_buf);

bool find_node(OpNodeVec* nodes, size_t id, size_t* idx);
bool parse_node(JsonLexer* lex, RoadNode* node);
bool parse_road(JsonLexer* lex, RoadNodeVec* nodes, RoadSeg* road);
bool parse_tags(JsonLexer* lex, OpTagVec* tags);
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

bool road_json_parse(char* input, RoadSegSlice* road_data)
{
    OpNodeSlice nodes = {0};
    OpWaySlice ways = {0};

    if (!generic_json_parse(input, &nodes, &ways))
        return false;

    RoadVec roads = vec_with_capacity(RoadVec, ways.len);
    for (size_t i = 0; i < ways.len; i++)
    {
        OpWay op_way = ways.items[i];

        RoadNodeVec inner_nodes = {0};

        // ni = node index
        for (size_t ni = 0; ni < op_way.nodes.len; ni++)
        {
            OpNode op_node = nodes.items[op_way.nodes.items[ni]];

            RoadNode r_node = {
                .id = op_node.id,
                .coords = op_node.coords,
            };

            vec_push(&inner_nodes, r_node);
        }

        // ti = tag index
        String material = {0};
        String name = {0};
        double speed_limit = 0;
        for (size_t ti = 0; ti < op_way.tags.len; ti++)
        {
            OpTag tag = op_way.tags.items[ti];
            if (strcmp(tag.key, "surface") == 0)
                str_append(&material, tag.val);

            if (strcmp(tag.key, "name") == 0)
                str_append(&name, tag.val);

            if (strcmp(tag.key, "maxspeed") == 0)
            {
                if (strcmp(tag.val, "none") == 0)
                    continue;

                else if (strcmp(tag.val, "walk") == 0)
                {
                    speed_limit = 4.166667;
                    continue;
                }

                char* endptr = NULL;
                double limit = strtod(tag.val, &endptr);
                if (*endptr == '\0')
                    speed_limit = limit * KMH_TO_MS;

                else if (*endptr == ' ' && strcmp(endptr + 1, "mph"))
                    speed_limit = limit * MPH_TO_MS;
            }
        }

        RoadSeg road = {
            .id = op_way.id,
            .nodes = vec_owned_slice(inner_nodes),
            .risk = 0,
            .risk_reason = 0,
            .speed_limit = speed_limit,
            .name = str_owned_slice(name).chars,
            .material = str_owned_slice(material).chars,
        };

        vec_free(inner_nodes);
        str_free(&material);
        str_free(&name);

        vec_push(&roads, road);
    }

    *road_data = (RoadSegSlice)vec_owned_slice(roads);

    for (size_t i = 0; i < nodes.len; i++)
        free_op_node(nodes.items[i]);

    for (size_t i = 0; i < ways.len; i++)
        free_op_way(ways.items[i]);

    slice_free(&nodes);
    slice_free(&ways);

    vec_free(roads);

    return true;
}

typedef VecDef(VegData) VegVec;

bool vegetation_json_parse(char* input, VegSlice* veg_data)
{
    OpNodeSlice nodes = {0};
    OpWaySlice ways = {0};

    if (!generic_json_parse(input, &nodes, &ways))
        return false;

    VegVec vegetation = {0};
    for (size_t i = 0; i < ways.len; i++)
    {
        OpWay op_way = ways.items[i];

        GPoly vertices = {0};

        // ni = node index
        for (size_t ni = 0; ni < op_way.nodes.len; ni++)
        {
            OpNode op_node = nodes.items[op_way.nodes.items[ni]];

            vec_push(&vertices, op_node.coords);
        }

        // ti = tag index
        VegType veg_type = VEG_NONE;
        for (size_t ti = 0; ti < op_way.tags.len; ti++)
        {
            OpTag tag = op_way.tags.items[ti];
            if (strcmp(tag.key, "landuse") == 0)
            {
                if (strcmp(tag.val, "forest") == 0)
                    veg_type = VEG_FOREST;

                if (strcmp(tag.val, "plant_nursery") == 0 || strcmp(tag.val, "vineyard") == 0)
                    veg_type = VEG_SHRUBLAND;

                else if (strcmp(tag.val, "farmland") == 0 ||
                         strcmp(tag.val, "greenhouse_horticulture") == 0)
                    veg_type = VEG_FARMLAND;

                else if (strcmp(tag.val, "grass") == 0 || strcmp(tag.val, "grassland") == 0 ||
                         strcmp(tag.val, "greenery") == 0 ||
                         strcmp(tag.val, "village_green") == 0 || strcmp(tag.val, "meadow") == 0 ||
                         strcmp(tag.val, "greenfield") == 0 || strcmp(tag.val, "brownfield") == 0)
                    veg_type = VEG_GRASS;

                else if (strcmp(tag.val, "residential") == 0 ||
                         strcmp(tag.val, "construction") == 0 || strcmp(tag.val, "commercial") ||
                         strcmp(tag.val, "industrial") == 0 || strcmp(tag.val, "retail") == 0 ||
                         strcmp(tag.val, "garages") == 0)
                    veg_type = VEG_BUILDINGS;

                else if (strcmp(tag.val, "recreation_ground") == 0 ||
                         strcmp(tag.val, "basin") == 0 || strcmp(tag.val, "railway") == 0)
                    veg_type = VEG_NONE;

                else if (strcmp(tag.val, "quarry") == 0 || strcmp(tag.val, "landfill") == 0)
                    veg_type = VEG_ROCK;
            }

            else if (strcmp(tag.key, "natural") == 0)
            {
                if (strcmp(tag.val, "water") == 0 || strcmp(tag.val, "shoal") == 0 ||
                    strcmp(tag.val, "salt_pond") == 0)
                    veg_type = VEG_WATER;

                else if (strcmp(tag.val, "wood") == 0 || strcmp(tag.val, "treerow") == 0 ||
                         strcmp(tag.val, "orchard") == 0)
                    veg_type = VEG_FOREST;

                else if (strcmp(tag.val, "wetland") == 0 || strcmp(tag.val, "mud") == 0)
                    veg_type = VEG_WETLAND;

                else if (strcmp(tag.val, "heath") == 0 || strcmp(tag.val, "scrub") == 0 ||
                         strcmp(tag.val, "shrubbery") == 0)
                    veg_type = VEG_SHRUBLAND;

                else if (strcmp(tag.val, "bare_rock") == 0 || strcmp(tag.val, "scree") == 0 ||
                         strcmp(tag.val, "shingle") == 0)
                    veg_type = VEG_ROCK;

                else if (strcmp(tag.val, "beach") == 0 || strcmp(tag.val, "sand") == 0)
                    veg_type = VEG_SAND;
            }

            else if (strcmp(tag.key, "leisure") == 0)
            {
                if (strcmp(tag.val, "garden") == 0 || strcmp(tag.val, "park") == 0)
                    veg_type = VEG_GRASS;
            }

            else if (strcmp(tag.key, "barrier") == 0)
            {
                if (strcmp(tag.val, "hedge") == 0)
                    veg_type = VEG_SHRUBLAND;
            }

            else if (strcmp(tag.key, "surface") == 0)
            {
                if (strcmp(tag.val, "gravel") == 0)
                    veg_type = VEG_ROCK;
            }

            else if (strcmp(tag.key, "amenity") == 0)
            {
                if (strcmp(tag.val, "parking") == 0 || strcmp(tag.val, "motorcycle_parking") == 0 ||
                    strcmp(tag.val, "bicycle_parking") == 0)
                    veg_type = VEG_NONE;
            }
        }

        VegData data = {
            .id = op_way.id,
            .type = veg_type,
            .area = vec_owned_slice(vertices),
        };

        vec_free(vertices);
        vec_push(&vegetation, data);
    }

    *veg_data = (VegSlice)vec_owned_slice(vegetation);

    for (size_t i = 0; i < nodes.len; i++)
        free_op_node(nodes.items[i]);

    for (size_t i = 0; i < ways.len; i++)
        free_op_way(ways.items[i]);

    slice_free(&nodes);
    slice_free(&ways);

    vec_free(vegetation);

    return true;
}

bool fires_json_parse(char* input, FireSlice* fire_data)
{
    OpNodeSlice nodes = {0};
    OpWaySlice ways = {0};

    if (!generic_json_parse(input, &nodes, &ways))
        return false;

    FireVec fires = vec_with_capacity(FireVec, ways.len);
    for (size_t i = 0; i < ways.len; i++)
    {
        OpWay op_way = ways.items[i];

        // ti = tag index
        GCoord gcoord = {0};
        double temperature = 0.0;
        double weatherIndex = 0.0;
        String category = {0};
        for (size_t ti = 0; ti < op_way.tags.len; ti++)
        {
            OpTag tag = op_way.tags.items[ti];
            if (strcmp(tag.key, "lat") == 0)
                gcoord.lat = strtod(tag.val, NULL);

            if (strcmp(tag.key, "lng") == 0)
                gcoord.lon = strtod(tag.val, NULL);

            if (strcmp(tag.key, "temperature") == 0)
                temperature = strtod(tag.val, NULL);

            if (strcmp(tag.key, "weatherIndex") == 0)
                weatherIndex = strtod(tag.val, NULL);

            if (strcmp(tag.key, "fireCategory") == 0)
                str_append(&category, tag.val);
        }



        FireArea fire = {
            .gcoord = gcoord,
            .temperature = temperature,
            .weatherIndex = weatherIndex,
            .category = category.chars
        };

        str_free(&category);

        vec_push(&fires, fire);
    }

    *fire_data = (FireSlice)vec_owned_slice(fires);

    for (size_t i = 0; i < nodes.len; i++)
        free_op_node(nodes.items[i]);

    for (size_t i = 0; i < ways.len; i++)
        free_op_way(ways.items[i]);

    slice_free(&nodes);
    slice_free(&ways);

    vec_free(fires);

    return true;
}

bool generic_json_parse(char* input, OpNodeSlice* node_buf, OpWaySlice* way_buf)
{
    JsonLexer lex = (JsonLexer){
        .buf = input,
        .idx = 0,
    };

    OpWayVec ways = {0};
    OpNodeVec nodes = {0};

    expect_token_and_free(json_lexer_next(&lex), JSON_OPEN_OBJ);

    // skip to the node list
    JsonToken tok = {0};

    while (tok.tag != JSON_OPEN_LIST && tok.tag != JSON_EOF)
        tok = json_lexer_next(&lex);

    while (lex.idx < strlen(lex.buf))
    {
        JsonToken tok = json_lexer_next(&lex);

        if (tok.tag == JSON_CLOSE_LIST || tok.tag == JSON_EOF)
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

            OpTagVec tags = {0};
            JsonToken peeked = json_lexer_peek(lex);
            if (peeked.tag == JSON_KEY && strcmp(peeked.key, "tags") == 0)
            {
                if (!parse_tags(&lex, &tags))
                {
                    json_token_free(peeked);
                    vec_free(tags);
                    return false;
                }
            }
            json_token_free(peeked);

            OpNode node = {
                .id = id,
                .coords = {.lat = lat, .lon = lon},
                .tags = vec_owned_slice(tags),
            };
            vec_free(tags);

            vec_push(&nodes, node);

            skip_object(&lex); // Make sure the next token is from the next object
            continue;
        }
        else if (strcmp(tok.string_val, "way") == 0)
        {
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

            IdxVec inner_nodes = {0};
            while (tok.tag != JSON_CLOSE_LIST)
            {
                tok = json_lexer_next(&lex);
                if (tok.tag == JSON_CLOSE_LIST)
                    break;

                expect_token(tok, JSON_NUMBER_VAL);

                size_t idx;
                if (!find_node(&nodes, tok.number_val, &idx))
                {
                    tok = json_lexer_next(&lex); // Skip the , or reach ]
                    continue;
                }

                vec_push(&inner_nodes, idx);
            }

            OpTagVec tags = {0};
            JsonToken peeked = json_lexer_peek(lex);
            if (peeked.tag == JSON_KEY && strcmp(peeked.key, "tags") == 0)
            {
                if (!parse_tags(&lex, &tags))
                {
                    json_token_free(peeked);
                    vec_free(tags);
                    return false;
                }
            }
            json_token_free(peeked);

            OpWay way = {
                .id = id,
                .nodes = vec_owned_slice(inner_nodes),
                .tags = vec_owned_slice(tags),
            };

            vec_free(inner_nodes);
            vec_free(tags);
            vec_push(&ways, way);

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

    *way_buf = (OpWaySlice)vec_owned_slice(ways);
    *node_buf = (OpNodeSlice)vec_owned_slice(nodes);

    vec_free(ways);
    vec_free(nodes);

    return true;
}

// {
//   "type": "node",
//   "id": 13204742634,
//   "lat": 57.0115201,
//   "lon": 9.9977530
// },

bool parse_node(JsonLexer* lex, RoadNode* node);

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
bool parse_road(JsonLexer* lex, RoadNodeVec* nodes, RoadSeg* road);

bool parse_tags(JsonLexer* lex, OpTagVec* tags)
{
    JsonToken tok = json_lexer_next(lex);

    if (tok.tag != JSON_KEY || strcmp(tok.key, "tags") != 0)
        return false;

    tok = json_lexer_next(lex);
    if (tok.tag != JSON_OPEN_OBJ)
        return true;

    while (tok.tag != JSON_CLOSE_LIST)
    {
        tok = json_lexer_next(lex);
        if (tok.tag == JSON_CLOSE_OBJ)
            break;

        expect_token(tok, JSON_KEY);
        String key_str = str_from(tok.key);
        json_token_free(tok);

        String value_str = {0};
        tok = json_lexer_next(lex);
        switch (tok.tag)
        {
            case JSON_NUMBER_VAL:
                str_appendf(&value_str, "%lf", tok.number_val);
                break;
            case JSON_BOOL_VAL:
                str_append(&value_str, tok.bool_val ? "true" : "false");
                break;
            case JSON_STRING_VAL:
                str_append(&value_str, tok.string_val);
                break;
            default:
                debug_log(ERROR, "Found invalid data value in json tag section: %d\n", tok.tag);
                str_free(&key_str);
                str_free(&value_str);
                json_token_free(tok);
                return false;
        }


        OpTag tag = {
            .key = str_owned_slice(key_str).chars,
            .val = str_owned_slice(value_str).chars,
        };

        str_free(&key_str);
        str_free(&value_str);
        json_token_free(tok);

        vec_push(tags, tag);
    }

    return true;
}

bool find_node(OpNodeVec* nodes, size_t id, size_t* idx)
{
    for (size_t i = 0; i < nodes->len; i++)
    {
        OpNode node = nodes->items[i];
        if (node.id == id)
        {
            *idx = i;
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

void free_op_node(OpNode node)
{
    for (size_t i = 0; i < node.tags.len; i++)
        free_op_tag(node.tags.items[i]);

    slice_free(&node.tags);
}

void free_op_way(OpWay way)
{
    for (size_t i = 0; i < way.tags.len; i++)
    {
        free_op_tag(way.tags.items[i]);
    }
    slice_free(&way.tags);
    slice_free(&way.nodes);
}

void free_op_tag(OpTag tag)
{
    free((void*)tag.key);
    free((void*)tag.val);
}
