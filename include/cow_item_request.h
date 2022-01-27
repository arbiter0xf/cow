#ifndef COW_FORMULATE_ITEM_REQUEST_H_DEFINED
#define COW_FORMULATE_ITEM_REQUEST_H_DEFINED

#include "cow_item.h"
#include "cow_user_identifier.h"

#define ITEM_REQUEST_DATA_SIZE ITEM_SIZE \
				 - USER_IDENTIFIER_SIZE \
				 - HEADER_TAG_SIZE

#define HEADER_TAG_REQUEST "REQU"
#define HEADER_TAG_REQUEST_TEST "REQUTEST"

struct item_request {
	char header_user_identifier[USER_IDENTIFIER_SIZE];
	char header_tag[HEADER_TAG_SIZE];
	char data[ITEM_REQUEST_DATA_SIZE];
};

void fill_item_request(struct item_request* item_request);
void fill_item_request_test(struct item_request* item_request);
int wrap_item_request_test(struct item* item);

#endif
