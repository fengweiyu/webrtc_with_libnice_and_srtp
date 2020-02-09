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
int HttpClient :: Send(const char * i_strMethod,char * i_strURL,char * i_acSendBuf,int i_iSendLen,const char * i_strContentType)
{
    int iRet = -1;
    char * pSendBuf =NULL;
    int iSendLen = 0;
    
    if(NULL == i_strMethod ||NULL == i_strURL )
    {
        cout<<"Send NULL"<<endl;
        return iRet;
    }
    pSendBuf = (char *)malloc(i_iSendLen+512);//512是头部大小
    if(NULL != pSendBuf)
    {
        memset(pSendBuf,0,i_iSendLen+512);
        iSendLen+=snprintf(pSendBuf+iSendLen,"%s %s %s\r\n",i_strMethod,i_strURL,HTTP_VERSION);
        if(NULL != i_strContentType)
        {
            iSendLen+=snprintf(pSendBuf+iSendLen,"Content-Length:%d\r\nContent-Type:%s\r\n",i_iSendLen,i_strContentType);
        }
        iSendLen+=snprintf(pSendBuf+iSendLen,"\r\n");
        if(NULL != i_acSendBuf)
        {
            memcpy(pSendBuf+iSendLen,i_acSendBuf,i_iSendLen);
            iSendLen+=i_iSendLen;
        }

        iRet=TcpClient::Send(pSendBuf,iSendLen);

        
        free(pSendBuf);
    }
    
    return iRet;
} 


/*****************************************************************************
-Fuction		: RecvBody
-Description	: 200返回码后续作判断
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: RecvBody(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen)
{
    int iRet = -1;
    char acRecvBuf[HTTP_PACKET_MAX_LEN];
    int iRecvLen=0;
    char *pBody = NULL;
    const char * strHttpBodyFlag = "\r\n\r\n";
    
    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen)
    {
        cout<<"Send NULL"<<endl;
        return iRet;
    }
    memset(acRecvBuf,0,sizeof(acRecvBuf));
    iRet=TcpClient::Recv(acRecvBuf,&iRecvLen,sizeof(acRecvBuf));
    if(iRet == 0)
    {
        pBody = strstr(acRecvBuf,strHttpBodyFlag);
        if(NULL != pBody && i_iRecvBufMaxLen>=iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag)))
        {
            *o_piRecvLen = iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag));
            memcpy(o_acRecvBuf,pBody+strlen(strHttpBodyFlag),*o_piRecvLen);
        }
        else
        {
            iRet = -1;
            printf("HttpClient :: Recv err ,%p,%d,%d\r\n",pBody,i_iRecvBufMaxLen,iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag)));
        }
    }

    return iRet;
}


