#include <curses.h>
#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>

#include "cobia.h"
#include "virt.h"
#include "xml.h"

#define TOPBAR_H 5
#define SIDEBAR_W 30

enum selected {
    DOMS,
    NETS
};

int HEIGHT                  = 0;
int WIDTH                   = 0;
int DOMLIST_H               = 0;
void *WIN                   = NULL;
void *MAIN_WIN              = NULL;
void *SIDE_WIN              = NULL;

int SELECTED                = DOMS;
int HIGHLIGHT               = 0;
int CHOICE                  = 0;

keyval_t settings[MAXSET]   = {0};

int TOTAL_VM_COUNT          = 0;
int TOTAL_NET_COUNT         = 0;
int TOTAL_KEY_COUNT         = 0;

void dom_list(virConnectPtr conn)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms == NULL) return;

    for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
        DOMLIST_H = i;
        if (!doms[i]) return;

        const char *name = virDomainGetName(doms[i]);
        int id = virDomainGetID(doms[i]);

        char id_str[32];
        snprintf(id_str, 32, "%d", id);
        if (HIGHLIGHT == i && SELECTED == DOMS) {
            init_pair(5, 0, 7);
            wcolor_set(SIDE_WIN, 5, NULL);
        } else {
            init_pair(4, 7, 0);
            wcolor_set(SIDE_WIN, 4, NULL);
        }

        mvwprintw(SIDE_WIN, i + 1, 1, "%s - %s", id == -1 ? "*" : id_str, name);
        virDomainFree(doms[i]);
    }
}

void net_list(virConnectPtr conn)
{
    virNetworkPtr *nets = get_all_networks(conn);
    if (nets == NULL) return;

    int start = DOMLIST_H + 4;
    for (int i = 0; i < TOTAL_NET_COUNT; ++i) {
        if (nets[i]) {
            const char *name = virNetworkGetName(nets[i]);
            int vpos = start + i;

            if (HIGHLIGHT == i && SELECTED == NETS) {
                init_pair(5, 0, 7);
                wcolor_set(SIDE_WIN, 5, NULL);
                mvwprintw(SIDE_WIN, vpos, 1, "* - %s", name);
            } else {
                init_pair(4, 7, 0);
                wcolor_set(SIDE_WIN, 4, NULL);
                mvwprintw(SIDE_WIN, vpos, 1, "* - %s", name);
            }
        }
    }
}

void show_info(virConnectPtr conn, const char *name)
{
    virDomainPtr dom = get_domain(conn, name);
    if (dom != NULL) {
        virDomainInfo dm;
        if (virDomainGetInfo(dom, &dm) == 0) {
            mvwprintw(MAIN_WIN, 1, 2, "State: %s", dm.state == 1 ? "Running" : "Stopped");
            mvwprintw(MAIN_WIN, 2, 2, "Max Memory: %lu", dm.maxMem);
            mvwprintw(MAIN_WIN, 3, 2, "CPU nr: %d", dm.nrVirtCpu);
        }

        if (dm.state == 1) {
            virDomainInterfacePtr *ifaces = NULL;
            int nc = virDomainInterfaceAddresses(dom, &ifaces, VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_LEASE, 0);
            for (int i = 0; i < nc; ++i) {
                for (size_t j = 0; j < ifaces[i]->naddrs; ++j) {
                    mvwprintw(MAIN_WIN, 4 + i, SIDEBAR_W + 1, "IP Addresses: %s", ifaces[i]->addrs[j].addr);
                }
            }
        }

        virDomainFree(dom);
    }
}

void show_ninfo(virConnectPtr conn, const char *name)
{
    char *netxml = get_network_settings(conn, name);
    if (netxml != NULL) {
        xmlDocPtr doc = xmlReadMemory(netxml, strlen(netxml), "n.xml", NULL, XML_PARSE_RECOVER | XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            fprintf(stderr, "Failed to parse network xml\n");
            return;
        }

        mvwprintw(MAIN_WIN, 1, 2, "Name: %s", xpath_get(doc, "/network/name"));
        mvwprintw(MAIN_WIN, 2, 2, "UUID: %s", xpath_get(doc, "/network/uuid"));
        mvwprintw(MAIN_WIN, 3, 2, "Bridge Name: %s", xpath_get(doc, "/network/bridge/@name"));
        mvwprintw(MAIN_WIN, 4, 2, "Mode: %s", xpath_get(doc, "/network/forward/@mode"));
        mvwprintw(MAIN_WIN, 5, 2, "IP: %s", xpath_get(doc, "/network/ip/@address"));
        mvwprintw(MAIN_WIN, 6, 2, "DCHP Range: %s - %s", xpath_get(doc, "/network/ip/dhcp/range/@start"),
                xpath_get(doc, "/network/ip/dhcp/range/@end"));
    }

    virNetworkPtr net = get_network(conn, name);
    if (net != NULL) {
        int state = virNetworkIsActive(net);
        if (state) {
            mvwprintw(MAIN_WIN, 7, 2, "State: Active");
        } else {
            mvwprintw(MAIN_WIN, 7, 2, "State: Inactive");
        }
    }

    return;
}

void set_title(void)
{
    char *title;
    switch (SELECTED) {
        case DOMS:
            title = " Domain ";
            break;
        case NETS:
            title = " Network ";
            break;
        default:
            break;
    }

    mvwaddstr(MAIN_WIN, 0, (WIDTH - SIDEBAR_W)/2 - strlen(title)/2, title);
}

int main(int argc, char **argv)
{
    virConnectPtr conn;
    conn = virConnectOpen("qemu:///system");
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to 'qemu:///system'.\n");
        return 1;
    }

    setlocale(LC_ALL, "");
    WIN = initscr();
    cbreak();
    noecho();
    nodelay(WIN, 1);
    curs_set(0);
    start_color();
    use_default_colors();
    init_pair(1, 15, 235);
    wbkgd(WIN, COLOR_PAIR(1));

    WIDTH = getmaxx(WIN);
    HEIGHT = getmaxy(WIN);

    char *title = "cobia v0.0.0";
    mvwaddstr(WIN, HEIGHT - 1, WIDTH - (int)strlen(title) - 1, title);

    init_pair(2, 7, 8);
    MAIN_WIN = newwin(HEIGHT - TOPBAR_H, WIDTH - SIDEBAR_W, 0, SIDEBAR_W);
    nodelay(MAIN_WIN, 1);
    box(MAIN_WIN, 0, 0);
    wbkgd(MAIN_WIN, COLOR_PAIR(2));

    init_pair(3, 15, 0);
    SIDE_WIN = newwin(HEIGHT - TOPBAR_H, SIDEBAR_W, 0, 0);
    nodelay(SIDE_WIN, 1);
    wbkgd(SIDE_WIN, COLOR_PAIR(3));

    wattron(SIDE_WIN, A_BOLD);
    char *dom_title = "Domains";
    mvwaddstr(SIDE_WIN, 0, 1 , dom_title);
    wattroff(SIDE_WIN, A_BOLD);
    dom_list(conn);

    wattron(SIDE_WIN, A_BOLD);
    char *net_title = "Networks";
    mvwaddstr(SIDE_WIN, DOMLIST_H + 3, 1, net_title);
    wattroff(SIDE_WIN, A_BOLD);
    net_list(conn);

    set_title();
    wrefresh(WIN);
    wrefresh(MAIN_WIN);
    wrefresh(SIDE_WIN);

    int running = 1;
    while(running) {
        char ch = wgetch(WIN);
        switch (ch) {
            case 'g':
                HIGHLIGHT = 0;
                break;
            case 'G':
                HIGHLIGHT = DOMLIST_H;
                break;
            case 'j':
                HIGHLIGHT += 1;
                break;
            case 'k':
                HIGHLIGHT -= 1;
                break;
            case 'q':
                running = 0;
                break;
            case '\t':
                if (SELECTED == DOMS) {
                    SELECTED = NETS;
                    HIGHLIGHT = 0;
                } else {
                    SELECTED = DOMS;
                    HIGHLIGHT = 0;
                }
                break;
            default:
                break;
        }

        if (ch == 10) CHOICE = HIGHLIGHT+1;
        if (ch == 10 && CHOICE == HIGHLIGHT+1) {
            wclear(MAIN_WIN);
            box(MAIN_WIN, 0, 0);
            set_title();

            if (SELECTED == DOMS) {
                virDomainPtr *doms = get_all_domains(conn);
                if (doms == NULL) return 1;
                for (size_t i = 0; i < TOTAL_VM_COUNT; ++i) {
                    if (!doms[i]) break;
                    if ((int)i + 1 == CHOICE) {
                        show_info(conn, virDomainGetName(doms[i]));
                        break;
                    }
                }
            } else {
                virNetworkPtr *nets = get_all_networks(conn);
                if (nets == NULL) return 1;
                for (size_t i = 0; i < TOTAL_VM_COUNT; ++i) {
                    if (!nets[i]) break;
                    if ((int)i + 1 == CHOICE) {
                        show_ninfo(conn, virNetworkGetName(nets[i]));
                        break;
                    }
                }
            }
        }

        dom_list(conn);
        net_list(conn);

        wrefresh(WIN);
        wrefresh(MAIN_WIN);
        wrefresh(SIDE_WIN);
        usleep(1000 * 16);
    }

    nocbreak();
    endwin();
    virConnectClose(conn);
    return 0;
}
