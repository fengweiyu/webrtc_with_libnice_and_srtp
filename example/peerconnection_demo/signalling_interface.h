/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SIGNALING_INTERFACE_H
#define SIGNALING_INTERFACE_H

#include "peerconnection_client.h"


/*****************************************************************************
-Class			: signalling_interface
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class SignalingInterface
{
public:
	SignalingInterface(int i_iIsAnswer);
	~SignalingInterface();
	int Login(char * i_strServerIp, int i_iServerPort, char * i_strSelfName);
	int GetMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);
	int GetCandidateMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);
	int SendMsg(int i_iPeerId, char * i_acSendBuf, int i_iSendLen,char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);
private:
	peerconnection_client * m_PeerConnectionClient;
	int m_iLoginSuccessFlag;//0 fail,1 sucess

};

#endif
