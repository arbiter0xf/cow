#include <criterion/criterion.h>

#include "cow_formulate_item.h"

#if 0
cr_log_warn("message");

void setup(void) {
}

void teardown(void) {
}

Test(formulate_item, data_is_appended_to_item, .init = setup, .fini = teardown)
#endif
Test(formulate_item, data_is_appended_to_item)
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

Test(formulate_item, maximum_amount_of_data_is_appended_to_item)
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
