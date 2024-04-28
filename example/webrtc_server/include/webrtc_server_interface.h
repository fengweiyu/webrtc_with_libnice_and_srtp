/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcServerInf.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_SERVER_INF_H
#define WEBRTC_SERVER_INF_H


/*****************************************************************************
-Class			: WebRtcServerInf
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class WebRtcServerInf
{
public:
	WebRtcServerInf(int i_iServerPort);
	virtual ~WebRtcServerInf();
    int Proc(char * i_strStunAddr,unsigned int i_dwStunPort);
    
private:
    void *m_pWebRtcServer;
};

#endif
