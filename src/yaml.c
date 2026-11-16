#include <stdio.h>

#include "yaml.h"
#include "cobia.h"

int DEPTH = 0;
int IS_SEQUENCE[MAXDEPTH]       = {0};
int IS_KEYMAP[MAXDEPTH]         = {0};
int SEQUENCE_ID[MAXDEPTH]       = {0};
char KEYBUF[MAXDEPTH][KVSIZE]   = {0};
char KEYS[MAXKEYS][KVSIZE]      = {0};
char VALUES[MAXKEYS][KVSIZE]    = {0};

void yaml_join_parts(const char *value, int len)
{
    if (DEPTH > 0 && !IS_SEQUENCE[DEPTH - 1] && IS_KEYMAP[DEPTH - 1]) {
        snprintf(KEYBUF[DEPTH - 1], KVSIZE, "%.*s", len, value);
        IS_KEYMAP[DEPTH - 1] = 0;
    } else {
        if (TOTAL_KEY_COUNT < MAXKEYS) {
            char path[KVSIZE * MAXDEPTH] = {0};
            for (int d = 0; d < DEPTH; ++d) {
                if (d > 0)
                    strncat(path, ".", sizeof(path) - strlen(path) - 1);
    
                if (IS_SEQUENCE[d]) {
                    char n[16];
                    snprintf(n, sizeof(n), "%d", SEQUENCE_ID[d]);
                    strncat(path, n, sizeof(path) - strlen(path) - 1);
                } else {
                    strncat(path, KEYBUF[d], sizeof(path) - strlen(path) - 1);
                }
            }
    
            snprintf(KEYS[TOTAL_KEY_COUNT], KVSIZE, "%s", path);
            snprintf(VALUES[TOTAL_KEY_COUNT], KVSIZE, "%.*s", len, value);
            ++TOTAL_KEY_COUNT;
        }
    
        if (DEPTH > 0) {
            if (IS_SEQUENCE[DEPTH - 1]) {
                ++SEQUENCE_ID[DEPTH - 1];
            } else {
                IS_KEYMAP[DEPTH - 1] = 1;
                KEYBUF[DEPTH - 1][0] = '\0';
            }
        }
    }
}

void parse_yaml(const char *filepath)
{
    FILE *fp = fopen(filepath, "rb");
    yaml_parser_t parser;
    yaml_event_t event;

    if (!yaml_parser_initialize(&parser))
        fprintf(stderr, "Failed to init yaml parser.\n");

    if (fp == NULL)
        fprintf(stderr, "Failed to open file\n");

    yaml_parser_set_input_file(&parser, fp);
    while(1) {
        if (!yaml_parser_parse(&parser, &event))
            break;

        if (event.type == YAML_MAPPING_START_EVENT) {
            IS_SEQUENCE[DEPTH] = 0;
            IS_KEYMAP[DEPTH] = 1;
            ++DEPTH;
        }

        if (event.type == YAML_SEQUENCE_START_EVENT) {
            IS_SEQUENCE[DEPTH] = 1;
            SEQUENCE_ID[DEPTH] = 0;
            ++DEPTH;
        }

        if (event.type == YAML_MAPPING_END_EVENT ||
            event.type == YAML_SEQUENCE_END_EVENT) {
            --DEPTH;
            if (DEPTH > 0) {
                if (IS_SEQUENCE[DEPTH - 1]) {
                    ++SEQUENCE_ID[DEPTH - 1];
                } else {
                    IS_KEYMAP[DEPTH - 1] = 1;
                    KEYBUF[DEPTH - 1][0] = '\0';
                }
            }
        }

        if (event.type == YAML_SCALAR_EVENT) {
            char *value = (char *)event.data.scalar.value;
            int len = event.data.scalar.length;
            yaml_join_parts(value, len);
        }

        if (event.type == YAML_STREAM_END_EVENT)
            break;

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(fp);

    // Assign parsed values
    for (int i = 0; i < TOTAL_KEY_COUNT; ++i) {
        settings[i].key = malloc(KVSIZE);
        settings[i].val = malloc(KVSIZE);
        memcpy(settings[i].key, KEYS[i], KVSIZE);
        memcpy(settings[i].val, VALUES[i], KVSIZE);

        fprintf(stdout, "%s: %s\n", settings[i].key, settings[i].val);
    }
}
