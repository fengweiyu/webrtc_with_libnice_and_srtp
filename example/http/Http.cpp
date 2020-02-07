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
#include "Http.h"

/*****************************************************************************
-Fuction		: HttpClient
-Description	: HttpClient
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
HttpClient :: HttpClient()
{



}

/*****************************************************************************
-Fuction		: ~HttpClient
-Description	: ~HttpClient
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
HttpClient :: ~HttpClient()
{


}


/*****************************************************************************
-Fuction		: Send
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: Send(const char * i_srtMethod,char * i_srtURL,char * i_acSendBuf,int i_iSendLen,const char * i_srtContentType)
{
    int iRet = -1;
    char * pSendBuf =NULL;
    if(NULL == i_srtMethod ||NULL == i_srtURL ||NULL == i_acSendBuf )
    {
        cout<<"GetRtpData NULL"<<endl;
        return iRet;
    }
TcpClient::Send
}


/*****************************************************************************
-Fuction		: Send
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen)
{



}


