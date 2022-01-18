#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

#include "config.h"

int load_certificate_and_private_key(SSL_CTX* ctx)
{
	int ret = 0;

	ret = SSL_CTX_use_certificate_file(ctx, CFG_CERT_NAME, SSL_FILETYPE_PEM);
	if (ret <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = SSL_CTX_use_PrivateKey_file(ctx, CFG_PRIVKEY_PATH, SSL_FILETYPE_PEM);
	if (ret <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = SSL_CTX_check_private_key(ctx);
	if (ret <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}

	return 0;
}

int main(void)
{
	const SSL_METHOD* method;
	SSL* ssl = 0;
	SSL_CTX* ctx;
	BIO *ssl_bio = 0;
	BIO *accept_bio = 0;
	int ret = 0;


#if OPENSSL_VERSION_NUMBER < 0x10100000L
	SSL_library_init();
#else
	OPENSSL_init_ssl(0, NULL);
#endif

	method = TLS_server_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Could not create SSL context");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = load_certificate_and_private_key(ctx);
	if (0 != ret) {
		return 1;
	}

	ssl = SSL_new(ctx);
	if (0 == ssl) {
		perror("Failed to create new SSL structure");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ssl_bio = BIO_new(BIO_f_ssl());
	if (0 == ssl_bio) {
		perror("Failed to allocate new SSL BIO");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	accept_bio = BIO_new(BIO_s_accept());
	if (0 == ssl_bio) {
		perror("Failed to allocate new accept BIO");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	// https://linux.die.net/man/3/bio_set_close
	ret = BIO_set_ssl(ssl_bio, ssl, BIO_CLOSE);
	if (ret <= 0) {
		perror("Failed to set SSL to SSL BIO");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = BIO_set_ssl_mode(ssl_bio, 0 /* set server mode */);
	if (ret <= 0) {
		perror("Failed to set SSL mode");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = BIO_set_accept_name(accept_bio, "localhost:2424");
	if (ret <= 0) {
		perror("Failed to set accept_name");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = BIO_set_nbio_accept(accept_bio, 0 /* set to blocking mode */);
	if (ret <= 0) {
		perror("Failed to set BIO to blocking");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = BIO_set_accept_bios(accept_bio, ssl_bio);
	if (ret <= 0) {
		perror("Failed to set SSL BIO to be duplicated"
				" for each accepted connection");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	ret = BIO_do_accept(accept_bio);
	if (ret <= 0) {
		perror("Failed to create and bind accept socket");
		ERR_print_errors_fp(stderr);
		return 1;
	}
	printf("Bound to address: %s:%s\n",
			BIO_get_accept_name(accept_bio),
			BIO_get_accept_port(accept_bio));

	printf("Waiting for connection\n");
	fflush(stdout);
	ret = BIO_do_accept(accept_bio);
	if (ret <= 0) {
		perror("Failed to create and bind accept socket");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	/* Support only one connection */
	ssl_bio = BIO_pop(accept_bio);
	BIO_free_all(accept_bio);
	accept_bio = 0;

	ret = BIO_do_handshake(ssl_bio);
	if (ret <= 0) {
		perror("Failed to do SSL handshake");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	printf("Exiting\n");
	if (0 != accept_bio) {
		// Free chain set by BIO_set_accept_bios()
		BIO_free_all(accept_bio);
	}
	SSL_CTX_free(ctx);

	return 0;
}
