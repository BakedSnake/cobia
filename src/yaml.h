#include <yaml.h>

enum ytype {
    DOM_YAML,
    NET_YAML,
};

void parse_yaml(const char *filepath, int type);
