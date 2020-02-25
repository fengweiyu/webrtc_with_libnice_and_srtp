/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_H
#define WEBRTC_H

#include "libnice_interface.h"
#include "srtp_interface.h"
#include "dtls_only_handshake.h"

typedef struct VideoInfo
{
    const char *pstrFormatName;
    unsigned int dwTimestampFrequency;
    unsigned short wPortNumForSDP;
    unsigned char ucRtpPayloadType;
    unsigned char res;

}T_VideoInfo;
/*****************************************************************************
-Class          : WebRTC
-Description    : WebRTC
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class WebRTC
{
public:
	WebRTC(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling);//:m_Libnice(i_strStunAddr,i_dwStunPort,i_iControlling),m_Srtp()
	~WebRTC();
    int Proc();
    int HandleOfferMsg(char * i_strOfferMsg);
    int HandleCandidateMsg(char * i_strCandidateMsg,T_VideoInfo *i_ptVideoInfo,char * o_strAnswerMsg,int i_iAnswerMaxLen);
    int GetSendReadyFlag();
    int SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen);
    int GetAnswerMsg(T_VideoInfo *i_ptVideoInfo,char * o_strAnswerMsg,int i_iAnswerMaxLen);
    
    static void HandshakeCb(void * pArg);//放到上层的目的是为了底层模块之间不要相互依赖
    static void HandleRecvDataCb(char * i_acData,int i_iLen,void * pArg);//后续可以改为private试试
	static int SendDataOutCb(char * i_acData,int i_iLen,void * pArg);
private:
    int GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen);
    static bool IsDtls(char *buf);
    
    Libnice m_Libnice;
	Srtp m_Srtp;
    DtlsOnlyHandshake * m_pDtlsOnlyHandshake;

    T_DtlsOnlyHandshakeCb m_tDtlsOnlyHandshakeCb;
    T_LibniceCb m_tLibniceCb;
	int m_iSrtpCreatedFlag;//0未创建,1已经创建

    
};



















#endif
