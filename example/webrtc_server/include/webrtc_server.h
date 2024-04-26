/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcServer.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_SERVER_H
#define WEBRTC_SERVER_H


#include "TcpSocket.h"

/*****************************************************************************
-Class			: WebRtcServer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class WebRtcServer : public TcpServer
{
public:
	WebRtcServer(int i_iServerPort);
	virtual ~WebRtcServer();
    int Proc(char * i_strStunAddr,unsigned int i_dwStunPort);

protected:

};

#endif
