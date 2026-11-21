#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xmlmemory.h>

/*
 * get_nvram_path()
 *
 * Returns nvram path.
 * For now this is used for debuggin  purposes  and
 * may be dropped in the future.
 *
 */

char *get_nvram_path(const char* name);

/*
 * node_has_attrs()
 *
 * Checks if an xml node contains  any  attributes.
 * Returns 1 if the node  contains  any  attributes
 * and 0 if there aren't any.
 *
 */

int node_has_attrs(xmlNodePtr item);

/*
 * node_has_text()
 *
 * Checks if an xml node contains any  text  value.
 * Returns 1 if the  node  contains  a  valid  text
 * value and 0 if there is no value.
 *
 */

int node_has_text(xmlNodePtr item);

/*
 * node_has_dup()
 *
 * Checks  if  an  xml  node  has  any  duplicates,
 * meaning if the node is part  of  a  sequence  or
 * not.
 * Returns 1 if the node has a duplicate and  0  if
 * not.
 *
 */

int node_has_dup(xmlNodePtr item);

/*
 * tag_is_ignored()
 *
 * Checks if for blacklisted tags.
 * Returns 1 if the current tag is blacklisted  and
 * 0 if not.
 *
 */

int tag_is_ignored(const xmlChar *name);

/*
 * dotnot_join_parts()
 *
 * Joins nested paths into dot notation, adding the
 * keys  and   the   values   to   the    settings.
 *
 */

void dotnot_join_parts(const char *parts[256],
        int n, const char *attr, const char *val);

/*
 * parse_xml()
 *
 * Walks down  from  the  provided  root  node  and
 * parses   xml   into   our     settings    array.
 * This function walks down the  tree  recursively.
 *
 */

void parse_xml(xmlNodePtr node, const char *path[256], int depth);

/*
 * xpath_get()
 *
 * Returns xml value from xpath.
 * Uses the ctx of a xml doc to evaluate  the  path
 * expression, then  grabs  the  content  from  the
 * node.
 *
 */

char* xpath_get(xmlDocPtr doc, const char *path);

/*
 * is_index()
 *
 * Returns 1 if the provided string is  a  sequence
 * index, otherwise returns 0.
 * In order to corretly parse from our dot notation
 * settings  we   must   correctly    account   for
 * sequences. To do that we check if  the  provided
 * string   (  key  path  part  )  is   an   index.
 *
 */

int is_index(const char *str);

/*
 * get_sequence_element()
 *
 * Returns  an  xml  node  pointer  to  a  sequence
 * element.
 * Looks within a sequence for  a  valid  node  and
 * returns it.
 *
 */

xmlNodePtr get_sequence_element(xmlNodePtr parent, long n);

/*
 * get_child()
 *
 * Returns an xml node pointer to the  children  of
 * the provided node ( parent ), if it is  a  valid
 * xml node.
 *
 */

xmlNodePtr get_child(xmlNodePtr parent, const char *name);

/*
 * walk_path()
 *
 * Returns an xml node pointer.
 * Walks down dot notation path from  settings  and
 * parses it into xml.
 * This function is meant for when we  are  parsing
 * from yaml into xml.
 *
 */

xmlNodePtr walk_path(xmlDocPtr doc, const char *path, const char *val);

/*
 * convert_to_xml()
 *
 * Returns an xml document pointer.
 * Using walk_path(), this  function  converts  our
 * dot notation  settings  into  xml  that  we  can
 * provide to libvirt.
 *
 */

xmlDocPtr convert_to_xml(void);
