/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcServer.c
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "webrtc_server_interface.h"
#include "webrtc_server.h"

/*****************************************************************************
-Fuction		: WebRtcServer
-Description	: WebRtcServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
WebRtcServerInf :: WebRtcServerInf(int i_iServerPort)
{
    m_pWebRtcServer=new WebRtcServer(i_iServerPort);
}

/*****************************************************************************
-Fuction		: ~WebRtcServer
-Description	: ~WebRtcServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
WebRtcServerInf :: ~WebRtcServerInf()
{
    if(NULL != m_pWebRtcServer)
    {
        delete (WebRtcServer *)m_pWebRtcServer;
    }
}

/*****************************************************************************
-Fuction		: Proc
-Description	: ×èÈû
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int WebRtcServerInf :: Proc(char * i_strStunAddr,unsigned int i_dwStunPort)
{
    WebRtcServer *pWebRtcServer = (WebRtcServer *)m_pWebRtcServer;
    return pWebRtcServer->Proc(i_strStunAddr,i_dwStunPort);
}


