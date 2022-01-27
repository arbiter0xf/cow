#ifndef COW_CLIENT_CONNECT_H_DEFINED
#define COW_CLIENT_CONNECT_H_DEFINED

#include "cow_formulate_item.h"

int cow_client_connection_setup();
void cow_client_connection_send_item(struct item* item_to_send);
int cow_client_connection_teardown();

#endif