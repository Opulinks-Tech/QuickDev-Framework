/*
 * Copyright 2010-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

/**
 * @file aws_iot_config.h
 * @brief AWS IoT specific configuration file
 */

#ifndef SRC_JOBS_IOT_JOB_CONFIG_H_
#define SRC_JOBS_IOT_JOB_CONFIG_H_

// #include "blewifi_configuration.h"

// Get from console
// =================================================
#define AWS_IOT_MQTT_HOST              "aodq6fudxv6nm-ats.iot.ap-northeast-1.amazonaws.com" ///< Customer specific MQTT HOST. The same will be used for Thing Shadow
#define AWS_IOT_MQTT_PORT              8883 ///< default port for MQTT/S
#define AWS_IOT_MQTT_CLIENT_ID         "Opulink0000" ///< MQTT client ID should be unique for every device
#define AWS_IOT_MY_THING_NAME          "Opulink0000" ///< Thing Name of the Shadow this device is associated with
#define AWS_IOT_ROOT_CA_FILENAME       "rootCA.crt" ///< Root CA file name
#define AWS_IOT_CERTIFICATE_FILENAME   "cert.pem" ///< device signed certificate file name
#define AWS_IOT_PRIVATE_KEY_FILENAME   "privkey.pem" ///< Device private key filename
// =================================================

// MQTT PubSub
#define AWS_IOT_MQTT_RX_BUF_LEN 2048
#define AWS_IOT_MQTT_TX_BUF_LEN 512 ///< Any time a message is sent out through the MQTT layer. The message is copied into this buffer anytime a publish is done. This will also be used in the case of Thing Shadow
#define AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS 5 ///< Maximum number of topic filters the MQTT client can handle at any given time. This should be increased appropriately when using Thing Shadow
#define MAX_SIZE_OF_TOPIC_LENGTH 128
#define YIELD_TIMEOUT 2000

// Shadow and Job common configs
#define MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES 80  ///< Maximum size of the Unique Client Id. For More info on the Client Id refer \ref response "Acknowledgments"
#define MAX_SIZE_CLIENT_ID_WITH_SEQUENCE MAX_SIZE_OF_UNIQUE_CLIENT_ID_BYTES + 10 ///< This is size of the extra sequence number that will be appended to the Unique client Id
#define MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE MAX_SIZE_CLIENT_ID_WITH_SEQUENCE + 20 ///< This is size of the the total clientToken key and value pair in the JSON
#define MAX_SIZE_OF_THING_NAME 40 ///< The Thing Name should not be bigger than this value. Modify this if the Thing Name needs to be bigger

// Thing Shadow specific configs
#define SHADOW_MAX_SIZE_OF_RX_BUFFER 512 ///< Maximum size of the SHADOW buffer to store the received Shadow message
#define MAX_ACKS_TO_COMEIN_AT_ANY_GIVEN_TIME 10 ///< At Any given time we will wait for this many responses. This will correlate to the rate at which the shadow actions are requested
#define MAX_THINGNAME_HANDLED_AT_ANY_GIVEN_TIME 10 ///< We could perform shadow action on any thing Name and this is maximum Thing Names we can act on at any given time
#define MAX_JSON_TOKEN_EXPECTED 120 ///< These are the max tokens that is expected to be in the Shadow JSON document. Include the metadata that gets published
#define MAX_SHADOW_TOPIC_LENGTH_WITHOUT_THINGNAME 60 ///< All shadow actions have to be published or subscribed to a topic which is of the format $aws/things/{thingName}/shadow/update/accepted. This refers to the size of the topic without the Thing Name
#define MAX_SHADOW_TOPIC_LENGTH_BYTES MAX_SHADOW_TOPIC_LENGTH_WITHOUT_THINGNAME + MAX_SIZE_OF_THING_NAME ///< This size includes the length of topic with Thing Name

// Job specific configs
#ifndef DISABLE_IOT_JOBS
#define MAX_SIZE_OF_JOB_ID 64
#define MAX_JOB_JSON_TOKEN_EXPECTED 120
#define MAX_SIZE_OF_JOB_REQUEST AWS_IOT_MQTT_TX_BUF_LEN

#define MAX_JOB_TOPIC_LENGTH_WITHOUT_JOB_ID_OR_THING_NAME 40
#define MAX_JOB_TOPIC_LENGTH_BYTES MAX_JOB_TOPIC_LENGTH_WITHOUT_JOB_ID_OR_THING_NAME + MAX_SIZE_OF_THING_NAME + MAX_SIZE_OF_JOB_ID + 2
#endif

// Auto Reconnect specific config
#define AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL 1000 ///< Minimum time before the First reconnect attempt is made as part of the exponential back-off algorithm

#ifdef BLEWIFI_ENHANCE_AWS
#define AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL 60000 // ms
#else
#define AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL 128000 ///< Maximum time interval after which exponential back-off will stop attempting to reconnect.
#endif

#ifdef AWS_SENSOR_CFG_DS
#define DISABLE_METRICS true
#else
#define DISABLE_METRICS false ///< Disable the collection of metrics by setting this to true
#endif


//Default keys Private key / Cert PEM

//unsigned char g_ubClientKey_Private_Key[] =
#define AWS_PRIVATE_KEY "-----BEGIN RSA PRIVATE KEY-----\n"\
    "MIIEpAIBAAKCAQEAubYm8m8mBIfZI9bk4WUMc/Hta303zbMlxnpnVixe655AqM7m\n"\
    "AHMcZolj/CTQamdXcGXRItXWemXRzUMp2YbLWXFohvhuIMPSGJWLaqBhW+yQNeYl\n"\
    "2CclR3rHlYbOGoVFaOQD+1FvMSLn+QbADjeIy62+NRKrqHxESgkZ5ppV8ICaLJA3\n"\
    "xPYrqqTZe3AwLlorxKoMLy+DbQrzkg5Y680zaUwuogxKsARUKbKOn9scqenVj12O\n"\
    "90zprzVf6P581/2VFqZflHqd9j8UTCBpAqKlp0BtXH+6hUGrKhbmqKXgGRx+NGO7\n"\
    "21bbxP2H/avOIXD8WXxPkLpCpZldJYgdhWblYwIDAQABAoIBAQC3/RgO/kF7IZ/m\n"\
    "WwP9rqAsytX+tVsxBzj4r9JrVKBh7TKATCMg9OqBR8LwTwTRNjsx1x3g1lycfP3j\n"\
    "z5PJuDsFDU+S0NRCz975jkdGx0hNy0yAqdh6kyXi8GiF4r2VFs5SgfzT47U2yC9s\n"\
    "z9w8+ZYkvKiCalH5Jcx+URABIIzeLMlfD0a8FkL354o2t2Vr5MODJO3F9nYGbT83\n"\
    "uQyXiWCoCH6cL9B3Xl+1u4ZpQPnazsT3/51Lg4fzR/G2ajFEkOQw3klaoAU6e9fr\n"\
    "bQXO3nrPLmYiUNHCSAUPBn3goSlldb1qdxD/3yZtK5Odi5TUzXGjBga9BWEPw+rR\n"\
    "X7QDpTRRAoGBAO373Me89ceHjlkWsyikb8l3txT/hy6OJp10cZKFeKhrufRN6ti4\n"\
    "weM5LTQa4UBDCxw5Yix6QhIQDXZRyesG/6+szWLxvyEC7OlBdcS3mO5KRqGHkhRh\n"\
    "7OwPpq1M9FubT9OHu8o+Rj/5MkrQsbB+2KAVFXCQaO/FLg72Cgm3Ow0JAoGBAMfF\n"\
    "QAOObyWx3Ygx+xGAP0YtW55pRbUJBPPOiwcA7SKbqOt9hCZZyTdtC2AnjvjZif2m\n"\
    "MPxJlgIiJJm+hePz/LbasRLGUHtgFPqNbL1IZivnpWNgiruF78VXLrQZmsaKylVD\n"\
    "8CpkwfL9N2vmpSfPVivjOMj3BSk0d5zIkIEb/iYLAoGBANDlr1Poyzl9BPewlE0N\n"\
    "ok6PdjbJYij8gGrgBr1dyZ7VCOKo7oeUDiVdUA+XtLeK4hrBSCrQmmRukNKWUo6q\n"\
    "kw/quFKs71+TyM+rNSwbGPO6YSty7EhV+PjxeBZRIYGz4CtGzxFk/pczL/E6JW9O\n"\
    "uOoz96DaMjKdu6NIV2FV5FC5AoGAUOfT6xzOaufEHpqPYiEcfyFzj453CsCiXtNx\n"\
    "ft6jK3b7TJRC5v1J/saM6t5aivIfbrjV8fcjZ63IQPPmadGwsaWUPVRP2Aert2V7\n"\
    "epVNu85PLxY0WYn+9189rPcJtgsTssCXDbRM0Eo5HdwFW0AfWBroLovMbUmwf7DC\n"\
    "xWc+4M0CgYBSiJREzxgdwGFZm0porgOhTgCM6O+gvIwZmxAIA2YJpo80sOCYt1F9\n"\
    "26xq7aH6Ac9X05ztYn6M4h+cC85MTdg83JXnxh/vdZY4vRjdF7gy2Fiuha8BkbJn\n"\
    "YSsXpweIikr1Baqvn7vp5zNZYmurxiHDax0dnzdbbxs779dxOdP/cQ==\n"\
    "-----END RSA PRIVATE KEY-----\n"

//unsigned char g_ubClientKey_Cert_PEM[] =
    
#define AWS_CERT_PEM "-----BEGIN CERTIFICATE-----\n"\
    "MIIDWjCCAkKgAwIBAgIVANy4ejlJwLCuEBgIgHteVIjxuTgSMA0GCSqGSIb3DQEB\n"\
    "CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"\
    "IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xOTExMjAwMjUx\n"\
    "NTVaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"\
    "dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC5tibybyYEh9kj1uTh\n"\
    "ZQxz8e1rfTfNsyXGemdWLF7rnkCozuYAcxxmiWP8JNBqZ1dwZdEi1dZ6ZdHNQynZ\n"\
    "hstZcWiG+G4gw9IYlYtqoGFb7JA15iXYJyVHeseVhs4ahUVo5AP7UW8xIuf5BsAO\n"\
    "N4jLrb41EquofERKCRnmmlXwgJoskDfE9iuqpNl7cDAuWivEqgwvL4NtCvOSDljr\n"\
    "zTNpTC6iDEqwBFQpso6f2xyp6dWPXY73TOmvNV/o/nzX/ZUWpl+Uep32PxRMIGkC\n"\
    "oqWnQG1cf7qFQasqFuaopeAZHH40Y7vbVtvE/Yf9q84hcPxZfE+QukKlmV0liB2F\n"\
    "ZuVjAgMBAAGjYDBeMB8GA1UdIwQYMBaAFJfzDy7P79Zmua96JFCqkK+rAoBEMB0G\n"\
    "A1UdDgQWBBQVsv77Of8Lsm1jLmO98gE8r9CFYzAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"\
    "DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAefueFgXjh3Lba9aMhENN6vqx\n"\
    "BYRA/elgoyxabwpbBr3/noxizezoMZkKS3K8oWI0s9vuhy22XAdK1jWhMTMCupe4\n"\
    "D3zIlJJK66XY2QuzkihfKgy7tQ2dQ7cYetF2LvML0iXE0CFrJa86htxjsJ9CqiaK\n"\
    "VO23abm/l8kRKYDp53ULOlPEUiuuaMkDFCvg6aFEMJgV11AMcm90KM0cDe8h05/2\n"\
    "kOyGqRfnUqvaPF6RArErPRTLzJVE+/xAsD5vl3TUUO4Zw2Pos2hugZfhQpu8tw3b\n"\
    "H/xB2ThEPkXtgLCjcq5OyRHU7WwcqhwVLBY+rkVxZkgoft9GPrFsALhvqUyHsw==\n"\
    "-----END CERTIFICATE-----\n"




#endif /* SRC_JOBS_IOT_JOB_CONFIG_H_ */
