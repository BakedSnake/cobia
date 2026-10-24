#include <libvirt/libvirt.h>

virDomainPtr *get_all_domains(virConnectPtr conn);

virDomainPtr get_domain(virConnectPtr conn, const char *name);

virNetworkPtr *get_all_networks(virConnectPtr conn);

virNetworkPtr get_network(virConnectPtr conn, const char *name);

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

char *get_network_xml(virConnectPtr conn, const char *name);
