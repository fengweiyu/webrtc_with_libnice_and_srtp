/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	rtp_adapter.h
* Description       : 	模块(对外)的依赖，比如一些系统相关的 定义 ，日志函数等，暂时先放到对外的include里
* Created           : 	2017.11.21.
* Author            : 	Yu Weifeng
* Function List     : 	
* Last Modified     : 	
* History           : 	
******************************************************************************/
#ifndef RTP_ADAPTER_H
#define RTP_ADAPTER_H

#ifndef TRUE
#define TRUE        0
#endif

#ifndef FALSE
#define FALSE       -1
#endif

#ifdef MEDIA_SEVER_TYPE_WEBRTC //0
#define  RTP_LOGW(...)     logi(MediaHandle) << lformat(MediaHandle,__VA_ARGS__) << lend 
#define  RTP_LOGE(...)     loge(MediaHandle) << lformat(MediaHandle,__VA_ARGS__) << lend
#define  RTP_LOGD(...)     logd(MediaHandle) << lformat(MediaHandle,__VA_ARGS__) << lend
#define  RTP_LOGI(...)     logi(MediaHandle) << lformat(MediaHandle,__VA_ARGS__) << lend
#else
#define  RTP_LOGW(...)     printf(__VA_ARGS__)
#define  RTP_LOGE(...)     printf(__VA_ARGS__)
#define  RTP_LOGD(...)     printf(__VA_ARGS__)
#define  RTP_LOGI(...)     printf(__VA_ARGS__)
#endif

typedef struct RtpMediaInfo
{
    int iVideoPayload;
    int iVideoEnc;
    int iAudioPayload;
    int iAudioEnc;
}T_RtpMediaInfo;//



#endif //RTP_ADAPTER_H
