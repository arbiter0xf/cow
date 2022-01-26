#include <openssl/rand.h>
#include <stdio.h>

#include "cow_client.h"
#include "cow_user_identifier.h"

static int initialize_random_number_generation(void)
{
	return RAND_load_file("/dev/random", RANDOM_BYTES_TO_READ);
}

int cow_client_init()
{
	int ret = 0;

	ret = initialize_random_number_generation();
	if (RANDOM_BYTES_TO_READ != ret) {
		perror("Failed to initialize random number generation");
		return -1;
	}

	ret = create_user_identifier_if_none();
	if (0 != ret) {
		perror("Failed to create user identifier");
		return -1;
	}

	ret = load_user_identifier();
	if (0 != ret) {
		perror("Failed to load user identifier");
		return -1;
	}

	return 0;
}
