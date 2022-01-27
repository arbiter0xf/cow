#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <string.h>

#include "cow_client_config.h"
#include "cow_client_connection.h"

static BIO* s_connect_bio = 0;
static SSL_CTX* s_ctx = 0;
static SSL* s_ssl = 0;

static int s_connection_up = 0;
static int s_do_not_free_ssl = 0;

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

/*
 * Try to establish a connection and verify that have connected to intended
 * server. Server certificate and hostname are checked.
 */
int cow_client_connection_setup()
{
	const SSL_METHOD* method = 0;
	BIO* connect_bio = 0;
	BIO* ssl_bio = 0;
	SSL* ssl = 0;
	SSL_CTX* ctx = 0;
	X509* server_certificate = 0;
	int ret = -1;

	method = TLS_client_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Could not create SSL context");
		goto fail;
	}

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, cert_verification_cb);
	ret = SSL_CTX_load_verify_locations(ctx, CFG_CA_FILE, NULL);
	if (ret <= 0) {
		perror("Failed to load CA file locations");
		goto fail;
	}

	ssl = SSL_new(ctx);
	s_ssl = ssl;
	SSL_set_connect_state(ssl);

	/* Setup hostname validation */
	SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_WILDCARDS);
	ret = SSL_set1_host(ssl, CFG_SERVER_HOSTNAME);
	if (0 == ret) {
		perror("Failed to set expected DNS hostname");
		goto fail;
	}

	ssl_bio = BIO_new(BIO_f_ssl());
	ret = BIO_set_ssl(ssl_bio, ssl, BIO_CLOSE);
	if (ret <= 0) {
		perror("Failed to set SSL pointer to BIO");
		goto fail;
	}

	connect_bio = BIO_new(BIO_s_connect());
	s_connect_bio = connect_bio;
	ret = BIO_set_conn_hostname(
			connect_bio,
			CFG_SERVER_HOSTNAME ":" CFG_SERVER_PORT);
	if (ret <= 0) {
		perror("Failed set hostname to BIO");
		goto fail;
	}
	BIO_set_nbio(connect_bio, 0);

	connect_bio = BIO_push(ssl_bio, connect_bio);
	s_connect_bio = connect_bio;
	s_do_not_free_ssl = 1;

	printf("Attempting to connect to %s:%s\n",
			BIO_get_conn_hostname(connect_bio),
			BIO_get_conn_port(connect_bio));
	fflush(stdout);
	ret = BIO_do_connect(connect_bio);
	if (ret <= 0) {
		perror("Failed to connect");
		goto fail;
	}
	s_connection_up = 1;

	ret = BIO_do_handshake(connect_bio);
	if (ret <= 0) {
		perror("Failed to do SSL handshake");
		goto fail;
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

	return 0;

fail:
	ERR_print_errors_fp(stderr);
	return -1;
}

void cow_client_connection_send_item(struct item* item_to_send)
{
	int data_moved = 0;

	printf("Sending test data\n");
	data_moved = BIO_write(
			s_connect_bio,
			item_to_send->data,
			strlen(item_to_send->data));
#if DEBUG_ENABLED
	printf("debug: Successfully wrote %d bytes\n", data_moved);
#endif
}

int cow_client_connection_teardown()
{
	if (s_connection_up) {
		BIO_ssl_shutdown(s_connect_bio);
		s_connection_up = 0;
	}

	if (s_connect_bio) {
		BIO_free_all(s_connect_bio); // Free chain set by BIO_push()
		s_connect_bio = 0;
	}

	if (s_ssl && !s_do_not_free_ssl) {
		SSL_free(s_ssl);
	}

	if (s_ctx) {
		SSL_CTX_free(s_ctx);
		s_ctx = 0;
	}
}
