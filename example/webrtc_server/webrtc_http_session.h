/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcHttpSession.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_HTTP_SESSION_H
#define WEBRTC_HTTP_SESSION_H

#include "HttpServer.h"
#include "TcpSocket.h"
#include "webrtc_session.h"

typedef struct WebRtcHttpSessionCb
{
    int (*HttpSessionExit)(void *i_pSrcIoHandle,int i_iClientSocketFd,void *i_pIoHandle);
    void *pObj;//
}T_WebRtcHttpSessionCb;

/*****************************************************************************
-Class			: WebRtcHttpSession
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class WebRtcHttpSession : public HttpServer,TcpServer
{
public:
	WebRtcHttpSession(int i_iClientSocketFd,char * i_strStunAddr,unsigned int i_dwStunPort,T_WebRtcHttpSessionCb tWebRtcHttpSessionCb);
	virtual ~WebRtcHttpSession();
    int Proc();
    int SendHttpContent(const char * i_strData);
    int SendErrCode(void *i_pSrcIoHandle,int i_iErrorCode);
    int Exit(void *i_pSrcIoHandle,int i_iErr);
private:
    static int SessionSendData(const char * i_strData,void *i_pIoHandle);
    static int SessionSendErrorCodeAndExit(void *i_pSrcIoHandle,int i_iErrorCode,void *i_pIoHandle);
    int HandleHttpReq(T_HttpReqPacket *i_ptHttpReqPacket,char *o_acBuf,int i_iBufMaxLen);
    WebRtcSession* NewWebRtcSession();

    int m_iClientSocketFd;
    char m_strStunAddr[32];
    unsigned int m_dwStunPort;
    WebRtcSession * m_pWebRtcSession;
    
    T_WebRtcHttpSessionCb m_tWebRtcHttpSessionCb;
    
    thread * m_pHttpSessionProc;
	int m_iHttpSessionProcFlag;
	int m_iExitProcFlag;

    int m_iRecvGetStatusReq;//0 ·ñ£¬1ÊÇ
};

#endif
