#ifndef COW_SERVER_CONFIG_H_DEFINED
#define COW_SERVER_CONFIG_H_DEFINED

#include <netdb.h>

#if LOCAL_SERVER_CONFIG
#define CFG_CERT_NAME "localhost.crt"
#define CFG_PRIVKEY_NAME "localhost.key"
#define CFG_ACCEPT_HOST "localhost"
#else
#define CFG_CERT_NAME "please_set_owner.crt"
#define CFG_PRIVKEY_NAME "please_set_owner.key"
/*
 * An IP address can be provided here. If set to "auto", will try to get IP of
 * first network device which is not loopback.
 */
#define CFG_ACCEPT_HOST "auto"
#endif

#define CFG_ACCEPT_PORT "2424"

// "65535"
#define ACCEPT_PORT_MAX 5

// NI_MAXHOST + ":" + ACCEPT_PORT_MAX + "\0"
#define ACCEPT_NAME_MAX NI_MAXHOST + 7

void cow_server_config_set_accept_host(const char* new_accept_host);
void cow_server_config_get_accept_host(char* accept_host_out);
void cow_server_config_set_accept_name(const char* new_accept_name);
void cow_server_config_get_accept_name(char* accept_name_out);

int cow_server_configure(void);

#endif
