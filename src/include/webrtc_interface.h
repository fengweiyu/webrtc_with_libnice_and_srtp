/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc_interface.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_INTERFACE_H
#define WEBRTC_INTERFACE_H

#include "webrtc_common.h"

typedef enum WebRtcRole
{
    WEBRTC_OFFER_ROLE=0,//使用这个失败
    WEBRTC_ANSWER_ROLE
}E_WebRtcRole;


typedef struct WebRtcCfg
{
    E_WebRtcRole eWebRtcRole;
    char strStunAddr[20];
    unsigned int dwStunPort;
    E_IceControlRole eControlling;
    char strCandidateSDP[512];
}T_WebRtcCfg;


/*****************************************************************************
-Class          : WebRtcInterface
-Description    : WebRtcInterface
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class WebRtcInterface
{
public:
	WebRtcInterface(T_WebRtcCfg i_tWebRtcCfg,T_WebRtcCb i_tWebRtcCb);//:m_Libnice(i_strStunAddr,i_dwStunPort,i_iControlling),m_Srtp()
	virtual ~WebRtcInterface();
    int SetCallback(T_WebRtcCb *i_ptWebRtcCb,void *pObj);
    int Proc();
    int StopProc();
    int GenerateLocalSDP(T_WebRtcMediaInfo *i_ptMediaInfo,char *o_strSDP,int i_iSdpMaxLen);//webrtc_client使用
    int HandleMsg(char * i_strMsg,int i_iNotJsonMsgFlag=0);
    int HandleCandidateMsg(char * i_strCandidateMsg,int i_iNotJsonMsgFlag=0);
    int GetGatheringDoneFlag();//-1还未收集好,0收集成功
    int GetSendReadyFlag();//-1不可发送,0准备好通道可以发送
    int SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen,int i_iStreamType=0);
    
private:
    void * m_pHandle;
    T_WebRtcCfg m_tWebRtcCfg;
};










#endif
