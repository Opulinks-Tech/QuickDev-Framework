#ifndef __SSL_CLIENT_H__
#define __SSL_CLIENT_H__

#define HTTP_ENABLE_TLS


#ifdef HTTP_ENABLE_TLS

#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#define SSL_HANDSHAKE_TIMEOUT        (4600)
#define SSL_SOCKET_TIMEOUT           (5000)    // ms

extern mbedtls_net_context g_server_fd;
extern mbedtls_ssl_context g_ssl;
extern mbedtls_ssl_config g_conf;
extern mbedtls_x509_crt g_cacert;
extern mbedtls_ctr_drbg_context g_ctr_drbg;
extern mbedtls_entropy_context g_entropy;

int ssl_Init(mbedtls_net_context *server_fd,
                           mbedtls_ssl_context *ssl,
                           mbedtls_ssl_config *conf,
                           mbedtls_x509_crt *cacert,
                           mbedtls_ctr_drbg_context *ctr_drbg,
                           mbedtls_entropy_context *entropy);

int ssl_Establish(mbedtls_net_context *server_fd,
                           mbedtls_ssl_context *ssl,
                           mbedtls_ssl_config *conf,
                           mbedtls_x509_crt *cacert,
                           mbedtls_ctr_drbg_context *ctr_drbg, const char *host, uint16_t port);


int ssl_Recv(mbedtls_ssl_context *ssl, char *buf, uint32_t len);

int ssl_Destroy(mbedtls_ssl_context *ssl);

int ssl_Send(mbedtls_ssl_context *ssl, unsigned char *buf, int len, uint32_t timeout_ms);

void ssl_Read_Timeout_Set(mbedtls_ssl_config *conf, uint32_t timeout);

#endif

#endif
