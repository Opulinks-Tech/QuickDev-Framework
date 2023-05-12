#include "ssl_client.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>
#include "lwip/netif.h"
// #include "blewifi_configuration.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sys_cfg.h"
// #include "app_configuration.h"
#include "qd_config.h"
#include "qd_module.h"

#ifdef HTTP_ENABLE_TLS

#if 0
const char ssl_client_ca_crt[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIICxDCCAi2gAwIBAgIJAKZOAGaffv/UMA0GCSqGSIb3DQEBBQUAMHoxCzAJBgNV\r\n"
    "BAYTAmNiMQswCQYDVQQIDAJnZDELMAkGA1UEBwwCc3oxEDAOBgNVBAoMB2Nvb2xr\r\n"
    "aXQxDDAKBgNVBAsMA2RldjETMBEGA1UEAwwKY29vbGtpdC5jbjEcMBoGCSqGSIb3\r\n"
    "DQEJARYNMjIwMzM1QHFxLmNvbTAgFw0xNzA3MTEwOTUzNTFaGA8yMTE3MDYxNzA5\r\n"
    "NTM1MVowejELMAkGA1UEBhMCY2IxCzAJBgNVBAgMAmdkMQswCQYDVQQHDAJzejEQ\r\n"
    "MA4GA1UECgwHY29vbGtpdDEMMAoGA1UECwwDZGV2MRMwEQYDVQQDDApjb29sa2l0\r\n"
    "LmNuMRwwGgYJKoZIhvcNAQkBFg0yMjAzMzVAcXEuY29tMIGfMA0GCSqGSIb3DQEB\r\n"
    "AQUAA4GNADCBiQKBgQDNib2yd5iOrhUahGb9YPxVXJU16uBIFMgbTlfJu0JzxdOk\r\n"
    "Ejt0i3+6Ijz6ISmNY+0/ojOLlXO7qPmBDl/DQtn0faigzVOtJFJZdNaiAnUkGVvp\r\n"
    "/4RCIhdmHVXj3fL2Ojcuh9ua6k2MaFUIroHiyD6c0Bict8jke1hIEpP8On2anQID\r\n"
    "AQABo1AwTjAdBgNVHQ4EFgQUwFM57JgjWg7fzO/tEPjZYdYDz74wHwYDVR0jBBgw\r\n"
    "FoAUwFM57JgjWg7fzO/tEPjZYdYDz74wDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0B\r\n"
    "AQUFAAOBgQC62kzGjskLHLuPY0Em+xl26SQmnx0mJOLgzLx13lpc5xf0vWWSsiI+\r\n"
    "IGCA+ybWXavTUZAbJ2waLA5eQJCGgEosnO5Nce3OR3kxfHCdW7k+fVvQqlmU0mQR\r\n"
    "K8U0/gOdogOGK7McH+UK4QYjeECcFZp1WD/uinsXg4u2hiuGBw7Dwg==\r\n"
    "-----END CERTIFICATE-----\r\n";
#endif


char ssl_client_ca_crt[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIICxDCCAi2gAwIBAgIJAKZOAGaffv/UMA0GCSqGSIb3DQEBBQUAMHoxCzAJBgNV\r\n"
    "BAYTAmNiMQswCQYDVQQIDAJnZDELMAkGA1UEBwwCc3oxEDAOBgNVBAoMB2Nvb2xr\r\n"
    "aXQxDDAKBgNVBAsMA2RldjETMBEGA1UEAwwKY29vbGtpdC5jbjEcMBoGCSqGSIb3\r\n"
    "DQEJARYNMjIwMzM1QHFxLmNvbTAgFw0xNzA3MTEwOTUzNTFaGA8yMTE3MDYxNzA5\r\n"
    "NTM1MVowejELMAkGA1UEBhMCY2IxCzAJBgNVBAgMAmdkMQswCQYDVQQHDAJzejEQ\r\n"
    "MA4GA1UECgwHY29vbGtpdDEMMAoGA1UECwwDZGV2MRMwEQYDVQQDDApjb29sa2l0\r\n"
    "LmNuMRwwGgYJKoZIhvcNAQkBFg0yMjAzMzVAcXEuY29tMIGfMA0GCSqGSIb3DQEB\r\n"
    "AQUAA4GNADCBiQKBgQDNib2yd5iOrhUahGb9YPxVXJU16uBIFMgbTlfJu0JzxdOk\r\n"
    "Ejt0i3+6Ijz6ISmNY+0/ojOLlXO7qPmBDl/DQtn0faigzVOtJFJZdNaiAnUkGVvp\r\n"
    "/4RCIhdmHVXj3fL2Ojcuh9ua6k2MaFUIroHiyD6c0Bict8jke1hIEpP8On2anQID\r\n"
    "AQABo1AwTjAdBgNVHQ4EFgQUwFM57JgjWg7fzO/tEPjZYdYDz74wHwYDVR0jBBgw\r\n"
    "FoAUwFM57JgjWg7fzO/tEPjZYdYDz74wDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0B\r\n"
    "AQUFAAOBgQC62kzGjskLHLuPY0Em+xl26SQmnx0mJOLgzLx13lpc5xf0vWWSsiI+\r\n"
    "IGCA+ybWXavTUZAbJ2waLA5eQJCGgEosnO5Nce3OR3kxfHCdW7k+fVvQqlmU0mQR\r\n"
    "K8U0/gOdogOGK7McH+UK4QYjeECcFZp1WD/uinsXg4u2hiuGBw7Dwg==\r\n"
    "-----END CERTIFICATE-----\r\n";



#if 1
static const int ciphersuite_preference[] =
{
    /* All AES-256 ephemeral suites */
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA,
    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,
    0
};
#endif

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    const char *p, *basename;

    /* Extract basename from file */
    for( p = basename = file; *p != '\0'; p++ )
        if( *p == '/' || *p == '\\' )
            basename = p + 1;

    printf("%s:%04d: |%d| %s", basename, line, level, str );
}

#define SSL_NET_SEND_TIMEOUT            (5) // sec

mbedtls_net_context g_server_fd;
mbedtls_ssl_context g_ssl;
mbedtls_ssl_config g_conf;
mbedtls_x509_crt g_cacert;
mbedtls_ctr_drbg_context g_ctr_drbg;
mbedtls_entropy_context g_entropy;

int ssl_Init(mbedtls_net_context *server_fd,
                           mbedtls_ssl_context *ssl,
                           mbedtls_ssl_config *conf,
                           mbedtls_x509_crt *cacert,
                           mbedtls_ctr_drbg_context *ctr_drbg,
                           mbedtls_entropy_context *entropy)
{
    int ret = 0;
    const char *pers = "ssl_client";

    if(ssl!=NULL)
        ssl_Destroy(ssl);

    printf("SSL client starts\n");

    /*
     * 0. Initialize the RNG and the session data
     */
    // mbedtls_ssl_config_max_content_len(1024*8);
    mbedtls_ssl_config_max_content_len(1024*6);

    mbedtls_ssl_setup_preference_ciphersuites(ciphersuite_preference);
    mbedtls_net_init( server_fd );
    mbedtls_ssl_init( ssl );
    mbedtls_ssl_config_init( conf );
    mbedtls_x509_crt_init( cacert );
    mbedtls_ctr_drbg_init( ctr_drbg );


    //printf("Seeding the random number generator...");

    mbedtls_entropy_init( entropy );
    //printf("entropy_inited");
    if ((ret = mbedtls_ctr_drbg_seed( ctr_drbg, mbedtls_entropy_func, entropy,
                               (const unsigned char *)pers, strlen(pers))) != 0)
    {
        printf("failed\n  ! mbedtls_ctr_drbg_seed returned -0x%x\n", -ret );
        return false;
    }

    /*
     * 0. Initialize certificates
     */
    printf("Loading the CA root certificate ...");

    ret = mbedtls_x509_crt_parse(cacert, (const unsigned char *) ssl_client_ca_crt,
                          sizeof(ssl_client_ca_crt));
    if (ret < 0)
    {
        printf("mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
        return false;
    }

    printf("ok (%d skipped)\n", ret);


    return true;
}

#define SOCKET_CONNECT_TIMEOUT       (3000)
#define TCP_LOCAL_PORT_RANGE_START        0xc000
#define TCP_LOCAL_PORT_RANGE_END          0xffff
#define TCP_ENSURE_LOCAL_PORT_RANGE(port) ((u16_t)(((port) & ~TCP_LOCAL_PORT_RANGE_START) + TCP_LOCAL_PORT_RANGE_START))

/*
 * Set the socket blocking or non-blocking
 */
static int mbedtls_net_ex_set_block(mbedtls_net_context *ctx)
{
    return ( fcntl( ctx->fd, F_SETFL, fcntl( ctx->fd, F_GETFL, 0 ) & ~O_NONBLOCK ) );
}

static int mbedtls_net_ex_set_nonblock(mbedtls_net_context *ctx)
{
    return ( fcntl( ctx->fd, F_SETFL, fcntl( ctx->fd, F_GETFL, 0 ) | O_NONBLOCK ) );
}

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
static int mbedtls_net_connect_ex( mbedtls_net_context *ctx, const char *host,
                         const char *port, int proto )
{
    int ret;
    struct addrinfo hints, *addr_list, *cur;
    struct sockaddr_in local_addr = {0};
    struct netif *iface = netif_find("st1");

    /* Do name resolution with both IPv6 and IPv4 */
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == MBEDTLS_NET_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;

    if( getaddrinfo( host, port, &hints, &addr_list ) != 0 ) {

        return( MBEDTLS_ERR_NET_UNKNOWN_HOST );
    }



    /* Try the sockaddrs until a connection succeeds */
    ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;
    for( cur = addr_list; cur != NULL; cur = cur->ai_next )
    {
        ctx->fd = (int) socket( cur->ai_family, cur->ai_socktype,
                            cur->ai_protocol );

        if( ctx->fd < 0 )
        {
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }

        srand(reg_read(0x40003044));

        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(TCP_ENSURE_LOCAL_PORT_RANGE((u32_t)rand()));
        local_addr.sin_addr.s_addr = iface->ip_addr.u_addr.ip4.addr;
        if (bind(ctx->fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) != 0) {
            //printf("bind failed\n");
        }

#if 0 //no Delay
        printf("[TCP]set sock option tcp_nodelay\r\n");
        int opt = 1;
        if (setsockopt(ctx->fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)))
        {
            printf("[TCP]set sock option tcp_nodelay fail\r\n");
        }

#endif
        mbedtls_net_ex_set_nonblock(ctx);

        if ( connect( ctx->fd, cur->ai_addr, cur->ai_addrlen ) == 0 ) {
            ret = 0;
            mbedtls_net_ex_set_block(ctx);

            break;
        }
        else
        {
            if (errno == EINPROGRESS)
            {
                fd_set rfds, wfds;
                struct timeval tv;

                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                FD_SET(ctx->fd, &rfds);
                FD_SET(ctx->fd, &wfds);

                tv.tv_sec = (SOCKET_CONNECT_TIMEOUT / 1000);
                tv.tv_usec = (SOCKET_CONNECT_TIMEOUT % 1000) * 1000;

                int selres = select(ctx->fd + 1, &rfds, &wfds, NULL, &tv);

                if (selres > 0) {
                    if (FD_ISSET(ctx->fd, &wfds)) {
                        ret = 0;
                        mbedtls_net_ex_set_block(ctx);

                        break;
                    }
                }
            }
        }


        close( ctx->fd );
        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    freeaddrinfo( addr_list );

    return (ret);
}

static int mbedtls_net_errno(int fd)
{
    int sock_errno = 0;
    u32_t optlen = sizeof(sock_errno);

    getsockopt(fd, SOL_SOCKET, SO_ERROR, &sock_errno, &optlen);

    return sock_errno;
}

static int net_would_block( const mbedtls_net_context *ctx, int *errout )
{
    int error = mbedtls_net_errno(ctx->fd);

    if ( errout ) {
        *errout = error;
    }

    /*
     * Never return 'WOULD BLOCK' on a non-blocking socket
     */
    if ( ( fcntl( ctx->fd, F_GETFL, 0) & O_NONBLOCK ) != O_NONBLOCK ) {
        return ( 0 );
    }

    switch ( error ) {
#if defined EAGAIN
    case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
        return ( 1 );
    }
    return ( 0 );
}


static int tcp_poll_write(int fd, int timeout_ms)
{
    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(fd, &writeset);
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    return select(fd + 1, NULL, &writeset, NULL, &timeout);
}

int mbedtls_net_send_timeout( void *ctx, const unsigned char *buf, size_t len )
{
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;
    int error = 0;
    struct timeval t;

    if ( fd < 0 ) {
        return ( MBEDTLS_ERR_NET_INVALID_CONTEXT );
    }
#if 0
    if ((ret = tcp_poll_write(fd, 2000)) <= 0) {
        hal_err("tcp write timeout");

        return -1;//select timeout, error
    }
#endif
    //hal_err("write B");
    t.tv_sec = SSL_NET_SEND_TIMEOUT;
    t.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) != 0) {
        //printf("set timeout failed.\n");

        if ((ret = tcp_poll_write(fd, (SSL_NET_SEND_TIMEOUT * 1000))) <= 0) {
            printf("tcp write timeout");

            return -1;//select timeout, error
        }
    }
    ret = (int) write( fd, buf, len );
    //hal_err("write E");
    if ( ret < 0 ) {
        printf("write fail, ret[%d]", ret);

        if ( net_would_block( ctx, &error ) != 0 ) {
            return ( MBEDTLS_ERR_SSL_WANT_WRITE );
        }

        if ( error == EPIPE || error == ECONNRESET ) {
            return ( MBEDTLS_ERR_NET_CONN_RESET );
        }

        if ( error == EINTR ) {
            return ( MBEDTLS_ERR_SSL_WANT_WRITE );
        }

        return ( MBEDTLS_ERR_NET_SEND_FAILED );
    }

    return ( ret );
}


int ssl_Establish(mbedtls_net_context *server_fd,
                           mbedtls_ssl_context *ssl,
                           mbedtls_ssl_config *conf,
                           mbedtls_x509_crt *cacert,
                           mbedtls_ctr_drbg_context *ctr_drbg, const char *host, uint16_t port)
{
    int ret = 0;
    uint32_t flags = 0;
    char Port[8] = {0};

    mbedtls_x509_crt_profile mbedtls_x509_crt_profile_default_temp =
    {

        MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA1 ) |
        MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_RIPEMD160) |
        MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA224 ) |
        MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA256 ) |
        MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ) |
        MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA512 ),
            0xFFFFFFF, /* Any PK alg    */
            0xFFFFFFF, /* Any curve     */
            2048,
    };


    /*
     * 1. Start the connection
     */
    printf("Connecting to tcp/%s:%d...\n\n", host, port);
    sprintf( Port, "%d", port );
    if ((ret = mbedtls_net_connect_ex(server_fd, host,
                                   Port, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        printf("mbedtls_net_connect_ex returned -0x%x\n\n", -ret);
        printf("[ATS]Websocket connected fail\r\n");
        return false;
    }

    /*
     * 2. Setup stuff
     */
    //printf("Setting up the SSL/TLS structure...\n\n");

    if ((ret = mbedtls_ssl_config_defaults(conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        printf("mbedtls_ssl_config_defaults returned -0x%x\n\n", -ret);
        return false;
    }

     const char *temp;

//    temp = mbedtls_ssl_get_version (ssl);
//    printf("TLS Version= %s\r\n", temp);
//    mbedtls_ssl_conf_min_version(conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_2);
//    mbedtls_ssl_conf_max_version(conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_2);

    temp = mbedtls_ssl_get_version (ssl);
    printf("TLS Version=: %s\r\n", temp);


    conf->cert_profile = &mbedtls_x509_crt_profile_default_temp;

    /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_read_timeout(conf, SSL_HANDSHAKE_TIMEOUT);
    mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain(conf, cacert, NULL );
    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);
    mbedtls_ssl_conf_dbg(conf, my_debug, stdout);

    if ((ret = mbedtls_ssl_setup(ssl, conf)) != 0)
    {
        printf("mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        return false;
    }

    if ((ret = mbedtls_ssl_set_hostname(ssl, host)) != 0)
    {
        printf("mbedtls_ssl_set_hostname returned -0x%x\n\n", -ret);
        return false;
    }

//    mbedtls_ssl_set_bio(ssl, server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    mbedtls_ssl_set_bio(ssl, server_fd, mbedtls_net_send_timeout, mbedtls_net_recv, mbedtls_net_recv_timeout);

    printf("conf->read_timeout=%d\r\n", conf->read_timeout);
    /*
     * 4. Handshake
     */
    printf("Performing the SSL/TLS handshake...\n\n");

#if defined(OPL2500_A0)
    // TODO: sets back to 143MHz while opl2500 is ready (Handshake clock rate)
	sys_cfg_clk_set(SYS_CFG_CLK_44_MHZ);
#else
	sys_cfg_clk_set(SYS_CFG_CLK_143_MHZ);
#endif

    while ((ret = mbedtls_ssl_handshake(ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            printf("mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
            printf("[ATS]Websocket handshake fail\r\n");
            sys_cfg_clk_set(SYS_CFG_CLK_RATE);
            return false;
        }
    }

    sys_cfg_clk_set(SYS_CFG_CLK_RATE);
    mbedtls_ssl_conf_read_timeout(conf, SSL_SOCKET_TIMEOUT);

    /*
     * 5. Verify the server certificate
     */
    //printf("Verifying peer X.509 certificate...\n\n");

    /* In real life, we probably want to bail out when ret != 0 */
    if ((flags = mbedtls_ssl_get_verify_result(ssl)) != 0)
    {
        //memset(buf, 0, sizeof(buf));
        printf("Failed to verify peer certificate! Flags = %d\n\n", flags);
        //mbedtls_x509_crt_verify_info((char*)&buf[0], sizeof(buf), "  ! ", flags);
        //printf("verification info: %s\n\n", buf);

        ssl->session->verify_result = 0;
        ssl->session_negotiate->verify_result = 0;
    }
    else
    {
        printf("Certificate verified.\n\n");
    }

    //printf("Cipher suite is %s", mbedtls_ssl_get_ciphersuite(ssl));

    return true;
}

int ssl_Recv(mbedtls_ssl_context *ssl, char *buf, uint32_t len)
{
    uint32_t        readLen = 0;
    int             net_status = 0;
    int             ret = -1;
    char            err_str[33];

    /*
     * Read the HTTP response
     */
//    printf("Reading HTTP response");
//    while (readLen < len) {
        ret = mbedtls_ssl_read(ssl, (unsigned char *)(buf + readLen), (len - readLen));
        if (ret > 0) {
            readLen += ret;
            net_status = 0;
        } else if (ret == 0) {
            /* if ret is 0 and net_status is -2, indicate the connection is closed during last call */
            return (net_status == -2) ? net_status : readLen;
        } else {
            if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == ret) {

//                mbedtls_strerror(ret, err_str, sizeof(err_str));
                printf("ssl recv error: code = %d, err_str = '%s'\n", ret, err_str);
                net_status = -2; /* connection is closed */
                return net_status;
//                break;
            } else if ((MBEDTLS_ERR_SSL_TIMEOUT == ret)
                       || (MBEDTLS_ERR_SSL_CONN_EOF == ret)
                       || (MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED == ret)
                       || (MBEDTLS_ERR_SSL_NON_FATAL == ret)) {
                /* read already complete */
                /* if call mbedtls_ssl_read again, it will return 0 (means EOF) */
                return readLen;
            } else {
//                mbedtls_strerror(ret, err_str, sizeof(err_str));
                printf("ssl recv error: code = %d, err_str = '%s'\n", ret, err_str);
                net_status = -1;
                return -1; /* Connection error */
            }
        }
//    }

//    printf("ccccccccnet_status=%d, readLen=%d, sizeof(buf)=%d\n",net_status, readLen, sizeof(buf));
    return (readLen > 0) ? readLen : net_status;
}

int ssl_Send(mbedtls_ssl_context *ssl, unsigned char *buf, int len, uint32_t timeout_ms)
{
    uint32_t writtenLen = 0;
    int ret = -1;
    /*
     * Write the GET request
     */
//    printf("Writing HTTP request\n\n");

//    struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
//    if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
//    {
//        interval.tv_sec = 0;
//        interval.tv_usec = 100;
//    }
//
//    setsockopt(g_server_fd.fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&interval,sizeof(struct timeval));

    while (writtenLen < len) {
        ret = mbedtls_ssl_write(ssl, (unsigned char *)(buf + writtenLen), (len - writtenLen));
        if (ret > 0) {
            writtenLen += ret;
            continue;
        } else if (ret == 0) {
            printf("ssl write timeout\n");
            return 0;
        } else {

            char err_str[33];
//            mbedtls_strerror(ret, err_str, sizeof(err_str));
            printf("ssl write fail, code=%d, str=%s\n", ret, err_str);
            return -1; /* Connnection error */
        }
    }

    return writtenLen;

}

int ssl_Destroy(mbedtls_ssl_context *ssl)
{
    int ret = 0;
//    ret = mbedtls_ssl_close_notify( ssl );

    if(ssl!=NULL){
        printf("SSL client Destory\n");
        do
        {
//            printf("mbedtls_ssl_close_notify\n");
            ret = mbedtls_ssl_close_notify( ssl );

        }while( ret == MBEDTLS_ERR_SSL_WANT_WRITE );

        mbedtls_ssl_session_reset(ssl);

        mbedtls_net_free( &g_server_fd );
        mbedtls_x509_crt_free( &g_cacert );
        mbedtls_ssl_free( ssl );
        mbedtls_ssl_config_free( &g_conf );
        mbedtls_ctr_drbg_free( &g_ctr_drbg );
        mbedtls_entropy_free( &g_entropy );
    }

    return ret;
}

void ssl_Read_Timeout_Set(mbedtls_ssl_config *conf, uint32_t timeout)
{
    mbedtls_ssl_conf_read_timeout(conf, timeout);
}

#endif
