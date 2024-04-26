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


#include "TcpSocket.h"

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
	WebRtcHttpSession(int i_iServerPort,char * i_strStunAddr,unsigned int i_dwStunPort);
	virtual ~WebRtcHttpSession();
    int Proc();

protected:

};

#endif
