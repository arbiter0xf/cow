#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "cow_state.h"

static void signal_handler(int sig)
{
	if (SIGINT == sig) {
		cow_state_should_stop_set(1);
	};
}

int install_signal_handler(void)
{
	struct sigaction sa = {0};
	int ret = 0;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = signal_handler;

	ret = sigaction(SIGINT, &sa, 0);
	if (0 != ret) {
		perror("Failed to set disposition of SIGINT");
		return -1;
	}

	return 0;
}
