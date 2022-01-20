#include <stdio.h>
#include <string.h>

#include "formulate_item.h"

int item_append(struct item* item, char* new_data)
{
	if ((item->unused_bytes - strlen(new_data)) < 0) {
		perror("Not enough space in item when trying to append");
		return -1;
	}

	strncat(item->data, new_data, strlen(new_data));
	item_count_unused_bytes(item);

	item->data[ITEM_SIZE - 1] = '\0';

	printf("debug: After appending, item has %d unused bytes\n",
			item->unused_bytes);

	return 0;
}

void item_initialize(struct item* item)
{
	bzero(item->data, ITEM_SIZE);
	item->unused_bytes = ITEM_SIZE;
}

void item_count_unused_bytes(struct item* item)
{
	item->unused_bytes = ITEM_SIZE - strlen(item->data);
}