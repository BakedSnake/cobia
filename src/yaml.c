#include <stdio.h>

#include "yaml.h"
#include "xml.h"
#include "cobia.h"

int DEPTH = 0;
int IS_SEQUENCE[MAXDEPTH]       = {0};
int IS_KEYMAP[MAXDEPTH]         = {0};
int SEQUENCE_ID[MAXDEPTH]       = {0};
char KEYBUF[MAXDEPTH][KVSIZE]   = {0};
char KEYS[MAXKEYS][KVSIZE]      = {0};
char VALUES[MAXKEYS][KVSIZE]    = {0};

enum stype {
    MAP_YAML,
    SEQ_YAML,
};

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

int scalar(yaml_emitter_t *emitter, const char *value) {
    yaml_event_t event;
    if (!value) value = "";
    yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t *)value, -1,
            1, 1, YAML_ANY_SCALAR_STYLE);

    int rc = yaml_emitter_emit(emitter, &event);
    if (!rc) {
        fprintf(stderr, "scalar(\"%s\") failed %s\n",
                value, emitter->problem);
    }

    return rc;
}

/*
 * a container can either be a mapping or a
 * sequence.
 */

int container(yaml_emitter_t *emitter, size_t type, int open)
{
    yaml_event_t event;

    if (open) {
        if (type == SEQ_YAML) {
            yaml_sequence_start_event_initialize(&event, NULL, NULL, 1,
                    YAML_BLOCK_SEQUENCE_STYLE);
        } else {
            yaml_mapping_start_event_initialize(&event, NULL, NULL, 1,
                    YAML_BLOCK_MAPPING_STYLE);
        }
    } else {
        if (type == SEQ_YAML) {
            yaml_sequence_end_event_initialize(&event);
        } else {
            yaml_mapping_end_event_initialize(&event);
        }
    }

    int rc = yaml_emitter_emit(emitter, &event);
    if (!rc) {
        fprintf(stderr, "contaner(%s, %s) failed: %s\n",
                type == SEQ_YAML ? "seq" : "map", open ? "true" : "false",
                emitter->problem);
    }

    return rc;
}

int convert_to_yaml(void)
{
    yaml_emitter_t emitter;
    yaml_event_t event;
    yaml_emitter_initialize(&emitter);
    yaml_emitter_set_output_file(&emitter, stdout);
    yaml_emitter_set_encoding(&emitter, YAML_UTF8_ENCODING);
    yaml_emitter_set_indent(&emitter, 2);

    yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
    if (!yaml_emitter_emit(&emitter, &event))
        return 1;

    yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 1);
    if (!yaml_emitter_emit(&emitter, &event))
        return 1;

    container(&emitter, MAP_YAML, 1); /* root */

    int depth = 0;
    char openstack[MAXDEPTH];
    char *prevbuf = NULL;
    char *prevparts[MAXDEPTH];
    size_t size = sizeof(settings)/sizeof(*settings);

    for (size_t i = 0; i < size && settings[i].key; ++i) {
        if (i && !strcmp(settings[i].key, settings[i - 1].key))
            continue;

        int nc = 0;
        char *curbuf = strdup(settings[i].key);
        char *curparts[MAXDEPTH];
        for (char *tok = strtok(curbuf, "."); tok && nc < MAXDEPTH; tok = strtok(NULL, "."))
            curparts[nc++] = tok;

        if (nc == 0) {
            free(curbuf);
            continue;
        }

        int current = 0;
        while (current < depth && current < nc && !strcmp(prevparts[current], curparts[current]))
            ++current;

        for (int k = depth; k-- > current; ) {
            container(&emitter, openstack[k], 0);
        }

        depth = current;

        for (int k = current; k < nc - 1; ++k) {
            int type = is_index(curparts[k + 1]) ? SEQ_YAML : MAP_YAML;
            if (!is_index(curparts[k]))
                scalar(&emitter, curparts[k]);

            container(&emitter, type, 1);
            openstack[depth++] = type;
        }

        if (!is_index(curparts[nc - 1]))
            scalar(&emitter, curparts[nc - 1]); /* key */

        scalar(&emitter, settings[i].val ? settings[i].val : ""); /* value */

        free(prevbuf);
        prevbuf = curbuf;
        memcpy(prevparts, curparts, (size_t)nc * sizeof(*curparts));
    }

    while (depth)
        container(&emitter, openstack[--depth], 0);

    container(&emitter, MAP_YAML, 0); /* close root */
    free(prevbuf);

    yaml_document_end_event_initialize(&event, 1);
    if (!yaml_emitter_emit(&emitter, &event))
        return 1;

    yaml_stream_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event))
        return 1;

    yaml_emitter_delete(&emitter);
    return 0;
}
