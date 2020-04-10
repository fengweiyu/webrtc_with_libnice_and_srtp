/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcClient.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_CLIENT_H
#define WEBRTC_CLIENT_H


#include "TcpSocket.h"


/*****************************************************************************
-Class			: peerconnection_client
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class WebRtcClient
{
public:
	WebRtcClient();
	virtual ~WebRtcClient();
    virtual int Login(char * i_strServerIp,int i_iServerPort)=0;
    virtual int PostSdpMsg(char * i_strSdp)=0;
    virtual int GetMsg()=0;
    virtual int GetSdpMsg(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen)=0;
    virtual int GetCandidateMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)=0;

protected:
	TcpClient *m_pTcpClient;
};

/*****************************************************************************
-Class			: peerconnection_client_offer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class WebRtcClientOffer : public WebRtcClient
{
public:
	WebRtcClientOffer();
	virtual ~WebRtcClientOffer();

    int Login(char * i_strServerIp,int i_iServerPort);
    int PostSdpMsg(char * i_strSdp);
    int GetMsg();
    int GetSdpMsg(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen);
    int GetCandidateMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);

private:
	string m_SDP;
	string m_Candidate;
};

#endif
