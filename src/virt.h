#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>

#define CONBUF_SIZE 4096

typedef struct {
    virStreamPtr stream;
    volatile int done;
} Conctx;

virDomainPtr *get_all_domains(virConnectPtr conn);

virDomainPtr get_domain(virConnectPtr conn, const char *name);

virNetworkPtr *get_all_networks(virConnectPtr conn);

virNetworkPtr get_network(virConnectPtr conn, const char *name);

void quiet_exit_console(void *opaque, virErrorPtr err);

void start_console(virConnectPtr conn, const char *name);

void *console_reader(void *op);

void create_domain(virConnectPtr conn, const char *name);

void delete_domain(virConnectPtr conn, const char *name);

void start_domain(virConnectPtr conn, const char *name);

void stop_domain(virConnectPtr conn, const char *name);

void show_domain_xml(virConnectPtr conn, const char *name);

void show_domains(virConnectPtr conn);

void show_domain_info(virConnectPtr conn, const char *name);

void show_networks(virConnectPtr conn);

void show_network_info(virConnectPtr conn, const char *name);

void create_network(virConnectPtr conn, const char *name);

void delete_network(virConnectPtr conn, const char *name);

void start_network(virConnectPtr conn, const char *name);

void stop_network(virConnectPtr conn, const char *name);

void show_dhcp_leases(virConnectPtr conn, const char *name);

void get_console(virConnectPtr conn, const char *name);

char *get_network_xml(virConnectPtr conn, const char *name);
