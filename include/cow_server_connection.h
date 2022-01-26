#ifndef COW_SERVER_CONNECTION_H_DEFINED
#define COW_SERVER_CONNECTION_H_DEFINED

#include <openssl/ssl.h>

void cow_server_connection_set_accept_BIO(BIO* new_accept_bio);
BIO* cow_server_connection_get_accept_BIO(void);

void cow_server_connection_set_SSL(SSL* new_ssl);
SSL* cow_server_connection_get_SSL(void);

void cow_server_connection_set_SSL_CTX(SSL_CTX* new_ssl_ctx);
SSL_CTX* cow_server_connection_get_SSL_CTX(void);

int cow_server_connection_setup();
int cow_server_connection_listen_and_handle();
void cow_server_connection_teardown();

#endif
