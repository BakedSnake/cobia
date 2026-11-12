#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

char* xpath_get(xmlDocPtr doc, const char *path);

char* get_nvram_path(const char* name);

xmlNodePtr get_child(xmlNodePtr parent, const char *name);

xmlNodePtr walk_path(xmlDocPtr doc, const char *path, const char *val);

xmlDocPtr convert_to_xml(void);
