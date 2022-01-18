#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

#include "config.h"

static int cert_verification_cb(int preverify_ok, X509_STORE_CTX* ctx)
{
	int err;

	if (preverify_ok <= 0) {
		err = X509_STORE_CTX_get_error(ctx);
		fprintf(
			stderr,
			"Server certificate verification failed: (%d) %s\n",
			err,
			X509_verify_cert_error_string(err));
		fflush(stderr);
	}

	return preverify_ok;
}

int main(void)
{
	const SSL_METHOD* method = 0;
	BIO* connect_bio = 0;
	BIO* ssl_bio = 0;
	SSL* ssl = 0;
	SSL_CTX* ctx = 0;
	X509* server_certificate = 0;
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

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, cert_verification_cb);
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

	server_certificate = SSL_get_peer_certificate(ssl);
	if (0 == server_certificate) {
		perror("Failed to get certificate from server");
		goto fail;
	}

	ret = SSL_get_verify_result(ssl);
	if (X509_V_OK != ret) {
		perror("Server certificate verification failed.");
		perror("NOTE: This is expected to be caught earlier and may ");
		perror("indicate changed implementation or a bug.");
		goto fail;
	}

	printf("Exiting\n");
	BIO_ssl_shutdown(connect_bio);
	BIO_free_all(connect_bio); // Free chain set by BIO_push()
	SSL_CTX_free(ctx);

	return 0;

fail:
	BIO_ssl_shutdown(connect_bio);
fail_no_connection:
	BIO_free_all(connect_bio); // Free chain set by BIO_push()
fail_no_bio_chain:
	SSL_CTX_free(ctx);
	return 1;
}
