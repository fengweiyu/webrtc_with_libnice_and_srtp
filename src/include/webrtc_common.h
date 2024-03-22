/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :    webrtc_common.h
* Description           :    ģ���ڲ����ⲿ�����߹�ͬ���������ŵ������include��
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
    ICE_CONTROLLED_ROLE=0,//ʹ�����ʧ��
    ICE_CONTROLLING_ROLE
}E_IceControlRole;

typedef struct VideoInfo
{
    const char *pstrFormatName;
    unsigned int dwTimestampFrequency;
    //9������Ƶʹ�ö˿�9������,����webrtc������һ�㲻ʹ��
	//�������Ϊ0������������Ƶ
    unsigned short wPortNumForSDP;//�˿�,�ٷ�demo���� 9
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
    int (*RecvVideoData)(T_VideoInfo *i_ptRtmpMediaInfo,char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//Annex-B��ʽ������00 00 00 01
    int (*RecvAudioData)(T_VideoInfo *i_ptRtmpMediaInfo,char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//aac��7�ֽ�ͷ
    int (*RecvScriptData)(char *i_strStreamName,unsigned int i_dwTimestamp,char * i_acDataBuf,int i_iDataLen);
}T_WebRtcCb;












#endif
