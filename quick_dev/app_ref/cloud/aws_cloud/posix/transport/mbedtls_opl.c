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

#include "mbedtls_opl.h"
#include "sys_cfg.h"
#include "qd_config.h"

// #include "controller_wifi.h"
// #include "controller_wifi_com.h"
// #include "lwip/etharp.h"


/* This is the value used for ssl read timeout */
#define IOT_SSL_READ_TIMEOUT 10

/* This defines the value of the debug buffer that gets allocated.
 * The value can be altered based on memory constraints
 */
#ifdef ENABLE_IOT_DEBUG
#define MBEDTLS_DEBUG_BUFFER_SIZE 2048
#endif

// each compilation unit that consumes the NetworkContext must defeine it.
// It should contain a single pointer as seen below whenever the header file
// of this transport implementation is included to your project.
//
// when using multiple transports in the same compilation unit,
// define this pointer as void *.
struct NetworkContext
{
    MbedtlsOplContext_t *tMbedtlsOplContext;
    MbedtlsOplCredentials_t *tMbedtlsOplCredentials;
};


static void contextInit(MbedtlsOplContext_t *ptMbedtlsOplContext)
{
    // assert(ptMbedtlsOplContext != NULL);

    mbedtls_net_init(&(ptMbedtlsOplContext->socketContext));
    mbedtls_ssl_init(&(ptMbedtlsOplContext->context));
    mbedtls_ssl_config_init(&(ptMbedtlsOplContext->config));
    mbedtls_x509_crt_init(&(ptMbedtlsOplContext->rootCa));
    mbedtls_x509_crt_init(&(ptMbedtlsOplContext->clientCert));
    mbedtls_pk_init(&(ptMbedtlsOplContext->privKey));
    mbedtls_ctr_drbg_init(&(ptMbedtlsOplContext->ctr_drbg));
    mbedtls_entropy_init(&(ptMbedtlsOplContext->entropy));
}

static void contextFree(MbedtlsOplContext_t *ptMbedtlsOplContext)
{
    if(NULL != ptMbedtlsOplContext)
    {
        mbedtls_net_free(&(ptMbedtlsOplContext->socketContext));
        mbedtls_ssl_free(&(ptMbedtlsOplContext->context));
        mbedtls_ssl_config_free(&(ptMbedtlsOplContext->config));
        mbedtls_x509_crt_free(&(ptMbedtlsOplContext->rootCa));
        mbedtls_x509_crt_free(&(ptMbedtlsOplContext->clientCert));
        mbedtls_pk_free(&(ptMbedtlsOplContext->privKey));
        mbedtls_ctr_drbg_free(&(ptMbedtlsOplContext->ctr_drbg));
        mbedtls_entropy_free(&(ptMbedtlsOplContext->entropy));
    }
}

MbedtlsOplStatus_t Mbedtls_Opl_ConfigureCertificates(MbedtlsOplContext_t *ptMbedtlsOplContext, const MbedtlsOplCredentials_t *ptMbedtlsOplCredentials)
{
    MbedtlsOplStatus_t ret = MBEDTLS_OPL_SUCCESS;
    int32_t mbedtlsError = 0;
    // bool result;

    // parse the server root CA certificate into the SSL context
    mbedtlsError = mbedtls_x509_crt_parse( &(ptMbedtlsOplContext->rootCa), 
                                           (const unsigned char *)ptMbedtlsOplCredentials->pRootCaLabel,
                                           strlen(ptMbedtlsOplCredentials->pRootCaLabel) + 1);
    
    if(0 > mbedtlsError)
    {
        LogError("failed\n ! mbedtls_x509_crt_parse returned -0x%x while parsing root CA %d\n\n", -mbedtlsError, strlen(ptMbedtlsOplCredentials->pRootCaLabel));
        ret = MBEDTLS_OPL_INVALID_CREDENTIALS;
    }
    else
    {
        mbedtls_ssl_conf_ca_chain(&(ptMbedtlsOplContext->config), &(ptMbedtlsOplContext->rootCa), NULL);

        // setup the client private key into the SSL context
        mbedtlsError = mbedtls_pk_parse_key( &(ptMbedtlsOplContext->privKey),
                                             (const unsigned char *)ptMbedtlsOplCredentials->pPrivateKeyLabel,
                                             strlen(ptMbedtlsOplCredentials->pPrivateKeyLabel) + 1,
                                             NULL,
                                             NULL);
    }

    if(0 > mbedtlsError)
    {
        LogError("failed\n ! mbedtls_pk_parse_key returned -0x%x while parsing private key\n\n", -mbedtlsError);
        ret = MBEDTLS_OPL_INVALID_CREDENTIALS;
    }
    else
    {
        // parse the client certificate into the SSL context
        mbedtlsError = mbedtls_x509_crt_parse( &(ptMbedtlsOplContext->clientCert),
                                               (const unsigned char *)ptMbedtlsOplCredentials->pClientCertLabel,
                                               strlen(ptMbedtlsOplCredentials->pClientCertLabel) + 1);
    }

    if(0 > mbedtlsError)
    {
        LogError("failed\n ! mbedtls_x509_crt_parse returned -0x%x while parsing client cert %d\n\n", -mbedtlsError, strlen(ptMbedtlsOplCredentials->pClientCertLabel));
        ret = MBEDTLS_OPL_INVALID_CREDENTIALS;
    }
    else
    {
        mbedtlsError = mbedtls_ssl_conf_own_cert( &(ptMbedtlsOplContext->config), 
                                                  &(ptMbedtlsOplContext->clientCert), 
                                                  &(ptMbedtlsOplContext->privKey));

        if(0 != mbedtlsError)
        {
            LogError("failed\n ! mbedtls_ssl_conf_own_cert returned %d\n\n", mbedtlsError);
            ret = MBEDTLS_OPL_INVALID_CREDENTIALS;
        }
    }

    return ret;
}

MbedtlsOplStatus_t Mbedtls_Opl_ConfigureSniAlpn( MbedtlsOplContext_t *ptMbedtlsOplContext,
                                                 const MbedtlsOplCredentials_t *ptMbedtlsOplCredentials,
                                                 const char *pHostName)
{
    MbedtlsOplStatus_t ret = MBEDTLS_OPL_SUCCESS;
    int32_t mbedtlsError = 0;
    const char *alpnProtocols[] = {"x-amzn-mqtt-ca", NULL};  // using "x-amzn-mqtt-ca" protocol for ALPN extension

    // include an application protocol list in the TLS ClientHello message
    mbedtlsError = mbedtls_ssl_conf_alpn_protocols(&(ptMbedtlsOplContext->config), alpnProtocols);

    if(0 != mbedtlsError)
    {
        LogError("failed\n ! mbedtls_ssl_conf_alpn_protocols returned -0x%x\n\n", -mbedtlsError);
        ret = MBEDTLS_OPL_CONNECT_FALIURE;
    }
    
    // enable SNI if requested
    if((MBEDTLS_OPL_SUCCESS == ret) && (ptMbedtlsOplCredentials->disableSni == false))
    {
        mbedtlsError = mbedtls_ssl_set_hostname(&(ptMbedtlsOplContext->context), pHostName);

        if(0 != mbedtlsError)
        {
            LogError("failed\n ! mbedtls_ssl_set_hostname returned -0x%x\n\n", -mbedtlsError);
            ret = MBEDTLS_OPL_CONNECT_FALIURE;
        }
    }

    return ret;
}

MbedtlsOplStatus_t Mbedtls_Opl_Configure( MbedtlsOplContext_t *ptMbedtlsOplContext, 
                                          const char *pHostName, 
                                          const MbedtlsOplCredentials_t *ptMbedtlsOplCredentials, 
                                          uint32_t u32TimeoutMs)
{
    MbedtlsOplStatus_t ret = MBEDTLS_OPL_SUCCESS;
    int32_t mbedtlsError = 0;
    const char *pers = "mbedtls_opl";


    static const int ciphersuite_perference[] = {
        // MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256,
        // MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA,
        // MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256,
        MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,
    };

    // restrict ciphers
    // mbedtls_ssl_conf_ciphersuites( &(ptMbedtlsOplContext->config), ciphersuite_perference);
    mbedtls_ssl_setup_preference_ciphersuites(ciphersuite_perference);

    mbedtls_ssl_config_max_content_len(1024 * 6);

    // Initialize the MbedTLS context structure
    contextInit(ptMbedtlsOplContext);

    mbedtlsError = mbedtls_ssl_config_defaults( &(ptMbedtlsOplContext->config), 
                                                MBEDTLS_SSL_IS_CLIENT,
                                                MBEDTLS_SSL_TRANSPORT_STREAM,
                                                MBEDTLS_SSL_PRESET_DEFAULT);

    if(0 != mbedtlsError)
    {
        LogError("failed\n ! mbedtls_ssl_config_default returned -0x%x\n\n", -mbedtlsError);
        ret = MBEDTLS_OPL_CONNECT_FALIURE;
    }
    else
    {
        // seeding the random number generator
        mbedtlsError = mbedtls_ctr_drbg_seed( &(ptMbedtlsOplContext->ctr_drbg),
                                              mbedtls_entropy_func, 
                                              &(ptMbedtlsOplContext->entropy),
                                              (const unsigned char *)pers,
                                              strlen(pers));

        if(0 != mbedtlsError)
        {
            LogError("failed\n ! mbedtls_ctr_drbg_seed returned -0x%x\n\n", -mbedtlsError);
            ret = MBEDTLS_OPL_CTR_DRBG_ENTROPY_SOURCE_FAILED;
        }
    }
    
    if(MBEDTLS_OPL_SUCCESS == ret)
    {
        // set SSL authmode and the RNG context
        mbedtls_ssl_conf_authmode(&(ptMbedtlsOplContext->config), MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_rng(&(ptMbedtlsOplContext->config), mbedtls_ctr_drbg_random, &(ptMbedtlsOplContext->ctr_drbg));
        mbedtls_ssl_conf_read_timeout(&(ptMbedtlsOplContext->config), u32TimeoutMs);
        // mbedtls_ssl_conf_cert_profile(&(ptMbedtlsOplContext->config), &(ptMbedtlsOplContext->certProfile));
        // mbedtls_ssl_conf_dbg(&ptMbedtlsOplContext->config, mbedtlsDebugPrint, NULL);
        // mbedtls_debug_set_threshold(MBEDTLS_DEBUG_LOG_LEVEL);

        ret = Mbedtls_Opl_ConfigureCertificates(ptMbedtlsOplContext, ptMbedtlsOplCredentials);
    }

    if(MBEDTLS_OPL_SUCCESS == ret)
    {
        ret = Mbedtls_Opl_ConfigureSniAlpn(ptMbedtlsOplContext, ptMbedtlsOplCredentials, pHostName);
    }

    if(MBEDTLS_OPL_SUCCESS == ret)
    {
        mbedtlsError = mbedtls_ssl_setup(&(ptMbedtlsOplContext->context), &(ptMbedtlsOplContext->config));

        if(0 != mbedtlsError)
        {
            LogError("failed\n ! mbedtls_ssl_setup returned -0x%x\n\n", -mbedtlsError);
            ret = MBEDTLS_OPL_CONNECT_FALIURE;
        }
    }

    if(MBEDTLS_OPL_SUCCESS == ret)
    {
        // set the underlying IO for the TLS connection
        mbedtls_ssl_set_bio( &(ptMbedtlsOplContext->context), 
                                &(ptMbedtlsOplContext->socketContext), 
                                mbedtls_net_send,
                                mbedtls_net_recv,
                                mbedtls_net_recv_timeout);
    }

    if(MBEDTLS_OPL_SUCCESS != ret)
    {
        contextFree(ptMbedtlsOplContext);
    }

    return ret;    
}

MbedtlsOplStatus_t Mbedtls_Opl_Connect( NetworkContext_t *ptNetworkContext, 
                                        const char *pHostName, 
                                        uint16_t u16Port, 
                                        MbedtlsOplCredentials_t *ptMbedtlsOplCredentials,
                                        uint32_t u32TimeoutMs)
{
    MbedtlsOplStatus_t ret = MBEDTLS_OPL_SUCCESS;
    MbedtlsOplContext_t *pMbedtlsOplContext = NULL;
    int32_t mbedtlsError = 0;
    char portStr[6] = {0};
//     char vrfy_buf[512];

    if( (ptNetworkContext == NULL) ||
        (ptNetworkContext->tMbedtlsOplContext == NULL) ||
        (ptNetworkContext->tMbedtlsOplCredentials == NULL) ||
        (ptMbedtlsOplCredentials == NULL) ||
        (ptMbedtlsOplCredentials->pRootCaLabel == NULL) ||
        (ptMbedtlsOplCredentials->pClientCertLabel == NULL) ||
        (ptMbedtlsOplCredentials->pPrivateKeyLabel == NULL))
    {
        LogError("Invalid input parameter %x %x %x %x %x %x %x\r\n",
                    ptNetworkContext,
                    ptNetworkContext->tMbedtlsOplContext,
                    ptNetworkContext->tMbedtlsOplCredentials,
                    ptMbedtlsOplCredentials,
                    ptMbedtlsOplCredentials->pRootCaLabel,
                    ptMbedtlsOplCredentials->pClientCertLabel,
                    ptMbedtlsOplCredentials->pPrivateKeyLabel);
        ret = MBEDTLS_OPL_INVALID_PARAMETER;
    }
    else
    {
        snprintf(portStr, sizeof(portStr), "%u", u16Port);
        pMbedtlsOplContext = ptNetworkContext->tMbedtlsOplContext;

        ret = Mbedtls_Opl_Configure(pMbedtlsOplContext, pHostName, ptMbedtlsOplCredentials, u32TimeoutMs);
    }

    if(MBEDTLS_OPL_SUCCESS == ret)
    {        
// #if defined(OPL1000_A2) || defined(OPL1000_A3)
//     CtrlWifi_PsStateForce(STA_PS_AWAKE_MODE, 0); // always wake-up
// #endif

        mbedtlsError = mbedtls_net_connect( &(pMbedtlsOplContext->socketContext), 
                                            pHostName, 
                                            portStr, 
                                            MBEDTLS_NET_PROTO_TCP);

// #if defined(OPL1000_A2) || defined(OPL1000_A3)
//     CtrlWifi_PsStateForce(STA_PS_NONE, 0);
// #endif

        // printf("cipher using %s\n", mbedtls_ssl_get_ciphersuite(&pMbedtlsOplContext->context));

        if(0 != mbedtlsError)
        {
            LogError("failed\n ! mbedtls_net_connect returned -0x%x\n\n", -mbedtlsError);
            ret = MBEDTLS_OPL_CONNECT_FALIURE;
        }
    }

    if(MBEDTLS_OPL_SUCCESS == ret)
    {
        // perform the TLS handshake

        mbedtls_ssl_conf_read_timeout(&(pMbedtlsOplContext->config), 5000);

#if defined(OPL2500_A0)
        // TODO: sets back to 143MHz while opl2500 is ready (Handshake clock rate)
        sys_cfg_clk_set(SYS_CFG_CLK_44_MHZ);
#else
	    sys_cfg_clk_set(SYS_CFG_CLK_143_MHZ);
#endif

        while((mbedtlsError = mbedtls_ssl_handshake(&(pMbedtlsOplContext->context))) != 0)
        {
            if(MBEDTLS_ERR_SSL_WANT_READ != mbedtlsError && MBEDTLS_ERR_SSL_WANT_WRITE != mbedtlsError)
            {
                LogError("failed\n ! mbedtls_ssl_handshake returned -0x%x\n", -mbedtlsError);
                
                if(MBEDTLS_ERR_X509_CERT_VERIFY_FAILED == mbedtlsError)
                {
                    LogError("Unable to verify the server's certificate. Either it is invalid,\n"
                           "or you didn't set ca_file or ca_path to an apporpriate value."
                           "Alternatively, you may want ot use auth_mode=optional for testing purpose.\n");
                }

                ret = MBEDTLS_OPL_HANDSHAKE_ERROR;

                break;
            }
        }

        sys_cfg_clk_set(SYS_CFG_CLK_RATE);

        mbedtls_ssl_conf_read_timeout(&(pMbedtlsOplContext->config), u32TimeoutMs);
    }

    if(MBEDTLS_OPL_SUCCESS != ret)
    {
        // clean up on failure
        contextFree(pMbedtlsOplContext);
    }

    return ret;
}

void Mbedtls_Opl_Disconnect(NetworkContext_t *ptNetworkContext)
{
    MbedtlsOplContext_t *pMbedtlsOplContext = NULL;
    int32_t mbedtlsError = 0;

    if( (ptNetworkContext == NULL) ||
        (ptNetworkContext->tMbedtlsOplContext == NULL))
    {
        LogError("Invalid input parameter\r\n");
    }
    else
    {
        pMbedtlsOplContext = ptNetworkContext->tMbedtlsOplContext;

        // attempting to terminate TLS connection
        mbedtlsError = mbedtls_ssl_close_notify(&(pMbedtlsOplContext->context));

        if(0 == mbedtlsError)
        {
            LogError("closing TLS connection: TLS close-notify sent\n");
        }
        else if((MBEDTLS_ERR_SSL_WANT_READ == mbedtlsError) && (MBEDTLS_ERR_SSL_WANT_WRITE == mbedtlsError))
        {
            LogError("TLS clos-notify send; receive %s as the TLS status which can be ignored for clos-notify.\n",
                    (mbedtlsError == MBEDTLS_ERR_SSL_WANT_READ) ? "WANT_READ" : "WANT_WRITE");
        }
        else
        {
            LogError("failed\n ! mbedtls_ssl_close_notify returned -0x%x\n\n", mbedtlsError);
        }

        // free context
        contextFree(pMbedtlsOplContext);
    }
}

int32_t Mbedtls_Opl_Recv(NetworkContext_t *ptNetworkContext, void *pBuffer, size_t bytesToRecv)
{
    MbedtlsOplContext_t *pMbedtlsOplContext = NULL;
    int32_t tlsStatus = 0;

    pMbedtlsOplContext = ptNetworkContext->tMbedtlsOplContext;
    tlsStatus = (int32_t)mbedtls_ssl_read( &(pMbedtlsOplContext->context),
                                           pBuffer,
                                           bytesToRecv);
    
    if( (MBEDTLS_ERR_SSL_TIMEOUT == tlsStatus) ||
        (MBEDTLS_ERR_SSL_WANT_READ == tlsStatus) ||
        (MBEDTLS_ERR_SSL_WANT_WRITE == tlsStatus))
    {
        LogError("failed to read data -0x%x. However, a read can be retried on this error.\n", -tlsStatus);

        // mark these set of errors as a timeout. The libraries may retry read on these errors.
        tlsStatus = 0; 
    }
    else if(0 > tlsStatus)
    {
        LogError("failed to read data -0x%x.\n", -tlsStatus);
    }
    else
    {
        // empty else marker
    }

    return tlsStatus;
}

int32_t Mbedtls_Opl_Send(NetworkContext_t *ptNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    MbedtlsOplContext_t *pMbedtlsOplContext = NULL;
    int32_t tlsStatus = 0;

    pMbedtlsOplContext = ptNetworkContext->tMbedtlsOplContext;
    tlsStatus = (int32_t)mbedtls_ssl_write( &(pMbedtlsOplContext->context),
                                            pBuffer,
                                            bytesToSend);
    
    if( (MBEDTLS_ERR_SSL_TIMEOUT == tlsStatus) ||
        (MBEDTLS_ERR_SSL_WANT_READ == tlsStatus) ||
        (MBEDTLS_ERR_SSL_WANT_WRITE == tlsStatus))
    {
        LogError("failed to send data -0x%x. However, a send can be retried on this error.\n", -tlsStatus); 
    }
    else if(0 > tlsStatus)
    {
        LogError("failed to send data -0x%x.\n", -tlsStatus);
    }
    else
    {
        // empty else marker
    }

    return tlsStatus;
}

#ifdef __cplusplus
}
#endif
