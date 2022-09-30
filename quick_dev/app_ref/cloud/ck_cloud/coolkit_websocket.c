#include "app_main.h"
#include "httpclient.h"
#include "infra_cjson.h"
#include "coolkit_websocket.h"
#include "cloud_http.h"
#include "wifi_mngr_api.h"
#include "mw_fim_default_group12_project.h"
#include "cloud_ctrl.h"
#include "cloud_data.h"
#include "ssl_client.h"
#include "mw_ota.h"
#include "etharp.h"

#include "evt_group.h"
#include "time_stmp.h"
#include "qd_config.h"
#include "qd_module.h"

#if defined(MAGIC_LED)
#include "cloud_cmd_data.h"
#include "wifi_mngr_api.h"
#endif

extern httpclient_t g_client;
// extern osTimerId    g_tmr_req_date;

int g_port = 0;
char g_pSocket[20]={0};
char g_Apikey[128]={0};
uint64_t g_msgid = 0;

osSemaphoreId g_tAppSemaphoreId;
int g_tcp_hdl_ID=-1;
int g_tx_ID=-1;
int g_rx_ID=-1;
uint8_t g_IsInitPost = 0;
uint16_t g_u16HbInterval = 60;

// uint64_t g_u64TmpSeqId = 0;
// uint8_t g_u8SeqIdOffset = 0;

static const uint32_t g_timeout_ms = 5000;

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char test_mask_key[4] = {0xB2, 0x39, 0x85, 0xA6};

uint64_t Coolkit_Cloud_GetNewSeqID(void)
{
    uint64_t u64TimeStamp = 0; // current seconds of today (based on time zone of current DevSched)

#if (TIME_STMP_ENABLED == 1)
    u64TimeStamp = TSTMP_TimestampGet();
#endif

    if(u64TimeStamp == g_u64TmpSeqId)
    {
        g_u8SeqIdOffset++;
    }
    else
    {
        g_u8SeqIdOffset=0;
    }

    g_u64TmpSeqId = u64TimeStamp;

    return (u64TimeStamp*1000) + g_u8SeqIdOffset;
}

#if defined(MAGIC_LED)
SHM_DATA int32_t Coolkit_Cloud_Send(uint8_t *u8aPayload, uint32_t u32PayloadLen)
#else
int32_t Coolkit_Cloud_Send(uint8_t *u8aPayload, uint32_t u32PayloadLen)
#endif
{
    char szBodyFmt[BUFFER_SIZE]={0};
    uint32_t ulTotalBodyLen=0;

    memset(szBodyFmt, 0, sizeof(szBodyFmt));

    ws_encode(szBodyFmt, &ulTotalBodyLen, (char *)&u8aPayload[0], u32PayloadLen);  // build the websocket data packet with header and encrypt key

    ws_sem_lock(osWaitForever);
    if(g_tcp_hdl_ID == -1)
    {
        ws_sem_unlock();
        return IOT_DATA_POST_RET_STOP_KEEP;
    }

    if(g_tx_ID!=g_tcp_hdl_ID)
    {
        g_tx_ID = g_tcp_hdl_ID;
    }
    ws_sem_unlock();

    int ret = ws_write(szBodyFmt, ulTotalBodyLen);
    if(ret<=0)
    {
        //Jeff App_Ctrl_MsgSend(APP_CTRL_MSG_DATA_POST_FAIL , NULL , 0);

        ws_sem_lock(osWaitForever);
        Cloud_TimerStop(CLOUD_TMR_REQ_DATE);
        // osTimerStop(g_tmr_req_date);

        if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP))
        {
            if (IOT_DATA_WAITING_TYPE_DATA_POST == g_u8WaitingRspType)
            {
                printf("[ATS]WIFI Send data fail(%llu)\r\n", g_msgid);
            }
        }

        Cloud_TimerStop(CLOUD_TMR_WAIT_RX_RSP);
        // osTimerStop(g_iot_tx_wait_timeout_timer);
        g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;
        EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, false);

        if((g_tcp_hdl_ID!=-1)
            &&(g_tx_ID == g_tcp_hdl_ID)
            && (true == Cloud_OnlineStatusGet()))
            // && (true == EG_StatusGet(g_tIotDataEventGroup , IOT_DATA_EVENT_BIT_CLOUD_CONNECTED)))
        {
            ret = ws_close();
            printf("[ATS]Cloud disconnect\r\n");
            printf("wt: ws_close(ret=%d)\n", ret);
            g_tx_ID = -1;
            g_tcp_hdl_ID = -1;

            Cloud_OnlineStatusSet(false);
            // EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, false);
            Cloud_ConnectRetry();
            ws_sem_unlock();
        }
        else
        {
            ws_sem_unlock();
        }

        return IOT_DATA_POST_RET_STOP_KEEP;
    }
    else  //send success need wait rsp
    {
#if defined(MAGIC_LED)

#else
        Cloud_TimerStart(CLOUD_TMR_WAIT_RX_RSP, CLOUD_TX_WAIT_TIMEOUT);
        // osTimerStop(g_iot_tx_wait_timeout_timer);
        // osTimerStart(g_iot_tx_wait_timeout_timer , CLOUD_TX_WAIT_TIMEOUT);
        ws_sem_lock(osWaitForever);
        g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_DATA_POST;
        ws_sem_unlock();

        EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, true);
#endif
    }

    return IOT_DATA_POST_RET_STOP_KEEP;
}

#if defined(MAGIC_LED)
SHM_DATA int32_t Coolkit_Cloud_Recv(uint8_t *u8RecvBuf, uint32_t *s32Len)
#else
int32_t Coolkit_Cloud_Recv(uint8_t *u8RecvBuf, uint32_t *s32Len)
#endif
{
    int32_t ret=0;

    ws_sem_lock(osWaitForever);

    if(g_tcp_hdl_ID == -1)
    {
        ws_sem_unlock();
        return -1;
    }

    if(g_rx_ID!=g_tcp_hdl_ID)
    {
        g_rx_ID = g_tcp_hdl_ID;
    }
    ws_sem_unlock();


    memset(u8RecvBuf,0, sizeof((char*)u8RecvBuf));
    ret = ws_recv((char*)u8RecvBuf, BUFFER_SIZE);
    if(ret<0)
    {
        printf("[ATS]WIFI Rcv data fail\r\n");
        ws_sem_lock(osWaitForever);
        Cloud_TimerStop(CLOUD_TMR_REQ_DATE);

        if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP))
        {
            if (IOT_DATA_WAITING_TYPE_DATA_POST == g_u8WaitingRspType)
            {
                printf("[ATS]WIFI Send data fail(%llu)\r\n", g_msgid);
            }
        }

        Cloud_TimerStop(CLOUD_TMR_WAIT_RX_RSP);
        // osTimerStop(g_iot_tx_wait_timeout_timer);
        g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;
        EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, false);

        if(((g_tcp_hdl_ID!=-1)
           &&(g_rx_ID == g_tcp_hdl_ID))
           &&(true == Cloud_OnlineStatusGet()))
        {
            int Res = ws_close();
            printf("[ATS]Cloud disconnect\r\n");
            printf("rd: ws_close(ret=%d)\n", Res);

            g_rx_ID = -1;
            g_tcp_hdl_ID = -1;
            Cloud_OnlineStatusSet(false);
            // EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, false);
            Cloud_ConnectRetry();
            ws_sem_unlock();
        }
        else
        {
            ws_sem_unlock();
        }
        goto fail;
    }

    *s32Len = ret;
    ret = 0;

fail:
    return ret;
}

void ws_sem_lock(uint32_t u32LockTime)
{
    if(NULL != g_tAppSemaphoreId)
    {
        osSemaphoreWait(g_tAppSemaphoreId, u32LockTime);
    }
}

void ws_sem_unlock(void)
{
    if(NULL != g_tAppSemaphoreId)
    {
        osSemaphoreRelease(g_tAppSemaphoreId);
    }
}

void ws_init()
{
    osSemaphoreDef_t tSemaphoreDef;
    // create the semaphore
    tSemaphoreDef.dummy = 0;                            // reserved, it is no used
    g_tAppSemaphoreId = osSemaphoreCreate(&tSemaphoreDef, 1);
    if (g_tAppSemaphoreId == NULL)
    {
        printf("To create the semaphore for AppSemaphore is fail.\n");
    }
}

int DataToXOR(char *szDataPayload, char *szMaskKey, uint32_t len)
{
    int size = 0;
    size = len;
    int nMaskLength = size;
    char *pMaskKeyStart;
    char *pMaskKeyEnd;
    pMaskKeyStart = szMaskKey;
    pMaskKeyEnd = szMaskKey + (WS_MASK_KEY_SIZE -1);
    if(size <= 0)
    {
        return 0;
    }
    else
    {
        while(size--)
        {
            //printf("+++ Pdata is %2x Pmask is %2x \n", *szDataPayload , *szMaskKey );
            (*szDataPayload) = (*szDataPayload) ^ (*szMaskKey);
            //printf("--- Pdata is %2x Pmask is %2x \n", *szDataPayload , *szMaskKey );
            if(szMaskKey == pMaskKeyEnd)
            {
                szMaskKey =  pMaskKeyStart;
            }
            else
            {
                szMaskKey++;
            }
            szDataPayload++;
        }
    }
    return nMaskLength;
}

WebSocketFrameType ws_decode(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length)
{
	//printf("getTextFrame()\n");
	if(in_length < 3)
        return INCOMPLETE_FRAME;

	unsigned char msg_opcode = in_buffer[0] & 0x0F;
	unsigned char msg_fin = (in_buffer[0] >> 7) & 0x01;
	unsigned char msg_masked = (in_buffer[1] >> 7) & 0x01;

	// *** message decoding
	int payload_length = 0;
	int pos = 2;
	int length_field = in_buffer[1] & (~0x80);
	unsigned int mask = 0;

	//printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

	if(length_field <= 125) {
		payload_length = length_field;
	}

	else if(length_field == 126) { //msglen is 16bit!
		//payload_length = in_buffer[2] + (in_buffer[3]<<8);
		payload_length = (
			(in_buffer[2] << 8) |
			(in_buffer[3])
		);
		pos += 2;
	}

	else if(length_field == 127) { //msglen is 64bit!

		payload_length = (
        ((uint64_t)in_buffer[2] << 56) |
        ((uint64_t)in_buffer[3] << 48) |
		((uint64_t)in_buffer[4] << 40) |
		((uint64_t)in_buffer[5] << 32) |
		(in_buffer[6] << 24) |
		(in_buffer[7] << 16) |
		(in_buffer[8] << 8) |
		(in_buffer[9])
		);
		pos += 8;
	}

	//printf("PAYLOAD_LEN: %08x\n", payload_length);
	if(in_length < payload_length+pos) {
        int ret=-1;
        if(BUFFER_SIZE*2>=payload_length+pos)
        {
            ret = ws_recv((char*)&in_buffer[in_length], payload_length+pos-in_length);
        }
        if (ret<payload_length+pos-in_length)
        {

	    	return INCOMPLETE_FRAME;
        }
	}

	if(msg_masked) {
		mask = *((unsigned int*)(in_buffer+pos));
		//printf("MASK: %08x\n", mask);
		pos += 4;
		// unmask data:
		unsigned char* c = in_buffer+pos;
		for(int i=0; i<payload_length; i++) {
			c[i] = c[i] ^ ((unsigned char*)(&mask))[i%4];
		}
	}

	if(payload_length > out_size) {
		//TODO: if output buffer is too small -- ERROR or resize(free and allocate bigger one) the buffer ?
	}

	memcpy((void*)out_buffer, (void*)(in_buffer+pos), payload_length);
	out_buffer[payload_length] = 0;
	*out_length = payload_length+1;

	//printf("TEXT: %s\n", out_buffer);


	if(msg_opcode == 0x0)
        return (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME; // continuation frame ?

	if(msg_opcode == 0x1)
        return (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME;

	if(msg_opcode == 0x2)
        return (msg_fin)?BINARY_FRAME:INCOMPLETE_BINARY_FRAME;

	if(msg_opcode == 0x9)
    {
        printf("recv 0x9 ping buffer\r\n");
        return PING_FRAME;
    }

	if(msg_opcode == 0xA)
    {
        printf("recv 0xA pong buffer\r\n");
        return PONG_FRAME;
    }

	return ERROR_FRAME;
}

void ws_encode( char* szBodyToSend, uint32_t* ulTotalBodyLen, char *szRequest_data, uint32_t ulLen)
{
      /*
         0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+ */


     /*     FIN:  1 bit

      Indicates that this is the final fragment in a message.  The first
      fragment MAY also be the final fragment.

      RSV1, RSV2, RSV3:  1 bit each

      MUST be 0 unless an extension is negotiated that defines meanings
      for non-zero values.  If a nonzero value is received and none of
      the negotiated extensions defines the meaning of such a nonzero
      value, the receiving endpoint MUST _Fail the WebSocket
      Connection_ */

     /*

      Opcode:  4 bits

      Defines the interpretation of the "Payload data".  If an unknown
      opcode is received, the receiving endpoint MUST _Fail the
      WebSocket Connection_.  The following values are defined.

      *  %x0 denotes a continuation frame

      *  %x1 denotes a text frame

      *  %x2 denotes a binary frame

      *  %x3-7 are reserved for further non-control frames

      *  %x8 denotes a connection close

      *  %x9 denotes a ping

      *  %xA denotes a pong

      *  %xB-F are reserved for further control frames */

   /*
   Payload length:  7 bits, 7+16 bits, or 7+64 bits

      The length of the "Payload data", in bytes: if 0-125, that is the
      payload length.  If 126, the following 2 bytes interpreted as a
      16-bit unsigned integer are the payload length.  If 127, the
      following 8 bytes interpreted as a 64-bit unsigned integer (the
      most significant bit MUST be 0) are the payload length. */

    *szBodyToSend = 0x81;
    *ulTotalBodyLen = 1;

    if((ulLen > 0) && (ulLen<=125))
    {
        *(szBodyToSend + 1) = 0x80 + ulLen;
        *ulTotalBodyLen = *ulTotalBodyLen + 1;
    }
    else if ((ulLen > 125) && (ulLen < 0xffff))
    {

        *(szBodyToSend + 1) = (char)(0x80 + 126);
        *(szBodyToSend + 2) = ulLen >> 8;
        *(szBodyToSend + 3) = (ulLen & 0xff);
        *ulTotalBodyLen = *ulTotalBodyLen + 3;
    }
    else
    {
        *(szBodyToSend + 1) = (char)(0x80 + 127);
        *(szBodyToSend + 2) = 0x00;
        *(szBodyToSend + 3) = 0x00;
        *(szBodyToSend + 4) = 0x00;
        *(szBodyToSend + 5) = 0x00;
        *(szBodyToSend + 6) = ((ulLen >> 24)  & 0xff);
        *(szBodyToSend + 7) = ((ulLen >> 16)  & 0xff);
        *(szBodyToSend + 8) = ((ulLen >> 8) & 0xff);
        *(szBodyToSend + 9) = (ulLen & 0xff);
        *ulTotalBodyLen = *ulTotalBodyLen + 9;
    }
    memcpy((szBodyToSend + *ulTotalBodyLen ), test_mask_key, WS_MASK_KEY_SIZE);
    *ulTotalBodyLen = *ulTotalBodyLen + 4;
    DataToXOR(szRequest_data, test_mask_key, ulLen);
    memcpy((szBodyToSend + *ulTotalBodyLen ), szRequest_data, ulLen);
    *ulTotalBodyLen = *ulTotalBodyLen + ulLen;
}

int ws_get_random(void *buf, int len)
{
    int n;
    char *p = (char *)buf;

    for (n = 0; n < len; n++)
        p[n] = (unsigned char)rand();

    return n;
}

unsigned char * base64_encode(const unsigned char *src, size_t len, size_t *out_len)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;
    size_t olen;
    int line_len;

    olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
    olen += olen / 72; /* line feeds */
    olen++; /* nul termination */
    if (olen < len)
        return NULL; /* integer overflow */
    out = malloc(olen);
    if (out == NULL)
        return NULL;

    end = src + len;
    in = src;
    pos = out;
    line_len = 0;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
        line_len += 4;
        if (line_len >= 72) {
            *pos++ = '\n';
            line_len = 0;
        }
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        } else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                      (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
        line_len += 4;
    }

    if (line_len)
        *pos++ = '\n';

    *pos = '\0';
    if (out_len)
        *out_len = pos - out;
    return out;
}

int ParseCjson(char* buf, int len)
{
    lite_cjson_t tRoot = {0};

    lite_cjson_t tProperty = {0};

    #define tPort           tProperty
    #define tReason         tProperty
    #define tIP             tProperty
    #define tError          tProperty

    /* Parse Request */
    if (lite_cjson_parse(buf, len, &tRoot)) {
        printf("JSON Parse Error");
        return CJSON_PARSE_FAIL;
    }

    if (!lite_cjson_object_item(&tRoot, "port", 4, &tPort)) {
        g_port = tPort.value_int;
        printf("port : %d\r\n", tPort.value_int);
    }
    else
    {
        return CJSON_PARSE_FAIL;
    }

    if (!lite_cjson_object_item(&tRoot, "reason", 6, &tReason)) {
        char sBuf[32] = {0};

        if(tReason.value_length < 32)
        {
            memcpy(sBuf, tReason.value, tReason.value_length);
            sBuf[tReason.value_length] = 0;
        }
        printf("Reason : %s\r\n", sBuf);
    }
    else
    {
        return CJSON_PARSE_FAIL;
    }

    if (!lite_cjson_object_item(&tRoot, "IP", 2, &tIP)) {

        if(tIP.value_length < 64)
        {
            memcpy(g_pSocket, tIP.value, tIP.value_length);
            g_pSocket[tIP.value_length] = 0;
        }
        printf("IP : %s\r\n", g_pSocket);
    }
    else
    {
        return CJSON_PARSE_FAIL;
    }

    if (!lite_cjson_object_item(&tRoot, "error", 5, &tError)) {
        printf("Error : %d\r\n", tError.value_int);
    }
    else
    {
        return CJSON_PARSE_FAIL;
    }

    return CJSON_PARSE_OK;

}

#if defined(MAGIC_LED)
SHM_DATA int Connect_coolkit_http(void)
#else
int Connect_coolkit_http(void)
#endif
{
    const uint8_t LEN = 128;
    int ret = -1;
    char URL[LEN] = {0};
    httpclient_data_t client_data = {0};


    g_client.timeout_ms = SSL_HANDSHAKE_TIMEOUT;
    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, false);

    lwip_one_shot_arp_enable();

    char* buf = malloc(LEN);
    if (buf == NULL)
    {
        printf("buf malloc failed.\r\n");
        goto Fail;
    }

    client_data.response_buf = buf;
    client_data.response_buf_len = LEN;
    client_data.is_chunked = true;

    ret = HTTPCLIENT_ERROR_CONN;
    // sprintf(URL, "https://as-dispd.coolkit.cc:443/dispatch/device");
    sprintf(URL , "https://%s%s" , g_tHostInfo.ubaHostInfoURL , g_tHostInfo.ubaHostInfoDIR);
    printf("url:%s\n", URL);
    ret = Cloud_Http_Get(&g_client, URL, &client_data);
    if(HTTPCLIENT_OK == ret)
    {
        printf("client_data.response_buf:%.*s\n", client_data.response_buf_len, client_data.response_buf);
        if(CJSON_PARSE_OK == ParseCjson(client_data.response_buf, strlen(client_data.response_buf)))
        {
            /*add code for Saving Websocket url*/
            ret = 0;
        }
        else
        {
            goto Fail;
        }
    }
    else
    {
        printf("http_connect failed! ret = %d\n" , ret);
        goto Fail;
    }

Fail:
    if (buf != NULL)
    {
        free(buf);
    }
    Cloud_Http_Close(&g_client);

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);

    return ret;
}

#if defined(MAGIC_LED)
SHM_DATA int RegDeviceToCoolkit()
#else
int RegDeviceToCoolkit()
#endif
{
    char property_payload[BUFFER_SIZE] = {0};
    uint32_t u32Offset = 0;
    char SendBody[BUFFER_SIZE]={0};
    uint32_t SendBodyLen=0;
    char szBuf[BUFFER_SIZE] = {0};
    char szOutBuf[BUFFER_SIZE] = {0};
    int out_length=0;
    int ret = 0;

#if defined(MAGIC_LED)
    uint8_t ubaMacAddr[6];

    // get the mac address from flash
    Opl_Wifi_MacAddr_Get(ubaMacAddr);
               
    u32Offset = snprintf( property_payload, 512,"{\"userAgent\":\"%s\",\"apikey\":\"%s\",\"deviceid\":\"%s\",\"action\":\"%s\",\"version\":\"%d\",\"romVersion\":\"%s\",\"model\":\"%s\",\"ts\":1584951963,\"d_seq\":1584951963,\"chipid\":\"%s\",\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\"}",
                "device",
                g_tHttpPostContent.ubaApiKey,
                g_tHttpPostContent.ubaDeviceId,
                "register",
                IOT_VERSION,
                "1.8.1",
                g_tHttpPostContent.ubaModelId,
                g_tHttpPostContent.ubaChipId,
                ubaMacAddr[0],ubaMacAddr[1],ubaMacAddr[2],ubaMacAddr[3],ubaMacAddr[4],ubaMacAddr[5]);

#elif defined(DOOR_SENSOR)
    g_msgid = Coolkit_Cloud_GetNewSeqID();

    u32Offset = snprintf( property_payload, BUFFER_SIZE-1,"{\"userAgent\":\"%s\",\"apikey\":\"%s\",\"deviceid\":\"%s\",\"action\":\"%s\",\"version\":\"%d\",\"romVersion\":\"%s\",\"model\":\"%s\",\"ts\":%llu,\"d_seq\":%llu,\"chipid\":\"%s\"}",
             "device",
             g_tHttpPostContent.ubaApiKey,
             g_tHttpPostContent.ubaDeviceId,
             "register",
             IOT_VERSION,
            "1.8.1",
            g_tHttpPostContent.ubaModelId,
            (g_msgid/1000),
            g_msgid,
            g_tHttpPostContent.ubaChipId);

#endif

    printf("----------\nPayload for WS:(%d)\n----------\n%s\n", u32Offset, property_payload);

    ws_encode(SendBody, &SendBodyLen, property_payload, u32Offset);
    ret = ws_write(SendBody, SendBodyLen);
    if(ret>0)
    {
        ret = ws_recv(szBuf, BUFFER_SIZE-1);
        if(ret>0)
        {
            ws_decode((unsigned char*)szBuf, ret, (unsigned char*)szOutBuf, BUFFER_SIZE, &out_length);
            printf("ws read ret is %d:\n%s\n", out_length, szOutBuf);

            lite_cjson_t tRoot = {0};
            lite_cjson_t tApikey = {0};
            lite_cjson_t tError = {0};
            lite_cjson_t tConfig = {0};
            lite_cjson_t tHbInterval = {0};

            /* update apikey */
            if (lite_cjson_parse(szOutBuf, ret, &tRoot)) {
                printf("JSON Parse Error\n");
                return CJSON_PARSE_FAIL;
            }

            if (!lite_cjson_object_item(&tRoot, "apikey", 6, &tApikey))
            {

                if(tApikey.value_length >0)
                {
                    memcpy(g_Apikey, tApikey.value, tApikey.value_length);
                    g_Apikey[tApikey.value_length] = 0;
//                  printf("apikey : %s\r\n", g_Apikey);
                }

            }

#if defined(DOOR_SENSOR)
            if (!lite_cjson_object_item(&tRoot, "config", 6, &tConfig))
            {
                if (!lite_cjson_object_item(&tConfig, "hbInterval", 10, &tHbInterval))
                {
                    g_u16HbInterval = tHbInterval.value_int;
                    g_u16HbInterval = (g_u16HbInterval-COOLKIT_HEART_BEAT_RESERVE)/2;
                    printf("hbInterval : %d(%d)\r\n", g_u16HbInterval, tHbInterval.value_int);
                }
            }
#endif

            if (!lite_cjson_object_item(&tRoot, "error", 5, &tError))
            {
                if(403 == tError.value_int || 404 == tError.value_int || 415 == tError.value_int)
                {
#if defined(MAGIC_LED)
#if (RESET_WHILE_DELETED_BY_APP == 1)
                    if(415 == tError.value_int)
                    {
                        // Receive 415 error code, which means the device has been deleted by APP
                        DeviceRecoverToInitState();
                    }
#endif
#endif

                    printf("error: %d, wait for register!!\r\n", tError.value_int);
                    ret = COOLKIT_NOT_REG;
                }
                else
                {
                    ret = COOLKIT_REG_SUCCESS;
                }
            }

        }
        else
        {
           printf("ws_recv failed!!, reg device failed(ret=%d)\r\n", ret);
        }
    }
    else
    {
       printf("ws_write failed!!, reg device failed(ret=%d)\r\n",ret);
    }

    return ret;
}

#if defined(MAGIC_LED)
SHM_DATA int Connect_coolkit_wss(void)
#else
int Connect_coolkit_wss(void)
#endif
{
    char URL[128] = {0};
    int ret = 0;
    char szBuf[BUFFER_SIZE] = {0};
    static char ws_header[300] = {0};
    const unsigned char random_hash[20] = {0};
    unsigned char* base64 = NULL;
    size_t  nLen_base64 = 0;
    T_CloudRingBufData stKeppAlive = {0}; //Keep alive has no data

    sprintf(URL , "https://%s:%d/api/ws" , g_pSocket , g_port);

    printf("wss path:%s\n", URL);;

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, false);

    lwip_one_shot_arp_enable();

    ret = ws_connect(g_pSocket, g_port);
    if(ret == false)
    {
        printf("ws_connect failed\n");
        goto fail;
    }

    ws_get_random((void *)random_hash, 16);
    base64 = base64_encode(random_hash, 16, &nLen_base64);

    memset(ws_header,0, sizeof(ws_header));
        snprintf(ws_header,300,
        "GET /api/ws HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Origin: http://%s:%d\r\n"
        "Sec-WebSocket-Key: %s"
        "Sec-WebSocket-Version: 13\r\n\r\n",
        g_pSocket, g_port, g_pSocket, g_port,
        base64
        );

    ret = ws_write(ws_header, strlen(ws_header));

    if(ret>0)
    {
        ret = ws_recv(szBuf, BUFFER_SIZE);
        printf("szBuf=%s\n", szBuf);
        if(ret > 0)
        {
            printf("----------\nResponse for WS connection:\n----------\n%s\n", szBuf);

            char* pu8Rlt = NULL;
            pu8Rlt = strstr(szBuf, "HTTP/1.1 101 Switching Protocols");

            if(strlen(pu8Rlt)>0)
            {
#if defined(MAGIC_LED)
                g_tcp_hdl_ID++;
                g_tcp_hdl_ID = g_tcp_hdl_ID%0xff;
#endif
                ret = RegDeviceToCoolkit();
                if(ret == COOLKIT_REG_SUCCESS )
                {
//                    ws_ReadTimeoutSet(SSL_SOCKET_TIMEOUT);
//                    ws_ReadTimeoutSet(SSL_SOCKET_TIMEOUT_FOREVER);

                    g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;
                    EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, false);
                    EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_LAST_POST_RETRY, false);

                    Cloud_RingBufReset(&g_stCloudRspQ);
                    Cloud_RingBufReset(&g_stKeepAliveQ);

                    // when boot, need to query the time information at the 1st connection
                    if (false == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_TIME_QUERY_WHEN_BOOT))
                    {
                        EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_TIME_QUERY_WHEN_BOOT, true);
                        Cloud_RingBufPush(&g_stKeepAliveQ, &stKeppAlive);
                    }

                    g_tcp_hdl_ID++;
                    g_tcp_hdl_ID = g_tcp_hdl_ID%0xff;

                    Cloud_OnlineStatusSet(true);
                    // EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, true);

                    EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_POST_FAIL_RECONNECT, false);

                    g_IsInitPost = 1;

                    #if 1
                    ws_sem_lock(osWaitForever);
                    g_u8PostRetry_IotRbData_Cnt = 0;
                    ws_sem_unlock();

                    Cloud_RingBufReset(&g_stIotRbData);

#if defined(DOOR_SENSOR)
                    APP_DoorSensorPostToCloud(TIMER_POST);
#endif

                    #else
                    if(IOT_RB_DATA_OK == IoT_Ring_Buffer_CheckEmpty(&g_stIotRbData))
                    {
                        printf("queue is empty , push the latest device status\r\n");
                        APP_DoorSensorPostToCloud(TIMER_POST);
                    }
                    else
                    {
                        Cloud_MsgSend(CLOUD_EVT_TYPE_POST, NULL, 0);
                        // Iot_Data_TxTask_MsgSend(IOT_DATA_TX_MSG_CLOUD_POST, NULL , 0);
                    }
                    #endif
//                    Coolkit_Req_Date(NULL);

#if defined(MAGIC_LED)
                    uint8_t u8Source = LOCAL_CTRL;
                    Cloud_MsgSend(CLOUD_EVT_TYPE_SEND_FIRST_POST, &u8Source, 1);

                    Cloud_TimerStart(CLOUD_TMR_REQ_DATE, 10*1000);//AUTO REPEAT after connect Server
#elif defined(DOOR_SENSOR)
                    Cloud_TimerStart(CLOUD_TMR_REQ_DATE, g_u16HbInterval*1000);//AUTO REPEAT after connect Server
#endif

                    g_u8PostRetry_KeepAlive_Cnt = 0;
                    g_u8PostRetry_KeepAlive_Fail_Round = 0;
                    g_u8CloudRetryIntervalIdx = 0;
                }
                else
                {
                    goto fail;
                }
            }
        }
        else
        {
            printf("ws_recv failed, change WS failed\r\n");
            goto fail;
        }
    }
    else
    {

        printf("ws_write failed, change WS failed\r\n");
        goto fail;
    }

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);

    if(base64!=NULL)
        free(base64);

    return ret;


fail:

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, true);

    ws_close();

    if(base64!=NULL)
        free(base64);

    return ret;
}

uint8_t ws_KeepAlive(void)
{
    char property_payload[BUFFER_SIZE] = {0};
    uint32_t u32Offset = 0;
    char SendBody[BUFFER_SIZE]={0};
    uint32_t SendBodyLen=0;
    int ret = 0;
//    int32_t s32TimeZoneSec = 8*3600;

    Opl_Wifi_Skip_Dtim_Set(g_u16IotDtimTxUse, false);

    lwip_one_shot_arp_enable();

    g_msgid = Coolkit_Cloud_GetNewSeqID();

    u32Offset = snprintf(property_payload, 200,"{\"action\":\"date\",\"apikey\":\"%s\",\"deviceid\":\"%s\",\"d_seq\":%llu,\"userAgent\":\"device\"}",
                    g_Apikey, g_tHttpPostContent.ubaDeviceId, g_msgid);

    printf("----------\nPayload for WS:(%d)\n----------\n%s\n", u32Offset, property_payload);

    ws_encode(SendBody, &SendBodyLen, property_payload, u32Offset);
    ret = ws_write(SendBody, SendBodyLen);

    if(ret<=0)
    {
        //Jeff App_Ctrl_MsgSend(APP_CTRL_MSG_DATA_POST_FAIL , NULL , 0);

        ws_sem_lock(osWaitForever);

        Cloud_TimerStop(CLOUD_TMR_REQ_DATE);

        if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP))
        {
            if (IOT_DATA_WAITING_TYPE_DATA_POST == g_u8WaitingRspType)
            {
                printf("[ATS]WIFI Send data fail(%llu)\r\n", g_msgid);
            }
        }

        Cloud_TimerStop(CLOUD_TMR_WAIT_RX_RSP);

        g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_NONE;

        EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, false);

        if((g_tcp_hdl_ID!=-1)
            &&(g_tx_ID == g_tcp_hdl_ID)
            && (true == Cloud_OnlineStatusGet()))
            // && (true == EG_StatusGet(g_tIotDataEventGroup , IOT_DATA_EVENT_BIT_CLOUD_CONNECTED)))
        {
            ret = ws_close();

            printf("[ATS]Cloud disconnect\r\n");
            printf("wt: ws_close(ret=%d)\n", ret);
            
            g_tx_ID = -1;
            g_tcp_hdl_ID = -1;

            Cloud_OnlineStatusSet(false);
            // EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_CLOUD_CONNECTED, false);

            Cloud_ConnectRetry();

            ws_sem_unlock();
        }
        else
        {
            ws_sem_unlock();
        }
    }
    else
    {
        Cloud_TimerStart(CLOUD_TMR_WAIT_RX_RSP, CLOUD_TX_WAIT_TIMEOUT);

        ws_sem_lock(osWaitForever);
        g_u8WaitingRspType = IOT_DATA_WAITING_TYPE_KEEPALIVE;
        ws_sem_unlock();

        EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_WAITING_RX_RSP, true);

        if (true == EG_StatusGet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_LAST_POST_RETRY))
        {
            EG_StatusSet(g_tIotDataEventGroup, IOT_DATA_EVENT_BIT_LAST_POST_RETRY, false);
#if defined(DOOR_SENSOR)
            APP_DoorSensorPostToCloud(TIMER_POST);
#endif
        }
    }
    return ret;
}


int ws_connect(const char *host, uint16_t port)
{
   /* Init */
    if (true != (ssl_Init(&g_server_fd, &g_ssl, &g_conf, &g_cacert, &g_ctr_drbg, &g_entropy)))
    {
        printf("Https_Init Failure\n\n");
        goto exit;
    }

    /* Establish */
    if (true != (ssl_Establish(&g_server_fd, &g_ssl, &g_conf, &g_cacert, &g_ctr_drbg, host, port)))
    {
        printf("Https_Establish Failure\n\n");
        goto exit;
    }

    return true;

exit:

    if (0 != ((ssl_Destroy(&g_ssl))))
    {
        printf("Destroy Failure\n\n");
    }

    printf("\nSSL client ends\n");

    return false;
}


int ws_write(char* data, uint32_t ulLen)//TODO
{
    int ret;
    ret = ssl_Send(&g_ssl, (unsigned char *)data, ulLen, g_timeout_ms);
    return ret;
}

int ws_recv(char *buf, uint32_t len)
{
    int ret=0;
    ret = ssl_Recv(&g_ssl, buf, len);
    return ret;
}

int ws_close()
{
    int ret=0;
    ret = ssl_Destroy(&g_ssl);
    printf("ssl_destory(ret=%d)\n", ret);

    return ret;
}

void ws_ReadTimeoutSet(uint32_t timeout)
{
    ssl_Read_Timeout_Set(&g_conf, timeout);
}
