/******************************************************************************
*  Copyright 2022, Opulinks Technology Ltd.
*  ----------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Opulinks Technology Ltd. (C) 2022
******************************************************************************/

/******************************************************************************
*  Filename:
*  ---------
*  httpclient_exp.c
*
*  Project:
*  --------
*  
*
*  Description:
*  ------------
*  
*
*  Author:
*  -------
*  AE Team
*
******************************************************************************/
/***********************
Head Block of The File
***********************/
// Sec 0: Comment block of the file

// Sec 1: Include File

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "httpclient_exp.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"
// #include "lwip/err.h"
// #include "lwip/sys.h"
// #include "lwip/errno.h"

#ifdef HTTPCLIENT_SSL_ENABLE
#include "mbedtls/debug.h"
#endif

#include "qd_config.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#if HTTPCLIENT_DEBUG
#define ERR(fmt,arg...)   printf(("[HTTPClient]: "fmt"\r\n"), ##arg)
#define WARN(fmt,arg...)  printf(("[HTTPClient]: "fmt"\r\n"), ##arg)
#define DBG(fmt,arg...)   printf(("[HTTPClient]: "fmt"\r\n"), ##arg)
#else
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)
#endif

#define HTTPC_INF(...)    \
	{\
	printf(__VA_ARGS__); \
	printf("\n"); \
	}

#define HTTPC_WARN(...)   \
	{ \
	printf("WARN:  %s L#%d ", __func__, __LINE__);  \
	printf(__VA_ARGS__); \
	printf("\n"); \
	}

#define HTTPC_ERR(...)  \
	{ \
	printf(__VA_ARGS__); \
	printf("\n"); \
	}

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

#define HTTPCLIENT_AUTHB_SIZE     128

#define HTTPCLIENT_CHUNK_SIZE     512
#define HTTPCLIENT_SEND_BUF_SIZE  512

#define HTTPCLIENT_MAX_HOST_LEN   64
#define HTTPCLIENT_MAX_URL_LEN    256

//#define HTTPCLIENT_TIME_DEBUG  1

#if defined(MBEDTLS_DEBUG_C)
#define DEBUG_LEVEL 2
#endif

#define SSL_NET_SEND_TIMEOUT            (5)     // sec
#define SSL_HANDSHAKE_TIMEOUT           (4600)  // ms
#define SSL_SOCKET_TIMEOUT              (5000)  // ms

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

// static int httpclient_parse_host(char *url, char *host, size_t maxhost_len);
static int httpclient_exp_parse_url(const char *url, char *scheme, size_t max_scheme_len, char *host, size_t maxhost_len, int *port, char *path, size_t max_path_len);
static int httpclient_exp_tcp_send_all(int sock_fd, char *data, int length);
static int httpclient_exp_conn(httpclient_exp_t *client, char *host);
static int httpclient_exp_recv(httpclient_exp_t *client, char *buf, int min_len, int max_len, int *p_read_len);
static int httpclient_exp_retrieve_content(httpclient_exp_t *client, char *data, int len, httpclient_exp_data_t *client_data);
static int httpclient_exp_response_parse(httpclient_exp_t *client, char *data, int len, httpclient_exp_data_t *client_data);
#ifdef HTTPCLIENT_SSL_ENABLE
static int httpclient_exp_ssl_conn(httpclient_exp_t *client, char *host);
static int httpclient_exp_ssl_send_all(mbedtls_ssl_context *ssl, const char *data, size_t length);
static int httpclient_exp_ssl_nonblock_recv(void *ctx, unsigned char *buf, size_t len);
static int httpclient_exp_ssl_close(httpclient_exp_t *client);
#endif

/***********
C Functions
***********/
// Sec 8: C Functions

static void httpclient_base64enc(char *out, const char *in)
{
    const char code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=" ;
    int i = 0, x = 0, l = 0;

    for (; *in; in++) {
        x = x << 8 | *in;
        for (l += 8; l >= 6; l -= 6) {
            out[i++] = code[(x >> (l - 6)) & 0x3f];
        }
    }
    if (l > 0) {
        x <<= 6 - l;
        out[i++] = code[x & 0x3f];
    }
    for (; i % 4;) {
        out[i++] = '=';
    }
    out[i] = '\0' ;
}

static void ms_to_timeval(int timeout_ms, struct timeval *tv)
{
    tv->tv_sec = timeout_ms / 1000;
    tv->tv_usec = (timeout_ms - (tv->tv_sec * 1000)) * 1000;
}

/*
 * Set the socket blocking or non-blocking
 */
static int httpclient_exp_net_set_block(httpclient_exp_t *client)
{
    return ( fcntl( client->socket, F_SETFL, fcntl( client->socket, F_GETFL, 0 ) & ~O_NONBLOCK ) );
}

static int httpclient_exp_net_set_nonblock(httpclient_exp_t *client)
{
    return ( fcntl( client->socket, F_SETFL, fcntl( client->socket, F_GETFL, 0 ) | O_NONBLOCK ) );
}

static int httpclient_exp_net_errno(int fd)
{
    int sock_errno = 0;
    socklen_t optlen = sizeof(sock_errno);
    int err = getsockopt(fd, SOL_SOCKET, SO_ERROR, &sock_errno, &optlen);
    if (err == -1) {
        ERR("getsockopt failed: %d", err);
        return -1;
    }
    return sock_errno;
}

int httpclient_exp_select_check(int sock_fd, int timeout_ms)
{
    int ret = 0;
    int times = 0;

    while (times++ < 1)
    {
        fd_set rfds, wfds;
        struct timeval tv;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(sock_fd, &rfds);
        FD_SET(sock_fd, &wfds);

        /* set select() time out */
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        int selres = select(sock_fd + 1, &rfds, &wfds, NULL, &tv);
        switch (selres)
        {
        case -1:
            ERR("select error");
            ret = -1;
            break;
        case 0:
            ERR("select time out");
            ret = -1;
            break;
        default:
            if (FD_ISSET(sock_fd, &rfds) || FD_ISSET(sock_fd, &wfds))
            {
                int error;
                socklen_t errlen = sizeof(error);
                if (-1 == getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &error, &errlen))
                {
                    ERR("getsockopt error return -1.");
                    ret = -1;
                    break;
                }
                else if (0 != error)
                {
                    ERR("getsockopt return errinfo = %d", error);
                    ret = -1;
                    break;
                }

                ret = 0;
                DBG("connect finish...");
            }
            else
            {
                DBG("haha");
            }
        }

        if (-1 != selres && (ret != 0))
        {
            DBG("check connect result again... %d", times);
            continue;
        }
        else
        {
            break;
        }
    }

    return ret;
}

int httpclient_exp_conn(httpclient_exp_t *client, char *host)
{
    struct addrinfo hints, *addr_list, *cur;
    int ret = 0;
    char port[10] = {0};
    int error = 0;

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    snprintf(port, sizeof(port), "%d", client->remote_port) ;
    if ( getaddrinfo( host, port , &hints, &addr_list ) != 0 ) {
        DBG("getaddrinfo != 0, return HTTPCLIENT_UNRESOLVED_DNS");
        return HTTPCLIENT_UNRESOLVED_DNS_T;
    }

    /* Try the sockaddrs until a connection succeeds */
    ret = HTTPCLIENT_UNRESOLVED_DNS_T;
    for ( cur = addr_list; cur != NULL; cur = cur->ai_next ) {
        client->socket = (int) socket( cur->ai_family, cur->ai_socktype,
                                        cur->ai_protocol );
        if ( client->socket < 0 ) {
            ret = HTTPCLIENT_ERROR_CONN_T;
            continue;
        }

        if (client->timeout_ms == 0) //default
        {
            if ( connect( client->socket, cur->ai_addr, (int)cur->ai_addrlen ) == 0 ) {
                ret = 0;
                break;
            }

            close( client->socket );
            ret = HTTPCLIENT_ERROR_CONN_T;
        }
        else
        {
            /* set non-blocking mode on socket*/
            httpclient_exp_net_set_nonblock(client);

            /* This call will not block */
            if ( connect( client->socket, cur->ai_addr, (int)cur->ai_addrlen ) == 0 ) {
                ret = 0;
                DBG("socket connect succeed immediately");
                httpclient_exp_net_set_block(client);
                break;
            }
            else
            {
                DBG("get the connect result by select()");
                error = httpclient_exp_net_errno(client->socket);
                if (error == EINPROGRESS)
                {
                    httpclient_exp_net_set_block(client);
                    if (httpclient_exp_select_check(client->socket, client->timeout_ms) == 0)
                    {
                        ret = 0;
                        break;
                    }
                }
                else
                {
                   DBG("connect to host %s:%d failed");
                   ret = error;
                }
            }

            close( client->socket );
            ret = HTTPCLIENT_ERROR_CONN_T;
        }
    }

    freeaddrinfo( addr_list );

    if (ret == HTTPCLIENT_OK_T)
    {
       client->net_handle = 1;
    }

    return ret;
}

int httpclient_exp_parse_url(const char *url, char *scheme, size_t max_scheme_len, char *host, size_t maxhost_len, int *port, char *path, size_t max_path_len)
{
    char *scheme_ptr = (char *) url;
    char *host_ptr = (char *) strstr(url, "://");
    size_t host_len = 0;
    size_t path_len;
    char *port_ptr;
    char *path_ptr;
    char *fragment_ptr;

    if (host_ptr == NULL) {
        WARN("Could not find host");
        return HTTPCLIENT_ERROR_PARSE_T; /* URL is invalid */
    }

    if ( max_scheme_len < host_ptr - scheme_ptr + 1 ) { /* including NULL-terminating char */
        WARN("Scheme str is too small (%d >= %d)", max_scheme_len, host_ptr - scheme_ptr + 1);
        return HTTPCLIENT_ERROR_PARSE_T;
    }
    memcpy(scheme, scheme_ptr, host_ptr - scheme_ptr);
    scheme[host_ptr - scheme_ptr] = '\0';

    host_ptr += 3;

    port_ptr = strchr(host_ptr, ':');
    path_ptr = strchr(host_ptr, '/');
    if (( port_ptr != NULL )&&( path_ptr != NULL )&&( port_ptr < path_ptr )||(( port_ptr != NULL )&&( path_ptr == NULL ))) {
        uint16_t tport;
        host_len = port_ptr - host_ptr;
        port_ptr++;
        if ( sscanf(port_ptr, "%hu", &tport) != 1) {
            WARN("Could not find port");
            return HTTPCLIENT_ERROR_PARSE_T;
        }
        *port = (int)tport;
    } else {
        *port = 0;
    }

    if ( host_len == 0 ) {
        host_len = path_ptr - host_ptr;
    }

    if ( maxhost_len < host_len + 1 ) { /* including NULL-terminating char */
        WARN("Host str is too small (%d >= %d)", maxhost_len, host_len + 1);
        return HTTPCLIENT_ERROR_PARSE_T;
    }
    memcpy(host, host_ptr, host_len);
    host[host_len] = '\0';

    fragment_ptr = strchr(host_ptr, '#');
    if (fragment_ptr != NULL) {
        path_len = fragment_ptr - path_ptr;
    } else {
        path_len = strlen(path_ptr);
    }

    if ( max_path_len < path_len + 1 ) { /* including NULL-terminating char */
        WARN("Path str is too small (%d >= %d)", max_path_len, path_len + 1);
        return HTTPCLIENT_ERROR_PARSE_T;
    }
    memcpy(path, path_ptr, path_len);
    path[path_len] = '\0';

    return HTTPCLIENT_OK_T;
}

#if 0
int httpclient_parse_host(char *url, char *host, size_t maxhost_len)
{
    char *host_ptr = (char *) strstr(url, "://");
    size_t host_len = 0;
    char *port_ptr;
    char *path_ptr;

    if (host_ptr == NULL) {
        WARN("Could not find host");
        return HTTPCLIENT_ERROR_PARSE; /* URL is invalid */
    }
    host_ptr += 3;

    port_ptr = strchr(host_ptr, ':');
    if ( port_ptr != NULL ) {
        uint16_t tport;
        host_len = port_ptr - host_ptr;
        port_ptr++;
        if ( sscanf(port_ptr, "%hu", &tport) != 1) {
            WARN("Could not find port");
            return HTTPCLIENT_ERROR_PARSE;
        }
    }

    path_ptr = strchr(host_ptr, '/');
    if ( host_len == 0 ) {
        host_len = path_ptr - host_ptr;
    }

    if ( maxhost_len < host_len + 1 ) { /* including NULL-terminating char */
        WARN("Host str is too small (%d >= %d)", maxhost_len, host_len + 1);
        return HTTPCLIENT_ERROR_PARSE;
    }
    memcpy(host, host_ptr, host_len);
    host[host_len] = '\0';

    return HTTPCLIENT_OK;
}
#endif

int httpclient_exp_get_info(httpclient_exp_t *client, char *send_buf, int *send_idx, char *buf, size_t len)   /* 0 on success, err code on failure */
{
    int ret ;
    int cp_len ;
    int idx = *send_idx;

    if (len == 0) {
        len = strlen(buf);
    }

    do {
        if ((HTTPCLIENT_SEND_BUF_SIZE - idx) >= len) {
            cp_len = len ;
        } else {
            cp_len = HTTPCLIENT_SEND_BUF_SIZE - idx ;
        }

        memcpy(send_buf + idx, buf, cp_len) ;
        idx += cp_len ;
        len -= cp_len ;

        if (idx == HTTPCLIENT_SEND_BUF_SIZE) {
            if (client->is_http == false) {
                ERR("send buffer overflow");
                return HTTPCLIENT_ERROR_T ;
            }
            ret = httpclient_exp_tcp_send_all(client->socket, send_buf, HTTPCLIENT_SEND_BUF_SIZE) ;
            if (ret) {
                return (ret) ;
            }
        }
    } while (len) ;

    *send_idx = idx;
    return HTTPCLIENT_OK_T ;
}

void httpclient_exp_set_custom_header(httpclient_exp_t *client, char *header)
{
    client->header = header ;
}

int httpclient_exp_basic_auth(httpclient_exp_t *client, char *user, char *password)
{
    if ((strlen(user) + strlen(password)) >= HTTPCLIENT_AUTHB_SIZE) {
        return HTTPCLIENT_ERROR_T ;
    }
    client->auth_user = user;
    client->auth_password = password;
    return HTTPCLIENT_OK_T ;
}

int httpclient_exp_send_auth(httpclient_exp_t *client, char *send_buf, int *send_idx)
{
    char b_auth[(int)((HTTPCLIENT_AUTHB_SIZE + 3) * 4 / 3 + 1)] ;
    char base64buff[HTTPCLIENT_AUTHB_SIZE + 3] ;

    httpclient_exp_get_info(client, send_buf, send_idx, "Authorization: Basic ", 0) ;
    sprintf(base64buff, "%s:%s", client->auth_user, client->auth_password) ;
    DBG("bAuth: %s", base64buff) ;
    httpclient_base64enc(b_auth, base64buff) ;
    b_auth[strlen(b_auth) + 1] = '\0' ;
    b_auth[strlen(b_auth)] = '\n' ;
    DBG("b_auth:%s", b_auth) ;
    httpclient_exp_get_info(client, send_buf, send_idx, b_auth, 0) ;
    return HTTPCLIENT_OK_T ;
}


int httpclient_exp_tcp_send_all(int sock_fd, char *data, int length)
{
    int written_len = 0;

    while (written_len < length) {
        int ret = send(sock_fd, data + written_len, length - written_len, 0);
        if (ret > 0) {
            written_len += ret;
            continue;
        } else if (ret == 0) {
            return written_len;
        } else {
            return -1; /* Connnection error */
        }
    }

    return written_len;
}

int httpclient_exp_send_header(httpclient_exp_t *client, char *url, int method, httpclient_exp_data_t *client_data)
{
    char scheme[8] = {0};
    char host[HTTPCLIENT_MAX_HOST_LEN] = {0};
    char path[HTTPCLIENT_MAX_URL_LEN] = {0};
    int len;
    char send_buf[HTTPCLIENT_SEND_BUF_SIZE] = {0};
    char buf[HTTPCLIENT_SEND_BUF_SIZE] = {0};
    char *meth = (method == HTTPCLIENT_GET_T) ? "GET" : (method == HTTPCLIENT_POST_T) ? "POST" : (method == HTTPCLIENT_PUT_T) ? "PUT" : (method == HTTPCLIENT_DELETE_T) ? "DELETE" : (method == HTTPCLIENT_HEAD_T) ? "HEAD" : "";
    int ret, port;

    /* First we need to parse the url (http[s]://host[:port][/[path]]) */
    int res = httpclient_exp_parse_url(url, scheme, sizeof(scheme), host, sizeof(host), &(port), path, sizeof(path));
    if (res != HTTPCLIENT_OK_T) {
        ERR("httpclient_parse_url returned %d", res);
        return res;
    }

    /* Send request */
    memset(send_buf, 0, HTTPCLIENT_SEND_BUF_SIZE);
    len = 0 ; /* Reset send buffer */

    snprintf(buf, sizeof(buf), "%s %s HTTP/1.1\r\nHost: %s\r\n", meth, path, host); /* Write request */
    ret = httpclient_exp_get_info(client, send_buf, &len, buf, strlen(buf));
    if (ret) {
        ERR("Could not write request");
        return HTTPCLIENT_ERROR_CONN_T;
    }

    /* Send all headers */
    if (client->auth_user) {
        httpclient_exp_send_auth(client, send_buf, &len) ; /* send out Basic Auth header */
    }

    /* Add user header information */
    if (client->header) {
        httpclient_exp_get_info(client, send_buf, &len, (char *)client->header, strlen(client->header));
    }

    if ( client_data->post_buf != NULL ) {
        snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", client_data->post_buf_len);
        httpclient_exp_get_info(client, send_buf, &len, buf, strlen(buf));

        if (client_data->post_content_type != NULL)  {
            snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", client_data->post_content_type);
            httpclient_exp_get_info(client, send_buf, &len, buf, strlen(buf));
        }
    }

    /* Close headers */
    httpclient_exp_get_info(client, send_buf, &len, "\r\n", 0);

    DBG("Trying to write %d bytes http header:%s", len, send_buf);

#ifdef HTTPCLIENT_SSL_ENABLE
    if (client->is_http == false) {
        DBG("Enter SSL_write");
        httpclient_exp_ssl_t *ssl = (httpclient_exp_ssl_t *)client->ssl;
        if (httpclient_exp_ssl_send_all(&ssl->ssl_ctx, send_buf, len) != len) {
            ERR("SSL_write failed");
            return HTTPCLIENT_ERROR_T;
        }
        return HTTPCLIENT_OK_T;
    }
#endif

    ret = httpclient_exp_tcp_send_all(client->socket, send_buf, len);
    if (ret > 0) {
        DBG("Written %d bytes, socket = %d", ret, client->socket);
    } else if ( ret == 0 ) {
        WARN("ret == 0,Connection was closed by server");
        return HTTPCLIENT_CLOSED_T; /* Connection was closed by server */
    } else {
        ERR("Connection error (send returned %d)", ret);
        return HTTPCLIENT_ERROR_CONN_T;
    }

    return HTTPCLIENT_OK_T;
}

int httpclient_exp_send_userdata(httpclient_exp_t *client, httpclient_exp_data_t *client_data)
{
    int ret = 0;

    if (client_data->post_buf && client_data->post_buf_len) {
        DBG("client_data->post_buf:%s", client_data->post_buf);
#ifdef HTTPCLIENT_SSL_ENABLE
        if (client->is_http == false) {
            httpclient_exp_ssl_t *ssl = (httpclient_exp_ssl_t *)client->ssl;
            if (httpclient_exp_ssl_send_all(&ssl->ssl_ctx, client_data->post_buf, client_data->post_buf_len) != client_data->post_buf_len) {
                ERR("SSL_write failed");
                return HTTPCLIENT_ERROR_T;
            }
        } else
#endif
        {
            ret = httpclient_exp_tcp_send_all(client->socket, client_data->post_buf, client_data->post_buf_len);
            if (ret > 0) {
                DBG("Written %d bytes", ret);
            } else if ( ret == 0 ) {
                WARN("ret == 0,Connection was closed by server");
                return HTTPCLIENT_CLOSED_T; /* Connection was closed by server */
            } else {
                ERR("Connection error (send returned %d)", ret);
                return HTTPCLIENT_ERROR_CONN_T;
            }
        }
    }

    return HTTPCLIENT_OK_T;
}

int httpclient_exp_recv(httpclient_exp_t *client, char *buf, int min_len, int max_len, int *p_read_len)   /* 0 on success, err code on failure */
{
    int ret = 0;
    size_t readLen = 0;
    struct timeval tv;

    if (client->is_http)
    {
        ms_to_timeval(client->timeout_ms, &tv);
        setsockopt(client->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    while (readLen < max_len) {
        buf[readLen] = '\0';
        if (client->is_http) {
        #if 1
            if (readLen < min_len) {
                ret = recv(client->socket, buf + readLen, min_len - readLen, 0);
                DBG("recv [blocking] return:%d", ret);
            } else {
                ret = recv(client->socket, buf + readLen, max_len - readLen, MSG_DONTWAIT);
                DBG("recv [not blocking] return:%d", ret);
                if (ret == -1 /*&& errno == EWOULDBLOCK*/) {
                    DBG("recv [not blocking] EWOULDBLOCK");
                    break;
                }
            }
        #else
            ret = recv(client->socket, buf + readLen, max_len - readLen, 0);
        #endif
        }
#ifdef HTTPCLIENT_SSL_ENABLE
        else {
            httpclient_exp_ssl_t *ssl = (httpclient_exp_ssl_t *)client->ssl;
            mbedtls_ssl_conf_read_timeout(&ssl->ssl_conf, client->timeout_ms);

        #if 1
            if (readLen < min_len) {
                mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
                ret = mbedtls_ssl_read(&ssl->ssl_ctx, (unsigned char *)buf + readLen, min_len - readLen);
                DBG("mbedtls_ssl_read [blocking] return:%d", ret);
            } else {
                mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, httpclient_exp_ssl_nonblock_recv, NULL);
                ret = mbedtls_ssl_read(&ssl->ssl_ctx, (unsigned char *)buf + readLen, max_len - readLen);
                DBG("mbedtls_ssl_read [not blocking] return:%d", ret);
                if (ret == -1 && errno == EWOULDBLOCK) {
                    DBG("mbedtls_ssl_read [not blocking] EWOULDBLOCK");
                    break;
                }
            }
        #else
            mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, mbedtls_net_recv, NULL);
            ret = mbedtls_ssl_read(&ssl->ssl_ctx, (unsigned char *)buf + readLen, max_len - readLen);
        #endif

            if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {/* read already complete(if call mbedtls_ssl_read again, it will return 0(eof)) */
                break;
            }
        }
#endif

        if (ret > 0) {
            readLen += ret;
        } else if (ret == 0) {
            break;
        } else {
            ERR("Connection error (recv returned %d)", ret);
            *p_read_len = readLen;
            return HTTPCLIENT_ERROR_CONN_T;
        }
    }

    DBG("Read %d bytes", readLen);
    *p_read_len = readLen;
    buf[readLen] = '\0';

    return HTTPCLIENT_OK_T;
}

int httpclient_exp_retrieve_content(httpclient_exp_t *client, char *data, int len, httpclient_exp_data_t *client_data)
{
    int count = 0;
    int templen = 0;
    int crlf_pos;
    /* Receive data */
    DBG("Receiving data:%s", data);
    client_data->is_more = true;

    if (client_data->response_content_len == -1 && client_data->is_chunked == false) {
        while(true)
        {
            int ret, max_len;
            if (count + len < client_data->response_buf_len - 1) {
                memcpy(client_data->response_buf + count, data, len);
                count += len;
                client_data->response_buf[count] = '\0';
            } else {
                memcpy(client_data->response_buf + count, data, client_data->response_buf_len - 1 - count);
                client_data->response_buf[client_data->response_buf_len - 1] = '\0';
                return HTTPCLIENT_RETRIEVE_MORE_DATA_T;
            }

            max_len = MIN(HTTPCLIENT_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count);
            ret = httpclient_exp_recv(client, data, 1, max_len, &len);

            /* Receive data */
            DBG("data len: %d %d", len, count);

            if (ret == HTTPCLIENT_ERROR_CONN_T) {
                DBG("ret == HTTPCLIENT_ERROR_CONN");
                return ret;
            }

            if (len == 0) {/* read no more data */
                DBG("no more len == 0");
                client_data->is_more = false;
                return HTTPCLIENT_OK_T;
            }
        }
    }

    while (true) {
        size_t readLen = 0;

        if ( client_data->is_chunked && client_data->retrieve_len <= 0) {
            /* Read chunk header */
            bool foundCrlf;
            int n;
            do {
                DBG("len: %d", len);
                foundCrlf = false;
                crlf_pos = 0;
                data[len] = 0;
                if (len >= 2) {
                    for (; crlf_pos < len - 2; crlf_pos++) {
                        if ( data[crlf_pos] == '\r' && data[crlf_pos + 1] == '\n' ) {
                            foundCrlf = true;
                            break;
                        }
                    }
                }
                if (!foundCrlf) { /* Try to read more */
                    if ( len < HTTPCLIENT_CHUNK_SIZE ) {
                        int new_trf_len, ret;
                        ret = httpclient_exp_recv(client, data + len, 0, HTTPCLIENT_CHUNK_SIZE - len - 1, &new_trf_len);
                        len += new_trf_len;
                        if (ret == HTTPCLIENT_ERROR_CONN_T) {
                            return ret;
                        } else {
                            continue;
                        }
                    } else {
                        return HTTPCLIENT_ERROR_T;
                    }
                }
            } while (!foundCrlf);
            data[crlf_pos] = '\0';
            //n = sscanf(data, "%x", &readLen);/* chunk length */

            readLen = strtoul(data, NULL, 16);
            printf("chunk len=%d\n",readLen);
            n = (0 == readLen) ? 0 : 1;
            client_data->retrieve_len = readLen;
            client_data->response_content_len += client_data->retrieve_len;
            if ( readLen == 0 ) {
                /* Last chunk */
                client_data->is_more = false;
                printf("no more (last chunk)\n");
                break;
            }

            if (n != 1) {
                ERR("Could not read chunk length");
                return HTTPCLIENT_ERROR_PRTCL_T;
            }

            memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2)); /* Not need to move NULL-terminating char any more */
            len -= (crlf_pos + 2);


        } else {
            readLen = client_data->retrieve_len;
        }

        DBG("Retrieving %d bytes, len:%d", readLen, len);

        do {
            DBG("readLen %d, len:%d", readLen, len);
            templen = MIN(len, readLen);
            if (count + templen < client_data->response_buf_len - 1) {
                memcpy(client_data->response_buf + count, data, templen);
                count += templen;
                client_data->response_buf[count] = '\0';
                client_data->retrieve_len -= templen;
            } else {
                memcpy(client_data->response_buf + count, data, client_data->response_buf_len - 1 - count);
                client_data->response_buf[client_data->response_buf_len - 1] = '\0';
                client_data->retrieve_len -= (client_data->response_buf_len - 1 - count);
                return HTTPCLIENT_RETRIEVE_MORE_DATA_T;
            }

            // if ( len > readLen ) {
            if ( len >= readLen ) {
                DBG("memmove %d %d %d", readLen, len, client_data->retrieve_len);
                memmove(data, &data[readLen], len - readLen); /* chunk case, read between two chunks */
                len -= readLen;
                readLen = 0;
                client_data->retrieve_len = 0;
            } else {
                readLen -= len;
            }

            if (readLen) {
                int ret;
                // int max_len = MIN(HTTPCLIENT_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count);
                int max_len = MIN(MIN(HTTPCLIENT_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count), readLen);
                ret = httpclient_exp_recv(client, data, 1, max_len, &len);
                if (ret == HTTPCLIENT_ERROR_CONN_T) {
                    return ret;
                }
            }
        } while (readLen);

        if ( client_data->is_chunked ) {
            if (len < 2) {
                int new_trf_len, ret;
                /* Read missing chars to find end of chunk */
                ret = httpclient_exp_recv(client, data + len, 2 - len, HTTPCLIENT_CHUNK_SIZE - len - 1, &new_trf_len);
                if (ret == HTTPCLIENT_ERROR_CONN_T) {
                    return ret;
                }
                len += new_trf_len;
            }
            if ( (data[0] != '\r') || (data[1] != '\n') ) {
                ERR("Format error, %s", data); /* after memmove, the beginning of next chunk */
                return HTTPCLIENT_ERROR_PRTCL_T;
            }
            memmove(data, &data[2], len - 2); /* remove the \r\n */
            len -= 2;
        } else {
            DBG("no more(content-length)");
            client_data->is_more = false;
            break;
        }

    }

    return HTTPCLIENT_OK_T;
}

int httpclient_exp_response_parse(httpclient_exp_t *client, char *data, int len, httpclient_exp_data_t *client_data)
{
    int crlf_pos;
    int header_buf_len = client_data->header_buf_len;
    char *header_buf = client_data->header_buf;

    // reset the header buffer
    memset(header_buf, 0, header_buf_len);

    client_data->response_content_len = -1;

    char *crlf_ptr = strstr(data, "\r\n");
    if (crlf_ptr == NULL) {
        ERR("\r\n not found");
        return HTTPCLIENT_ERROR_PRTCL_T;
    }

    crlf_pos = crlf_ptr - data;
    data[crlf_pos] = '\0';

    /* Parse HTTP response */
    if ( sscanf(data, "HTTP/%*d.%*d %d %*[^\r\n]", &(client->response_code)) != 1 ) {
        /* Cannot match string, error */
        ERR("Not a correct HTTP answer : %s", data);
        return HTTPCLIENT_ERROR_PRTCL_T;
    }

    if ( (client->response_code < 200) || (client->response_code >= 400) ) {
        /* Did not return a 2xx code; TODO fetch headers/(&data?) anyway and implement a mean of writing/reading headers */
        WARN("Response code %d", client->response_code);
    }

    HTTPC_INF("Read header :%s", data);

    memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1); /* Be sure to move NULL-terminating char as well */
    len -= (crlf_pos + 2);

    client_data->is_chunked = false;

    /* Now get headers */
    while ( true ) {
        char *colon_ptr, *key_ptr, *value_ptr;
        int key_len, value_len;

        crlf_ptr = strstr(data, "\r\n");
        if (crlf_ptr == NULL) {
            if ( len < HTTPCLIENT_CHUNK_SIZE - 1 ) {
                int new_trf_len, ret;
                ret = httpclient_exp_recv(client, data + len, 1, HTTPCLIENT_CHUNK_SIZE - len - 1, &new_trf_len);
                len += new_trf_len;
                data[len] = '\0';
                DBG("Read %d chars; In buf: [%s]", new_trf_len, data);
                if (ret == HTTPCLIENT_ERROR_CONN_T) {
                    return ret;
                } else {
                    continue;
                }
            } else {
                DBG("header len > chunksize");
                return HTTPCLIENT_ERROR_T;
            }
        }

        crlf_pos = crlf_ptr - data;
        if (crlf_pos == 0) { /* End of headers */
            memmove(data, &data[2], len - 2 + 1); /* Be sure to move NULL-terminating char as well */
            len -= 2;
            break;
        }

        colon_ptr = strstr(data, ": ");
        if (colon_ptr) {
            if (header_buf_len >= crlf_pos + 2) {
                /* copy response header to caller buffer */
                memcpy(header_buf, data, crlf_pos + 2);
                header_buf += crlf_pos + 2;
                header_buf_len -= crlf_pos + 2;
            }

            key_len = colon_ptr - data;
            value_len = crlf_ptr - colon_ptr - strlen(": ");
            key_ptr = data;
            value_ptr = colon_ptr + strlen(": ");

            HTTPC_INF("Read header : %.*s: %.*s", key_len, key_ptr, value_len, value_ptr);
            if (0 == strncasecmp(key_ptr, "Content-Length", key_len)) {
                sscanf(value_ptr, "%d[^\r]", &(client_data->response_content_len));
                client_data->retrieve_len = client_data->response_content_len;
            } else if (0 == strncasecmp(key_ptr, "Transfer-Encoding", key_len)) {
                value_len = strlen("Chunked");
                if (0 == strncasecmp(value_ptr, "Chunked", value_len)) {
                    client_data->is_chunked = true;
                    client_data->response_content_len = 0;
                    client_data->retrieve_len = 0;
                }
            }

            memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1); /* Be sure to move NULL-terminating char as well */
            len -= (crlf_pos + 2);
        } else {
            ERR("Could not parse header");
            return HTTPCLIENT_ERROR_T;
        }
    }

    return httpclient_exp_retrieve_content(client, data, len, client_data);
}

HTTPCLIENT_RESULT_T httpclient_exp_connect(httpclient_exp_t *client, char *url)
{
    int ret = HTTPCLIENT_ERROR_CONN_T;
    char host[HTTPCLIENT_MAX_HOST_LEN] = {0};
    char scheme[8] = {0};
    char path[HTTPCLIENT_MAX_URL_LEN] = {0};

#ifdef HTTPCLIENT_TIME_DEBUG
    int start_time = 0;
    int end_time = 0;
#endif

    /* First we need to parse the url (http[s]://host[:port][/[path]]) */
    int res = httpclient_exp_parse_url(url, scheme, sizeof(scheme), host, sizeof(host), &(client->remote_port), path, sizeof(path));
    if (res != HTTPCLIENT_OK_T) {
        ERR("httpclient_parse_url returned %d", res);
        return (HTTPCLIENT_RESULT_T)res;
    }

    // http or https
    if (strcmp(scheme, "https") == 0)
        client->is_http = false;
    else if (strcmp(scheme, "http") == 0)
        client->is_http = true;

    // default http 80 port, https 443 port
    if (client->remote_port == 0) {
        if (client->is_http) {
            client->remote_port = HTTP_PORT;
        } else {
            client->remote_port = HTTPS_PORT;
        }
    }

    DBG("http?:%d, port:%d, host:%s", client->is_http, client->remote_port, host);

#ifdef HTTPCLIENT_TIME_DEBUG
    start_time = sys_now();
#endif
    client->socket = -1;
    if (client->is_http)
        ret = httpclient_exp_conn(client, host);
#ifdef HTTPCLIENT_SSL_ENABLE
    else {
        ret = httpclient_exp_ssl_conn(client, host);
        if (0 == ret) {
            httpclient_exp_ssl_t *ssl = (httpclient_exp_ssl_t *)client->ssl;
            client->socket = ssl->net_ctx.fd;
        }
    }
#endif

    if (ret == HTTPCLIENT_OK_T)
    {
       client->net_handle = 1;
    }

#ifdef HTTPCLIENT_TIME_DEBUG
    end_time = sys_now();
    printf("client connect time =%d\r\n", end_time - start_time);
#endif

    HTTPC_INF("httpclient_connect() result:%d, client:%p", ret, client);
    return (HTTPCLIENT_RESULT_T)ret;
}

HTTPCLIENT_RESULT_T httpclient_exp_send_request(httpclient_exp_t *client, char *url, int method, httpclient_exp_data_t *client_data)
{
    int ret = HTTPCLIENT_ERROR_CONN_T;

    if (client->socket < 0) {
        return (HTTPCLIENT_RESULT_T)ret;
    }

    ret = httpclient_exp_send_header(client, url, method, client_data);
    if (ret != 0) {
        return (HTTPCLIENT_RESULT_T)ret;
    }

    if (method == HTTPCLIENT_POST_T || method == HTTPCLIENT_PUT_T) {
        ret = httpclient_exp_send_userdata(client, client_data);
    }

    DBG("httpclient_send_request() result:%d, client:%p", ret, client);
    return (HTTPCLIENT_RESULT_T)ret;
}

HTTPCLIENT_RESULT_T httpclient_exp_recv_response(httpclient_exp_t *client, httpclient_exp_data_t *client_data)
{
    int reclen = 0;
    int ret = HTTPCLIENT_ERROR_CONN_T;
    // TODO: header format:  name + value must not bigger than HTTPCLIENT_CHUNK_SIZE.
    char buf[HTTPCLIENT_CHUNK_SIZE] = {0}; // char buf[HTTPCLIENT_CHUNK_SIZE*2] = {0};

    if (client->socket < 0) {
        return (HTTPCLIENT_RESULT_T)ret;
    }

    if (client_data->is_more) {
        client_data->response_buf[0] = '\0';
        ret = httpclient_exp_retrieve_content(client, buf, reclen, client_data);
    } else {
        ret = httpclient_exp_recv(client, buf, 1, HTTPCLIENT_CHUNK_SIZE - 1, &reclen);
        if (ret != 0) {
            return (HTTPCLIENT_RESULT_T)ret;
        }

        buf[reclen] = '\0';

        if (reclen) {
            DBG("reclen:%d, buf:%s", reclen, buf);
            ret = httpclient_exp_response_parse(client, buf, reclen, client_data);
        }
    }

    DBG("httpclient_recv_response() result:%d, client:%p", ret, client);
    return (HTTPCLIENT_RESULT_T)ret;
}

void httpclient_exp_close(httpclient_exp_t *client)
{
    if (client->is_http) {
        if (client->socket >= 0)
            close(client->socket);
    }
#ifdef HTTPCLIENT_SSL_ENABLE
    else
        httpclient_exp_ssl_close(client);
#endif

    client->socket = -1;
    DBG("httpclient_close() client:%p", client);
}

int httpclient_exp_get_response_code(httpclient_exp_t *client)
{
    return client->response_code;
}

static HTTPCLIENT_RESULT_T httpclient_exp_common(httpclient_exp_t *client, char *url, int method, httpclient_exp_data_t *client_data)
{
    HTTPCLIENT_RESULT_T ret = HTTPCLIENT_ERROR_CONN_T;
#ifdef HTTPCLIENT_TIME_DEBUG
    int start_time, end_time;
#endif

    ret = httpclient_exp_connect(client, url);

    if (!ret) {
        ret = httpclient_exp_send_request(client, url, method, client_data);

        if (!ret) {
#ifdef HTTPCLIENT_TIME_DEBUG
            start_time = sys_now();
#endif
            ret = httpclient_exp_recv_response(client, client_data);
#ifdef HTTPCLIENT_TIME_DEBUG
            end_time = sys_now();
            printf("recv_response time =%d\r\n", end_time - start_time);
#endif
        }
    }

    httpclient_exp_close(client);
    return ret;
}

HTTPCLIENT_RESULT_T httpclient_exp_get(httpclient_exp_t *client, char *url, httpclient_exp_data_t *client_data)
{
    return httpclient_exp_common(client, url, HTTPCLIENT_GET_T, client_data);
}

HTTPCLIENT_RESULT_T httpclient_exp_post(httpclient_exp_t *client, char *url, httpclient_exp_data_t *client_data)
{
    return httpclient_exp_common(client, url, HTTPCLIENT_POST_T, client_data);
}

HTTPCLIENT_RESULT_T httpclient_exp_put(httpclient_exp_t *client, char *url, httpclient_exp_data_t *client_data)
{
    return httpclient_exp_common(client, url, HTTPCLIENT_PUT_T, client_data);
}

HTTPCLIENT_RESULT_T httpclient_exp_delete(httpclient_exp_t *client, char *url, httpclient_exp_data_t *client_data)
{
    return httpclient_exp_common(client, url, HTTPCLIENT_DELETE_T, client_data);
}

int httpclient_exp_get_response_header_value(char *header_buf, char *name, int *val_pos, int *val_len)
{
    char *data = header_buf;
    char *crlf_ptr, *colon_ptr, *key_ptr, *value_ptr;
    int key_len, value_len;

    if (header_buf == NULL || name == NULL || val_pos == NULL  || val_len == NULL )
        return -1;

    while (true) {
        crlf_ptr = strstr(data, "\r\n");
        colon_ptr = strstr(data, ": ");
        if (colon_ptr) {
            key_len = colon_ptr - data;
            value_len = crlf_ptr - colon_ptr - strlen(": ");
            key_ptr = data;
            value_ptr = colon_ptr + strlen(": ");

            DBG("Response header: %.*s: %.*s", key_len, key_ptr, value_len, value_ptr);
            if (0 == strncasecmp(key_ptr, name, key_len)) {
                *val_pos = value_ptr - header_buf;
                *val_len = value_len;
                return 0;
            } else {
                data = crlf_ptr + 2;
                continue;
            }
        } else
            return -1;

    }
}

#ifdef HTTPCLIENT_SSL_ENABLE
static int httpclient_exp_ssl_nonblock_recv( void *ctx, unsigned char *buf, size_t len )
{
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;
    if ( fd < 0 ) {
        return ( MBEDTLS_ERR_NET_INVALID_CONTEXT );
    }

    ret = (int) recv( fd, buf, len, MSG_DONTWAIT );

    if ( ret < 0 ) {
#if ( defined(_WIN32) || defined(_WIN32_WCE) ) && !defined(EFIX64) && \
    !defined(EFI32)
        if ( WSAGetLastError() == WSAECONNRESET ) {
            return ( MBEDTLS_ERR_NET_CONN_RESET );
        }
#else
        if ( errno == EPIPE || errno == ECONNRESET ) {
            return ( MBEDTLS_ERR_NET_CONN_RESET );
        }

        if ( errno == EINTR ) {
            return ( MBEDTLS_ERR_SSL_WANT_READ );
        }
#endif
        if (ret == -1 && errno == EWOULDBLOCK) {
            return ret;
        }
        return ( MBEDTLS_ERR_NET_RECV_FAILED );
    }

    return ( ret );
}

static void httpclient_debug( void *ctx, int level, const char *file, int line, const char *str )
{
    // printf("%s\n", str);
    DBG("%s", str);
}

static int httpclient_exp_ssl_send_all(mbedtls_ssl_context *ssl, const char *data, size_t length)
{
    size_t written_len = 0;

    while (written_len < length) {
        int ret = mbedtls_ssl_write(ssl, (unsigned char *)(data + written_len), (length - written_len));
        if (ret > 0) {
            written_len += ret;
            continue;
        } else if (ret == 0) {
            return written_len;
        } else {
            return -1; /* Connnection error */
        }
    }

    return written_len;
}

static int httpclient_exp_ssl_conn(httpclient_exp_t *client, char *host)
{
    int authmode = MBEDTLS_SSL_VERIFY_NONE;
    const char *pers = "https";
    int value, ret = 0;
    uint32_t flags;
    char port[10] = {0};
    httpclient_exp_ssl_t *ssl;

    const int force_ciphersuite[] =
    {
        MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256,
        MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA,
        MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256,
        MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,
		0
    };

    client->ssl = malloc(sizeof(httpclient_exp_ssl_t));
    if (!client->ssl) {
        HTTPC_ERR("malloc error.");
        ret = -1;
        goto exit;
    }
    ssl = (httpclient_exp_ssl_t *)client->ssl;

    if (client->server_cert)
        authmode = MBEDTLS_SSL_VERIFY_REQUIRED;

    mbedtls_ssl_config_max_content_len(1024*6);

    /*
     * Initialize the RNG and the session data
     */
#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif
    mbedtls_net_init(&ssl->net_ctx);
    mbedtls_ssl_init(&ssl->ssl_ctx);
    mbedtls_ssl_config_init(&ssl->ssl_conf);
    mbedtls_x509_crt_init(&ssl->cacert);
    mbedtls_x509_crt_init(&ssl->clicert);
    mbedtls_pk_init(&ssl->pkey);
    mbedtls_ctr_drbg_init(&ssl->ctr_drbg);
    mbedtls_entropy_init(&ssl->entropy);
    if ((value = mbedtls_ctr_drbg_seed(&ssl->ctr_drbg,
                               mbedtls_entropy_func,
                               &ssl->entropy,
                               (const unsigned char*)pers,
                               strlen(pers))) != 0) {
        DBG("mbedtls_ctr_drbg_seed() failed, value:-0x%x.", -value);
        ret = -1;
        goto exit;
    }

    /*
    * Load the Client certificate
    */
    if (client->client_cert && client->client_pk) {
        ret = mbedtls_x509_crt_parse(&ssl->clicert, (const unsigned char *)client->client_cert, client->client_cert_len);
        if (ret < 0) {
            HTTPC_ERR("Loading cli_cert failed! mbedtls_x509_crt_parse returned -0x%x.", -ret);
            goto exit;
        }

        ret = mbedtls_pk_parse_key(&ssl->pkey, (const unsigned char *)client->client_pk, client->client_pk_len, NULL, 0);
        if (ret != 0) {
            HTTPC_ERR("failed! mbedtls_pk_parse_key returned -0x%x.", -ret);
            goto exit;
        }
    }

    /*
    * Load the trusted CA
    */
    /* cert_len passed in is gotten from sizeof not strlen */
    if (client->server_cert && ((value = mbedtls_x509_crt_parse(&ssl->cacert,
                                        (const unsigned char *)client->server_cert,
                                        client->server_cert_len)) < 0)) {
        HTTPC_ERR("mbedtls_x509_crt_parse() failed, value:-0x%x.", -value);
        ret = -1;
        goto exit;
    }

    /*
     * Start the connection
     */
    snprintf(port, sizeof(port), "%d", client->remote_port) ;
    if ((ret = mbedtls_net_connect(&ssl->net_ctx, host, port, MBEDTLS_NET_PROTO_TCP)) != 0) {
        HTTPC_ERR("failed! mbedtls_net_connect returned %d, port:%s.", ret, port);
        goto exit;
    }

    /*
     * Setup stuff
     */
    if ((value = mbedtls_ssl_config_defaults(&ssl->ssl_conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        DBG("mbedtls_ssl_config_defaults() failed, value:-0x%x.", -value);
        ret = -1;
        goto exit;
    }

    /* restrict ciphers */
    mbedtls_ssl_conf_ciphersuites( &ssl->ssl_conf, force_ciphersuite );

    //mbedtls_ssl_set_in_content_len(1024*6);
    //mbedtls_ssl_set_out_content_len(1024*2);

    // TODO: add customerization encryption algorithm
    memcpy(&ssl->profile, ssl->ssl_conf.cert_profile, sizeof(mbedtls_x509_crt_profile));
    ssl->profile.allowed_mds = ssl->profile.allowed_mds | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_MD5);
    mbedtls_ssl_conf_cert_profile(&ssl->ssl_conf, &ssl->profile);

    mbedtls_ssl_conf_authmode(&ssl->ssl_conf, authmode);
    mbedtls_ssl_conf_ca_chain(&ssl->ssl_conf, &ssl->cacert, NULL);

    if (client->client_cert && (ret = mbedtls_ssl_conf_own_cert(&ssl->ssl_conf, &ssl->clicert, &ssl->pkey)) != 0) {
        DBG(" failed! mbedtls_ssl_conf_own_cert returned %d.", ret );
        goto exit;
    }

    mbedtls_ssl_conf_rng(&ssl->ssl_conf, mbedtls_ctr_drbg_random, &ssl->ctr_drbg);
    mbedtls_ssl_conf_dbg(&ssl->ssl_conf, httpclient_debug, NULL);

    if ((value = mbedtls_ssl_setup(&ssl->ssl_ctx, &ssl->ssl_conf)) != 0) {
        HTTPC_ERR("mbedtls_ssl_setup() failed, value:-0x%x.", -value);
        ret = -1;
        goto exit;
    }

    mbedtls_ssl_conf_read_timeout(&ssl->ssl_conf, SSL_HANDSHAKE_TIMEOUT);
    mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    /*
    * Handshake
    */
    sys_cfg_clk_set(SYS_CFG_CLK_44_MHZ);

    while ((ret = mbedtls_ssl_handshake(&ssl->ssl_ctx)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            HTTPC_ERR("mbedtls_ssl_handshake() failed, ret:-0x%x.", -ret);
            ret = -1;
            sys_cfg_clk_set(SYS_CFG_CLK_RATE);
            goto exit;
        }
    }
    sys_cfg_clk_set(SYS_CFG_CLK_RATE);

    mbedtls_ssl_conf_read_timeout(&ssl->ssl_conf, SSL_SOCKET_TIMEOUT);

    /*
     * Verify the server certificate
     */
    /* In real life, we would have used MBEDTLS_SSL_VERIFY_REQUIRED so that the
        * handshake would not succeed if the peer's cert is bad.  Even if we used
        * MBEDTLS_SSL_VERIFY_OPTIONAL, we would bail out here if ret != 0 */
    if ((flags = mbedtls_ssl_get_verify_result(&ssl->ssl_ctx)) != 0) {
        char vrfy_buf[512];
        HTTPC_INF("svr_cert varification failed.");
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
        HTTPC_INF("%s", vrfy_buf);
    }
    else
        HTTPC_INF("svr_cert varification ok.");

    HTTPC_INF("Cipher suite is %s\r\n", mbedtls_ssl_get_ciphersuite(&ssl->ssl_ctx));

exit:
    HTTPC_INF("ret=%d.", ret);
    return ret;
}

static int httpclient_exp_ssl_close(httpclient_exp_t *client)
{
    httpclient_exp_ssl_t *ssl = (httpclient_exp_ssl_t *)client->ssl;
    client->client_cert = NULL;
    client->server_cert = NULL;
    client->client_pk = NULL;

    if (!ssl)
        return -1;

    mbedtls_ssl_close_notify(&ssl->ssl_ctx);
    mbedtls_net_free(&ssl->net_ctx);
    mbedtls_x509_crt_free(&ssl->cacert);
    mbedtls_x509_crt_free(&ssl->clicert);
    mbedtls_pk_free(&ssl->pkey);
    mbedtls_ssl_free(&ssl->ssl_ctx);
    mbedtls_ssl_config_free(&ssl->ssl_conf);
    mbedtls_ctr_drbg_free(&ssl->ctr_drbg);
    mbedtls_entropy_free(&ssl->entropy);

    free(ssl);
    return 0;
}
#endif
