
#include "transport_interface.h"
#include "mbedtls/config.h"

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"

#include "log.h"

#define MBED_OPL_LOG_ENABLED            (0)

#if (MBED_OPL_LOG_ENABLED == 1)
// all log level messages will logged.
#define LogError(...)                   OPL_LOG_ERRO(MBED_OPL, __VA_ARGS__)
#define LogWarn(...)                    OPL_LOG_WARN(MBED_OPL, __VA_ARGS__)
#define LogInfo(...)                    OPL_LOG_INFO(MBED_OPL, __VA_ARGS__)
#define LogDebug(...)                   OPL_LOG_DEBG(MBED_OPL, __VA_ARGS__)
#else
#define LogError(...)
#define LogWarn(...)
#define LogInfo(...)
#define LogDebug(...)
#endif

typedef enum MbedtlsOplStatus
{
    MBEDTLS_OPL_SUCCESS = 0,
    MBEDTLS_OPL_INVALID_PARAMETER,
    MBEDTLS_OPL_INSUFFICIENT_MEMORY,
    MBEDTLS_OPL_INVALID_CREDENTIALS,
    MBEDTLS_OPL_HANDSHAKE_ERROR,
    MBEDTLS_OPL_INTERNAL_ERROR,
    MBEDTLS_OPL_CONNECT_FALIURE,
    MBEDTLS_OPL_CTR_DRBG_ENTROPY_SOURCE_FAILED,
} MbedtlsOplStatus_t;

typedef struct MbedtlsOplContext
{
    mbedtls_net_context socketContext;    /**< @brief MbedTLS socket context. */
    mbedtls_ssl_config config;            /**< @brief SSL connection configuration. */
    mbedtls_ssl_context context;          /**< @brief SSL connection context */
    // mbedtls_x509_crt_profile certProfile; /**< @brief Certificate security profile for this connection. */
    mbedtls_x509_crt rootCa;              /**< @brief Root CA certificate context. */
    mbedtls_x509_crt clientCert;          /**< @brief Client certificate context. */
    mbedtls_pk_context privKey;           /**< @brief Client private key context. */
    // mbedtls_pk_info_t privKeyInfo;        /**< @brief Client private key info. */
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    // uint32_t flags;

    // /* PKCS #11. */
    // CK_FUNCTION_LIST_PTR pP11FunctionList; /**< @brief PKCS #11 function list. */
    // CK_SESSION_HANDLE p11Session;          /**< @brief PKCS #11 session. */
    // CK_OBJECT_HANDLE p11PrivateKey;        /**< @brief PKCS #11 handle for the private key to use for client authentication. */
    // CK_KEY_TYPE keyType;                   /**< @brief PKCS #11 key type corresponding to #p11PrivateKey. */
} MbedtlsOplContext_t;

typedef struct MbedtlsOplCredentials
{
    bool disableSni;
    char *pRootCaLabel;
    char *pClientCertLabel;
    char *pPrivateKeyLabel;
    uint32_t timeout_ms;
} MbedtlsOplCredentials_t;

MbedtlsOplStatus_t Mbedtls_Opl_Connect( NetworkContext_t *ptNetworkContext, 
                                        const char *pHostName, 
                                        uint16_t u16Port, 
                                        MbedtlsOplCredentials_t *ptMbedtlsOplCredentials, 
                                        uint32_t u32TimeoutMs);

void Mbedtls_Opl_Disconnect(NetworkContext_t *ptNetworkContext);

int32_t Mbedtls_Opl_Recv(NetworkContext_t *ptNetworkContext, void *pBuffer, size_t bytesToRecv);

int32_t Mbedtls_Opl_Send(NetworkContext_t *ptNetworkContext, const void *pBuffer, size_t bytesToSend);
