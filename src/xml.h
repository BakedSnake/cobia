#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

const char* create_domain_xml(xmlBufferPtr buf);

const char* create_network_xml(xmlBufferPtr buf);

char* xpath_get(xmlDocPtr doc, const char *path);

char* get_nvram_path(const char* name);
