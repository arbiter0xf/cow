#ifndef COW_SERVER_ITEM_CONSUME_H_DEFINED
#define COW_SERVER_ITEM_CONSUME_H_DEFINED

#include "cow_item.h"

/*
 * Returns an item to send as reply, if any.
 */
void cow_server_item_consume(
		struct item* received_item,
		struct item* response_item);

#endif
