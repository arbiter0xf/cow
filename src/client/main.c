#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "config.h"

int main(void) {
	const SSL_METHOD* method;
	SSL_CTX* ctx;

	method = TLS_server_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Could not create SSL context");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	SSL_CTX_load_verify_locations(ctx, CFG_CA_FILE, NULL);

	printf("Exiting\n");

	return 0;
}
