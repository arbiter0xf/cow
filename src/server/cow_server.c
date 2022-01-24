#include <stdio.h>

#include "cow_formulate_item.h"
#include "cow_server_config.h"
#include "cow_server_connect.h"
#include "cow_signal.h"
#include "cow_synchronize.h"

int cow_server_init(void)
{
	int ret = 0;

	ret = install_signal_handler();
	if (0 != ret) {
		perror("Failed to install signal handler");
		return -1;
	}

	ret = cow_server_configure();
	if (0 != ret) {
		perror("Failed to configure server");
		return -1;
	}

	return 0;
}

int cow_server_serve(void)
{
	int ret = 0;

	ret = cow_server_init();
	if (0 != ret) {
		perror("Failed to initialize server");
		return -1;
	}

	ret = cow_server_connection_setup();
	if (0 != ret) {
		perror("Failed setup server connection");
		return -1;
	}

	ret = cow_server_connection_listen_and_handle();
	if (0 != ret) {
		perror("Fatal error while serving clients");
		return -1;
	}

	return 0;
}

int cow_server_teardown(void)
{
	cow_server_connection_teardown();
	(void) cow_locks_teardown();

	return 0;
}
