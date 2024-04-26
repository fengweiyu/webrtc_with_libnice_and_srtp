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
#include "webrtc_server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
WebRtcServer :: WebRtcServer(int i_iServerPort)
{
    TcpServer::Init(NULL,i_iServerPort);
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
WebRtcServer :: ~WebRtcServer()
{
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
WebRtcServer :: Proc(char * i_strStunAddr,unsigned int i_dwStunPort)
{
    int iClientSocketFd=-1;
    
    while(1)
    {
        iClientSocketFd=TcpServer::Accept();
        if(iClientSocketFd<0)  
        {  
            continue;
        } 

    }
}


