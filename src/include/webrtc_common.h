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

/*offer (��������)��һ��Ϊ controlling ��ɫ
answer (��������)��һ��Ϊ controlled ��ɫ
full ice agent �� controlling role��lite ice agent �� controlled ,srs ��֧�� lite ice*/
typedef enum IceControlRole
{
    ICE_CONTROLLED_ROLE=0,////������Զ��sdp(connecting),�ٷ��ͱ���sdpȻ��ȴ�Զ�������󲢽�������
    ICE_CONTROLLING_ROLE
}E_IceControlRole;
/*ICE ģʽFULL ICE��˫����Ҫ������ͨ�Լ�飻ice �ͻ���ʵ�֣���ģʽ�ȿ����� binding request��
Ҳ���Է� binding requestLite ICE���� FULL ICE �� Lite ICE ��ͨʱ��ֻ��Ҫ FULL ICE һ��������ͨ�Լ�飬
Lite һ��ֻ���Ӧ response ��Ϣ����ģʽ���ڲ����ڹ������豸�Ƚϳ��ã�
ֻ���ܲ��ظ� binding request ���󣬲����������� binding request ������Է�
sdp ���� a=ice-lite ����srs ���������� lite-ice ģʽ */

typedef struct VideoInfo
{
    const char *pstrFormatName;
    unsigned int dwTimestampFrequency;
    //9������Ƶʹ�ö˿�9������,����webrtc������һ�㲻ʹ��
	//�������Ϊ0������������Ƶ
    unsigned short wPortNumForSDP;//�˿�,�ٷ�demo���� 9
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
    //9������Ƶʹ�ö˿�9������,����webrtc������һ�㲻ʹ��
	//�������Ϊ0������������Ƶ
    unsigned short wPortNumForSDP;//�˿�,�ٷ�demo���� 9
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
    int (*RecvVideoData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//Annex-B��ʽ������00 00 00 01
    int (*RecvAudioData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//aac��7�ֽ�ͷ
    int (*RecvRtpData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
    int (*IsRtp)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//0 �� ,1 ��
    int (*IsRtcp)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
}T_WebRtcCb;












#endif
