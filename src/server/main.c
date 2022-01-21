#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

#include "cow_server_config.h"
#include "cow_formulate_item.h"
#include "cow_host.h"
#include "cow_signal.h"
#include "cow_state.h"
#include "cow_synchronize.h"

void process_received_item(struct item* received_item)
{
	printf("Received connection data: %s\n", received_item->data);
	fflush(stdout);
}

int load_certificate_and_private_key(SSL_CTX* ctx)
{
	int ret = 0;

	printf("Loading certificate: %s\n", CFG_CERT_NAME);
	fflush(stdout);
	ret = SSL_CTX_use_certificate_file(ctx, CFG_CERT_NAME, SSL_FILETYPE_PEM);
	if (ret <= 0) {
		ERR_print_errors_fp(stderr);
		return 1;
	}

	printf("Loading private key: %s\n", CFG_PRIVKEY_NAME);
	fflush(stdout);
	ret = SSL_CTX_use_PrivateKey_file(ctx, CFG_PRIVKEY_NAME, SSL_FILETYPE_PEM);
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

int serve_connecting_clients(BIO* accept_bio)
{
	BIO* tmp_bio = 0;
	struct item received_item;
	int ret = 0;
	int data_moved = 0;

	while (1) {
		if (cow_state_should_stop_get()) {
			printf("Stopping to serve clients due to state.\n");
			break;
		}

		printf("Waiting for connection\n");
		fflush(stdout);
		ret = BIO_do_accept(accept_bio);
		if (ret <= 0) {
			perror("Failed to accept connection");
			ERR_print_errors_fp(stderr);
			continue;
		}

		ret = BIO_do_handshake(accept_bio);
		if (ret <= 0) {
			perror("Failed to do SSL handshake");
			ERR_print_errors_fp(stderr);
			continue;
		}

		item_initialize(&received_item);
		printf("debug: Reading %d bytes\n", ITEM_SIZE);
		data_moved = BIO_read(
				accept_bio,
				received_item.data,
				ITEM_SIZE);
		item_count_unused_bytes(&received_item);

		printf("debug: Read %d bytes\n", data_moved);
		if (data_moved <= 0) {
			// No data successfully read
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

		process_received_item(&received_item);
	}

	return 0;
}

int main(void)
{
	const SSL_METHOD* method = 0;
	SSL* ssl = 0;
	SSL_CTX* ctx = 0;
	BIO* ssl_bio = 0;
	BIO* accept_bio = 0;
	int do_not_free_ssl = 0;
	int err = 0;
	int ret = 0;

	// NI_MAXHOST + ":" + "65535" + "\0"
	char accept_name[NI_MAXHOST + 7] = {0};

	char accept_host[NI_MAXHOST] = {0};
	char* accept_port = CFG_ACCEPT_PORT;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#error OpenSSL version is lower than 1.1.0.
#else
	// Library has initialized itself automatically.
	// https://wiki.openssl.org/index.php/Library_Initialization
#endif

	ret = cow_locks_init();
	if (0 != ret) {
		perror("Failed initialize locks");
		return 1;
	}

	ret = install_signal_handler();
	if (0 != ret) {
		perror("Failed to install signal handler");
		goto fail;
	}

	if (0 == strncmp(CFG_ACCEPT_HOST, "auto", 4)) {
		ret = get_host_of_first_nonloopback_device(accept_host);
		if (0 != ret) {
			perror("Failed to autoconfigure accept host");
			goto fail;
		}
	} else {
		strncpy(accept_host, CFG_ACCEPT_HOST, sizeof(accept_host));
	}
	strncpy(accept_name, accept_host, NI_MAXHOST);
	strcat(accept_name, ":");
	strncat(accept_name, accept_port, 5);
	accept_name[NI_MAXHOST + 7 - 1] = '\0';

	method = TLS_server_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		perror("Could not create SSL context");
		goto fail;
	}

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
	do_not_free_ssl = 1;

	ret = BIO_do_accept(accept_bio);
	if (ret <= 0) {
		perror("Failed to create and bind accept socket");
		goto fail;
	}
	printf("Bound to address: %s:%s\n",
			BIO_get_accept_name(accept_bio),
			BIO_get_accept_port(accept_bio));

	ret = serve_connecting_clients(accept_bio);
	if (0 != ret) {
		perror("Fatal error while serving clients");
		goto fail;
	}

	printf("Exiting\n");
	// Free chain set by BIO_set_accept_bios()
	BIO_free_all(accept_bio);
	SSL_CTX_free(ctx);

	return 0;

fail:
	ERR_print_errors_fp(stderr);

	if (0 != accept_bio) {
		BIO_free_all(accept_bio);
	}

	if (0 != ssl && !do_not_free_ssl) {
		SSL_free(ssl);
	}

	if (0 != ctx) {
		SSL_CTX_free(ctx);
	}

	(void) cow_locks_teardown();

	return 1;
}
