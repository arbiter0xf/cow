#include <stdio.h>
#include <string.h>

#include "cow_item_request.h"
#include "cow_user_identifier.h"

static void item_request_initialize(struct item_request* item_request)
{
	bzero(item_request, sizeof(*item_request));
}

/*
 * Bare item request is unfinished and not intended to be sent to server. Only
 * more specific types such as request test, REQUTEST, are meant for sending.
 */
void fill_item_request(struct item_request* item_request)
{
	char user_identifier[USER_IDENTIFIER_SIZE] = {0};

	item_request_initialize(item_request);

	get_copy_of_user_identifier(user_identifier);
	strncpy(item_request->header_user_identifier, user_identifier,
			USER_IDENTIFIER_SIZE);

	strncpy(item_request->header_tag, HEADER_TAG_REQUEST, HEADER_TAG_SIZE);
}

void fill_item_request_test(struct item_request* item_request)
{
	fill_item_request(item_request);

	strncpy(item_request->header_tag, HEADER_TAG_REQUEST_TEST,
			HEADER_TAG_SIZE);

	strncpy(item_request->data, REQUEST_TEST_DATA, ITEM_REQUEST_DATA_SIZE);
}

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
