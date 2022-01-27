#include <criterion/criterion.h>

#include "cow_item.h"
#include "cow_item_request.h"

#if 0
cr_log_warn("message");

void setup(void) {
}

void teardown(void) {
}

Test(formulate_item, data_is_appended_to_item, .init = setup, .fini = teardown)
#endif
Test(item, data_is_appended_to_item)
{
	struct item test_item;
	const char* test_data = "abcd1234";
	const char* test_data_result = "abcd1234abcd1234abcd1234";

	item_initialize(&test_item);

	item_append(&test_item, test_data);
	item_append(&test_item, test_data);
	item_append(&test_item, test_data);

	cr_expect_str_eq(test_item.data, test_data_result);
}

Test(item, maximum_amount_of_data_is_appended_to_item)
{
	struct item test_item;
	const char* test_data = "abcd1234"; // Append 8 bytes at a time
	const int data_max = ITEM_SIZE / strlen(test_data);

	item_initialize(&test_item);

	for (int i = 0; i < data_max - 1; i++) {
		item_append(&test_item, test_data);
	}

	cr_expect(8 == test_item.unused_bytes);
	item_append(&test_item, test_data);
	cr_expect(0 == test_item.unused_bytes);
}

Test(item, cannot_add_more_than_maximum_amount_of_data_to_item)
{
	struct item test_item;
	const char* test_data = "abcd1234"; // Append 8 bytes at a time
	const char* test_data_short = "a";
	const int data_max = ITEM_SIZE / strlen(test_data);
	int ret = 0;

	item_initialize(&test_item);

	for (int i = 0; i < data_max; i++) {
		ret = item_append(&test_item, test_data);
	}
	cr_expect(0 == ret);
	cr_expect(0 == test_item.unused_bytes);

	ret = item_append(&test_item, test_data_short);
	cr_expect(0 != ret);
	// Last char is '\0', so ITEM_SIZE - 1
	cr_expect(ITEM_SIZE - 1 == strlen(test_item.data));
}

Test(item_request, item_request_fits_in_item_data)
{
	struct item_request item_request = {0};
	struct item item = {0};

	cr_expect(sizeof(item_request) == sizeof(item.data));
}

Test(item_request, item_request_contains_expected_header_tag)
{
	struct item_request item_request = {0};

	fill_item_request(&item_request);

	cr_expect_str_eq(item_request.header_tag, HEADER_TAG_REQUEST);
}

Test(item_request, new_item_request_test_contains_expected_header_tag)
{
	struct item_request request_test = {0};

	fill_item_request_test(&request_test);

	cr_expect_str_eq(request_test.header_tag, HEADER_TAG_REQUEST_TEST);
}
