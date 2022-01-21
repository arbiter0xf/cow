#include <assert.h>
#include <string.h>

#include "cow_user_identifier.h"

/*
 * When acting as a setter, pass_user_identifier must be 0.
 * When acting as a getter, new_user_identifier must be 0.
 */
static void user_identifier_internal(
		const char* new_user_identifier,
		char* user_identifier_copy)
{
	static char user_identifier[USER_IDENTIFIER_SIZE] = {0};
	int only_new_param_set = 0;
	int only_copy_param_set = 0;

	only_new_param_set =
		(0 != new_user_identifier) && (0 == user_identifier_copy);
	only_copy_param_set =
		(0 == new_user_identifier) && (0 != user_identifier_copy);
	assert(only_new_param_set || only_copy_param_set);

	if (0 != new_user_identifier) {
		strncpy(user_identifier, new_user_identifier, USER_IDENTIFIER_SIZE);
	} else if (0 != user_identifier_copy) {
		strncpy(user_identifier_copy, user_identifier, USER_IDENTIFIER_SIZE);
	}
}

void set_user_identifier(const char* new_user_identifier)
{
	user_identifier_internal(new_user_identifier, 0);
}

void get_copy_of_user_identifier(char* user_identifier_copy)
{
	user_identifier_internal(0, user_identifier_copy);
}
