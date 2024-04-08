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
	WebRTC(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling,int i_iStreamType=2);//:m_Libnice(i_strStunAddr,i_dwStunPort,i_iControlling),m_Srtp()
	virtual ~WebRTC();
    int SetCallback(T_WebRtcCb *i_ptWebRtcCb,void *pObj);
    int Proc();
    int StopProc();
    int GetStopedFlag();//0 ��1��
    virtual int HandleMsg(char * i_strMsg,int i_iNotJsonMsgFlag=0)=0;
    int HandleCandidateMsg(char * i_strCandidateMsg,int i_iNotJsonMsgFlag=0);
    int GetGatheringDoneFlag();//-1��δ�ռ���,0�ռ��ɹ�
    int GetSendReadyFlag();//-1���ɷ���,0׼����ͨ�����Է���
    int SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen,int i_iStreamType=0);
    virtual int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strMsg,int i_iMaxLen)=0;
    int GenerateLocalCandidateMsg(T_VideoInfo *i_ptVideoInfo,char * o_strCandidateMsg,int i_iCandidateMaxLen);
    virtual int GenerateLocalSDP(T_WebRtcMediaInfo *i_ptMediaInfo,char *o_strSDP,int i_iSdpMaxLen);//webrtc_clientʹ��
    
    static void HandshakeCb(void * pArg);//�ŵ��ϲ��Ŀ����Ϊ�˵ײ�ģ��֮�䲻Ҫ�໥����
    static void HandleRecvDataCb(char * i_acData,int i_iLen,void * pArg,void * pWebRtcArg);//�������Ը�Ϊprivate����
	static int SendVideoDataOutCb(char * i_acData,int i_iLen,void * pArg);
	static int SendAudioDataOutCb(char * i_acData,int i_iLen,void * pArg);

    T_WebRtcCb m_tWebRtcCb;
    void *m_pWebRtcCbObj;
	Srtp *m_pVideoSrtp;
	Srtp *m_pAudioSrtp;
    DtlsOnlyHandshake * m_pVideoDtlsOnlyHandshake;//ֻ����Ƶͨ����һ��ͨ����Э��
    DtlsOnlyHandshake * m_pAudioDtlsOnlyHandshake;//��Ƶͨ��
protected:
    int SendProtectedVideoRtp(char * i_acRtpBuf,int i_iRtpBufLen);
    int SendProtectedAudioRtp(char * i_acRtpBuf,int i_iRtpBufLen);
    static bool IsDtls(char *buf);
    
    Libnice m_Libnice;
	int m_iVideoSrtpCreatedFlag;//0δ����,1�Ѿ�����
	int m_iAudioSrtpCreatedFlag;//0δ����,1�Ѿ�����
    Sctp * m_pSctp;//Ӧ�����ݺ���Ƶ���ݹ���һ��ͨ������
    
    //T_DtlsOnlyHandshakeCb m_tDtlsOnlyHandshakeCb;
    T_LibniceCb m_tLibniceCb;
    T_SctpCb m_tSctpCb;
	int m_iStreamType;//0 ֻ����Ƶ����1ֻ����Ƶ����2 ������������Ƶ������
    static const int s_iAvMultiplex;//0 ��ʾ����Ƶ����һ��ͨ��(ʹ����Ƶͨ��,ֻ����һ��ͨ��),1��ʾ����Ƶ����һ��ͨ��

    int m_iLibniceProcStartedFlag;//0 ��1��
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
	WebRtcOffer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling,int i_iStreamType=2);
	virtual ~WebRtcOffer();
    int HandleMsg(char * i_strAnswerMsg,int i_iNotJsonMsgFlag=0);
    int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strOfferMsg,int i_iOfferMaxLen);
    int GenerateLocalSDP(T_WebRtcMediaInfo *i_ptMediaInfo,char *o_strSDP,int i_iSdpMaxLen);

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
	WebRtcAnswer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling,int i_iStreamType=2);
	virtual ~WebRtcAnswer();
	
    int HandleMsg(char * i_strOfferMsg,int i_iNotJsonMsgFlag=0);
    int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strAnswerMsg,int i_iAnswerMaxLen);

    int GenerateLocalSDP(T_WebRtcMediaInfo *i_ptMediaInfo,char *o_strSDP,int i_iSdpMaxLen);
private:
    int GenerateVideoSDP(T_LocalCandidate *ptLocalCandidate,char *strLocalFingerprint,T_VideoInfo *ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen);
    int GenerateAudioSDP(T_LocalCandidate *ptLocalCandidate,char *strLocalFingerprint,T_AudioInfo *ptAudioInfo,char *o_strSDP,int i_iSdpMaxLen);

    string *m_pVideoID;//Answer��ID�ӶԷ������sdp�еĻ�ȡ
    string *m_pAudioID;
    int m_iAvDiff;//�ж�����Ƶǰ��˳��
};













#endif
