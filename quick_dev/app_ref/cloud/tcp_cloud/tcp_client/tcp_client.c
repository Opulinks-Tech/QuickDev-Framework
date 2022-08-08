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
*  tcp_client.c
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

#include "tcp_client.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

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

/***********
C Functions
***********/
// Sec 8: C Functions

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
uintptr_t TCP_Establish(const char *host, uint16_t port)
{
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = -1;
    char service[6];
    int sockopt = 1;

    memset(&hints, 0, sizeof(hints));

    OPL_LOG_DEBG(TCP, "tcp connect %s:%u", host, port);

    hints.ai_family = AF_INET; /* only IPv4 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0)
    {
        OPL_LOG_ERRO(TCP, "getaddrinfo erro[%d] errno[%d] %s", rc, errno, strerror(errno));
        return (uintptr_t)(-1);
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next)
    {
        if (cur->ai_family != AF_INET)
        {
            OPL_LOG_ERRO(TCP, "socket type error");
            rc = -1;
            continue;
        }

        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0)
        {
            OPL_LOG_ERRO(TCP, "create socket error: fd[%d] errno[%d], %s", fd, errno, strerror(errno));
            rc = -1;
            continue;
        }

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &sockopt, sizeof(sockopt));

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            rc = fd;
            break;
        }

        close(fd);
        rc = -1;
    }

    if (-1 == rc)
    {
        OPL_LOG_ERRO(TCP, "tcp connect fail");
    }

    if (addrInfoList != NULL)
    {
        freeaddrinfo(addrInfoList);
    }

    return (uintptr_t)rc;
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
int TCP_Disconnect(uintptr_t fd)
{
    int rc;
    OPL_LOG_DEBG(TCP, "tcp disconnect");

    /* Shutdown both send and receive operations. */
    if (fd>0 || fd==0)
    {
#if 0
        rc = shutdown((int) fd, 2);
        if (0 != rc)
        {
            OPL_LOG_DEBG(TCP, "shutdown error: fd[%d] rc[%d] %s", fd, rc, strerror(errno));
//            return -1;
        }
#endif
        rc = close((int) fd);
        if (0 != rc)
        {
            OPL_LOG_ERRO(TCP, "close socket error: fd[%d] rc[%d] %s", fd, rc, strerror(errno));
            return -1;
        }
    }

    return 0;
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
int TCP_Send(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret;
    uint32_t len_sent;
    int net_err = 0;
    struct timeval timeout;

    timeout.tv_sec = timeout_ms/1000;
    timeout.tv_usec = (timeout_ms - (timeout.tv_sec * 1000)) * 1000;

    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        OPL_LOG_WARN(TCP, "failed to set socket sending timeout");
        net_err = -2;
        return net_err;
    }

    ret = send(fd, buf, len, 0);
    if (ret > 0)
    {
        len_sent += ret;
    }
    else if (0 == ret)
    {
        OPL_LOG_WARN(TCP, "no data to send");
    }
    else
    {
        OPL_LOG_ERRO(TCP, "send data fail fd: fd[%d] ret[%d] %s", fd, ret, strerror(errno));
        net_err = 1;
    }

    if (net_err)
    {
        return -1;
    }
    else
    {
        return len_sent;
    }
}

/*************************************************************************
* FUNCTION:
*   none
*
* DESCRIPTION:
*   none
*
* PARAMETERS
*   none
*
* RETURNS
*   none
*
*************************************************************************/
int TCP_Recv(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code;
    uint32_t len_recv;
    fd_set sets;
    struct timeval timeout;

    len_recv = 0;
    err_code = 0;

    FD_ZERO(&sets);
    FD_SET(fd, &sets);

    timeout.tv_sec = timeout_ms/1000;
    timeout.tv_usec = (timeout_ms - (timeout.tv_sec * 1000)) * 1000;

    ret = select(fd + 1, &sets, NULL, NULL, &timeout);
    if (ret > 0)
    {
        if (FD_ISSET(fd, &sets))
        {
            ret = recv(fd, buf, len, 0);
            if (ret > 0)
            {
                len_recv += ret;
            } 
            else if (0 == ret)
            {
                OPL_LOG_DEBG(TCP, "connection is closed");
                err_code = -1;
            }
            else
            {
                OPL_LOG_ERRO(TCP, "recv data fail: fd[%d] ret[%d] %s", fd, ret, strerror(errno));
                err_code = -2;
            }
        }
    }
    else if (0 == ret)
    {
        //    printf("select-recv return 0\n");  //select timeout, nothing to do
    }
    else
    {
        //if (EINTR == errno) {
        //    continue;
        //}
        OPL_LOG_ERRO(TCP, "select-recv fail: fd[%d] ret[%d] %s", fd, ret, strerror(errno));
        err_code = -2;
    }

    /* priority to return data bytes if any data be received from TCP connection. */
    /* It will get error code on next calling */
    return (0 != len_recv) ? len_recv : err_code;
}
