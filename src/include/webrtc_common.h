/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc_common.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_COMMON_H
#define WEBRTC_COMMON_H

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
