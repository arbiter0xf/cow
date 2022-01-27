#include <stdio.h>

#include "cow_item_request.h"
#include "cow_user_identifier.h"

static int wrap_item_request(struct item* item)
{
	int ret = 0;
	char user_identifier[USER_IDENTIFIER_SIZE] = {0};

	item_initialize(item);

	get_copy_of_user_identifier(user_identifier);
	ret = item_append(item, user_identifier);
	if (0 != ret) {
		perror("Failed to append user identifier");
		return -1;
	}

	return 0;
}

int wrap_item_request_test(struct item* item)
{
	int ret = 0;
	char* test_data = "test test";

	ret = wrap_item_request(item);
	if (0 != ret) {
		perror("Failed to wrap item request");
		return -1;
	}

	ret = item_append(item, HEADER_TAG_REQUEST_TEST);
	if (0 != ret) {
		perror("Failed to append header tag");
		return -1;
	}

	ret = item_append(item, test_data);
	if (0 != ret) {
		perror("Failed to append test data");
		return -1;
	}

	return 0;
}
