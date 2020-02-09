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
	~peerconnection_client();
    int login(char * i_strServerIp,int i_iServerPort,char * i_strSelfName);
    int get_peer_sdp(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen);
    int post_sdp_to_peer(int i_iPeerId,char * i_acSendBuf,int i_iSendLen);
    
private:
	HttpClient * m_pHttpClient;
	int m_iMyId;
	int m_iPeerId;
};






#endif
