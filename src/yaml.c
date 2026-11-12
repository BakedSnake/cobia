#include <stdio.h>

#include "yaml.h"
#include "cobia.h"

void parse_yaml(const char *filepath)
{
    int depth = 0;
    int is_sequence[MAXDEPTH]       = {0};
    int is_keymap[MAXDEPTH]         = {0};
    int sequence_id[MAXDEPTH]       = {0};
    char keybuf[MAXDEPTH][KVSIZE]   = {0};
    char keys[MAXKEYS][KVSIZE]      = {0};
    char values[MAXKEYS][KVSIZE]    = {0};

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
            is_sequence[depth] = 0;
            is_keymap[depth] = 1;
            ++depth;
        }

        if (event.type == YAML_SEQUENCE_START_EVENT) {
            is_sequence[depth] = 1;
            sequence_id[depth] = 0;
            ++depth;
        }

        if (event.type == YAML_MAPPING_END_EVENT ||
            event.type == YAML_SEQUENCE_END_EVENT) {
            --depth;
            if (depth > 0) {
                if (is_sequence[depth - 1]) {
                    ++sequence_id[depth - 1];
                } else {
                    is_keymap[depth - 1] = 1;
                    keybuf[depth - 1][0] = '\0';
                }
            }
        }

        if (event.type == YAML_SCALAR_EVENT) {
            char *value = (char *)event.data.scalar.value;
            int len = event.data.scalar.length;

            if (depth > 0 && !is_sequence[depth - 1] && is_keymap[depth - 1]) {
                snprintf(keybuf[depth - 1], KVSIZE, "%.*s", len, value);
                is_keymap[depth - 1] = 0;
            } else {
                if (TOTAL_KEY_COUNT < MAXKEYS) {
                    char path[KVSIZE * MAXDEPTH] = {0};
                    for (int d = 0; d < depth; ++d) {
                        if (d > 0)
                            strncat(path, ".", sizeof(path) - strlen(path) - 1);

                        if (is_sequence[d]) {
                            char n[16];
                            snprintf(n, sizeof(n), "%d", sequence_id[d]);
                            strncat(path, n, sizeof(path) - strlen(path) - 1);
                        } else {
                            strncat(path, keybuf[d], sizeof(path) - strlen(path) - 1);
                        }
                    }

                    snprintf(keys[TOTAL_KEY_COUNT], KVSIZE, "%s", path);
                    snprintf(values[TOTAL_KEY_COUNT], KVSIZE, "%.*s", len, value);
                    ++TOTAL_KEY_COUNT;
                }

                if (depth > 0) {
                    if (is_sequence[depth - 1]) {
                        ++sequence_id[depth - 1];
                    } else {
                        is_keymap[depth - 1] = 1;
                        keybuf[depth - 1][0] = '\0';
                    }
                }
            }
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
        memcpy(settings[i].key, keys[i], KVSIZE);
        memcpy(settings[i].val, values[i], KVSIZE);

        fprintf(stdout, "%s: %s\n", settings[i].key, settings[i].val);
    }
}
