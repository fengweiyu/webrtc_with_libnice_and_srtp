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
#include "sctp_interface.h"
#include "webrtc_common.h"


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
	virtual ~WebRTC();
    int Proc();
    int StopProc();
    virtual int HandleMsg(char * i_strMsg,int i_iNotJsonMsgFlag=0)=0;
    int HandleCandidateMsg(char * i_strCandidateMsg,int i_iNotJsonMsgFlag=0);
    int GetGatheringDoneFlag();
    int GetSendReadyFlag();
    int SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen);
    virtual int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strMsg,int i_iMaxLen)=0;
    int GenerateLocalCandidateMsg(T_VideoInfo *i_ptVideoInfo,char * o_strCandidateMsg,int i_iCandidateMaxLen);
    virtual int GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen);//webrtc_clientʹ��
    
    static void HandshakeCb(void * pArg);//�ŵ��ϲ��Ŀ����Ϊ�˵ײ�ģ��֮�䲻Ҫ�໥����
    static void HandleRecvDataCb(char * i_acData,int i_iLen,void * pArg);//�������Ը�Ϊprivate����
	static int SendVideoDataOutCb(char * i_acData,int i_iLen,void * pArg);
protected:
    static bool IsDtls(char *buf);
    
    Libnice m_Libnice;
	Srtp m_Srtp;
    DtlsOnlyHandshake * m_pDtlsOnlyHandshake;//ֻ����Ƶͨ����һ��ͨ����Э��
    Sctp * m_pSctp;//Ӧ�����ݺ���Ƶ���ݹ���һ��ͨ������
    
    T_DtlsOnlyHandshakeCb m_tDtlsOnlyHandshakeCb;
    T_LibniceCb m_tLibniceCb;
    T_SctpCb m_tSctpCb;
	int m_iSrtpCreatedFlag;//0δ����,1�Ѿ�����
	int m_iSendReadyFlag;//0 no ready,1 ready
    
};

/*****************************************************************************
-Class          : WebRtcOffer
-Description    : WebRtcOffer
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class WebRtcOffer : public WebRTC
{
public:
	WebRtcOffer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling);
	virtual ~WebRtcOffer();
    int HandleMsg(char * i_strAnswerMsg,int i_iNotJsonMsgFlag=0);
    int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strOfferMsg,int i_iOfferMaxLen);
    int GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen);

};


/*****************************************************************************
-Class          : WebRtcAnswer
-Description    : WebRtcAnswer
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class WebRtcAnswer : public WebRTC
{
public:
	WebRtcAnswer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling);
	virtual ~WebRtcAnswer();
	
    int HandleMsg(char * i_strOfferMsg,int i_iNotJsonMsgFlag=0);
    int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strAnswerMsg,int i_iAnswerMaxLen);

    int GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen);
};













#endif
