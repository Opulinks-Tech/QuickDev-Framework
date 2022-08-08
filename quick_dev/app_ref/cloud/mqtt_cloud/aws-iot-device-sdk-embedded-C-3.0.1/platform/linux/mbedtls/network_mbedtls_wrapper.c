/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <timer_platform.h>
#include <network_interface.h>

// #include "mw_fim_default_group14_project.h"
#include "aws_iot_error.h"
#include "aws_iot_log.h"
#include "network_interface.h"
#include "network_platform.h"

#include "opl_aws_iot_config.h"


#undef LWIP_TIMEVAL_PRIVATE
#define LWIP_TIMEVAL_PRIVATE    0
#include "lwip/sockets.h"


#include "cloud_config.h"

#ifdef BLEWIFI_CONNECT_NON_BLOCKING
#include "netdb.h"
#endif


/* This is the value used for ssl read timeout */
#define IOT_SSL_READ_TIMEOUT 10

/* This defines the value of the debug buffer that gets allocated.
 * The value can be altered based on memory constraints
 */
#ifdef ENABLE_IOT_DEBUG
#define MBEDTLS_DEBUG_BUFFER_SIZE 2048
#endif

const char ssl_client_ca_crt[] =
    "-----BEGIN CERTIFICATE-----\n"\
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"\
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"\
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"\
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"\
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"\
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"\
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"\
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"\
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"\
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"\
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"\
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"\
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"\
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"\
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"\
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"\
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"\
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"\
    "-----END CERTIFICATE-----\n";


/*
 * This is a function to do further verification if needed on the cert received
 */

#if 0
extern T_MwFim_GP14_AWS_PRIVATE_KEY g_tAWSDevicePrivateKeys;
extern T_MwFim_GP14_AWS_CERT_PEM    g_tAWSDeviceCertPEM;
#endif


static int _iot_tls_verify_cert(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags) {
	char buf[1024];
	((void) data);

	IOT_DEBUG("\nVerify requested for (Depth %d):\n", depth);
	mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", crt);
	IOT_DEBUG("%s", buf);

	if((*flags) == 0) {
		IOT_DEBUG("  This certificate has no flags\n");
	} else {
		IOT_DEBUG(buf, sizeof(buf), "  ! ", *flags);
		IOT_DEBUG("%s\n", buf);
	}

	return 0;
}

void _iot_tls_set_connect_params(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
								 char *pDevicePrivateKeyLocation, char *pDestinationURL,
								 uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag) {
	pNetwork->tlsConnectParams.DestinationPort = destinationPort;
	pNetwork->tlsConnectParams.pDestinationURL = pDestinationURL;
	pNetwork->tlsConnectParams.pDeviceCertLocation = pDeviceCertLocation;
	pNetwork->tlsConnectParams.pDevicePrivateKeyLocation = pDevicePrivateKeyLocation;
	pNetwork->tlsConnectParams.pRootCALocation = pRootCALocation;
	pNetwork->tlsConnectParams.timeout_ms = timeout_ms;
	pNetwork->tlsConnectParams.ServerVerificationFlag = ServerVerificationFlag;
}

IoT_Error_t iot_tls_init(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
						 char *pDevicePrivateKeyLocation, char *pDestinationURL,
						 uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag) {
	_iot_tls_set_connect_params(pNetwork, pRootCALocation, pDeviceCertLocation, pDevicePrivateKeyLocation,
								pDestinationURL, destinationPort, timeout_ms, ServerVerificationFlag);

	pNetwork->connect = iot_tls_connect;
	pNetwork->read = iot_tls_read;
	pNetwork->write = iot_tls_write;
	pNetwork->disconnect = iot_tls_disconnect;
	pNetwork->isConnected = iot_tls_is_connected;
	pNetwork->destroy = iot_tls_destroy;

	pNetwork->tlsDataParams.flags = 0;

	return SUCCESS;
}

IoT_Error_t iot_tls_is_connected(Network *pNetwork) {
	/* Use this to add implementation which can check for physical layer disconnect */
	return NETWORK_PHYSICAL_LAYER_CONNECTED;
}

#ifdef BLEWIFI_CONNECT_NON_BLOCKING
int mbedtls_net_connect_patch( mbedtls_net_context *ctx, const char *host, const char *port, int proto )
{
    int ret = -1;
    struct addrinfo hints, *addr_list, *cur;

    /*
    if ( ( ret = net_prepare() ) != 0 ) {
        return ( ret );
    }
    */

    /* Do name resolution with both IPv6 and IPv4 */
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == MBEDTLS_NET_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;

    if ( getaddrinfo( host, port, &hints, &addr_list ) != 0 ) {
        return ( MBEDTLS_ERR_NET_UNKNOWN_HOST );
    }

    /* Try the sockaddrs until a connection succeeds */
    ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;
    for ( cur = addr_list; cur != NULL; cur = cur->ai_next ) {
        int fd = socket( cur->ai_family, cur->ai_socktype, cur->ai_protocol );

        if ( fd < 0 ) {
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }

    #if 1 // non-blocking
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        if ( connect( fd, cur->ai_addr, cur->ai_addrlen ) == 0 ) {
            ctx->fd = fd; // connected!
            ret = 0;
            break;
        } else {
            if (errno == EINPROGRESS) {
                fd_set rfds, wfds;
                struct timeval tv;

                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                FD_SET(fd, &rfds);
                FD_SET(fd, &wfds);

                tv.tv_sec = BLEWIFI_CONNECT_TIMEOUT;
                tv.tv_usec = 0;

                int selres = select(fd + 1, &rfds, &wfds, NULL, &tv);

                if (selres > 0) {
                    //if (FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds)) {
                    if (FD_ISSET(fd, &wfds)) {
                        ctx->fd = fd; // connected!
                        ret = 0;
                        break;
                    }
                }
            }
        }
    #else
        if ( connect( fd, cur->ai_addr, cur->ai_addrlen ) == 0 ) {
            ctx->fd = fd; // connected!
            ret = 0;
            break;
        }
    #endif

        close( fd );
        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    freeaddrinfo( addr_list );

    if(!ret)
    {
        // restore to blocking socket
        fcntl(ctx->fd, F_SETFL, fcntl( ctx->fd, F_GETFL, 0 ) & ~O_NONBLOCK);
    }

    return ( ret );
}
#endif //#ifdef BLEWIFI_CONNECT_NON_BLOCKING

IoT_Error_t iot_tls_connect(Network *pNetwork, TLSConnectParams *params) {
	int ret = 0;
	const char *pers = "aws_iot_tls_wrapper";
	TLSDataParams *tlsDataParams = NULL;
	char portBuffer[6];
	char vrfy_buf[512];
	const char *alpnProtocols[] = { "x-amzn-mqtt-ca", NULL };

    static const int ciphersuite_preference[] =
    {MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256,
     MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA,
     MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256,
     MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,    };

    mbedtls_ssl_setup_preference_ciphersuites(ciphersuite_preference);

#ifdef ENABLE_IOT_DEBUG
	unsigned char buf[MBEDTLS_DEBUG_BUFFER_SIZE];
#endif

	if(NULL == pNetwork) {
		return NULL_VALUE_ERROR;
	}

	if(NULL != params) {
		_iot_tls_set_connect_params(pNetwork, params->pRootCALocation, params->pDeviceCertLocation,
									params->pDevicePrivateKeyLocation, params->pDestinationURL,
									params->DestinationPort, params->timeout_ms, params->ServerVerificationFlag);
	}

	tlsDataParams = &(pNetwork->tlsDataParams);

	mbedtls_net_init(&(tlsDataParams->server_fd));
	mbedtls_ssl_init(&(tlsDataParams->ssl));
	mbedtls_ssl_config_init(&(tlsDataParams->conf));
	mbedtls_ctr_drbg_init(&(tlsDataParams->ctr_drbg));
	mbedtls_x509_crt_init(&(tlsDataParams->cacert));
	mbedtls_x509_crt_init(&(tlsDataParams->clicert));
	mbedtls_pk_init(&(tlsDataParams->pkey));

	IOT_DEBUG("\n  . Seeding the random number generator...");
	mbedtls_entropy_init(&(tlsDataParams->entropy));
	if((ret = mbedtls_ctr_drbg_seed(&(tlsDataParams->ctr_drbg), mbedtls_entropy_func, &(tlsDataParams->entropy),
									(const unsigned char *) pers, strlen(pers))) != 0) {
		IOT_ERROR(" failed\n  ! mbedtls_ctr_drbg_seed returned -0x%x\n", -ret);
		return NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED;
	}

	IOT_DEBUG("  . Loading the CA root certificate ...");
    //Goter, need to fix  //change to mbedtls_x509_crt_parse( mbedtls_x509_crt *chain, const unsigned char *buf, size_t buflen )
    //ret = mbedtls_x509_crt_parse_file(&(tlsDataParams->cacert), pNetwork->tlsConnectParams.pRootCALocation);
    //ret = mbedtls_x509_crt_parse(&(tlsDataParams->cacert), (const unsigned char *) ssl_client_ca_crt, sizeof(ssl_client_ca_crt ));
    if(ret < 0) {
		IOT_ERROR(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing root cert\n\n", -ret);
		return NETWORK_X509_ROOT_CRT_PARSE_ERROR;
	}
	IOT_DEBUG(" ok (%d skipped)\n", ret);

	IOT_DEBUG("  . Loading the client cert. and key...");
    //Goter, need to fix
#if 0
	//ret = mbedtls_x509_crt_parse_file(&(tlsDataParams->clicert), pNetwork->tlsConnectParams.pDeviceCertLocation);
    ret = mbedtls_x509_crt_parse(&(tlsDataParams->clicert), (const unsigned char *)g_tAWSDeviceCertPEM.CertPEM, sizeof(g_tAWSDeviceCertPEM.CertPEM));
    if(ret != 0) {
		IOT_ERROR(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing device cert\n\n", -ret);
		return NETWORK_X509_DEVICE_CRT_PARSE_ERROR;
	}
    //Goter, need to fix
    //DLL_TLS_API int mbedtls_pk_parse_key(mbedtls_pk_context *ctx,
    //                                 const unsigned char *key, size_t keylen,
    //                                 const unsigned char *pwd, size_t pwdlen);
	//ret = mbedtls_pk_parse_keyfile(&(tlsDataParams->pkey), pNetwork->tlsConnectParams.pDevicePrivateKeyLocation, "");
    ret = mbedtls_pk_parse_key(&(tlsDataParams->pkey), (const unsigned char *)g_tAWSDevicePrivateKeys.PrivateKey, sizeof(g_tAWSDevicePrivateKeys.PrivateKey), NULL, NULL);
    if(ret != 0) {
		IOT_ERROR(" failed\n  !  mbedtls_pk_parse_key returned -0x%x while parsing private key\n\n", -ret);
		IOT_DEBUG(" path : %s ", pNetwork->tlsConnectParams.pDevicePrivateKeyLocation);
		return NETWORK_PK_PRIVATE_KEY_PARSE_ERROR;
	}
#endif
	IOT_DEBUG(" ok\n");
	snprintf(portBuffer, 6, "%d", pNetwork->tlsConnectParams.DestinationPort);
	IOT_DEBUG("  . Connecting to %s/%s...", pNetwork->tlsConnectParams.pDestinationURL, portBuffer);
#ifdef BLEWIFI_CONNECT_NON_BLOCKING
    if((ret = mbedtls_net_connect_patch(&(tlsDataParams->server_fd), pNetwork->tlsConnectParams.pDestinationURL,
								  portBuffer, MBEDTLS_NET_PROTO_TCP)) != 0) {
        IOT_ERROR(" failed\n  ! mbedtls_net_connect_patch returned -0x%x\n\n", -ret);
#else
	if((ret = mbedtls_net_connect(&(tlsDataParams->server_fd), pNetwork->tlsConnectParams.pDestinationURL,
								  portBuffer, MBEDTLS_NET_PROTO_TCP)) != 0) {
		IOT_ERROR(" failed\n  ! mbedtls_net_connect returned -0x%x\n\n", -ret);
#endif
		switch(ret) {
			case MBEDTLS_ERR_NET_SOCKET_FAILED:
				return NETWORK_ERR_NET_SOCKET_FAILED;
			case MBEDTLS_ERR_NET_UNKNOWN_HOST:
				return NETWORK_ERR_NET_UNKNOWN_HOST;
			case MBEDTLS_ERR_NET_CONNECT_FAILED:
			default:
				return NETWORK_ERR_NET_CONNECT_FAILED;
		};
	}

	ret = mbedtls_net_set_block(&(tlsDataParams->server_fd));
	if(ret != 0) {
		IOT_ERROR(" failed\n  ! net_set_(non)block() returned -0x%x\n\n", -ret);
		return SSL_CONNECTION_ERROR;
	} IOT_DEBUG(" ok\n");

    {
        struct timeval t;

        t.tv_sec = 3;
        t.tv_usec = 0;

        if (setsockopt(tlsDataParams->server_fd.fd, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) != 0)
        {
            printf("[%s %d] SO_SNDTIMEO fail: fd[%d]\n", __func__, __LINE__, tlsDataParams->server_fd.fd);
        }
    }

	IOT_DEBUG("  . Setting up the SSL/TLS structure...");
	if((ret = mbedtls_ssl_config_defaults(&(tlsDataParams->conf), MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
										  MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		IOT_ERROR(" failed\n  ! mbedtls_ssl_config_defaults returned -0x%x\n\n", -ret);
		return SSL_CONNECTION_ERROR;
	}

	mbedtls_ssl_conf_verify(&(tlsDataParams->conf), _iot_tls_verify_cert, NULL);
	if(pNetwork->tlsConnectParams.ServerVerificationFlag == true) {
		mbedtls_ssl_conf_authmode(&(tlsDataParams->conf), MBEDTLS_SSL_VERIFY_REQUIRED);
	} else {
		mbedtls_ssl_conf_authmode(&(tlsDataParams->conf), MBEDTLS_SSL_VERIFY_OPTIONAL);
	}
	mbedtls_ssl_conf_rng(&(tlsDataParams->conf), mbedtls_ctr_drbg_random, &(tlsDataParams->ctr_drbg));

	mbedtls_ssl_conf_ca_chain(&(tlsDataParams->conf), &(tlsDataParams->cacert), NULL);
#if 0
	if((ret = mbedtls_ssl_conf_own_cert(&(tlsDataParams->conf), &(tlsDataParams->clicert), &(tlsDataParams->pkey))) !=
	   0) {
		IOT_ERROR(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
		return SSL_CONNECTION_ERROR;
	}
#endif
	mbedtls_ssl_conf_read_timeout(&(tlsDataParams->conf), pNetwork->tlsConnectParams.timeout_ms);

	/* Use the AWS IoT ALPN extension for MQTT if port 443 is requested. */
	if(443 == pNetwork->tlsConnectParams.DestinationPort) {
		if((ret = mbedtls_ssl_conf_alpn_protocols(&(tlsDataParams->conf), alpnProtocols)) != 0) {
			IOT_ERROR(" failed\n  ! mbedtls_ssl_conf_alpn_protocols returned -0x%x\n\n", -ret);
			return SSL_CONNECTION_ERROR;
		}
	}

	/* Assign the resulting configuration to the SSL context. */
	if((ret = mbedtls_ssl_setup(&(tlsDataParams->ssl), &(tlsDataParams->conf))) != 0) {
		IOT_ERROR(" failed\n  ! mbedtls_ssl_setup returned -0x%x\n\n", -ret);
		return SSL_CONNECTION_ERROR;
	}
	if((ret = mbedtls_ssl_set_hostname(&(tlsDataParams->ssl), pNetwork->tlsConnectParams.pDestinationURL)) != 0) {
		IOT_ERROR(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
		return SSL_CONNECTION_ERROR;
	}
	IOT_DEBUG("\n\nSSL state connect : %d ", tlsDataParams->ssl.state);
	mbedtls_ssl_set_bio(&(tlsDataParams->ssl), &(tlsDataParams->server_fd), mbedtls_net_send, NULL,
						mbedtls_net_recv_timeout);
	IOT_DEBUG(" ok\n");

	IOT_DEBUG("\n\nSSL state connect : %d ", tlsDataParams->ssl.state);
	IOT_DEBUG("  . Performing the SSL/TLS handshake...");
	while((ret = mbedtls_ssl_handshake(&(tlsDataParams->ssl))) != 0) {
		if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			IOT_ERROR(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n", -ret);
			if(ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
				IOT_ERROR("    Unable to verify the server's certificate. "
							  "Either it is invalid,\n"
							  "    or you didn't set ca_file or ca_path "
							  "to an appropriate value.\n"
							  "    Alternatively, you may want to use "
							  "auth_mode=optional for testing purposes.\n");
			}
			return SSL_CONNECTION_ERROR;
		}
	}

	IOT_DEBUG(" ok\n    [ Protocol is %s ]\n    [ Ciphersuite is %s ]\n", mbedtls_ssl_get_version(&(tlsDataParams->ssl)),
		  mbedtls_ssl_get_ciphersuite(&(tlsDataParams->ssl)));
	if((ret = mbedtls_ssl_get_record_expansion(&(tlsDataParams->ssl))) >= 0) {
		IOT_DEBUG("    [ Record expansion is %d ]\n", ret);
	} else {
		IOT_DEBUG("    [ Record expansion is unknown (compression) ]\n");
	}

	IOT_DEBUG("  . Verifying peer X.509 certificate...");

	if(pNetwork->tlsConnectParams.ServerVerificationFlag == true) {
		if((tlsDataParams->flags = mbedtls_ssl_get_verify_result(&(tlsDataParams->ssl))) != 0) {
			IOT_ERROR(" failed\n");
			mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", tlsDataParams->flags);
			IOT_ERROR("%s\n", vrfy_buf);
			ret = SSL_CONNECTION_ERROR;
		} else {
			IOT_DEBUG(" ok\n");
			ret = SUCCESS;
		}
	} else {
		IOT_DEBUG(" Server Verification skipped\n");
		ret = SUCCESS;
	}

#ifdef ENABLE_IOT_DEBUG
	if (mbedtls_ssl_get_peer_cert(&(tlsDataParams->ssl)) != NULL) {
		IOT_DEBUG("  . Peer certificate information    ...\n");
		mbedtls_x509_crt_info((char *) buf, sizeof(buf) - 1, "      ", mbedtls_ssl_get_peer_cert(&(tlsDataParams->ssl)));
		IOT_DEBUG("%s\n", buf);
	}
#endif

	//mbedtls_ssl_conf_read_timeout(&(tlsDataParams->conf), IOT_SSL_READ_TIMEOUT);
	mbedtls_ssl_conf_read_timeout(&(tlsDataParams->conf), YIELD_TIMEOUT);

	return (IoT_Error_t) ret;
}

IoT_Error_t iot_tls_write(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *written_len) {
	size_t written_so_far;
	bool isErrorFlag = false;
	int frags;
	int ret = 0;
	TLSDataParams *tlsDataParams = &(pNetwork->tlsDataParams);

    //IOT_INFO(" %s ---> t expired  %d  %d", __FUNCTION__, timer->end_time.tv_sec, timer->end_time.tv_usec);
	for(written_so_far = 0, frags = 0;
		written_so_far < len && !has_timer_expired(timer); written_so_far += ret, frags++) {
        //IOT_INFO(" %s --->----> t expired  &d  %d", __FUNCTION__, timer->end_time.tv_sec, timer->end_time.tv_usec);
		while(!has_timer_expired(timer) &&
			  (ret = mbedtls_ssl_write(&(tlsDataParams->ssl), pMsg + written_so_far, len - written_so_far)) <= 0) {
			if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				IOT_ERROR(" failed\n  ! mbedtls_ssl_write returned -0x%x\n\n", -ret);
				/* All other negative return values indicate connection needs to be reset.
		 		* Will be caught in ping request so ignored here */
				isErrorFlag = true;
				break;
			}
		}
        //IOT_INFO(" %s <----<--- t expired  %d  %d", __FUNCTION__, timer->end_time.tv_sec, timer->end_time.tv_usec);
		if(isErrorFlag) {
			break;
		}
	}

	*written_len = written_so_far;

	if(isErrorFlag) {
		return NETWORK_SSL_WRITE_ERROR;
	} else if(has_timer_expired(timer) && written_so_far != len) {
	    //IOT_INFO(" %s <-----> expired  %d  %d", __FUNCTION__, timer->end_time.tv_sec, timer->end_time.tv_usec);
		return NETWORK_SSL_WRITE_TIMEOUT_ERROR;
	}

	return SUCCESS;
}

IoT_Error_t iot_tls_read(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *read_len) {
	mbedtls_ssl_context *ssl = &(pNetwork->tlsDataParams.ssl);
	size_t rxLen = 0;
	int ret;

	while (len > 0) {
		// This read will timeout after IOT_SSL_READ_TIMEOUT if there's no data to be read
		ret = mbedtls_ssl_read(ssl, pMsg, len);
		if (ret > 0) {
			rxLen += ret;
			pMsg += ret;
			len -= ret;
		} else if (ret == 0 || (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_TIMEOUT)) {
            printf("[%s %d] mbedtls_ssl_read fail: ret[%d]\n", __func__, __LINE__, ret);
			return NETWORK_SSL_READ_ERROR;
		}

		// Evaluate timeout after the read to make sure read is done at least once

		if (has_timer_expired(timer)) {
            //IOT_INFO(" %s <-----> expired  %d  %d", __FUNCTION__, timer->end_time.tv_sec, timer->end_time.tv_usec);
			break;
		}
	}

	if (len == 0) {
		*read_len = rxLen;
		return SUCCESS;
	}

	if (rxLen == 0) {
		return NETWORK_SSL_NOTHING_TO_READ;
	} else {
		return NETWORK_SSL_READ_TIMEOUT_ERROR;
	}
}

IoT_Error_t iot_tls_disconnect(Network *pNetwork) {
	mbedtls_ssl_context *ssl = &(pNetwork->tlsDataParams.ssl);
	int ret = 0;
	do {
		ret = mbedtls_ssl_close_notify(ssl);
	} while(ret == MBEDTLS_ERR_SSL_WANT_WRITE);

	/* All other negative return values indicate connection needs to be reset.
	 * No further action required since this is disconnect call */

	return SUCCESS;
}

IoT_Error_t iot_tls_destroy(Network *pNetwork) {
	TLSDataParams *tlsDataParams = &(pNetwork->tlsDataParams);

	mbedtls_net_free(&(tlsDataParams->server_fd));

	mbedtls_x509_crt_free(&(tlsDataParams->clicert));
	mbedtls_x509_crt_free(&(tlsDataParams->cacert));
	mbedtls_pk_free(&(tlsDataParams->pkey));
	mbedtls_ssl_free(&(tlsDataParams->ssl));
	mbedtls_ssl_config_free(&(tlsDataParams->conf));
	mbedtls_ctr_drbg_free(&(tlsDataParams->ctr_drbg));
	mbedtls_entropy_free(&(tlsDataParams->entropy));

	return SUCCESS;
}

#ifdef __cplusplus
}
#endif
