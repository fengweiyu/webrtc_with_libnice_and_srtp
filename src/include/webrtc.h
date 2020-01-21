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
	WebRTC(char * i_strStunAddr,unsigned int i_dwStunPort,int i_iControlling):m_Libnice(i_strStunAddr,i_dwStunPort,i_iControlling),m_Srtp();
	~WebRTC();
    int Proc();
    int HandleOfferMsg(char * i_strOfferMsg,char * o_strAnswerMsg,int i_iAnswerMaxLen);
    int SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen);


    
private:
    Libnice m_Libnice;
	Srtp m_Srtp;
    DtlsOnlyHandshake * m_pDtlsOnlyHandshake;

    T_DtlsOnlyHandshakeCb m_tDtlsOnlyHandshakeCb;
    T_LibniceCb m_tLibniceCb;
	int m_iSrtpCreatedFlag;//0未创建,1已经创建

    
};



















#endif
