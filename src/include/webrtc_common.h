/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :    webrtc_common.h
* Description           :    模块内部与外部调用者共同的依赖，放到对外的include里
* Created               :    2020.01.13.
* Author                :    Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_COMMON_H
#define WEBRTC_COMMON_H


#ifdef MEDIA_SEVER_TYPE_WEBRTC0
#include "XLog.h"
#define  WEBRTC_LOGW2(val,...)     logi(WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend 
#define  WEBRTC_LOGE2(val,...)     loge(WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend 
#define  WEBRTC_LOGD2(val,...)     logd(WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend 
#define  WEBRTC_LOGW(...)     logi(WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend 
#define  WEBRTC_LOGE(...)     loge(WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend
#define  WEBRTC_LOGD(...)     logd(WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend
#define  WEBRTC_LOGI(...)     logi(WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend
#else
#define  WEBRTC_LOGW2(val,...)      printf(__VA_ARGS__)
#define  WEBRTC_LOGE2(val,...)      printf(__VA_ARGS__)
#define  WEBRTC_LOGD2(val,...)      printf(__VA_ARGS__)
#define  WEBRTC_LOGW(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGE(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGD(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGI(...)     printf(__VA_ARGS__)
#endif

/*offer (主动发起)的一方为 controlling 角色
answer (被动接受)的一方为 controlled 角色
full ice agent 是 controlling role，lite ice agent 是 controlled ,srs 仅支持 lite ice*/
typedef enum IceControlRole
{
    ICE_CONTROLLED_ROLE=0,////先设置远端sdp(connecting),再发送本身sdp然后等待远端来请求并建立链接
    ICE_CONTROLLING_ROLE
}E_IceControlRole;
/*ICE 模式FULL ICE，双方都要进行连通性检查；ice 客户端实现，该模式既可以收 binding request，
也可以发 binding requestLite ICE，在 FULL ICE 和 Lite ICE 互通时，只需要 FULL ICE 一方进行连通性检查，
Lite 一方只需回应 response 消息，该模式对于部署在公网的设备比较常用；
只接受并回复 binding request 请求，不会主动发送 binding request 请求给对方
sdp 中有 a=ice-lite 字样srs 服务器采用 lite-ice 模式 */

typedef struct VideoInfo
{
    const char *pstrFormatName;
    unsigned int dwTimestampFrequency;
    //9代表视频使用端口9来传输,但在webrtc中现在一般不使用
	//如果设置为0，代表不传输视频
    unsigned short wPortNumForSDP;//端口,官方demo都是 9
    unsigned char ucRtpPayloadType;
    unsigned char res;
	int iID;//0
	unsigned int dwProfileLevelId;
	char * strSPS_Base64;
	char * strPPS_Base64;
	unsigned int dwSSRC;
}T_VideoInfo;

typedef struct AudioInfo
{
    const char *pstrFormatName;
    unsigned int dwTimestampFrequency;
    //9代表视频使用端口9来传输,但在webrtc中现在一般不使用
	//如果设置为0，代表不传输视频
    unsigned short wPortNumForSDP;//端口,官方demo都是 9
    unsigned char ucRtpPayloadType;
    unsigned char res;
	int iID;//1/
	unsigned int dwSSRC;
}T_AudioInfo;

typedef struct WebRtcMediaInfo
{
    T_VideoInfo tVideoInfo;
	T_AudioInfo tAudioInfo;
}T_WebRtcMediaInfo;

typedef struct WebRtcCb
{
    int (*RecvVideoData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//Annex-B格式裸流带00 00 00 01
    int (*RecvAudioData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//aac带7字节头
    int (*RecvRtpData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
    int (*IsRtp)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//0 否 ,1 是
    int (*IsRtcp)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
}T_WebRtcCb;












#endif
