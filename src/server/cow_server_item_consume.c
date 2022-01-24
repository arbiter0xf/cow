#include <stdio.h>

#include "cow_server_item_consume.h"

void cow_server_item_consume(
		struct item* received_item,
		struct item* response_item)
{
	printf("Received connection data: %s\n", received_item->data);
	fflush(stdout);
}
