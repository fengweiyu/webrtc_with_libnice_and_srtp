/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       HttpCommon.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H


#define  HTTP_LOGW(...)     printf(__VA_ARGS__)
#define  HTTP_LOGE(...)     printf(__VA_ARGS__)
#define  HTTP_LOGD(...)     printf(__VA_ARGS__)
#define  HTTP_LOGI(...)     printf(__VA_ARGS__)



#define HTTP_PACKET_MAX_LEN (10240)
#define HTTP_PACKET_BODY_MAX_LEN (8*1024)

#define HTTP_METHOD_OPTIONS "OPTIONS"
#define HTTP_METHOD_GET "GET"
#define HTTP_METHOD_PUT "PUT"
#define HTTP_METHOD_POST "POST"

#define HTTP_VERSION "HTTP/1.1"// "HTTP/1.0"

#define HTTP_CONTENT_TYPE_TEXT     "text/plain"
#define HTTP_CONTENT_FLAG "\r\n\r\n"



typedef enum HttpReqMethod
{
    HTTP_REQ_METHOD_UNKNOW=0,
    HTTP_REQ_METHOD_OPTIONS,
    HTTP_REQ_METHOD_GET,
    HTTP_REQ_METHOD_PUT,
    HTTP_REQ_METHOD_POST
}E_HttpReqMethod;

typedef struct HttpReqPacket
{
    char strMethod[16];
    char strURL[1024];
    char strVersion[16];
    char strConnection[32];
    int iContentLength;
    char strContentType[128];
    char strUserAgent[128];

    char * pcBody;
    int iBodyCurLen;
    int iBodyMaxLen;
}T_HttpReqPacket;

typedef struct HttpResPacket
{
    char strVersion[16];
    int iStatusCode;
    char strStatusMsg[128];
    char strConnection[32];
    int iContentLength;
    char strContentType[128];

    char * pcBody;
    int iBodyCurLen;
    int iBodyMaxLen;
}T_HttpResPacket;

#endif
