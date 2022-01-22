#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>

#include "cow_client_config.h"
#include "cow_formulate_item_request.h"
#include "cow_user_identifier.h"

#define RANDOM_BYTES_TO_READ 32

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

static int initialize_random_number_generation(void)
{
	return RAND_load_file("/dev/random", RANDOM_BYTES_TO_READ);
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

static int create_user_identifier_if_none(void)
{
	BIO* bio_user_id_file = 0;
	BIO* bio_base64 = 0;
	char user_id_buf[20] = {0};
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

	BIO_write(bio_base64, user_id_buf, strlen(user_id_buf));
	BIO_flush(bio_base64);

	BIO_free_all(bio_base64);

	return 0;
}

static int load_user_identifier(void)
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

int main(void)
{
	const SSL_METHOD* method = 0;
	BIO* connect_bio = 0;
	BIO* ssl_bio = 0;
	SSL* ssl = 0;
	SSL_CTX* ctx = 0;
	X509* server_certificate = 0;
	struct item item_to_send = {0};
	int data_moved = 0;
	int ret = -1;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#error OpenSSL version is lower than 1.1.0.
#else
	// Library has initialized itself automatically.
	// https://wiki.openssl.org/index.php/Library_Initialization
#endif

	ret = initialize_random_number_generation();
	if (RANDOM_BYTES_TO_READ != ret) {
		perror("Failed to initialize random number generation");
		return 1;
	}

	ret = create_user_identifier_if_none();
	if (0 != ret) {
		perror("Failed to create user identifier");
		return 1;
	}

	ret = load_user_identifier();
	if (0 != ret) {
		perror("Failed to load user identifier");
		return 1;
	}

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

	/* Setup hostname validation */
	SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_WILDCARDS);
	ret = SSL_set1_host(ssl, CFG_SERVER_HOSTNAME);
	if (0 == ret) {
		perror("Failed to set expected DNS hostname");
		ERR_print_errors_fp(stderr);
		return 1;
	}

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
		ERR_print_errors_fp(stderr);
		goto fail;
	}

	ret = SSL_get_verify_result(ssl);
	if (X509_V_OK != ret) {
		perror("Server certificate verification failed.");
		perror("NOTE: This is expected to be caught earlier and may ");
		perror("indicate changed implementation or a bug.");
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

	printf("Sending test data\n");
	data_moved = BIO_write(
			connect_bio,
			item_to_send.data,
			strlen(item_to_send.data));
#if DEBUG_ENABLED
	printf("debug: Successfully wrote %d bytes\n", data_moved);
#endif

shutdown:
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
