#include <string.h>
#include <stdio.h>

#include "cow_host.h"
#include "cow_server_config.h"

static char s_accept_host[NI_MAXHOST] = {0};
static char s_accept_name[ACCEPT_NAME_MAX] = {0};

void cow_server_config_set_accept_host(const char* new_accept_host)
{
	strncpy(s_accept_host, new_accept_host, NI_MAXHOST);
}

void cow_server_config_get_accept_host(char* accept_host_out)
{
	strncpy(accept_host_out, s_accept_host, NI_MAXHOST);
}

void cow_server_config_set_accept_name(const char* new_accept_name)
{
	strncpy(s_accept_name, new_accept_name, ACCEPT_NAME_MAX);
}

void cow_server_config_get_accept_name(char* accept_name_out)
{
	strncpy(accept_name_out, s_accept_name, ACCEPT_NAME_MAX);
}

int cow_server_configure(void)
{
	char accept_host[NI_MAXHOST] = {0};
	char accept_name[ACCEPT_NAME_MAX] = {0};

	int ret = 0;

	if (0 == strncmp(CFG_ACCEPT_HOST, "auto", 4)) {
		ret = get_host_of_first_nonloopback_device(accept_host);
		if (0 != ret) {
			perror("Failed to autoconfigure accept host");
			return -1;
		}
	} else {
		strncpy(accept_host, CFG_ACCEPT_HOST, sizeof(accept_host));
	}
	cow_server_config_set_accept_host(accept_host);

	strncpy(accept_name, accept_host, NI_MAXHOST);
	strcat(accept_name, ":");
	strncat(accept_name, CFG_ACCEPT_PORT, ACCEPT_PORT_MAX);
	accept_name[ACCEPT_NAME_MAX - 1] = '\0';
	cow_server_config_set_accept_name(accept_name);

	return 0;
}
