#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

#include "config.h"

int main(void) {
	const SSL_METHOD* method = 0;
	BIO* connect_bio = 0;
	BIO* ssl_bio = 0;
	SSL* ssl = 0;
	SSL_CTX* ctx = 0;
	int ret = -1;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	SSL_library_init();
#else
	OPENSSL_init_ssl(0, NULL);
#endif

	method = TLS_client_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Could not create SSL context");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	ret = SSL_CTX_load_verify_locations(ctx, CFG_CA_FILE, NULL);
	if (ret <= 0) {
		perror("Failed to load CA file locations");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ssl = SSL_new(ctx);
	SSL_set_connect_state(ssl);

	ssl_bio = BIO_new(BIO_f_ssl());
	BIO_set_ssl(ssl_bio, ssl, BIO_CLOSE);

	connect_bio = BIO_new(BIO_s_connect());
	BIO_set_conn_hostname(
			connect_bio,
			CFG_SERVER_HOSTNAME ":" CFG_SERVER_PORT);
	BIO_set_nbio(connect_bio, 0);

	connect_bio = BIO_push(ssl_bio, connect_bio);

	printf("Attempting to connect to %s:%s\n",
			BIO_get_conn_hostname(connect_bio),
			BIO_get_conn_port(connect_bio));
	fflush(stdout);
	ret = BIO_do_connect(connect_bio);
	if (ret <= 0) {
		perror("Failed to connect");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = BIO_do_handshake(connect_bio);
	if (ret <= 0) {
		perror("Failed to do SSL handshake");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	printf("Exiting\n");
	BIO_free_all(connect_bio); // Free chain set by BIO_push()
	SSL_CTX_free(ctx);

	return 0;
}
