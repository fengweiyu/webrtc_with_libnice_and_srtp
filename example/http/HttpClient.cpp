/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       HttpClient.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#if 0

#include "HttpClient.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>


using std::cout;
using std::endl;

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
    m_pTcpClient =NULL;
    m_iServerPort = -1;
    m_strServerIp.assign("");

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
    if(NULL != m_pTcpClient)
        delete m_pTcpClient;
}

/*****************************************************************************
-Fuction		: ~Init
-Description	: ~Init
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: Init(char * i_strIP,unsigned short i_wPort)
{
    int iRet=-1;
    if(NULL==i_strIP || i_wPort<0)
    {
        printf("HttpClient :: Init NULL\r\n");
        return iRet;
    }
    m_strServerIp.assign(i_strIP);
    m_iServerPort=i_wPort;
    iRet=0;
    return iRet;
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
    
    if(NULL != m_pTcpClient)
    {
        delete m_pTcpClient;
        m_pTcpClient =NULL;
    }
    m_pTcpClient = new TcpClient();//http短连接从新建立连接
    if(NULL == m_pTcpClient)
    {
        printf("HttpClient :: Send NULL\r\n");
        return iRet;
    }
    m_pTcpClient->Init(m_strServerIp,m_iServerPort);

    pSendBuf = (char *)malloc(i_iSendLen+512);//512是头部大小
    if(NULL != pSendBuf)
    {
        memset(pSendBuf,0,i_iSendLen+512);
        iSendLen+=snprintf(pSendBuf+iSendLen,i_iSendLen+512-iSendLen,"%s %s %s\r\n",i_strMethod,i_strURL,HTTP_VERSION);
        if(NULL != i_strContentType)
        {
            iSendLen+=snprintf(pSendBuf+iSendLen,i_iSendLen+512-iSendLen,"Content-Length:%d\r\nContent-Type:%s\r\n",i_iSendLen,i_strContentType);
        }
        iSendLen+=snprintf(pSendBuf+iSendLen,i_iSendLen+512-iSendLen,"\r\n");
        if(NULL != i_acSendBuf)
        {
            memcpy(pSendBuf+iSendLen,i_acSendBuf,i_iSendLen);
            iSendLen+=i_iSendLen;
        }

        iRet=m_pTcpClient->Send(pSendBuf,iSendLen);

        
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
        cout<<"RecvBody NULL"<<endl;
        return iRet;
    }
    if(NULL == m_pTcpClient)
    {
        printf("HttpClient :: RecvBody err no request\r\n");
        return iRet;
    }
    
    memset(acRecvBuf,0,sizeof(acRecvBuf));
    iRet=m_pTcpClient->Recv(acRecvBuf,&iRecvLen,sizeof(acRecvBuf));
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
            printf("HttpClient :: Recv err ,%p,%d,%d\r\n",pBody,i_iRecvBufMaxLen,(int)(iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag))));
        }
    }
    //delete m_pTcpClient;//每次发的时候会释放
    //m_pTcpClient =NULL;//因为可能有接收两次的情况
    return iRet;
}


/*****************************************************************************
-Fuction		: Recv
-Description	: 返回原始数据
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen)
{
    int iRet = -1;
    
    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen)
    {
        cout<<"Recv NULL"<<endl;
        return iRet;
    }
    if(NULL == m_pTcpClient)
    {
        printf("HttpClient :: RecvBody err no request\r\n");
        return iRet;
    }
    
    iRet=m_pTcpClient->Recv(o_acRecvBuf,o_piRecvLen,i_iRecvBufMaxLen);
    delete m_pTcpClient;
    m_pTcpClient =NULL;
    return iRet;
}

#endif
