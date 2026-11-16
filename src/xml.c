#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cobia.h"
#include "xml.h"

#define PLACEHOLDER BAD_CAST "__item"

char* get_nvram_path(const char *name)
{
    char *nvram = (char *)malloc(KVSIZE);
    snprintf(nvram, KVSIZE, "/var/lib/libvirt/qemu/nvram/%s_VARS.qcow2", name);
    return nvram;
}

char *xpath_get(xmlDocPtr doc, const char *path)
{
    char *result = (char *)malloc(KVSIZE);
    xmlXPathContext *ctx = xmlXPathNewContext(doc);
    xmlXPathObject *obj = xmlXPathEvalExpression((const xmlChar*)path, ctx);

    if (obj && obj->nodesetval && obj->nodesetval->nodeNr > 0) {
        xmlChar *s = xmlNodeGetContent(obj->nodesetval->nodeTab[0]);
        if (s) {
            strcpy(result, (char *)s);
            xmlFree(s);
        }
    }

    xmlXPathFreeObject(obj);
    xmlXPathFreeContext(ctx);
    return result;
}

int get_sequence_index(xmlNodePtr node, int *id)
{
    *id = 0;
    int i = 1;
    for (xmlNodePtr prev = node->prev; prev; prev = prev->prev) {
        if (prev->type == XML_ELEMENT_NODE && xmlStrEqual(prev->name, node->name)) {
            ++i;
            ++(*id);
        }
    }

    for (xmlNodePtr next = node->next; next; next = next->next)
        if (next->type == XML_ELEMENT_NODE && xmlStrEqual(next->name, node->name))
            ++i;

    return i;
}

void join_parts(const char *parts[KVSIZE], int n, const char *attr, const char *val)
{
    if (TOTAL_KEY_COUNT >= MAXSET) return;

    size_t len = 0;
    char keybuf[PATHSIZE];
    for (size_t i = 0; i < n; ++i)
        len += snprintf(keybuf+len, sizeof(keybuf) - len, "%s%s", i ? "." : "", parts[i]);

    if (attr) snprintf(keybuf+len, sizeof(keybuf) - len, ".@%s", attr);

    settings[TOTAL_KEY_COUNT].key = strdup(keybuf);
    settings[TOTAL_KEY_COUNT].val = strdup(val);
    ++TOTAL_KEY_COUNT;
}

void parse_xml(xmlNodePtr node, const char **parts, int depth)
{
    for (; node; node = node->next ) {
        if (node->type != XML_ELEMENT_NODE)
            continue;

        int id, total = get_sequence_index(node, &id);
        char part[KVSIZE];
        if (total > 1) {
            snprintf(part, sizeof(part), "%s.%d", (char *)node->name, id);
        } else {
            snprintf(part, sizeof(part), "%s", (char *)node->name);
        }

        parts[depth] = part;

        for (xmlAttrPtr attr = node->properties; attr; attr = attr->next) {
            xmlChar *prop = xmlGetProp(node, attr->name);
            join_parts(parts, depth+1, (char *)attr->name, prop ? (char *)prop : "");
            xmlFree(prop);
        }

        xmlNodePtr children = node->children;
        while(children && children->type != XML_ELEMENT_NODE)
            children = children->next;

        if (!children) {
            xmlChar *str = xmlNodeGetContent(node);
            join_parts(parts, depth+1, NULL, str ? (char *)str : "");
            xmlFree(str);
        }

        parse_xml(node->children, parts, depth + 1);
    }
}

int is_index(const char *str)
{
    if (*str == '\0') return 0;

    for (; *str; ++str)
        if (!isdigit((unsigned char)*str)) return 0;

    return 1;
}

xmlNodePtr get_sequence_element(xmlNodePtr parent, long n)
{
    long seen = 0;
    for (xmlNodePtr current = parent->children; current; current = current->next) {
        if (current->type != XML_ELEMENT_NODE)
            continue;

        if (seen == n)
            return current;

        ++seen;
    }

    while (seen <= n) {
        xmlNodePtr placeholder = xmlNewChild(parent, NULL, PLACEHOLDER, NULL);
        if (!placeholder)
            return NULL;

        if (seen == n)
            return placeholder;

        ++seen;
    }

    return NULL;
}

xmlNodePtr get_child(xmlNodePtr parent, const char *name)
{
    for (xmlNodePtr current = parent->children; current; current = current->next) {
        if (current->type == XML_ELEMENT_NODE &&
                xmlStrcmp(current->name, BAD_CAST name) == 0)
            return current;
    }

    return NULL;
}

xmlNodePtr walk_path(xmlDocPtr doc, const char *path, const char *val)
{
    char buf[KVSIZE * MAXDEPTH];
    snprintf(buf, sizeof(buf), "%s", path);

    xmlNodePtr current = xmlDocGetRootElement(doc);
    char *tok = strtok(buf, ".");

    if (tok && !current) {
        if (tok[0] == '@' || tok[0] == '#' || is_index(tok))
            return NULL;

        current = xmlNewNode(NULL, BAD_CAST tok);
        if (!current) return NULL;

        xmlDocSetRootElement(doc, current);
        tok = strtok(NULL, ".");
    } else if (tok && xmlStrcmp(current->name, BAD_CAST tok) == 0) {
        tok = strtok(NULL, ".");
    }

    for (; tok; tok = strtok(NULL, ".")) {
        if (tok[0] == '@') {
            xmlSetProp(current, BAD_CAST tok+1, BAD_CAST val);
            return NULL;
        }

        if (tok[0] == '#') return current;

        if (is_index(tok)) {
            current = get_sequence_element(current, strtol(tok, NULL, 10));
            if (!current)
                return NULL;

            continue;
        }

        if (xmlStrcmp(current->name, PLACEHOLDER) == 0) {
            xmlNodeSetName(current, BAD_CAST tok);
        } else {
            xmlNodePtr next = get_child(current, tok);
            if (!next)
                next = xmlNewChild(current, NULL, BAD_CAST tok, NULL);

            if (!next)
                return NULL;

            current = next;
        }
    }

    return current;
}

xmlDocPtr convert_to_xml(void)
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");

    for (int i = 0; i < TOTAL_KEY_COUNT; ++i) {
        xmlNodePtr leaf = walk_path(doc, settings[i].key, settings[i].val);
        if (!leaf) continue;

        if (leaf->children) {
            xmlNodePtr rep = xmlNewChild(leaf->parent, NULL, leaf->name, NULL);
            xmlNodeAddContent(rep, BAD_CAST settings[i].val);
        } else {
            xmlNodeAddContent(leaf, BAD_CAST settings[i].val);
        }
    }

    return doc;
}
