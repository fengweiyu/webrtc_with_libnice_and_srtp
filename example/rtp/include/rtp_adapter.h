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
#include "XLog.h"
#define  RTP_LOGW(...)     logw2(WEBRTC,MEDIA)<< lformat(RTP,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  RTP_LOGE(...)     loge2(WEBRTC,MEDIA)<< lformat(RTP,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  RTP_LOGD(...)     logd2(WEBRTC,MEDIA)<< lformat(RTP,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  RTP_LOGI(...)     logi2(WEBRTC,MEDIA) << lformat(RTP,__VA_ARGS__) << lend;printf(__VA_ARGS__)
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
