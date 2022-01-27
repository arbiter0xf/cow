#include <stdio.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

#include "cow_client.h"
#include "cow_client_connection.h"
#include "cow_formulate_item_request.h"

int main(void)
{
	struct item item_to_send = {0};
	int ret = -1;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#error OpenSSL version is lower than 1.1.0.
#else
	// Library has initialized itself automatically.
	// https://wiki.openssl.org/index.php/Library_Initialization
#endif

	ret = cow_client_init();
	if (0 != ret) {
		perror("Failed to initialize");
		goto fail;
	}

	ret = cow_client_connection_setup();
	if (0 != ret) {
		perror("Failed to connect");
		goto fail;
	}
	printf("Connected\n");

	ret = formulate_item_request_test(&item_to_send);
	if (0 != ret) {
		perror("Failed to formulate test request");
		goto fail;
	}
#if DEBUG_ENABLED
	printf("debug: constructed item data: %s\n", item_to_send.data);
#endif

	cow_client_connection_send_item(&item_to_send);

	printf("Exiting\n");
	cow_client_connection_teardown();

	return 0;

fail:
	cow_client_connection_teardown();

	return 1;
}
