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
	WebRtcSession(int i_iServerPort,char * i_strStunAddr,unsigned int i_dwStunPort);
	virtual ~WebRtcSession();
    int Proc();

protected:

};

#endif
