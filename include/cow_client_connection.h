#ifndef COW_CLIENT_CONNECT_H_DEFINED
#define COW_CLIENT_CONNECT_H_DEFINED

#include "cow_item.h"

int cow_client_connection_setup();
int cow_client_connection_send_item(struct item* item_to_send);
void cow_client_connection_teardown();

#endif
