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


#if 0
#define  WEBRTC_LOGW2(val,...)     logi(WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend 
#define  WEBRTC_LOGE2(val,...)     loge(WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend 
#define  WEBRTC_LOGD2(val,...)     logd(WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend 
#define  WEBRTC_LOGW(...)     logi(WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend 
#define  WEBRTC_LOGE(...)     loge(WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend
#define  WEBRTC_LOGD(...)     logd(WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend
#define  WEBRTC_LOGI(...)     logi(WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend
#else
#define  WEBRTC_LOGW(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGE(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGD(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGI(...)     printf(__VA_ARGS__)
#endif


typedef enum IceControlRole
{
    ICE_CONTROLLED_ROLE=0,//使用这个失败
    ICE_CONTROLLING_ROLE
}E_IceControlRole;

typedef struct VideoInfo
{
    const char *pstrFormatName;
    unsigned int dwTimestampFrequency;
    //9代表视频使用端口9来传输,但在webrtc中现在一般不使用
	//如果设置为0，代表不传输视频
    unsigned short wPortNumForSDP;//端口,官方demo都是 9
    unsigned char ucRtpPayloadType;
    unsigned char res;
	int iID;
	unsigned int dwProfileLevelId;
	char * strSPS_Base64;
	char * strPPS_Base64;
	unsigned int dwSSRC;
}T_VideoInfo;

typedef struct WebRtcCb
{
    int (*RecvVideoData)(T_VideoInfo *i_ptRtmpMediaInfo,char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//Annex-B格式裸流带00 00 00 01
    int (*RecvAudioData)(T_VideoInfo *i_ptRtmpMediaInfo,char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//aac带7字节头
    int (*RecvScriptData)(char *i_strStreamName,unsigned int i_dwTimestamp,char * i_acDataBuf,int i_iDataLen);
}T_WebRtcCb;












#endif
