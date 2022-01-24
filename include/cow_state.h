#ifndef COW_STATE_H_DEFINED
#define COW_STATE_H_DEFINED

struct cow_state {
	int should_stop;
	int listening;
};

void cow_state_should_stop_set(int should_stop);
int cow_state_should_stop_get(void);
void cow_state_listening_set(int listening);
int cow_state_listening_get(void);

#endif
