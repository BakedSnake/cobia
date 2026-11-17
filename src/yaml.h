#include <yaml.h>

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
 * convert_to_yaml()
 *
 * Walks down dot notation path from  settings  and
 * parses it into yaml.
 * This function is meant for when we  are  parsing
 * from xml into yaml.
 *
 */

int convert_to_yaml(void);
