#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

#include "cow_server_config.h"
#include "cow_server.h"
#include "cow_signal.h"
#include "cow_synchronize.h"

int main(void)
{
	int ret = 0;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#error OpenSSL version is lower than 1.1.0.
#else
	// Library has initialized itself automatically.
	// https://wiki.openssl.org/index.php/Library_Initialization
#endif

	ret = cow_server_serve();
	if (0 != ret) {
		goto fail;
	}

	printf("Exiting\n");
	cow_server_teardown();

	return 0;

fail:
	ERR_print_errors_fp(stderr);
	cow_server_teardown();

	return 1;
}
