#ifndef __COOLKIT_WEBSOCKET_H__
#define __COOLKIT_WEBSOCKET_H__

#include "cmsis_os.h"
// #include "common.h"

#define CJSON_PARSE_OK             0
#define CJSON_PARSE_FAIL          -1
#define HTTPCLIENT_MAX_URL_LEN    256
#define WS_MASK_KEY_SIZE           4
#define COOLKIT_NOT_REG           -20
#define COOLKIT_REG_SUCCESS        1

#define COOLKIT_HEART_BEAT_RESERVE  (60)

typedef enum{

	ERROR_FRAME             =0xFF00,
	INCOMPLETE_FRAME        =0xFE00,

	OPENING_FRAME           =0x3300,
	CLOSING_FRAME           =0x3400,


	INCOMPLETE_TEXT_FRAME   =0x01,
	INCOMPLETE_BINARY_FRAME =0x02,


	TEXT_FRAME              =0x81,
	BINARY_FRAME            =0x82,


	PING_FRAME              =0x19,
	PONG_FRAME              =0x1A

}WebSocketFrameType;

#define COOLLINK_WS_REPLY_BODY "{\"error\":%d,\"deviceid\":\"%s\",\"apikey\":\"%s\",\"userAgent\":\"device\",\"sequence\":\"%s\"}"

#define COOLLINK_WS_UPDATE_BODY_WITH_PARAMS "{\"action\":\"update\",\"deviceid\":\"%s\",\"apikey\":\"%s\",\"userAgent\":\"device\",\"d_seq\":%llu,\"params\":%s}"

extern int g_port;
extern char g_Apikey[128];
extern char g_pSocket[20];
extern uint64_t g_msgid;
extern osSemaphoreId g_tAppSemaphoreId;
extern int g_tcp_hdl_ID;
extern int g_tx_ID;
extern int g_rx_ID;
extern uint8_t g_IsInitPost;
extern uint16_t g_u16HbInterval;

// extern uint64_t Coolkit_Cloud_GetNewSeqID(void);

uint64_t Coolkit_Cloud_GetNewSeqID(void);
int Connect_coolkit_http(void);
int Connect_coolkit_wss(void);
int32_t Coolkit_Cloud_Send(uint8_t *u8aPayload, uint32_t u32PayloadLen);
int32_t Coolkit_Cloud_Recv(uint8_t *u8RecvBuf, uint32_t *s32Len);

WebSocketFrameType ws_decode(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length);
void ws_encode( char* szBodyToSend, uint32_t* ulTotalBodyLen, char *szRequest_data, uint32_t ulLen);
int ws_write(char* data, uint32_t ulLen);
int ws_recv(char *buf, uint32_t len);
int ws_connect(const char *host, uint16_t port);
int ws_close(void);
void ws_init(void);
uint8_t ws_KeepAlive(void);
void ws_ReadTimeoutSet(uint32_t timeout);

#endif

