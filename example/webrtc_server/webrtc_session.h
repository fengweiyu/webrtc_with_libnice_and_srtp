/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcSession.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_SESSION_H
#define WEBRTC_SESSION_H

#include "webrtc_interface.h"
#include "MediaHandle.h"
#include "rtp_interface.h"

#define  WEBRTCS_LOGW(...)     printf(__VA_ARGS__)
#define  WEBRTCS_LOGE(...)     printf(__VA_ARGS__)
#define  WEBRTCS_LOGD(...)     printf(__VA_ARGS__)
#define  WEBRTCS_LOGI(...)     printf(__VA_ARGS__)


typedef struct WebRtcSessionCb
{
    int (*SendDataOut)(const char * i_strData,void *i_pIoHandle);
    int (*SendErrorCodeAndExit)(void *i_pSrcIoHandle,int i_iErrorCode,void *i_pIoHandle);
    void *pObj;//
}T_WebRtcSessionCb;

/*****************************************************************************
-Class			: WebRtcSession
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class WebRtcSession
{
public:
	WebRtcSession(char * i_strStunAddr,unsigned int i_dwStunPort,T_WebRtcSessionCb i_tWebRtcSessionCb,int i_iID);
	virtual ~WebRtcSession();
    int Proc();
	string * m_pFileName;
private:
    T_WebRtcSessionCb m_tWebRtcSessionCb;

    WebRtcInterface * m_pWebRTC;
    RtpInterface * m_pRtpInterface;
    thread *m_pWebRtcProc;

    T_WebRtcSdpMediaInfo m_tWebRtcSdpMediaInfo;
    unsigned char *m_pbPacketsBuf;
    unsigned char **m_ppbPacketBuf;
    int * m_piEveryPacketLen;
    int m_iSendSdpSuccess;//0 success
    int m_iPackNum;//0 success
    string m_strReqBody;
    string m_strReqURL;

    int m_iLogID;
    T_MediaFrameInfo m_tFileFrameInfo;
    thread *m_pFileProc;
    int m_iFileProcFlag;
    int m_iFileExitProcFlag;
    int m_iTalkTestFlag;//0 否，1是
};

#endif
