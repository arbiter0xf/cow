#include <assert.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string.h>
#include <sys/stat.h>

#include "cow_user_identifier.h"

#define USER_ID_BUF_LEN 20

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

static void set_user_identifier(const char* new_user_identifier)
{
	user_identifier_internal(new_user_identifier, 0);
}

void get_copy_of_user_identifier(char* user_identifier_copy)
{
	user_identifier_internal(0, user_identifier_copy);
}

static int is_regular_file(const char* filename)
{
	struct stat sb;
	int ret = 0;

	ret = stat(filename, &sb);
	if (0 == ret && S_IFREG == (sb.st_mode & S_IFMT)) {
		return 1;
	}

	return 0;
}

int create_user_identifier_if_none(void)
{
	BIO* bio_user_id_file = 0;
	BIO* bio_base64 = 0;
	unsigned char user_id_buf[USER_ID_BUF_LEN] = {0};
	int ret = 0;
	int err = 0;

	if (is_regular_file(USER_ID_FILE_NAME)) {
#if DEBUG_ENABLED
		printf("debug: User identifier file exists already\n");
#endif
		return 0;
	}

	printf("Generating user identifier file: %s\n", USER_ID_FILE_NAME);

	ret = RAND_bytes(user_id_buf, sizeof(user_id_buf));
	if (1 != ret) {
		err = ERR_get_error();
		fprintf(
			stderr,
			"Failed to read random bytes: (%d) %s",
			err,
			ERR_error_string(err, NULL));
		return -1;
	}

	/*
	 * Base64 encoding for user ID to make it a string. Try to ease
	 * development and debugging.
	 */
	bio_base64 = BIO_new(BIO_f_base64());
	bio_user_id_file = BIO_new_file(USER_ID_FILE_NAME, "w");
	BIO_push(bio_base64, bio_user_id_file);

	BIO_write(bio_base64, user_id_buf, USER_ID_BUF_LEN);
	BIO_flush(bio_base64);

	BIO_free_all(bio_base64);

	return 0;
}

int load_user_identifier(void)
{
	FILE* id_file = 0;
	const int items_to_read = 1;
	char user_identifier[USER_IDENTIFIER_SIZE] = {0};
	int data_read = 0;
	int err = 0;

	id_file = fopen(USER_ID_FILE_NAME, "r");
	if (0 == id_file) {
		err = errno;
		fprintf(
			stderr,
			"Failed to open identifier file %s: (%d) %s\n",
			USER_ID_FILE_NAME,
			err,
			strerror(err));
		return -1;
	}

	data_read = fread(
			user_identifier,
			sizeof(user_identifier),
			items_to_read,
			id_file);
	if (items_to_read != data_read) {
		fprintf(
			stderr,
			"Failed to read user identifier from %s\n",
			USER_ID_FILE_NAME);
		return -1;
	}
	user_identifier[sizeof(user_identifier) - 1] = '\0';

	set_user_identifier(user_identifier);
#if DEBUG_ENABLED
	printf("debug: Did load user identifier: %s\n", user_identifier);
#endif

	fclose(id_file);

	return 0;
}
