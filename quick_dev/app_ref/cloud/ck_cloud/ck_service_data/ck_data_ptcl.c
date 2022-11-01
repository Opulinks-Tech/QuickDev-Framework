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
*  ck_data_ptcl.c
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

#include "app_main.h"
#include "ck_svc.h"
#include "ble_mngr_api.h"
#include "net_mngr_api.h"
#include "wifi_mngr_api.h"

#include "at_cmd_common.h"
#include "cmsis_os.h"
#include "cbc_encrypt\cbc_encrypt.h"
#include "hal_auxadc_patch.h"
#include "hal_system.h"
#include "cloud_ctrl.h"
#include "cloud_data.h"
#include "lwip/netif.h"
#include "mw_fim_default_group03.h"
#include "mw_fim_default_group12_project.h"
#include "ota_mngr.h"
#include "qd_config.h"
#include "qd_module.h"
#include "ring_buffer.h"
#if defined(MAGIC_LED)
#include "cloud_cmd_data.h"
#include "evt_group.h"
#endif

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define HI_UINT16(a)    (((a) >> 8) & 0xFF)
#define LO_UINT16(a)    ((a) & 0xFF)

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list

typedef struct S_CkDataPacket{
    uint16_t total_len;
    uint16_t remain;
    uint16_t offset;
    uint8_t *aggr_buf;
} T_CkDataPacket;

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

static T_CkDataEventTable g_tCkDataEventHandlerTbl[] =
{
    {CK_DATA_REQ_AUTH,                      CK_DataProtocol_Auth},
    {CK_DATA_REQ_AUTH_TOKEN,                CK_DataProtocol_AuthToken},
    {CK_DATA_REQ_SCAN,                      CK_DataProtocol_Scan},
    {CK_DATA_REQ_CONNECT,                   CK_DataProtocol_Connect},
    {CK_DATA_REQ_MANUAL_CONNECT_AP,         CK_DataProtocol_Manually_Connect_AP},
    {CK_DATA_REQ_DISCONNECT,                CK_DataProtocol_Disconnect},
    {CK_DATA_REQ_RECONNECT,                 CK_DataProtocol_Reconnect},
    {CK_DATA_REQ_APP_DEVICE_INFO,           CK_DataProtocol_AppDeviceInfo},
    {CK_DATA_REQ_APP_HOST_INFO,             CK_DataProtocol_AppHostInfo},
    {CK_DATA_REQ_READ_DEVICE_INFO,          CK_DataProtocol_ReadDeviceInfo},
    {CK_DATA_REQ_WRITE_DEVICE_INFO,         CK_DataProtocol_WriteDeviceInfo},
    {CK_DATA_REQ_WIFI_STATUS,               CK_DataProtocol_WifiStatus},
    {CK_DATA_REQ_RESET,                     CK_DataProtocol_Reset},
#if defined(MAGIC_LED)
    {CK_DATA_REQ_PROTOCOL_CMD,              CK_DataProtocol_ProtocolCmd},
#endif

#if (OTA_ENABLED == 1)
    {CK_DATA_REQ_OTA_VERSION,               CK_DataProtocol_OtaVersion},
    {CK_DATA_REQ_OTA_UPGRADE,               CK_DataProtocol_OtaUpgrade},
    {CK_DATA_REQ_OTA_RAW,                   CK_DataProtocol_OtaRaw},
    {CK_DATA_REQ_OTA_END,                   CK_DataProtocol_OtaEnd},
#endif

#if 0
    {CK_DATA_REQ_MP_CAL_VBAT,               CK_DataProtocol_MpCalVbat},
    {CK_DATA_REQ_MP_CAL_IO_VOLTAGE,         CK_DataProtocol_MpCalIoVoltage},
    {CK_DATA_REQ_MP_CAL_TMPR,               CK_DataProtocol_MpCalTmpr},
    {CK_DATA_REQ_MP_SYS_MODE_WRITE,         CK_DataProtocol_MpSysModeWrite},
    {CK_DATA_REQ_MP_SYS_MODE_READ,          CK_DataProtocol_MpSysModeRead},

    {CK_DATA_REQ_ENG_SYS_RESET,             CK_DataProtocol_EngSysReset},
    {CK_DATA_REQ_ENG_WIFI_MAC_WRITE,        CK_DataProtocol_EngWifiMacWrite},
    {CK_DATA_REQ_ENG_WIFI_MAC_READ,         CK_DataProtocol_EngWifiMacRead},
    {CK_DATA_REQ_ENG_BLE_MAC_WRITE,         CK_DataProtocol_EngBleMacWrite},
    {CK_DATA_REQ_ENG_BLE_MAC_READ,          CK_DataProtocol_EngBleMacRead},
    {CK_DATA_REQ_ENG_BLE_CMD,               CK_DataProtocol_EngBleCmd},
    {CK_DATA_REQ_ENG_MAC_SRC_WRITE,         CK_DataProtocol_EngMacSrcWrite},
    {CK_DATA_REQ_ENG_MAC_SRC_READ,          CK_DataProtocol_EngMacSrcRead},
#endif

    {0xFFFFFFFF,                            NULL},
};

T_CkDataPacket g_CkDataPacket = {0};

#if (OTA_ENABLED == 1)
T_CkOtaData *gTheOta = 0;
static uint16_t g_u16OtaSeqId = 0;
#endif

osTimerId g_tCkDataDelayToStopBleAdvTimer;

unsigned char g_ucSecretKey[SECRETKEY_LEN + 1] = {0};
unsigned char g_ucAppCode[UUID_SIZE + 1] = {0};
unsigned char g_ucDeviceCode[UUID_SIZE + 1] = {0};

static bool g_blInPairing = false;

// Sec 7: declaration of static function prototype

#if 0
static int _CK_DataWifiCbcEncrypt(void *src , int len , unsigned char *iv , const unsigned char *key , void *out);
static int _CK_DataWifiCbcDecrypt(void *src, int len , unsigned char *iv , const unsigned char *key, void *out);
#endif

static int _CK_DataWifiUuidGenerate(unsigned char *ucUuid , uint16_t u16BufSize);
static void _CK_DataWifiSendDeviceInfo(T_WmDeviceInfo *dev_info);
static void _CK_DataWifiSendStatusInfo(uint16_t u16Type);
static void CK_DataProtocolProc(uint16_t type, uint8_t *data, int len);

/***********
C Functions
***********/
// Sec 8: C Functions

#if 0
static int _CK_DataWifiCbcEncrypt(void *src , int len , unsigned char *iv , const unsigned char *key , void *out)
{
    int len1 = len & 0xfffffff0;
    int len2 = len1 + 16;
    int pad = len2 - len;
    uint32_t u32Keybits = 128;
    uint16_t i = 0;
    uint16_t u16BlockNum = 0;
    int ret = 0;
    void * pTempSrcPos = src;
    void * pTempOutPos = out;

    if((pTempSrcPos == NULL) || (pTempOutPos == NULL))
    {
        return -1;
    }
    mbedtls_aes_context aes_ctx = {0};

    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_enc(&aes_ctx , key , u32Keybits);

    if (len1) //execute encrypt for n-1 block
    {
        u16BlockNum = len >> 4 ;
        for (i = 0; i < u16BlockNum ; ++i)
        {
            ret = mbedtls_aes_crypt_cbc(&aes_ctx , MBEDTLS_AES_ENCRYPT , AES_BLOCK_SIZE, iv , (unsigned char *)pTempSrcPos , (unsigned char *)pTempOutPos);
            pTempSrcPos = ((char*)pTempSrcPos)+16;
            pTempOutPos = ((char*)pTempOutPos)+16;
        }
    }
    if (pad) //padding & execute encrypt for last block
    {
        char buf[16];
        memcpy((char *)buf, (char *)src + len1, len - len1);
        memset((char *)buf + len - len1, pad, pad);
        ret = mbedtls_aes_crypt_cbc(&aes_ctx , MBEDTLS_AES_ENCRYPT , AES_BLOCK_SIZE, iv , (unsigned char *)buf , (unsigned char *)out + len1);
    }
    mbedtls_aes_free(&aes_ctx);

    if(ret != 0)
        return -1;
    else
        return 0;
}

static int _CK_DataWifiCbcDecrypt(void *src, int len , unsigned char *iv , const unsigned char *key, void *out)
{
    mbedtls_aes_context aes_ctx = {0};
    int n = len >> 4;
    char *out_c = NULL;
    int offset = 0;
    int ret = 0;
    uint32_t u32Keybits = 128;
    uint16_t u16BlockNum = 0;
    char pad = 0;
    void * pTempSrcPos = src;
    void * pTempOutPos = out;
    uint16_t i = 0;

    if((pTempSrcPos == NULL) || (pTempOutPos == NULL))
    {
        return -1;
    }

    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx , key , u32Keybits);

    //decrypt n-1 block
    u16BlockNum = n - 1;
    if (n > 1)
    {
        for (i = 0; i < u16BlockNum ; ++i)
        {
            ret = mbedtls_aes_crypt_cbc(&aes_ctx , MBEDTLS_AES_DECRYPT , AES_BLOCK_SIZE, iv , (unsigned char *)pTempSrcPos , (unsigned char *)pTempOutPos);
            pTempSrcPos = ((char*)pTempSrcPos)+16;
            pTempOutPos = ((char*)pTempOutPos)+16;
        }

    }

    out_c = (char *)out;
    offset = n > 0 ? ((n - 1) << 4) : 0;
    out_c[offset] = 0;

    //decrypt last block
    ret = mbedtls_aes_crypt_cbc(&aes_ctx , MBEDTLS_AES_DECRYPT , AES_BLOCK_SIZE, iv , (unsigned char *)src + offset , (unsigned char *)out_c + offset);

    //paddind data set 0
    pad = out_c[len - 1];
    out_c[len - pad] = 0;

    mbedtls_aes_free(&aes_ctx);

    if(ret != 0)
        return -1;
    else
        return 0;
}
#endif

static int _CK_DataWifiUuidGenerate(unsigned char *ucUuid , uint16_t u16BufSize)
{
    uint8_t i = 0;
    uint8_t u8Random = 0;
    if(u16BufSize < 36)
    {
        return false;
    }
    srand(osKernelSysTick());
    for(i = 0; i<36 ; i++)
    {
        if((i == 8) || (i == 13) || (i == 18) || (i == 23))
        {
            ucUuid[i] = '-';
        }
        else
        {
            u8Random = rand()%16;
            if(u8Random < 10)
            {
                ucUuid[i] = u8Random + '0';
            }
            else
            {
                ucUuid[i] = (u8Random - 10) + 'a';
            }
        }
    }
    return true;
}

static void _CK_DataWifiSendDeviceInfo(T_WmDeviceInfo *dev_info)
{
    uint8_t *pu8Data;
    int sDataLen;
    uint8_t *pu8Pos;

    pu8Pos = pu8Data = malloc(sizeof(T_WmScanInfo));
    if (pu8Data == NULL) {
        printf("malloc error\r\n");
        return;
    }

    memcpy(pu8Data, dev_info->device_id, WIFI_MAC_ADDRESS_LENGTH);
    pu8Pos += 6;

    if (dev_info->name_len > WM_MANUFACTURER_NAME_LEN)
    {
        dev_info->name_len = WM_MANUFACTURER_NAME_LEN;
    }

    *pu8Pos++ = dev_info->name_len;
    memcpy(pu8Pos, dev_info->manufacturer_name, dev_info->name_len);
    pu8Pos += dev_info->name_len;
    sDataLen = (pu8Pos - pu8Data);

    // BLEWIFI_DUMP("device info data", pu8Data, sDataLen);

    /* create device info data packet */
    CK_DataSendEncap(CK_DATA_RSP_READ_DEVICE_INFO, pu8Data, sDataLen);

    free(pu8Data);
}

static void _CK_DataWifiSendStatusInfo(uint16_t u16Type)
{
    uint8_t *pu8Data, *pu8Pos;
    uint8_t u8Status = 0, u8StrLen = 0;
    uint16_t u16DataLen;
    uint8_t u8aIp[4], u8aNetMask[4], u8aGateway[4];
    wifi_scan_info_t stInfo;
    struct netif *iface = netif_find("st1");

    printf("[CK Data] Recv CK_DATA_REQ_WIFI_STATUS\r\n");

    u8aIp[0] = (iface->ip_addr.u_addr.ip4.addr >> 0) & 0xFF;
    u8aIp[1] = (iface->ip_addr.u_addr.ip4.addr >> 8) & 0xFF;
    u8aIp[2] = (iface->ip_addr.u_addr.ip4.addr >> 16) & 0xFF;
    u8aIp[3] = (iface->ip_addr.u_addr.ip4.addr >> 24) & 0xFF;

    u8aNetMask[0] = (iface->netmask.u_addr.ip4.addr >> 0) & 0xFF;
    u8aNetMask[1] = (iface->netmask.u_addr.ip4.addr >> 8) & 0xFF;
    u8aNetMask[2] = (iface->netmask.u_addr.ip4.addr >> 16) & 0xFF;
    u8aNetMask[3] = (iface->netmask.u_addr.ip4.addr >> 24) & 0xFF;

    u8aGateway[0] = (iface->gw.u_addr.ip4.addr >> 0) & 0xFF;
    u8aGateway[1] = (iface->gw.u_addr.ip4.addr >> 8) & 0xFF;
    u8aGateway[2] = (iface->gw.u_addr.ip4.addr >> 16) & 0xFF;
    u8aGateway[3] = (iface->gw.u_addr.ip4.addr >> 24) & 0xFF;

    // get ap info
    Opl_Wifi_ApInfo_Get((wifi_ap_record_t *)&stInfo);
    // BleWifi_Wifi_Query_Status(BLEWIFI_WIFI_QUERY_AP_INFO , (void *)&stInfo);

    pu8Pos = pu8Data = malloc(sizeof(T_WmWifiStatusInfo));
    if (pu8Data == NULL) 
    {
        printf("malloc error\r\n");
        return;
    }

    u8StrLen = strlen((char *)&stInfo.ssid);

    if (u8StrLen == 0)
    {
        u8Status = 1; // Return Failure
        if (u16Type == CK_DATA_IND_IP_STATUS_NOTIFY)     // if failure, don't notify the status
            goto release;
    }
    else
    {
        u8Status = 0; // Return success
    }

    /* Status */
    *pu8Pos++ = u8Status;

    /* ssid length */
    *pu8Pos++ = u8StrLen;

   /* SSID */
    if (u8StrLen != 0)
    {
        memcpy(pu8Pos, stInfo.ssid, u8StrLen);
        pu8Pos += u8StrLen;
    }

   /* BSSID */
    memcpy(pu8Pos, stInfo.bssid, 6);
    pu8Pos += 6;

    /* IP */
    memcpy(pu8Pos, (char *)u8aIp, 4);
    pu8Pos += 4;

    /* MASK */
    memcpy(pu8Pos,  (char *)u8aNetMask, 4);
    pu8Pos += 4;

    /* GATEWAY */
    memcpy(pu8Pos,  (char *)u8aGateway, 4);
    pu8Pos += 4;

    u16DataLen = (pu8Pos - pu8Data);

    // BLEWIFI_DUMP("Wi-Fi status info data", pu8Data, u16DataLen);

    /* create Wi-Fi status info data packet */
    CK_DataSendEncap(u16Type, pu8Data, u16DataLen);
    //CK_DataSendEncap(BLEWIFI_RSP_WIFI_STATUS, pu8Data, u16DataLen);

release:
    free(pu8Data);
}

static void CK_DataProtocol_Auth(uint16_t type, uint8_t *data, int len)
{
    unsigned char iv[IV_SIZE] = {0};
    unsigned char ucBase64Dec[MAX_AUTH_DATA_SIZE + 1] = {0};
    size_t uBase64DecLen = 0;
    unsigned char ucUuidEncData[UUID_ENC_SIZE + 1] = {0};
    unsigned char ucUuidtoBaseData[ENC_UUID_TO_BASE64_SIZE + 1] = {0};  // to base64 max size ((4 * n / 3) + 3) & ~3
    size_t uBase64EncLen = 0;

    printf("BLEWIFI: Recv BLEWIFI_REQ_AUTH \r\n");

    if(len > MAX_AUTH_DATA_SIZE)
    {
        uBase64EncLen = 0;
        CK_DataSendEncap(CK_DATA_RSP_AUTH, ucUuidtoBaseData, uBase64EncLen);
        return;
    }

    memset(g_ucSecretKey, 0, sizeof(g_ucSecretKey));

#if defined(MAGIC_LED)
    if(true == CK_DataPairingModeGet())
    {
        // auth data for pairing
        mbedtls_md5((unsigned char *)&g_tHttpPostContent.ubaApiKey , strlen(g_tHttpPostContent.ubaApiKey) , g_ucSecretKey);
    }
    else
    {
        printf("User API Key = %s\r\n", g_tHostInfo.ubaUserApikey);

        // auth data normal connection
        mbedtls_md5((unsigned char *)&g_tHostInfo.ubaUserApikey, strlen(g_tHostInfo.ubaUserApikey), g_ucSecretKey);

        // extern char g_Apikey[128];
        // memcpy(g_Apikey, g_tHostInfo.ubaUserApikey, strlen(g_tHostInfo.ubaUserApiKey));
        // g_Apikey[strlen(g_tHostInfo.ubaUserApikey)] = 0;
    }
#elif defined(DOOR_SENSOR)
		// auth data for pairing
		mbedtls_md5((unsigned char *)&g_tHttpPostContent.ubaApiKey , strlen(g_tHttpPostContent.ubaApiKey) , g_ucSecretKey);
#endif

    // TODO: set the pairing mode back to un-pair type (should be move to ble disconnect)
    CK_DataPairingModeSet(false);

    mbedtls_base64_decode(ucBase64Dec , MAX_AUTH_DATA_SIZE + 1 , &uBase64DecLen , (unsigned char *)data , len);
    memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"
    _CK_DataWifiCbcDecrypt((void *)ucBase64Dec , uBase64DecLen , iv , g_ucSecretKey , g_ucAppCode);

    printf("g_ucAppCode = %s\r\n", g_ucAppCode);

    //UUID generate
    memset(g_ucDeviceCode , 0 , UUID_SIZE);
    if(_CK_DataWifiUuidGenerate(g_ucDeviceCode , (UUID_SIZE + 1)) == false)
    {
        uBase64EncLen = 0;
        CK_DataSendEncap(CK_DATA_RSP_AUTH, ucUuidtoBaseData, uBase64EncLen);
        return;
    }

    printf("g_ucDeviceCode = %s\r\n", g_ucDeviceCode);

    memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"
    _CK_DataWifiCbcEncrypt((void *)g_ucDeviceCode , UUID_SIZE , iv , g_ucSecretKey , (void *)ucUuidEncData);
    mbedtls_base64_encode((unsigned char *)ucUuidtoBaseData , ENC_UUID_TO_BASE64_SIZE + 1  ,&uBase64EncLen ,(unsigned char *)ucUuidEncData , UUID_ENC_SIZE);

    printf("ucUuidtoBaseData = %s\r\n", ucUuidtoBaseData);

    CK_DataSendEncap(CK_DATA_RSP_AUTH, ucUuidtoBaseData, uBase64EncLen);
}

static void CK_DataProtocol_AuthToken(uint16_t type, uint8_t *data, int len)
{
    unsigned char iv[IV_SIZE + 1] = {0};
    unsigned char ucBase64Dec[MAX_AUTH_TOKEN_DATA_SIZE + 1] = {0};
    size_t uBase64DecLen = 0;
    unsigned char ucCbcDecData[MAX_AUTH_TOKEN_DATA_SIZE + 1] = {0};
    uint8_t u8Ret = 0; // 0 success , 1 fail
    char * pcToken = NULL;

    printf("[CK Data] Recv CK_DATA_REQ_AUTH_TOKEN\r\n");

    if(len > MAX_AUTH_TOKEN_DATA_SIZE)
    {
        CK_DataSendResponse(CK_DATA_RSP_AUTH_TOKEN, u8Ret);
        return;
    }

    mbedtls_base64_decode(ucBase64Dec , MAX_AUTH_TOKEN_DATA_SIZE + 1 , &uBase64DecLen , (unsigned char *)data , len);
    memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"
    _CK_DataWifiCbcDecrypt((void *)ucBase64Dec , uBase64DecLen , iv , g_ucSecretKey , ucCbcDecData);

    printf("ucCbcDecData = %s\r\n", ucCbcDecData);

    pcToken = strtok((char *)ucCbcDecData , "_");
    if(strcmp(pcToken ,(char *)g_ucAppCode) != 0)
    {
        u8Ret = 1;
    }

    pcToken = strtok(NULL , "_");
    if(strcmp(pcToken ,(char *)g_ucDeviceCode) != 0)
    {
        u8Ret = 1;
    }

    CK_DataSendResponse(CK_DATA_RSP_AUTH_TOKEN, u8Ret);

    printf("[ATS]BLE Auth %s\r\n", u8Ret ? "fail":"success");

    // calling callback to notify authenticate finish
    g_tCkDataProtocolNotifyCb((T_CkDataEvent)type, u8Ret ? OPL_ERR:OPL_OK);
}

static void CK_DataProtocol_Scan(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv Scan Request\r\n");

    // reset connection config table
    memset(&g_tCkDataConnCfg, 0, sizeof(T_CkDataConnCfg));

    g_tCkDataConnCfg.u8ConnectType = CK_DATA_CONN_TYPE_BSSID;

    // trigger scan request
    APP_NmWifiScanReq(CK_DataHandler_WifiScanDoneIndCb);
}

static void CK_DataProtocol_Connect(uint16_t type, uint8_t *data, int len)
{
    unsigned char ucDecPassword[CK_DATA_MAX_REC_PASSWORD_SIZE + 1] = {0};
    unsigned char iv[IV_SIZE + 1] = {0};
    size_t u16DecPasswordLen = 0;

    printf("[CK Data] Recv Connect Request\r\n");

    // reset connection config table
    memset(&g_tCkDataConnCfg, 0, sizeof(T_CkDataConnCfg));

    g_tCkDataConnCfg.u8ConnectType = CK_DATA_CONN_TYPE_BSSID;

    // copy bssid
    memcpy(g_tCkDataConnCfg.u8aBssid, &data[0], WIFI_MAC_ADDRESS_LENGTH);

    // set connected and timeout variable
#if CK_DATA_USE_CONNECTED
    g_tCkDataConnCfg.u8Connected = 0; // ignore data[6]
#endif
    g_tCkDataConnCfg.u8Timeout = data[7]; // second

    // check and copy password
    if(data[8] == 0) //password len = 0
    {
        printf("password_length = 0\r\n");
        g_tCkDataConnCfg.u8PwdLen = 0;
        memset((char *)g_tCkDataConnCfg.u8aPwd, 0 , WIFI_LENGTH_PASSPHRASE);
    }
    else
    {
        if(data[8] > CK_DATA_MAX_REC_PASSWORD_SIZE)
        {
            printf("\r\n Not do Manually connect %d\r\n", __LINE__);
            CK_DataSendResponse(CK_DATA_RSP_CONNECT, CK_DATA_WIFI_PASSWORD_FAIL);
            return;
        }

        mbedtls_base64_decode(ucDecPassword , CK_DATA_MAX_REC_PASSWORD_SIZE + 1 , &u16DecPasswordLen , (unsigned char *)&data[9] , data[8]);

        memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"

        _CK_DataWifiCbcDecrypt((void *)ucDecPassword , u16DecPasswordLen , iv , g_ucSecretKey , (void *)g_tCkDataConnCfg.u8aPwd);

        g_tCkDataConnCfg.u8PwdLen = strlen((char *)g_tCkDataConnCfg.u8aPwd);

        //printf("password = %s\r\n" , wifi_config_req_connect.sta_config.password);
        //printf("password_length = %u\r\n" , wifi_config_req_connect.sta_config.password_length);
    }

    ////// for debug  ////////////////
    printf("[%s %d]conn_config.password=%s\n", __FUNCTION__, __LINE__, g_tCkDataConnCfg.u8aPwd);
#if CK_DATA_USE_CONNECTED
    printf("[%s %d]conn_config.connected=%d\n", __FUNCTION__, __LINE__, g_tCkDataConnCfg.u8Connected);
#endif
    //////////////////////////////////

    T_NmWifiCnctConfig stConnConfig = {0};

    memcpy(stConnConfig.u8aBssid, g_tCkDataConnCfg.u8aBssid, WIFI_MAC_ADDRESS_LENGTH);
    memcpy(stConnConfig.u8aPwd, g_tCkDataConnCfg.u8aPwd, WIFI_LENGTH_PASSPHRASE);
    stConnConfig.u8PwdLen = g_tCkDataConnCfg.u8PwdLen;
    stConnConfig.u8Timeout = g_tCkDataConnCfg.u8Timeout;

    // clear auto connect list & wifi profile
    Opl_Wifi_Profile_Clear();

    // rsp reset
    CK_DataSendResponse(CK_DATA_RSP_RESET, 0);

    // trigger wifi connect
    APP_NmWifiCnctReq(&stConnConfig, CK_DataHandler_WifiConnectionIndCb);

}

static void CK_DataProtocol_Manually_Connect_AP(uint16_t type, uint8_t *data, int len)
{
    unsigned char ucDecPassword[CK_DATA_MAX_REC_PASSWORD_SIZE + 1] = {0};
    unsigned char iv[IV_SIZE + 1] = {0};
    size_t u16DecPasswordLen = 0;

    printf("[CK Data] Recv Manually Connect Request\r\n");

    // data format for connecting a hidden AP
    //--------------------------------------------------------------------------------------
    //|        1     |    1~32    |    1      |     1    |         1          |   8~63     |
    //--------------------------------------------------------------------------------------
    //| ssid length  |    ssid    | Connected |  timeout |   password_length  |   password |
    //--------------------------------------------------------------------------------------

    if (len >= (1 + data[0] + 1 + 1 + 1) ) // ssid length(1) + ssid (data(0)) + Connected(1) + timeout + password_length (1)
    {
        printf("\r\n Do Manually connect %d\r\n", __LINE__);

        // reset connection config table
        memset(&g_tCkDataConnCfg, 0, sizeof(T_CkDataConnCfg));

        g_tCkDataConnCfg.u8ConnectType = CK_DATA_CONN_TYPE_SSID;

        // copy ssid len & ssid
        g_tCkDataConnCfg.u8SsidLen = data[0];
        memcpy(g_tCkDataConnCfg.u8aSsid, &data[1], data[0]);

        printf("\r\n %d  Recv Connect Request SSID is %s\r\n", __LINE__, g_tCkDataConnCfg.u8aSsid);

        // copy connection timeout time
        g_tCkDataConnCfg.u8Timeout = data[data[0] + 2];

        // check and copy password
        if(data[data[0] + 3] == 0) //password len = 0
        {
            printf("password_length = 0\r\n");
            g_tCkDataConnCfg.u8PwdLen = 0;
            memset((char *)g_tCkDataConnCfg.u8aPwd, 0 , WIFI_LENGTH_PASSPHRASE);
        }
        else
        {
            if(data[data[0] + 3] > CK_DATA_MAX_REC_PASSWORD_SIZE)
            {
                printf("\r\n Not do Manually connect %d\r\n", __LINE__);
                CK_DataSendResponse(CK_DATA_RSP_CONNECT, CK_DATA_WIFI_PASSWORD_FAIL);
                return;
            }

            mbedtls_base64_decode(ucDecPassword , CK_DATA_MAX_REC_PASSWORD_SIZE + 1 , &u16DecPasswordLen , (unsigned char *)&data[data[0] + 4] , data[data[0] + 3]);

            memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"

            _CK_DataWifiCbcDecrypt((void *)ucDecPassword , u16DecPasswordLen , iv , g_ucSecretKey , (void *)g_tCkDataConnCfg.u8aPwd);

            g_tCkDataConnCfg.u8PwdLen = strlen((char *)g_tCkDataConnCfg.u8aPwd);

            //printf("password = %s\r\n" , wifi_config_req_connect.sta_config.password);
            //printf("password_length = %u\r\n" , wifi_config_req_connect.sta_config.password_length);
        }

        g_u8IsManuallyConnectScanRetry = false;

        // clear auto connect list & wifi profile
        Opl_Wifi_Profile_Clear();

        // rsp reset
        CK_DataSendResponse(CK_DATA_RSP_RESET, 0);

        APP_NmWifiScanReq(CK_DataHandler_WifiScanDoneIndCb);
    }
    else
    {
        printf("\r\n Not do Manually connect %d\r\n", __LINE__);

        // clear auto connect list & wifi profile
        Opl_Wifi_Profile_Clear();

        // rsp reset
        CK_DataSendResponse(CK_DATA_RSP_RESET, 0);

        CK_DataSendResponse(CK_DATA_RSP_CONNECT, CK_DATA_WIFI_CONNECTED_FAIL);
    }
}


static void CK_DataProtocol_Disconnect(uint16_t type, uint8_t *data, int len)
{
    // printf("[CK Data] Recv CK_DATA_REQ_DISCONNECT\r\n");

    // trigger disconnect
    Opl_Wifi_Disc_Req(CK_DataHandler_WifiDisconnectionIndCb);
}

static void CK_DataProtocol_Reconnect(uint16_t type, uint8_t *data, int len)
{
    // printf("[CK Data] Recv CK_DATA_REQ_RECONNECT\r\n");
}

static void CK_DataProtocol_AppDeviceInfo(uint16_t type, uint8_t *data, int len)
{
    int Idx = 0;
    uint8_t *Data;
    uint8_t DeviceIDLength = 0;
    uint8_t ApiKeyLength = 0;
    uint8_t ChipIDLength = 0;
    uint8_t TotalSize = 0;
    unsigned char ucCbcEncData[128];
    size_t uCbcEncLen = 0;
    unsigned char ucBaseData[128];
    size_t uBaseLen = 0;
    unsigned char iv[17]={0};

    printf("[CK Data] Recv CK_DATA_REQ_APP_DEVICE_INFO\r\n");

    DeviceIDLength =  strlen(g_tHttpPostContent.ubaDeviceId);
    ApiKeyLength =  MAX_RSP_BASE64_API_KEY_LEN;
    ChipIDLength =  MAX_RSP_BASE64_CHIP_ID_LEN;

    TotalSize = DeviceIDLength + ApiKeyLength + ChipIDLength + 3;

    Data = (uint8_t*) malloc((TotalSize + 1) * sizeof(uint8_t));

    memcpy(&Data[Idx], &DeviceIDLength, 1);
    Idx = Idx + 1;
    memcpy(&Data[Idx],  g_tHttpPostContent.ubaDeviceId, DeviceIDLength);
    Idx = Idx + DeviceIDLength;

    memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"
    uCbcEncLen = strlen(g_tHttpPostContent.ubaApiKey);
    uCbcEncLen = (((uCbcEncLen  >> 4) + 1) << 4);
    _CK_DataWifiCbcEncrypt((void *)g_tHttpPostContent.ubaApiKey , strlen(g_tHttpPostContent.ubaApiKey) , iv , g_ucSecretKey , (void *)ucCbcEncData);
    mbedtls_base64_encode((unsigned char *)ucBaseData , 128  ,&uBaseLen ,(unsigned char *)ucCbcEncData , uCbcEncLen);
    memcpy(&Data[Idx], &uBaseLen, 1);
    Idx = Idx + 1;
    memcpy(&Data[Idx],  ucBaseData, uBaseLen);
    Idx = Idx + uBaseLen;

    memset(ucCbcEncData , 0 , 128);
    memset(ucBaseData , 0 , 128);
    memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"
    uCbcEncLen = strlen(g_tHttpPostContent.ubaChipId);
    uCbcEncLen = (((uCbcEncLen  >> 4) + 1) << 4);
    _CK_DataWifiCbcEncrypt((void *)g_tHttpPostContent.ubaChipId , strlen(g_tHttpPostContent.ubaChipId) , iv , g_ucSecretKey , (void *)ucCbcEncData);
    mbedtls_base64_encode((unsigned char *)ucBaseData , 128  ,&uBaseLen ,(unsigned char *)ucCbcEncData , uCbcEncLen);
    memcpy(&Data[Idx], &uBaseLen, 1);
    Idx = Idx + 1;
    memcpy(&Data[Idx],  ucBaseData, uBaseLen);
    Idx = Idx + uBaseLen;

    CK_DataSendEncap(CK_DATA_RSP_APP_DEVICE_INFO, Data, Idx);

    printf("[ATS]Device info (%d) %s \r\n", Idx, Data);

    free(Data);
}

static void CK_DataProtocol_AppHostInfo(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_APP_HOST_INFO\r\n");

    int totallen = data[0] + (data[1] << 8);
    char *AllURL = NULL;
    char HeaderStr[12] = {0};
    const char *PosStart = (char *) &data[2];
    const char *PosResult;
    const char *NeedleStart = "/";
    uint8_t TerminalNull = 1;

    T_MwFim_GP12_HttpHostInfo HostInfo;
    memset(&HostInfo ,0, sizeof(T_MwFim_GP12_HttpHostInfo));

    if(NULL != strstr(PosStart, "http://"))
    {
        strcpy(HeaderStr, "http://");
    }
    else if(NULL != strstr(PosStart, "https://"))
    {
        strcpy(HeaderStr, "https://");
    }
    else
    {
        CK_DataSendResponse(CK_DATA_RSP_APP_HOST_INFO, 1);

        g_tCkDataProtocolNotifyCb((T_CkDataEvent)type, OPL_ERR);

        return;
    }

    AllURL = (char *)malloc(totallen + TerminalNull);
    if (NULL == AllURL)
    {
        CK_DataSendResponse(CK_DATA_RSP_APP_HOST_INFO, 1);

        g_tCkDataProtocolNotifyCb((T_CkDataEvent)type, OPL_ERR_ALLOC_MEMORY_FAIL);

        return;
    }

    memset(AllURL ,0, totallen);

    if ((PosStart = strstr(PosStart, HeaderStr)) != NULL)
    {
        PosResult = PosStart;
    }

    // Copy http://testapi.coolkit.cn:8080/api/user/device/update to AllURL
    memcpy(AllURL, PosResult, totallen);
    AllURL[totallen] = '\0';

    PosResult = strstr ((AllURL + strlen(HeaderStr)), NeedleStart);

    // Copy testapi.coolkit.cn:8080 / testapi.coolkit.cn to URL.
    // Calculate URL length = PosResult - AllURL - strlen(HeaderStr)
    strncpy (HostInfo.ubaHostInfoURL, (AllURL + strlen(HeaderStr)), PosResult - AllURL - strlen(HeaderStr));

    memcpy (HostInfo.ubaHostInfoDIR, (AllURL + strlen(HeaderStr) + strlen(HostInfo.ubaHostInfoURL)), (totallen - strlen(HeaderStr) - strlen(HostInfo.ubaHostInfoURL)));

    // determine is the user api key exist
#if defined(MAGIC_LED)
    if(2 <= (len - totallen - 2))
    {
        printf("user api key exist\r\n");

        // user api key exist

        int user_apikey_length = 0;
        // char *u8aUserApikeyEncrypt = NULL;
        unsigned char u8auserApiKey[UUID_SIZE] = {0};
        unsigned char iv[IV_SIZE + 1] = {0};
        unsigned char ucBase64Dec[MAX_AUTH_DATA_SIZE + 1] = {0};
        size_t uBase64DecLen = 0;

        user_apikey_length = data[totallen + 2] + (data[totallen+ 3] << 8);

        // u8aUserApikeyEncrypt = (char *)malloc(user_apikey_length);

        // if(NULL == u8aUserApikeyEncrypt)
        // {
        //     CK_DataSendResponse(CK_DATA_RSP_APP_HOST_INFO, 1);
        //     free(AllURL);
        //     return;
        // }

        // memset(u8aUserApikeyEncrypt, 0, user_apikey_length);
        // memcpy(u8aUserApikeyEncrypt, &data[totallen + 4], user_apikey_length);

        // Decrypt User APIkey
        mbedtls_base64_decode(ucBase64Dec, MAX_AUTH_DATA_SIZE + 1, &uBase64DecLen, (unsigned char *)&data[totallen + 4], user_apikey_length);
        memset(iv, '0', IV_SIZE);
        _CK_DataWifiCbcDecrypt((void *)ucBase64Dec, uBase64DecLen, iv, g_ucSecretKey, u8auserApiKey);

        strncpy(HostInfo.ubaUserApikey, (char *)u8auserApiKey, strlen((char *)u8auserApiKey));

        // Need to change SecretKey to decrypt BLE control data
        mbedtls_md5((unsigned char *)&HostInfo.ubaUserApikey, strlen(HostInfo.ubaUserApikey), g_ucSecretKey);
    }
#endif

    memcpy(&g_tHostInfo, &HostInfo, sizeof(T_MwFim_GP12_HttpHostInfo));

    if (MW_FIM_OK != MwFim_FileWrite(MW_FIM_IDX_GP12_PROJECT_HOST_INFO, 0, MW_FIM_GP12_HTTP_HOST_INFO_SIZE, (uint8_t *)&HostInfo))
    {
        printf("Write host info fail\r\n");
        CK_DataSendResponse(CK_DATA_RSP_APP_HOST_INFO, 1);
    }

    //if cloud is connected, do cloud disconnect first then execute cloud connection
    Cloud_MsgSend(CLOUD_EVT_TYPE_DISCONNECT, NULL, 0);

    RingBuf_Reset(&g_stIotRbData);

    // BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_WAIT_UPDATE_HOST, false);

    Cloud_MsgSend(CLOUD_EVT_TYPE_ESTABLISH, NULL, 0);

    // BleWifi_COM_EventStatusSet(g_tAppCtrlEventGroup, APP_CTRL_EVENT_BIT_CHANGE_HTTPURL, true);

    CK_DataSendResponse(CK_DATA_RSP_APP_HOST_INFO, 0);

    // start timer for delay to avoid disconnect directly
    CK_DataDelayToStopBleAdvTimerStart(NETWORK_STOP_DELAY);

    // calling callback to notify the pairing was finish
    g_tCkDataProtocolNotifyCb((T_CkDataEvent)type, OPL_OK);

    if(AllURL != NULL)
    {
        free(AllURL);
    }

    // if(u8aUserApikeyEncrypt != NULL)
    // {
    //     free(u8aUserApikeyEncrypt);
    // }
}

static void CK_DataProtocol_ReadDeviceInfo(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_READ_DEVICE_INFO\r\n");

    T_WmDeviceInfo stDevInfo = {0};
    char u8aManufacturerName[33] = {0};

    // get mac address
    Opl_Wifi_MacAddr_Get((uint8_t *)&stDevInfo.device_id[0]);

    // get manufacturer name
    Opl_Wifi_ManufName_Get((uint8_t *)&u8aManufacturerName);

    stDevInfo.name_len = strlen(u8aManufacturerName);
    memcpy(stDevInfo.manufacturer_name, u8aManufacturerName, stDevInfo.name_len);
    _CK_DataWifiSendDeviceInfo(&stDevInfo);
}

static void CK_DataProtocol_WriteDeviceInfo(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_WRITE_DEVICE_INFO\r\n");

    T_WmDeviceInfo stDevInfo = {0};

    memset(&stDevInfo, 0, sizeof(T_WmDeviceInfo));
    memcpy(stDevInfo.device_id, &data[0], WIFI_MAC_ADDRESS_LENGTH);
    stDevInfo.name_len = data[6];
    memcpy(stDevInfo.manufacturer_name, &data[7], stDevInfo.name_len);

    // set wifi mac address
    Opl_Wifi_MacAddr_Set((uint8_t *)&stDevInfo.device_id[0]);

    // set wifi manufacturer name
    Opl_Wifi_ManufName_Set((uint8_t *)&stDevInfo.manufacturer_name[0]);

    CK_DataSendResponse(CK_DATA_RSP_WRITE_DEVICE_INFO, 0);

    printf("[CK Data] Device ID: \""MACSTR"\"\r\n", MAC2STR(stDevInfo.device_id));
    printf("[CK Data] Device Manufacturer: %s", stDevInfo.manufacturer_name);
}

static void CK_DataProtocol_WifiStatus(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_WIFI_STATUS\r\n");

    _CK_DataWifiSendStatusInfo(CK_DATA_RSP_WIFI_STATUS);
}

static void CK_DataProtocol_Reset(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_RESET\r\n");

    // TODO: call clear profile
    Opl_Wifi_Profile_Clear();

    // TODO: call disconnect wifi
    Opl_Wifi_Disc_Req(NULL);

    CK_DataSendResponse(CK_DATA_RSP_RESET, 0);
}

#if defined(MAGIC_LED)
static void CK_DataProtocol_ProtocolCmd(uint16_t type, uint8_t *data, int len)
{
    unsigned char iv[IV_SIZE] = {0};
    unsigned char *ucBase64Dec;
    size_t uBase64DecLen = 0;
    uint8_t *u8BleCmd = NULL;

    ucBase64Dec = malloc(len);
    if( NULL == ucBase64Dec )
    {
        printf("ucBase64Dec malloc failed!\n");
        goto done;
    }

    u8BleCmd = malloc(512);
    if( NULL == u8BleCmd )
    {
        printf("BleCmd malloc failed!\n");
        goto done;
    }

    mbedtls_base64_decode(ucBase64Dec , len+1 , &uBase64DecLen , data, len);
    memset(iv, '0' , IV_SIZE); //iv = "0000000000000000"

    _CK_DataWifiCbcDecrypt((void *)ucBase64Dec , uBase64DecLen , iv , g_ucSecretKey , (void*)u8BleCmd);

    // for debug
    // if(strlen((char*)u8BleCmd) < 10)
    // {
        // BleWifi_HexDump("Org BLE data", data, len);
        // printf("BleCmd(len=%d)%s\r\n", strlen((char*)u8BleCmd), u8BleCmd);
    // }

    printf("BleCmd(len=%d)%s\r\n", strlen((char *)u8BleCmd), u8BleCmd);
    Ble_DataParser(u8BleCmd, strlen((char *)u8BleCmd));
    // App_Json_Parser_Handle((uint8_t*)u8BleCmd, strlen((char*)u8BleCmd), BLE_CTRL);

done:
    if( NULL != u8BleCmd )
        free(u8BleCmd);

    if( NULL != ucBase64Dec )
        free(ucBase64Dec);  
}
#endif

#if (OTA_ENABLED == 1)
static void CK_OtaSendVersionRsp(uint8_t status, uint16_t pid, uint16_t cid, uint16_t fid)
{
	uint8_t data[7];
	uint8_t *p = (uint8_t *)data;

	*p++ = status;
	*p++ = LO_UINT16(pid);
	*p++ = HI_UINT16(pid);
	*p++ = LO_UINT16(cid);
	*p++ = HI_UINT16(cid);
	*p++ = LO_UINT16(fid);
	*p++ = HI_UINT16(fid);

	CK_DataSendEncap(CK_DATA_RSP_OTA_VERSION, data, 7);
}

static void CK_OtaSendUpgradeRsp(uint8_t status)
{
	CK_DataSendEncap(CK_DATA_RSP_OTA_UPGRADE, &status, 1);
}

static void CK_SendEndRsp(uint8_t status, uint8_t stop)
{
	CK_DataSendEncap(CK_DATA_RSP_OTA_END, &status, 1);

    if (stop)
    {
        if (gTheOta)
        {
            if (status != CK_DATA_OTA_SUCCESS)
            {
                OTA_UpgradeGiveUp(g_u16OtaSeqId);
            }

            free(gTheOta);
            gTheOta = NULL;

            if (status != CK_DATA_OTA_SUCCESS)
            {
                // App_Ctrl_MsgSend(APP_CTRL_MSG_OTHER_OTA_OFF_FAIL, NULL, 0);
            }
            else
            {
                OTA_TriggerReboot(3000);
            }
        }
    }
}

static void CK_OtaTimeoutIndCb(void)
{
    printf("[CK DATA] OTA Timeout\r\n");

    CK_OtaSendUpgradeRsp(CK_DATA_OTA_ERR_NOT_ACTIVE);

    // OTA_TriggerReboot(3000);
}

static void CK_DataProtocol_OtaVersion(uint16_t type, uint8_t *data, int len)
{
    printf("[CK DATA] Recv BLEWIFI_REQ_OTA_VERSION \r\n");

	uint16_t pid;
	uint16_t cid;
	uint16_t fid;

    T_OplErr tEvtRst = OTA_CurrentVersionGet(&pid, &cid, &fid);

	if (OPL_OK != tEvtRst)
		CK_OtaSendVersionRsp(CK_DATA_OTA_ERR_HW_FAILURE, 0, 0, 0);
	else
		CK_OtaSendVersionRsp(CK_DATA_OTA_SUCCESS, pid, cid, fid);
}

static void CK_DataProtocol_OtaUpgrade(uint16_t type, uint8_t *data, int len)
{
    printf("[CK DATA] Recv BLEWIFI_REQ_OTA_UPGRADE \r\n");

    T_CkOtaData *ota = gTheOta;

    T_OplErr tEvtRst = OPL_OK;

	if (len != 26)
	{
		CK_OtaSendUpgradeRsp(CK_DATA_OTA_ERR_INVALID_LEN);
		return;
	}

	if (ota)
	{
		CK_OtaSendUpgradeRsp(CK_DATA_OTA_ERR_IN_PROGRESS);
		return;
	}

	ota = malloc(sizeof(T_CkOtaData));

	if (ota)
	{
		T_MwOtaFlashHeader *ota_hdr= (T_MwOtaFlashHeader*) &data[2];

		ota->pkt_idx = 0;
		ota->idx     = 0;
        ota->rx_pkt  = *(uint16_t *)&data[0];
        ota->proj_id = ota_hdr->uwProjectId;
        ota->chip_id = ota_hdr->uwChipId;
        ota->fw_id   = ota_hdr->uwFirmwareId;
        ota->total   = ota_hdr->ulImageSize;
        ota->chksum  = ota_hdr->ulImageSum;
		ota->curr 	 = 0;

        tEvtRst = OTA_UpgradeBegin(&g_u16OtaSeqId, ota_hdr, CK_OtaTimeoutIndCb);

        if(OPL_OK == tEvtRst)
        {
            CK_OtaSendUpgradeRsp(CK_DATA_OTA_SUCCESS);
            gTheOta = ota;    
        }
        else
        {
            CK_SendEndRsp(CK_DATA_OTA_ERR_HW_FAILURE, TRUE);
        }
    }
    else
    {
        CK_OtaSendUpgradeRsp(CK_DATA_OTA_ERR_MEM_CAPACITY_EXCEED);
    }
}

static uint32_t CK_OtaAdd(uint8_t *data, int len)
{
	uint16_t i;
	uint32_t sum = 0;

	for (i = 0; i < len; i++)
    {
		sum += data[i];
    }

    return sum;
}

static void CK_DataProtocol_OtaRaw(uint16_t type, uint8_t *data, int len)
{
    printf("[CK DATA] Recv BLEWIFI_REQ_OTA_RAW \r\n");

    T_CkOtaData *ota = gTheOta;

    T_OplErr tEvtRst = OPL_OK;

	if (!ota)
	{
		CK_SendEndRsp(CK_DATA_OTA_ERR_NOT_ACTIVE, FALSE);
        goto err;
	}

	if ((ota->curr + len) > ota->total)
	{
		CK_SendEndRsp(CK_DATA_OTA_ERR_INVALID_LEN, TRUE);
		goto err;
    }

	ota->pkt_idx++;
	ota->curr += len;
	ota->curr_chksum += CK_OtaAdd(data, len);

	if ((ota->idx + len) >= 256)
	{
		UINT16 total = ota->idx + len;
		UINT8 *s = data;
		UINT8 *e = data + len;
		UINT16 cpyLen = 256 - ota->idx;

		if (ota->idx)
		{
			MemCopy(&ota->buf[ota->idx], s, cpyLen);
			s += cpyLen;
			total -= 256;
			ota->idx = 0;

            tEvtRst = OTA_WriteData(g_u16OtaSeqId, ota->buf, 256);
		}

        if(OPL_OK == tEvtRst)
        {
            while (total >= 256)
            {
                tEvtRst = OTA_WriteData(g_u16OtaSeqId, s, 256);
                s += 256;
                total -= 256;

                if(OPL_OK != tEvtRst) break;
            }

            if(OPL_OK == tEvtRst)
            {
                MemCopy(ota->buf, s, e - s);
                ota->idx = e - s;

                if((ota->curr == ota->total) && ota->idx)
                {
                    tEvtRst = OTA_WriteData(g_u16OtaSeqId, ota->buf, ota->idx);
                }
            }
        }
	}
	else
	{
		MemCopy(&ota->buf[ota->idx], data, len);
		ota->idx += len;

		if ((ota->curr == ota->total) && ota->idx)
		{
            tEvtRst = OTA_WriteData(g_u16OtaSeqId, ota->buf, ota->idx);
		}
	}

    if (OPL_OK == tEvtRst)
    {
        if (ota->rx_pkt && (ota->pkt_idx >= ota->rx_pkt))
        {
            CK_DataSendEncap(CK_DATA_RSP_OTA_RAW, 0, 0);
            ota->pkt_idx = 0;
        }
    }
    else
        CK_SendEndRsp(CK_DATA_OTA_ERR_HW_FAILURE, TRUE);

err:
	return;
}

static void CK_DataProtocol_OtaEnd(uint16_t type, uint8_t *data, int len)
{
    printf("[CK DATA] Recv BLEWIFI_REQ_OTA_END \r\n");
    
    T_CkOtaData *ota = gTheOta;

    uint8_t status = data[0];

    if (!ota)
    {
        CK_SendEndRsp(CK_DATA_OTA_ERR_NOT_ACTIVE, FALSE);
        goto err;
    }

    if (status == CK_DATA_OTA_SUCCESS)
    {
        if (ota->curr == ota->total)
        {
            if(OPL_OK == OTA_UpgradeFinish(g_u16OtaSeqId))
            {
                CK_SendEndRsp(CK_DATA_OTA_SUCCESS, TRUE);
            }
            else
            {
                CK_SendEndRsp(CK_DATA_OTA_ERR_CHECKSUM, TRUE);
            }
        }
        else
        {
            CK_SendEndRsp(CK_DATA_OTA_ERR_INVALID_LEN, TRUE);
        }
    }
    else
    {
        if (ota) OTA_UpgradeGiveUp(g_u16OtaSeqId);

        // APP stop OTA
        CK_SendEndRsp(CK_DATA_OTA_SUCCESS, TRUE);
    }

    err:
    return;
}
#endif

#if 0
static void CK_DataProtocol_MpCalVbat(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_MP_CAL_VBAT\r\n");

    float fTargetVbat;

    memcpy(&fTargetVbat, &data[0], 4);
    Hal_Aux_VbatCalibration(fTargetVbat);
    CK_DataSendResponse(CK_DATA_RSP_MP_CAL_VBAT, 0);
}

static void CK_DataProtocol_MpCalIoVoltage(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_MP_CAL_IO_VOLTAGE\r\n");

    float fTargetIoVoltage;
    uint8_t ubGpioIdx;

    memcpy(&ubGpioIdx, &data[0], 1);
    memcpy(&fTargetIoVoltage, &data[1], 4);
    Hal_Aux_IoVoltageCalibration(ubGpioIdx, fTargetIoVoltage);
    CK_DataSendResponse(CK_DATA_RSP_MP_CAL_IO_VOLTAGE, 0);
}

static void CK_DataProtocol_MpCalTmpr(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_MP_CAL_TMPR\r\n");

    CK_DataSendResponse(CK_DATA_RSP_MP_CAL_TMPR, 0);
}

static void CK_DataProtocol_MpSysModeWrite(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_MP_SYS_MODE_WRITE\r\n");

    T_MwFim_SysMode tSysMode;

    // set the settings of system mode
    tSysMode.ubSysMode = data[0];
    if (tSysMode.ubSysMode < MW_FIM_SYS_MODE_MAX)
    {
        if (MW_FIM_OK == MwFim_FileWrite(MW_FIM_IDX_GP03_PATCH_SYS_MODE, 0, MW_FIM_SYS_MODE_SIZE, (uint8_t*)&tSysMode))
        {
            // TODO: set system mode
            // App_Ctrl_SysModeSet(tSysMode.ubSysMode);

            CK_DataSendResponse(CK_DATA_RSP_MP_SYS_MODE_WRITE, 0);
            return;
        }
    }

    CK_DataSendResponse(CK_DATA_RSP_MP_SYS_MODE_WRITE, 1);
}

static void CK_DataProtocol_MpSysModeRead(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_MP_SYS_MODE_READ\r\n");

    uint8_t ubSysMode;

    // TODO: get system mode
    // ubSysMode = App_Ctrl_SysModeGet();

    CK_DataSendResponse(CK_DATA_RSP_MP_SYS_MODE_READ, ubSysMode);
}

static void CK_DataProtocol_EngSysReset(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DAA_REQ_ENG_SYS_RESET\r\n");

    CK_DataSendResponse(CK_DATA_RSP_ENG_SYS_RESET, 0);

    // wait the BLE response, then reset the system
    osDelay(3000);
    Hal_Sys_SwResetAll();
}

static void CK_DataProtocol_EngWifiMacWrite(uint16_t type, uint8_t *data, int len)
{
    uint8_t u8aMacAddr[6];
    T_WmWifiSetConfigSource stSetConfigSource;

    printf("[CK Data] Recv CK_DATA_REQ_ENG_WIFI_MAC_WRITE\r\n");

    // save the mac address into flash
    memcpy(u8aMacAddr, &data[0], 6);

    // set wifi mac address
    Opl_Wifi_MacAddr_Set((uint8_t *)&u8aMacAddr[0]);

    // set config source
    // apply the mac address from flash
    stSetConfigSource.iface = MAC_IFACE_WIFI_STA;
    stSetConfigSource.type = MAC_SOURCE_FROM_FLASH;
    Opl_Wifi_ConfigSource_Set(&stSetConfigSource);

    CK_DataSendResponse(CK_DATA_RSP_ENG_WIFI_MAC_WRITE, 0);
}

static void CK_DataProtocol_EngWifiMacRead(uint16_t type, uint8_t *data, int len)
{
    uint8_t u8aMacAddr[6];

    printf("[CK Data] Recv CK_DATA_REQ_ENG_WIFI_MAC_READ\r\n");

    // get wifi mac address
    // get the mac address from flash
    Opl_Wifi_MacAddr_Get((uint8_t *)&u8aMacAddr[0]);

    CK_DataSendEncap(CK_DATA_RSP_ENG_WIFI_MAC_READ, u8aMacAddr, sizeof(u8aMacAddr));
}

static void CK_DataProtocol_EngBleMacWrite(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_ENG_BLE_MAC_WRITE\r\n");

    // set ble mac address
    Opl_Ble_MacAddr_Write(data);

    CK_DataSendResponse(CK_DATA_RSP_ENG_BLE_MAC_WRITE, 0);
}

static void CK_DataProtocol_EngBleMacRead(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_ENG_BLE_MAC_READ\r\n");

    // get ble mac address
    Opl_Ble_MacAddr_Read(data);

    CK_DataSendEncap(CK_DATA_RSP_ENG_BLE_MAC_READ, data, 6);
}

static void CK_DataProtocol_EngBleCmd(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_ENG_BLE_CMD\r\n");

    msg_print_uart1("+BLE:%s\r\n", data);

    CK_DataSendResponse(CK_DATA_RSP_ENG_BLE_CMD, 0);
}

static void CK_DataProtocol_EngMacSrcWrite(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_ENG_MAC_SRC_WRITE\r\n");

    T_OplErr tRet = OPL_ERR;
    T_WmWifiSetConfigSource stSetConfigSource;

    uint8_t sta_type, ble_type;

    sta_type = data[0];
    ble_type = data[1];

    printf("[CK Data] Enter Mac Src Write: WiFi MAC Src=%d, BLE MAC Src=%d\n", sta_type, ble_type);

    stSetConfigSource.iface = MAC_IFACE_WIFI_STA;
    stSetConfigSource.type = (mac_source_type_t)sta_type;

    // set config source
    tRet = Opl_Wifi_ConfigSource_Set(&stSetConfigSource);

    if(OPL_OK != tRet)
    {
        goto done;
    }

    stSetConfigSource.iface = MAC_IFACE_BLE;
    stSetConfigSource.type = (mac_source_type_t)ble_type;

    // set config source
    tRet = Opl_Wifi_ConfigSource_Set(&stSetConfigSource);

    if(OPL_OK != tRet)
    {
        goto done;
    }

done:
    if (OPL_OK == tRet)
    {
        CK_DataSendResponse(CK_DATA_RSP_ENG_MAC_SRC_WRITE, 0);
    }
    else
    {
        CK_DataSendResponse(CK_DATA_RSP_ENG_MAC_SRC_WRITE, 1);
    }
}

static void CK_DataProtocol_EngMacSrcRead(uint16_t type, uint8_t *data, int len)
{
    printf("[CK Data] Recv CK_DATA_REQ_ENG_MAC_SRC_READ\r\n");

    T_OplErr tRet = OPL_ERR;
    T_WmWifiGetConfigSource stGetConfigSource;

    uint8_t sta_type, ble_type;
    uint8_t MacSrc[2]={0};

    stGetConfigSource.iface = MAC_IFACE_WIFI_STA;
    stGetConfigSource.type = (mac_source_type_t *)&sta_type;

    // get config source
    tRet = Opl_Wifi_ConfigSource_Get(&stGetConfigSource);

    if(OPL_OK != tRet)
    {
        goto done;
    }

    stGetConfigSource.iface = MAC_IFACE_BLE;
    stGetConfigSource.type = (mac_source_type_t *)&ble_type;

    // get config source
    tRet = Opl_Wifi_ConfigSource_Get(&stGetConfigSource);

    if(OPL_OK != tRet)
    {
        goto done;
    }

    MacSrc[0] = sta_type;
    MacSrc[1] = ble_type;

    printf("[CK Data] WiFi MAC Src=%d, BLE MAC Src=%d\n", MacSrc[0], MacSrc[1]);

done:
    if (OPL_OK == tRet)
    {
        CK_DataSendEncap(CK_DATA_RSP_ENG_MAC_SRC_READ, MacSrc, sizeof(MacSrc));
    }
    else
    {
        CK_DataSendResponse(CK_DATA_RSP_ENG_MAC_SRC_READ, 1);
    }
}
#endif

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
static void CK_DataProtocolProc(uint16_t type, uint8_t *data, int len)
{
    uint32_t i = 0;

    while (g_tCkDataEventHandlerTbl[i].u32EventId != 0xFFFFFFFF)
    {
        // match
        if (g_tCkDataEventHandlerTbl[i].u32EventId == type)
        {
            g_tCkDataEventHandlerTbl[i].fpFunc(type, data, len);
            break;
        }

        i++;
    }

    // TODO: not match
    if (g_tCkDataEventHandlerTbl[i].u32EventId == 0xFFFFFFFF)
    {
        printf("[CK Data] event %x not found\r\n", type);
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
void CK_DataRecvHandler(uint8_t *pu8Data, uint16_t u16DataLen)
{
    T_CkDataHdrTag *tHdrTag = NULL;
    uint16_t u16HdrLen = sizeof(T_CkDataHdrTag);

    /* 1.aggregate fragment data packet, only first frag packet has header */
    /* 2.handle blewifi data packet, if data frame is aggregated completely */


    if (g_CkDataPacket.offset == 0)
    {
        tHdrTag = (T_CkDataHdrTag*)pu8Data;

        printf("[CK Data] Event:%x DataLen:%d\r\n", tHdrTag->u16EventId, tHdrTag->u16DataLen);

        g_CkDataPacket.total_len = tHdrTag->u16DataLen + u16HdrLen;
        g_CkDataPacket.remain = g_CkDataPacket.total_len;
        g_CkDataPacket.aggr_buf = malloc(g_CkDataPacket.total_len);

        if (g_CkDataPacket.aggr_buf == NULL)
        {
           printf("%s no mem, len %d\n", __func__, g_CkDataPacket.total_len);
           return;
        }
    }

    // error handle
    // if the size is overflow, don't copy the whole data
    if (u16DataLen > g_CkDataPacket.remain)
    {
        u16DataLen = g_CkDataPacket.remain;
    }


    memcpy(g_CkDataPacket.aggr_buf + g_CkDataPacket.offset, pu8Data, u16DataLen);
    g_CkDataPacket.offset += u16DataLen;
    g_CkDataPacket.remain -= u16DataLen;

    /* no frag or last frag packet */
    if (g_CkDataPacket.remain == 0)
    {
        tHdrTag = (T_CkDataHdrTag*)g_CkDataPacket.aggr_buf;
        CK_DataProtocolProc(tHdrTag->u16EventId, g_CkDataPacket.aggr_buf + u16HdrLen,  (g_CkDataPacket.total_len - u16HdrLen));
        g_CkDataPacket.offset = 0;
        g_CkDataPacket.remain = 0;
        free(g_CkDataPacket.aggr_buf);
        g_CkDataPacket.aggr_buf = NULL;
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
void CK_DataSendEncap(uint16_t u16Type, uint8_t *pu8Data, uint32_t u32TotalDataLen)
{
    T_CkDataHdrTag *tHdrTag = NULL;
    int remain_len = u32TotalDataLen;

    /* 1.fragment data packet to fit MTU size */

    /* 2.Pack blewifi header */
    tHdrTag = malloc(sizeof(T_CkDataHdrTag) + remain_len);
    if (tHdrTag == NULL)
    {
        printf("[CK Data] memory alloc fail\r\n");
        return;
    }

    tHdrTag->u16EventId = u16Type;
    tHdrTag->u16DataLen = remain_len;
    if (tHdrTag->u16DataLen)
        memcpy(tHdrTag->au8Data, pu8Data, tHdrTag->u16DataLen);

    // BLEWIFI_DUMP("[BLEWIFI]:out packet", (uint8_t*)tHdrTag, (tHdrTag->data_len + sizeof(T_CkDataHdrTag)));

    /* 3.send app data to BLE stack */
    Opl_Ble_Send_Message(CK_SVC_EVT_SEND_DATA, (uint8_t *)tHdrTag, (tHdrTag->u16DataLen + sizeof(T_CkDataHdrTag)), 0);

    free(tHdrTag);
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
void CK_DataSendResponse(uint16_t type_id, uint8_t status)
{
    CK_DataSendEncap(type_id, &status, 1);
}

#if defined(MAGIC_LED)
void CK_DataSendProtocolCmd(uint8_t *pu8Data, uint32_t u32DataLen)
{
    // this function will send data out in CK_DATA_RES_PROTOCOL_CMD command type

    unsigned char iv[IV_SIZE + 1] = {0};
    unsigned char ucCbcEncData[BUFFER_SIZE];
    size_t uCbcEncLen = 0;
    unsigned char *ucBaseData;
    size_t uBaseLen = 0;

    //encrypt data base CBC and base 64
    memset(iv, '0', IV_SIZE); //iv = "0000000000000000"
    uCbcEncLen = u32DataLen;
    uCbcEncLen = (((uCbcEncLen >> 4) + 1) << 4);
    _CK_DataWifiCbcEncrypt((void *)&pu8Data[0], u32DataLen, iv, g_ucSecretKey, (void *)ucCbcEncData);

    int sSize = ((4 * uCbcEncLen / 3) + 3) & ~3;
    ucBaseData = malloc(sSize + 1);

    if(ucBaseData == NULL)
    {
        return;
    }

    mbedtls_base64_encode((unsigned char *)ucBaseData, sSize + 1, &uBaseLen ,(unsigned char *)ucCbcEncData, uCbcEncLen);

    printf("[ATS]BLE Send data success(%llu)\r\n", g_u64WaitSeqId);
    CK_DataSendEncap(CK_DATA_RES_PROTOCOL_CMD, ucBaseData, uBaseLen);

    free(ucBaseData);
}
#endif

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
void CK_DataPairingModeSet(bool blInPair)
{
    g_blInPairing = blInPair;
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
bool CK_DataPairingModeGet(void)
{
    return g_blInPairing;
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
static void CK_DataDelayToStopBleAdvTimeoutHandler(void const *argu)
{
    printf("[CK Data] delay time to stop ble adv\r\n");
    APP_SendMessage(APP_EVT_BLE_STOP_ADV, NULL, 0);
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
void CK_DataDelayToStopBleAdvTimerStart(uint32_t u32MilliSec)
{
    osTimerStop(g_tCkDataDelayToStopBleAdvTimer);
    osTimerStart(g_tCkDataDelayToStopBleAdvTimer, u32MilliSec);
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
void CK_DataDelayToStopBleAdvTimerStop(void)
{
    osTimerStop(g_tCkDataDelayToStopBleAdvTimer);
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
void CK_DataDelayToStopBleAdvTimerInit(void)
{
    osTimerDef_t tTimerDef;

    // create delay to stop network timer
    tTimerDef.ptimer = CK_DataDelayToStopBleAdvTimeoutHandler;
    g_tCkDataDelayToStopBleAdvTimer = osTimerCreate(&tTimerDef, osTimerOnce, NULL);
    if (g_tCkDataDelayToStopBleAdvTimer == NULL)
    {
        printf("[CK Data] Create delay to stop network timeout timer fail \r\n");
    }
}
