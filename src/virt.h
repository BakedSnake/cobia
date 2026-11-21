#include <libvirt/libvirt.h>

/*
 * get_all_domains()
 *
 * Returns   a   pointer   to   domain     pointer.
 * Gets all domains.
 *
 */

virDomainPtr *get_all_domains(virConnectPtr conn);

/*
 * get_domain()
 *
 * Returns a domain pointer.
 * Gets    a    single    domain,    by       name.
 *
 */

virDomainPtr get_domain(virConnectPtr conn, const char *name);

/*
 * get_all_networks()
 *
 * Returns   a   pointer   to   network    pointer.
 * Gets all networks.
 *
 */

virNetworkPtr *get_all_networks(virConnectPtr conn);

/*
 * get_network()
 *
 * Returns a network pointer.
 * Gets    a    single    network,    by      name.
 *
 */

virNetworkPtr get_network(virConnectPtr conn, const char *name);

/*
 * create_domain()
 *
 * Creates a new domain from a  yaml  config  file.
 * Converts the yaml into  dot  notation  settings,
 * which are then converted to xml.  Then  the  xml
 * data is used to create the domain  via  libvirt.
 *
 */

void create_domain(virConnectPtr conn, const char *name);

/*
 * delete_domain()
 *
 * Deletes   a   single    domain,    by      name.
 *
 */

void delete_domain(virConnectPtr conn, const char *name);

/*
 * start_domain()
 *
 * Starts a domain, by name.
 *
 */

void start_domain(virConnectPtr conn, const char *name);

/*
 * stop_domain()
 *
 * Stops a domain, by name.
 *
 */

void stop_domain(virConnectPtr conn, const char *name);

/*
 * show_domain_settings()
 *
 * Prints  domain  settings  in  yaml  format  into
 * stdout.
 *
 */

void show_domain_settings(virConnectPtr conn, const char *name);

/*
 * show_domains()
 *
 * Prints  a  list  of  all  domains  into  stdout.
 *
 */

void show_domains(virConnectPtr conn);

/*
 * show_domain_info()
 *
 * Prints domain info into stdout.
 *
 */

void show_domain_info(virConnectPtr conn, const char *name);

/*
 * show_networks()
 *
 * Prints network list into stdout.
 *
 */

void show_networks(virConnectPtr conn);

/*
 * show_network_info()
 *
 * Prints network info into stdout.
 *
 */

void show_network_info(virConnectPtr conn, const char *name);

/*
 * create_network()
 *
 * Creates a new network from a yaml  config  file.
 * Converts the yaml into  dot  notation  settings,
 * which are then converted to xml.  Then  the  xml
 * data is used to create the network  via libvirt.
 *
 */

void create_network(virConnectPtr conn, const char *name);

/*
 * delete_network()
 *
 * Deletes a single network, by name.
 *
 */

void delete_network(virConnectPtr conn, const char *name);

/*
 * start_network()
 *
 * Start a network, by name.
 *
 */

void start_network(virConnectPtr conn, const char *name);

/*
 * stop_network()
 *
 * Stop a network, by name.
 *
 */

void stop_network(virConnectPtr conn, const char *name);

/*
 * show_dhcp_leases()
 *
 * Prints all  dhcp  leases  from  a  network.  The
 * network is selected by name.
 *
 */

void show_dhcp_leases(virConnectPtr conn, const char *name);

/*
 * get_network_settings()
 *
 * Prints network  settings  in  yaml  format  into
 * stdout.  The  network  is  selected   by   name.
 *
 */

char *get_network_settings(virConnectPtr conn, const char *name);
