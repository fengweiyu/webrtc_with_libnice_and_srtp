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


#ifdef MEDIA_SEVER_TYPE_WEBRTC
#include "XLog.h"
#define  WEBRTC_LOGW2(val,...)     logi2(WEBRTC,WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  WEBRTC_LOGE2(val,...)     loge2(WEBRTC,WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  WEBRTC_LOGD2(val,...)     logd2(WEBRTC,WEBRTC) << lkv(ClientPort, val) << lformat(WEBRTC,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  WEBRTC_LOGW(...)     logi2(WEBRTC,WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  WEBRTC_LOGE(...)     loge2(WEBRTC,WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  WEBRTC_LOGD(...)     logd2(WEBRTC,WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#define  WEBRTC_LOGI(...)     logi2(WEBRTC,WEBRTC) << lformat(WEBRTC,__VA_ARGS__) << lend;printf(__VA_ARGS__)
#else
#define  WEBRTC_LOGW2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  WEBRTC_LOGE2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  WEBRTC_LOGD2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  WEBRTC_LOGW(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGE(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGD(...)     printf(__VA_ARGS__)
#define  WEBRTC_LOGI(...)     printf(__VA_ARGS__)
#endif


#define WEBRTC_SDP_MEDIA_INFO_MAX_NUM 20

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
    char strFormatName[8];
    unsigned int dwTimestampFrequency;
    //9������Ƶʹ�ö˿�9������,����webrtc������һ�㲻ʹ��
	//�������Ϊ0������������Ƶ
    unsigned short wPortNumForSDP;//�˿�,�ٷ�demo���� 9
    unsigned char bRtpPayloadType;
    /*
    packetization-mode:packetization-mode��ʾͼ�����ݰ��ֲ��͵ķ�ʽ��
    0: Single NAL (Network Abstraction Layer)��ÿ֡ͼ������ȫ������һ��NAL��Ԫ���ͣ�
    1: Not Interleaved��ÿ֡ͼ�����ݱ���ŵ����NAL��Ԫ���ͣ���ЩNAL��Ԫ���͵�˳���ǰ��ս����˳���ͣ�
    2: Interleaved��ÿ֡ͼ�����ݱ���ŵ����NAL��Ԫ���ͣ�������ЩNAL��Ԫ���͵�˳����Բ����ս����˳����
    ʵ���ϣ�ֻ��I֡���Ա���ַ��ͣ�P֡��B֡�����ܱ���ַ��͡��������packetization-mode=1(һ�㶼��1)������ζ��I֡�ᱻ��ַ��͡�
    */
    unsigned char bPacketizationMode;
    unsigned char bLevelAsymmetryAllowed;//��ʾ�Ƿ��������˱����Level��һ�¡�ע��������˵�SDP�и�ֵ��Ϊ1����Ч��
    unsigned char res[3];
	char strMediaID[8];//0/mid
	unsigned int dwProfileLevelId;
	char * strSPS_Base64;
	char * strPPS_Base64;
	unsigned int dwSSRC;
}T_VideoInfo;

typedef struct AudioInfo
{
    char strFormatName[8];
    unsigned int dwTimestampFrequency;
    //9������Ƶʹ�ö˿�9������,����webrtc������һ�㲻ʹ��
	//�������Ϊ0������������Ƶ
    unsigned short wPortNumForSDP;//�˿�,�ٷ�demo���� 9
    unsigned char bRtpPayloadType;
    unsigned char res;
	char strMediaID[8];//1/mid
	unsigned int dwSSRC;
}T_AudioInfo;

typedef struct WebRtcMediaInfo
{
    T_VideoInfo tVideoInfo;
	T_AudioInfo tAudioInfo;
}T_WebRtcMediaInfo;


typedef struct WebRtcSdpMediaInfo
{
    T_VideoInfo tVideoInfos[WEBRTC_SDP_MEDIA_INFO_MAX_NUM];
	T_AudioInfo tAudioInfos[WEBRTC_SDP_MEDIA_INFO_MAX_NUM];
	int iAvMediaDiff;//����Ƶ��Ϣ��sdp��ǰ���ƫ��λ��
}T_WebRtcSdpMediaInfo;//Ŀǰֻ֧��sdp��ֻ����һ·��Ƶ��һ·��Ƶ


typedef struct WebRtcCb
{
    int (*RecvVideoData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//Annex-B��ʽ������00 00 00 01
    int (*RecvAudioData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//aac��7�ֽ�ͷ
    int (*RecvRtpData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
    int (*RecvRtcpData)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
    int (*RecvStopMsg)(void *i_pIoHandle);
    int (*IsRtp)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);//0 �� ,1 ��
    int (*IsRtcp)(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
}T_WebRtcCb;












#endif
