#include <yaml.h>

#define MAXDEPTH 8

typedef struct {
    char *buf;
    char *tok[MAXDEPTH];
    int len;
} dot_path_t;

enum stype {
    MAP_YAML,
    SEQ_YAML,
};

/*
 * yaml_join_parts()
 *
 * Joins nested paths into dot notation, adding the
 * keys  and   the   values   to   the    settings.
 *
 */

void yaml_join_parts(const char *value, int len);

/*
 * parse_yaml()
 *
 * Walks down yaml  tree  and  parses  the  content
 * into our settings array.
 *
 */

void parse_yaml(const char *filepath);

/*
 * scalar()
 *
 * Emits a scalar when parsing do notation settings
 * into yaml.
 *
 */

int scalar(yaml_emitter_t *emitter, const char *value);

/*
 * container()
 *
 * Emits either a mapping or a  sequence  depending
 * on the provided type, when parsing dot  notation
 * settings into yaml.
 * Open specifies if we  are  opening  or  closing.
 *
 */

int container(yaml_emitter_t *emitter, size_t type, int open);

/*
 * parse_dot_path()
 *
 * Returns dot_path_t.
 * Parses  a  dot  notation  settings   key   path.
 * Used when parsing  dot  notation  settings  into
 * yaml.
 *
 */

dot_path_t parse_dot_path(const char *key);

/*
 * get_items_in_path()
 *
 * Returns the number of  items  in  the  same  key
 * path.
 *
 */

int get_items_in_path(dot_path_t prev, dot_path_t path, int depth);

/*
 * is_attr()
 *
 * Returns 1 if  the  provided  token  is  a  valid
 * attribute,    otherwise      it    returns    0.
 *
 */

int is_attr(const char *tok);

/*
 * is_seq()
 *
 * Returns 1 if the provided  dot  path  key  is  a
 * sequence,    otherwise       it    returns    0.
 *
 */

int is_seq(size_t i, dot_path_t *current, int k);

/*
 * yaml_walk_path()
 *
 * Walks down dot notation path from  settings  and
 * parses it into yaml.
 * This function is meant for when we  are  parsing
 * from xml into yaml.
 *
 */

void yaml_walk_path(yaml_emitter_t *emitter);

/*
 * convert_to_yaml()
 *
 * Returns 0 if successfully  converted,  otherwise
 * it returns 1.
 * Using yaml_walk_path(), this  function  converts
 * dot notation  settings  into yaml  that  we  can
 * provide to the user.
 *
 */

int convert_to_yaml(void);
