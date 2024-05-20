/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcHttpSession.c
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "webrtc_http_session.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
WebRtcHttpSession :: WebRtcHttpSession(int i_iClientSocketFd,char * i_strStunAddr,unsigned int i_dwStunPort,T_WebRtcHttpSessionCb tWebRtcHttpSessionCb)
{
    m_iClientSocketFd=i_iClientSocketFd;
    snprintf(m_strStunAddr,sizeof(m_strStunAddr),"%s",i_strStunAddr);
    m_dwStunPort=i_dwStunPort;
    m_pWebRtcSession = NULL;

    memcpy(&m_tWebRtcHttpSessionCb,&tWebRtcHttpSessionCb,sizeof(T_WebRtcHttpSessionCb));
    
    m_iExitProcFlag = 1;
    m_iHttpSessionProcFlag = 0;
    m_pHttpSessionProc = new thread(&WebRtcHttpSession::Proc, this);
    m_pHttpSessionProc->detach();//注意线程回收
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
WebRtcHttpSession :: ~WebRtcHttpSession()
{
    if(NULL!= m_pHttpSessionProc)
    {
        WEBRTC_LOGW("WebRtcHttpSession start exit\r\n");
        m_iHttpSessionProcFlag = 0;//m_pWebRTC->StopProc();
        while(0 == m_iExitProcFlag){usleep(10);};
        //m_pHttpSessionProc->join();//用join()等待退出这里会一直阻塞//自己join自己肯定不行
        delete m_pHttpSessionProc;
        m_pHttpSessionProc = NULL;
    }
    if(NULL!= m_pWebRtcSession)
    {
        delete m_pWebRtcSession;
        m_pWebRtcSession = NULL;
    }
    WEBRTC_LOGW("~WebRtcHttpSession exit\r\n");
}

/*****************************************************************************
-Fuction		: Proc
-Description	: 阻塞
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int WebRtcHttpSession :: Proc()
{
    int iRet=-1;
    char *pcRecvBuf=NULL;
    char *pcSendBuf=NULL;
    int iRecvLen=-1;
    T_HttpReqPacket tHttpReqPacket;
    char *pcBody=NULL;
    timeval tTimeValue;
    
    if(m_iClientSocketFd < 0)
    {
        WEBRTC_LOGE("WebRtcHttpSession m_iClientSocketFd < 0 err\r\n");
        return -1;
    }
    pcRecvBuf = new char[HTTP_PACKET_MAX_LEN];
    if(NULL == pcRecvBuf)
    {
        WEBRTC_LOGE("WebRtcHttpSession NULL == pcRecvBuf err\r\n");
        return -1;
    }
    pcSendBuf = new char[HTTP_PACKET_MAX_LEN];
    if(NULL == pcSendBuf)
    {
        WEBRTC_LOGE("WebRtcHttpSession NULL == pcSendBuf err\r\n");
        delete[] pcRecvBuf;
        return -1;
    }
    pcBody= new char[HTTP_PACKET_BODY_MAX_LEN];
    if(NULL == pcBody)
    {
        WEBRTC_LOGE("WebRtcHttpSession NULL == ptHttpReqPacket err\r\n");
        delete[] pcRecvBuf;
        delete[] pcSendBuf;
        return -1;
    }
    
    m_iHttpSessionProcFlag = 1;
    m_iExitProcFlag = 0;
    WEBRTC_LOGW("WebRtcHttpSession start Proc\r\n");
    while(m_iHttpSessionProcFlag)
    {
        iRecvLen = 0;
        memset(pcRecvBuf,0,HTTP_PACKET_MAX_LEN);
        tTimeValue.tv_sec = 0;//超时时间，超时返回错误
        tTimeValue.tv_usec = (30*1000);//加快出图时间
        iRet=TcpServer::Recv(pcRecvBuf,&iRecvLen,HTTP_PACKET_MAX_LEN,m_iClientSocketFd,&tTimeValue);
        if(iRet < 0)
        {
            WEBRTC_LOGE("TcpServer::Recv err exit %d\r\n",iRecvLen);
            break;
        }
        if(iRecvLen<=0)
        {
            continue;
        }
        memset(&tHttpReqPacket,0,sizeof(T_HttpReqPacket));
        tHttpReqPacket.pcBody=pcBody;
        tHttpReqPacket.iBodyMaxLen= HTTP_PACKET_BODY_MAX_LEN;
        iRet=HttpServer::ParseRequest(pcRecvBuf,iRecvLen,&tHttpReqPacket);
        if(iRet < 0)
        {
            WEBRTC_LOGE("HttpServer::ParseRequest err%d\r\n",iRecvLen);
            continue;
        }
        memset(pcSendBuf,0,HTTP_PACKET_MAX_LEN);
        iRet=this->HandleHttpReq(&tHttpReqPacket,pcSendBuf,HTTP_PACKET_MAX_LEN);
        if(iRet > 0)
        {
            TcpServer::Send(pcSendBuf,iRet,m_iClientSocketFd);
            continue;
        }
    }
    
    if(m_iClientSocketFd>=0)
    {
        TcpServer::Close(m_iClientSocketFd);//主动退出,
        WEBRTC_LOGW("WebRtcHttpSession::Close m_iClientSocketFd Exit%d\r\n",m_iClientSocketFd);
    }
    if(NULL != pcSendBuf)
    {
        delete[] pcSendBuf;
    }
    if(NULL != pcRecvBuf)
    {
        delete[] pcRecvBuf;
    }
    if(NULL != pcBody)
    {
        delete[] pcBody;//delete (T_HttpReqPacket *)ptHttpReqPacket;
    }
    m_iExitProcFlag = 1;
    
    if(NULL != m_tWebRtcHttpSessionCb.HttpSessionExit && NULL != m_tWebRtcHttpSessionCb.pObj)
    {
        iRet=m_tWebRtcHttpSessionCb.HttpSessionExit(this,m_iClientSocketFd,m_tWebRtcHttpSessionCb.pObj);
    }
    return 0;
}

/*****************************************************************************
-Fuction		: HandleHttpReq
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int WebRtcHttpSession :: HandleHttpReq(T_HttpReqPacket *i_ptHttpReqPacket,char *o_acBuf,int i_iBufMaxLen)
{
    int iRet=-1;
    if(NULL == i_ptHttpReqPacket ||NULL == o_acBuf)
    {
        WEBRTC_LOGE("HandleHttpReq NULL err\r\n");
        return -1;
    }

    if(0 == strcmp(i_ptHttpReqPacket->strMethod,HTTP_METHOD_OPTIONS))
    {
        WEBRTC_LOGW("HandleHttpReq HTTP_METHOD_OPTIONS\r\n");
        HttpServer *pHttpServer=new HttpServer();
        iRet=pHttpServer->CreateResponse();
        iRet|=pHttpServer->SetResHeaderValue("Access-Control-Allow-Method", "POST, GET, OPTIONS, DELETE, PUT");
        iRet|=pHttpServer->SetResHeaderValue("Access-Control-Max-Age", "600");
        iRet|=pHttpServer->SetResHeaderValue("Access-Control-Allow-Headers", "access-control-allow-headers,accessol-allow-origin,content-type");//解决浏览器跨域问题
        iRet|=pHttpServer->SetResHeaderValue("Access-Control-Allow-Origin","*");
        iRet|=pHttpServer->SetResHeaderValue("Connection", "Keep-Alive");
        iRet=pHttpServer->FormatResToStream(NULL,0,o_acBuf,i_iBufMaxLen);
        delete pHttpServer;
        return iRet;
    }
    if(0 == strcmp(i_ptHttpReqPacket->strMethod,HTTP_METHOD_POST))
    {
        if(NULL != strstr(i_ptHttpReqPacket->strURL,".webrtc"))
        {
            if(NULL != m_pWebRtcSession)
            {
                WEBRTC_LOGE("unsupport repeat req HTTP_METHOD_POST %s\r\n",i_ptHttpReqPacket->strURL);
            }
            m_pWebRtcSession = NewWebRtcSession();
            iRet = m_pWebRtcSession->SetReqData(i_ptHttpReqPacket->strURL, i_ptHttpReqPacket->pcBody);
        }
        return iRet;
    }
    if(0 == strcmp(i_ptHttpReqPacket->strMethod,HTTP_METHOD_GET))
    {
        if(NULL != strstr(i_ptHttpReqPacket->strURL,".webrtc"))
        {
            WEBRTC_LOGW("HandleHttpReq recv get status msg\r\n");
            iRet = 0;
        }
        return iRet;
    }
    return iRet;
}


/*****************************************************************************
-Fuction        : NewWebRtcSession
-Description    : 非阻塞
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
WebRtcSession* WebRtcHttpSession::NewWebRtcSession()
{
    WebRtcSession* pWebRTC = NULL;
    T_WebRtcSessionCb tWebRtcSessionCb;
    
    memset(&tWebRtcSessionCb,0,sizeof(T_WebRtcSessionCb));
    tWebRtcSessionCb.SendDataOut = WebRtcHttpSession::SessionSendData;
    tWebRtcSessionCb.SendErrorCodeAndExit = WebRtcHttpSession::SessionSendErrorCodeAndExit;
    tWebRtcSessionCb.pObj= this;
    
    pWebRTC = new WebRtcSession(m_strStunAddr,m_dwStunPort,tWebRtcSessionCb,m_iClientSocketFd);
    
    WEBRTC_LOGI("WebRtcHttpSession pWebRtc[%p]\r\n", pWebRTC);
    return pWebRTC;
}

/*****************************************************************************
-Fuction        : SessionSendData
-Description    : 其他对象的线程操作
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcHttpSession::SessionSendData(const char * i_strData,void *i_pIoHandle)
{
    int iRet = -1;

    if(NULL == i_strData ||NULL == i_pIoHandle)
    {
        WEBRTC_LOGE("SessionSendData NULL\r\n");
        return -1;   
    }
    WebRtcHttpSession * pWebRtcHttpSession = (WebRtcHttpSession *)i_pIoHandle;
    return pWebRtcHttpSession->SendHttpContent(i_strData);
}
/*****************************************************************************
-Fuction        : SessionSendErrorCodeAndExit
-Description    : 其他对象的线程操作
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcHttpSession::SessionSendErrorCodeAndExit(void *i_pSrcIoHandle,int i_iErrorCode,void *i_pIoHandle) 
{
    int iRet =-1;
    if(NULL == i_pSrcIoHandle ||NULL == i_pIoHandle)
    {
        WEBRTC_LOGE("SessionSendErrorCodeAndExit NULL!!!%p %p\r\n",i_pSrcIoHandle, i_pIoHandle);
        return -1;
    }
    WebRtcHttpSession * pWebRtcHttpSession = (WebRtcHttpSession *)i_pIoHandle;
    iRet = pWebRtcHttpSession->SendErrCode(i_pSrcIoHandle,i_iErrorCode);
    iRet |= pWebRtcHttpSession->Exit(i_pSrcIoHandle,i_iErrorCode);
    return iRet;
}

/*****************************************************************************
-Fuction        : PushVideoData
-Description    : 其他对象的线程操作
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcHttpSession::SendHttpContent(const char * i_strData)
{
    int iRet = -1;
    char *pcSendBuf=NULL;


    if(NULL == i_strData)
    {
        WEBRTC_LOGE("SessionSendData NULL\r\n");
        return -1;   
    }
    if(m_iClientSocketFd < 0)
    {
        WEBRTC_LOGE("WebRtcHttpSession m_iClientSocketFd < 0 err\r\n");
        return -1;
    }
    pcSendBuf = new char[HTTP_PACKET_MAX_LEN];
    if(NULL == pcSendBuf)
    {
        WEBRTC_LOGE("WebRtcHttpSession NULL == pcSendBuf err\r\n");
        return -1;
    }
    HttpServer *pHttpServer=new HttpServer();
    iRet=pHttpServer->CreateResponse();
    iRet|=pHttpServer->SetResHeaderValue("Connection", "Keep-Alive");
    iRet|=pHttpServer->SetResHeaderValue("Content-Type", "application/json; charset=utf-8");
    iRet|=pHttpServer->SetResHeaderValue("Content-Length", (int)strlen(i_strData));
    iRet|=pHttpServer->SetResHeaderValue("Access-Control-Allow-Origin", "*");
    iRet=pHttpServer->FormatResToStream((char *)i_strData,strlen(i_strData),pcSendBuf,HTTP_PACKET_MAX_LEN);
    delete pHttpServer;
    if(iRet > 0)
    {
        TcpServer::Send(pcSendBuf,iRet,m_iClientSocketFd);
    }

    if(NULL != pcSendBuf)
    {
        delete[] pcSendBuf;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : SendErrorCode
-Description    : 其他对象的线程操作
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcHttpSession::SendErrCode(void *i_pSrcIoHandle,int i_iErrorCode) 
{
    int iRet =-1;
    const char *strErrCode = NULL;
    int iCode = 0;
    char strHttpRes[512];
    
    if(NULL == i_pSrcIoHandle)
    {
        WEBRTC_LOGE("SendErrCodeAndDelSession NULL!!!%p \r\n",i_pSrcIoHandle);
        return -1;
    }
    if(m_iClientSocketFd < 0)
    {
        WEBRTC_LOGE("WebRtcHttpSession SendErrCodeAndDelSession m_iClientSocketFd < 0 err\r\n");
        return -1;
    }
    
    iCode = i_iErrorCode;
    switch(i_iErrorCode)
    {
        case 0:
        {
            iCode = 204;
            strErrCode = "Client Exit";
            break;
        }
        case 500:
        {
            iCode = 500;
            strErrCode = "500 URL Timeout";
            break;
        }
        case 502:
        {
            iCode = 500;
            strErrCode = "500 Media Timeout";
            break;
        }
        case 400:
        {
            strErrCode = "bad request";
            break;
        }
        case 402:
        {
            strErrCode = "Media Limited";
            break;
        }
        case 404:
        default:
        {
            strErrCode = "Not Found";
            break;
        }
    }
    HttpServer *pHttpServer=new HttpServer();
    iRet=pHttpServer->CreateResponse(iCode,strErrCode);
    iRet|=pHttpServer->SetResHeaderValue("Connection", "Keep-Alive");
    iRet|=pHttpServer->SetResHeaderValue("Access-Control-Allow-Origin", "*");
    iRet=pHttpServer->FormatResToStream(NULL,0,strHttpRes,sizeof(strHttpRes));
    delete pHttpServer;
    if(iRet > 0)
    {
        TcpServer::Send(strHttpRes,iRet,m_iClientSocketFd);
    }
	return iRet; 
}

/*****************************************************************************
-Fuction        : PushVideoData
-Description    : 其他对象的线程操作
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcHttpSession::Exit(void *i_pSrcIoHandle,int i_iErr)
{
    int iRet = -1;

    if(NULL == i_pSrcIoHandle)
    {
        WEBRTC_LOGE("DelSession NULL!!!%p \r\n",i_pSrcIoHandle);
        return -1;
    }
    /*WebRtcSession * pSrcWebRtcSession = (WebRtcSession *)i_pSrcIoHandle;
    if(NULL != m_pWebRtcSession && pSrcWebRtcSession == m_pWebRtcSession)
    {
        WEBRTC_LOGW("m_pWebRtcSession::Exit\r\n");
        delete m_pWebRtcSession;
        m_pWebRtcSession = NULL;
    }*/
    
    WEBRTC_LOGW("WebRtcHttpSession::Exit\r\n");
    m_iHttpSessionProcFlag = 0;

    return 0;
}

