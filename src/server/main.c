#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "config.h"

int main(void) {
	const SSL_METHOD* method;
	SSL_CTX* ctx;
	int ret = 0;

	method = TLS_server_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Could not create SSL context");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = SSL_CTX_use_certificate_file(ctx, CFG_CERT_NAME, SSL_FILETYPE_PEM);
	if (ret <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}

	printf("Exiting\n");

	return 0;
}
