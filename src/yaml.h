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
