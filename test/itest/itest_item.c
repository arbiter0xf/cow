#include <criterion/criterion.h>
#include <stdio.h>

#include "cow_client.h"
#include "cow_item_request.h"

Test(item_request, item_request_contains_user_identifier_in_use)
{
	FILE* id_file = 0;
	struct item_request item_request = {0};
	char user_identifier[USER_IDENTIFIER_SIZE] = {0};
	const int items_to_read = 1;
	int data_read = 0;
	int ret = 0;

	id_file = fopen(USER_ID_FILE_NAME, "r");
	cr_expect(0 != id_file);

	data_read = fread(
			user_identifier,
			sizeof(user_identifier),
			items_to_read,
			id_file);
	cr_expect(items_to_read == data_read);
	user_identifier[sizeof(user_identifier) - 1] = '\0';

	ret = cow_client_init();
	cr_expect(0 == ret);

	fill_item_request(&item_request);

	cr_expect_str_eq(item_request.header_user_identifier, user_identifier);
}
