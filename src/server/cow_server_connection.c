#include <openssl/err.h>

#include "cow_item.h"
#include "cow_server_config.h"
#include "cow_server_connection.h"
#include "cow_state.h"
#include "cow_server_item_consume.h"

static BIO* s_accept_bio = 0;
static SSL* s_ssl = 0;
static SSL_CTX* s_ssl_ctx = 0;

static int s_do_not_free_ssl = 0;

static int load_certificate_and_private_key(SSL_CTX* ctx)
{
	int ret = 0;

	printf("Loading certificate: %s\n", CFG_CERT_NAME);
	fflush(stdout);
	ret = SSL_CTX_use_certificate_file(ctx, CFG_CERT_NAME, SSL_FILETYPE_PEM);
	if (ret <= 0) {
		perror("Failed to set certificate file to use");
		return 1;
	}

	printf("Loading private key: %s\n", CFG_PRIVKEY_NAME);
	fflush(stdout);
	ret = SSL_CTX_use_PrivateKey_file(ctx, CFG_PRIVKEY_NAME, SSL_FILETYPE_PEM);
	if (ret <= 0) {
		perror("Failed to set private key file to use");
		return 1;
	}

	ret = SSL_CTX_check_private_key(ctx);
	if (ret <= 0) {
		perror("Private key check failed");
		return 1;
	}

	return 0;
}

static void cow_server_connection_set_do_not_free_ssl(int new_do_not_free_ssl)
{
	s_do_not_free_ssl = new_do_not_free_ssl;
}

static int cow_server_connection_get_do_not_free_ssl()
{
	return s_do_not_free_ssl;
}

void cow_server_connection_set_accept_BIO(BIO* new_accept_bio)
{
	s_accept_bio = new_accept_bio;
}

BIO* cow_server_connection_get_accept_BIO(void)
{
	return s_accept_bio;
}

void cow_server_connection_set_SSL(SSL* new_ssl)
{
	s_ssl = new_ssl;
}

SSL* cow_server_connection_get_SSL(void)
{
	return s_ssl;
}

void cow_server_connection_set_SSL_CTX(SSL_CTX* new_ssl_ctx)
{
	s_ssl_ctx = new_ssl_ctx;
}

SSL_CTX* cow_server_connection_get_SSL_CTX(void)
{
	return s_ssl_ctx;
}

int cow_server_connection_setup()
{
	SSL_CTX* ctx = 0;
	const SSL_METHOD* method = 0;
	BIO* accept_bio = 0;
	BIO* ssl_bio = 0;
	SSL* ssl = 0;

	char accept_name[ACCEPT_NAME_MAX] = {0};

	int ret = 0;

	method = TLS_server_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Could not create SSL context");
		goto fail;
	}
	cow_server_connection_set_SSL_CTX(ctx);

	ret = load_certificate_and_private_key(ctx);
	if (0 != ret) {
		perror("Failed to load certificate and private key");
		goto fail;
	}

	ssl = SSL_new(ctx);
	if (0 == ssl) {
		perror("Failed to create new SSL structure");
		goto fail;
	}
	cow_server_connection_set_SSL(ssl);

	ssl_bio = BIO_new(BIO_f_ssl());
	if (0 == ssl_bio) {
		perror("Failed to allocate new SSL BIO");
		goto fail;
	}

	accept_bio = BIO_new(BIO_s_accept());
	if (0 == ssl_bio) {
		perror("Failed to allocate new accept BIO");
		goto fail;
	}
	cow_server_connection_set_accept_BIO(accept_bio);

	// https://linux.die.net/man/3/bio_set_close
	ret = BIO_set_ssl(ssl_bio, ssl, BIO_CLOSE);
	if (ret <= 0) {
		perror("Failed to set SSL to SSL BIO");
		goto fail;
	}

	ret = BIO_set_ssl_mode(ssl_bio, 0 /* set server mode */);
	if (ret <= 0) {
		perror("Failed to set SSL mode");
		goto fail;
	}

	cow_server_config_get_accept_name(accept_name);
	ret = BIO_set_accept_name(accept_bio, accept_name);
	if (ret <= 0) {
		perror("Failed to set name and port for accepting connections");
		goto fail;
	}

	ret = BIO_set_nbio_accept(accept_bio, 0 /* set to blocking mode */);
	if (ret <= 0) {
		perror("Failed to set BIO to blocking");
		goto fail;
	}

	ret = BIO_set_accept_bios(accept_bio, ssl_bio);
	if (ret <= 0) {
		perror("Failed to set SSL BIO to be duplicated"
				" for each accepted connection");
		goto fail;
	}
	cow_server_connection_set_do_not_free_ssl(1);

	ret = BIO_do_accept(accept_bio);
	if (ret <= 0) {
		perror("Failed to create and bind accept socket");
		goto fail;
	}
	printf("Bound to address: %s:%s\n",
			BIO_get_accept_name(accept_bio),
			BIO_get_accept_port(accept_bio));

	return 0;

fail:

	return -1;
}

int cow_server_connection_listen_and_handle()
{
	BIO* accept_bio;
	BIO* tmp_bio = 0;
	struct item received_item;
	int ret = 0;
	int data_moved = 0;

	accept_bio = cow_server_connection_get_accept_BIO();

	while (1) {
		if (cow_state_should_stop_get()) {
			printf("Stopping to serve clients due to state\n");
			break;
		}

		printf("Waiting for connection\n");
		fflush(stdout);
		cow_state_listening_set(1);
		ret = BIO_do_accept(accept_bio);
		if (ret <= 0) {
			perror("Failed to accept connection");
			ERR_print_errors_fp(stderr);
			continue;
		}
		cow_state_listening_set(0);

		ret = BIO_do_handshake(accept_bio);
		if (ret <= 0) {
			perror("Failed to do SSL handshake");
			ERR_print_errors_fp(stderr);
			continue;
		}

		item_initialize(&received_item);
#if DEBUG_ENABLED
		printf("debug: Reading %d bytes\n", ITEM_SIZE);
#endif
		data_moved = BIO_read(
				accept_bio,
				received_item.data,
				ITEM_SIZE);
		item_count_unused_bytes(&received_item);

#if DEBUG_ENABLED
		printf("debug: Read %d bytes\n", data_moved);
#endif
		if (data_moved <= 0) {
			/*
			 * No data successfully read.
			 *
			 * Did exit with code 141 when client had first
			 * successfully connected using BIO and then proceeded
			 * to BIO_write(), using uninitialized BIO.
			 */
			tmp_bio = BIO_pop(accept_bio);
			BIO_free_all(tmp_bio);
			continue;
		}

		// TODO respond here, instead of reading more. Currently
		// overwriting already read data.
		data_moved = BIO_read(
				accept_bio,
				received_item.data,
				ITEM_SIZE);
		if (data_moved > 0) {
			printf("Client is sending more data than expected\n");
		}

		tmp_bio = BIO_pop(accept_bio);
		BIO_free_all(tmp_bio);

		cow_server_item_consume(&received_item, 0);
	}

	return 0;
}

void cow_server_connection_teardown(void)
{
	BIO* accept_bio = cow_server_connection_get_accept_BIO();
	SSL* ssl = cow_server_connection_get_SSL();
	SSL_CTX* ctx = cow_server_connection_get_SSL_CTX();

	int do_not_free_ssl = cow_server_connection_get_do_not_free_ssl();

	if (0 != accept_bio) {
		// Free chain set by BIO_set_accept_bios()
		BIO_free_all(accept_bio);
	}

	if (0 != ssl && !do_not_free_ssl) {
		SSL_free(ssl);
	}

	if (0 != ctx) {
		SSL_CTX_free(ctx);
	}
}
