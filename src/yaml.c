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

        //fprintf(stdout, "%s: %s\n", settings[i].key, settings[i].val);
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

dot_path_t parse_dot_path(const char *key)
{
    dot_path_t path = { .buf = strdup(key) };

    for (char *tok = strtok(path.buf, ".");
            tok && path.len < MAXDEPTH; tok = strtok(NULL, "."))
        path.tok[path.len++] = tok;

    return path;
}

int get_items_in_path(dot_path_t prev, dot_path_t path, int depth)
{
    int i = 0;
    while (i < depth && i < path.len && !strcmp(prev.tok[i], path.tok[i]))
        ++i;

    return i;
}

int is_attr(const char *tok) { return tok[0] == '@'; }

int is_seq(size_t i, dot_path_t *current, int k)
{
    size_t size = sizeof(settings)/sizeof(*settings);
    for (size_t j = i; j < size && settings[j].key; ++j) {
        dot_path_t path = parse_dot_path(settings[j].key);

        int duplicate = path.len > k + 1;
        for (int t = 0; duplicate && t <= k; ++t) {
            duplicate = !strcmp(path.tok[t], current->tok[t]);
        }

        int seq = duplicate && is_index(path.tok[k + 1]);
        free(path.buf);

        if (seq) return 1;
        if (!duplicate) break;
    }

    return 0;
}

void yaml_walk_path(yaml_emitter_t *emitter)
{
    container(emitter, MAP_YAML, 1); /* root */

    char stack[MAXDEPTH];
    int depth = 0;
    dot_path_t prev = {0};

    size_t size = sizeof(settings)/sizeof(*settings);
    for (size_t i = 0; i < size && settings[i].key; ++i) {
        if (i && !strcmp(settings[i].key, settings[i - 1].key))
            continue;

        dot_path_t current = parse_dot_path(settings[i].key);
        if (current.len == 0) {
            free(current.buf);
            current.buf = NULL;
            continue;
        }

        int shared_count = get_items_in_path(prev, current, depth);

        /* emit close mapping down to the current stack */
        for (int k = depth; k-- > shared_count; )
            container(emitter, stack[k], 0);
        depth = shared_count;

        /* emit open mapping up to size of the current buffer */
        for (int k = shared_count; k < current.len - 1; ++k) {
            int type = is_seq(i, &current, k);

            if (!is_index(current.tok[k]))
                scalar(emitter, current.tok[k]);

            container(emitter, type, 1);
            stack[depth++] = type;
        }

        /* emit key and value pairs */
        const char *key_leaf = current.tok[current.len - 1];
        const char *val = settings[i].val ? settings[i].val : "";

        if (!is_index(key_leaf))
            scalar(emitter, key_leaf);

        scalar(emitter, val);

        free(prev.buf);
        prev.buf = NULL;
        prev = current;
    }

    while (depth)
        container(emitter, stack[--depth], 0);

    container(emitter, MAP_YAML, 0); /* close root */
    free(prev.buf);
    prev.buf = NULL;
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

    yaml_walk_path(&emitter);

    yaml_document_end_event_initialize(&event, 1);
    if (!yaml_emitter_emit(&emitter, &event))
        return 1;

    yaml_stream_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event))
        return 1;

    yaml_emitter_delete(&emitter);
    return 0;
}
