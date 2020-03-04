/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       peerconnection_client.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef PEERCONNECTION_CLIENT_H
#define PEERCONNECTION_CLIENT_H


#include "Http.h"


/*****************************************************************************
-Class			: peerconnection_client
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class peerconnection_client
{
public:
	peerconnection_client();
	virtual ~peerconnection_client();
    virtual int Login(char * i_strServerIp,int i_iServerPort,char * i_strSelfName)=0;
    int GetMsgFromPeer(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen);
    int PostMsgToPeer(int i_iPeerId,char * i_acSendBuf,int i_iSendLen,char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);
    int GetCandidateMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);

	virtual int GetMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)=0;
	virtual int SendMsg(int i_iPeerId, char * i_acSendBuf, int i_iSendLen,char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)=0;
protected:
	HttpClient * m_pHttpClient;
	int m_iMyId;
	int m_iPeerId;
};

/*****************************************************************************
-Class			: peerconnection_client_offer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class peerconnection_client_offer : public peerconnection_client
{
public:
	peerconnection_client_offer();
	virtual ~peerconnection_client_offer();

	int Login(char * i_strServerIp,int i_iServerPort,char * i_strSelfName);
	int GetMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);
	int SendMsg(int i_iPeerId, char * i_acSendBuf, int i_iSendLen,char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);


};


/*****************************************************************************
-Class			: peerconnection_client_answer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class peerconnection_client_answer : public peerconnection_client
{
public:
	peerconnection_client_answer();
	virtual ~peerconnection_client_answer();

	int Login(char * i_strServerIp,int i_iServerPort,char * i_strSelfName);

	int GetMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);
	int SendMsg(int i_iPeerId, char * i_acSendBuf, int i_iSendLen,char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen);
	
};

#endif
