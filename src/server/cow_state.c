#include "cow_state.h"

static struct cow_state main_state = {0};

void cow_state_should_stop_set(int should_stop)
{
	main_state.should_stop = should_stop;
}

int cow_state_should_stop_get(void)
{
	return main_state.should_stop;
}
