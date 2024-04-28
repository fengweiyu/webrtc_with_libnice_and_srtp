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
-Description	: ����
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
    WebRtcHttpSession *pWebRtcHttpSession = NULL;
    T_WebRtcHttpSessionCb tWebRtcHttpSessionCb;
    memset(&tWebRtcHttpSessionCb,0,sizeof(T_WebRtcHttpSessionCb));
    tWebRtcHttpSessionCb.HttpSessionExit = HttpSessionExit;
    tWebRtcHttpSessionCb.pObj = this;
    while(1)
    {
        iClientSocketFd=TcpServer::Accept();
        if(iClientSocketFd<0)  
        {  
            CheckHttpSession();
            continue;
        } 
        pWebRtcHttpSession = new WebRtcHttpSession(iClientSocketFd,i_strStunAddr,i_dwStunPort,tWebRtcHttpSessionCb);
        AddMapHttpSession(pWebRtcHttpSession,iClientSocketFd);
    }
}

/*****************************************************************************
-Fuction        : HttpSessionExit
-Description    : ����������̲߳���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcServer::HttpSessionExit(void *i_pSrcIoHandle,int i_iClientSocketFd,void *i_pIoHandle)
{
    int iRet = -1;

    if(NULL == i_pSrcIoHandle ||NULL == i_pIoHandle)
    {
        WEBRTC_LOGE("HttpSessionExit NULL!!!%p %p\r\n",i_pSrcIoHandle, i_pIoHandle);
        return -1;
    }
    WebRtcServer * pWebRtcServer = (WebRtcServer *)i_pIoHandle;//�����Լ�ɾ�Լ��������ⲿ�߳�ɾ
    return pWebRtcServer->DelHttpSession(i_pSrcIoHandle,i_iClientSocketFd);
}

/*****************************************************************************
-Fuction        : DelHttpSession
-Description    : ����������̲߳���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcServer::DelHttpSession(void *i_pSrcIoHandle,int i_iClientSocketFd)
{
    int iRet = -1;

    if(NULL == i_pSrcIoHandle)
    {
        WEBRTC_LOGE("DelSession NULL!!!%p \r\n",i_pSrcIoHandle);
        return -1;
    }
    WebRtcHttpSession * pWebRtcHttpSession = (WebRtcHttpSession *)i_pSrcIoHandle;
    
    std::lock_guard<std::mutex> lock(m_DelMapMtx);//std::lock_guard������������������ʱ�Զ��ͷŻ�����
    m_DelHttpSessionMap.insert(make_pair(i_iClientSocketFd,pWebRtcHttpSession));
    return 0;
}

/*****************************************************************************
-Fuction        : PushVideoData
-Description    : ����������̲߳���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcServer::CheckHttpSession()
{
    int iRet = -1;

    std::lock_guard<std::mutex> lock(m_DelMapMtx);//std::lock_guard������������������ʱ�Զ��ͷŻ�����
    for (map<int, WebRtcHttpSession *>::iterator iter = m_DelHttpSessionMap.begin(); iter != m_DelHttpSessionMap.end();)
    {
        DelMapHttpSession(iter->first,iter->second);
        iter=m_HttpSessionMap.erase(iter);// ����Ԫ�ز�������һ��Ԫ�صĵ�����
    }
    return 0;
}

/*****************************************************************************
-Fuction        : PushVideoData
-Description    : ����������̲߳���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcServer::DelMapHttpSession(int i_iClientSocketFd,WebRtcHttpSession * i_pWebRtcHttpSession)
{
    int iRet = -1;

    if(NULL == i_pWebRtcHttpSession)
    {
        WEBRTC_LOGE("DelMapHttpSession NULL!!!%p \r\n",i_pWebRtcHttpSession);
        return -1;
    }
    std::lock_guard<std::mutex> lock(m_MapMtx);//std::lock_guard������������������ʱ�Զ��ͷŻ�����
    for (map<int, WebRtcHttpSession *>::iterator iter = m_HttpSessionMap.begin(); iter != m_HttpSessionMap.end();)
    {
        if(i_iClientSocketFd == iter->first && i_pWebRtcHttpSession == iter->second)
        {
            WEBRTC_LOGW("DelHttpSession[%p]%d",i_pWebRtcHttpSession,iter->first);
            delete i_pWebRtcHttpSession;
            iter=m_HttpSessionMap.erase(iter);// ����Ԫ�ز�������һ��Ԫ�صĵ�����
        }
        else
        {
            iter++;// ����������һ��Ԫ��
        }
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : AddMapHttpSession
-Description    : ����������̲߳���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcServer::AddMapHttpSession(WebRtcHttpSession * i_pWebRtcHttpSession,int i_iClientSocketFd)
{
    int iRet = -1;

    if(NULL == i_pWebRtcHttpSession)
    {
        WEBRTC_LOGE("AddMapHttpSession NULL!!!%p\r\n",i_pWebRtcHttpSession);
        return -1;
    }
    std::lock_guard<std::mutex> lock(m_MapMtx);//std::lock_guard������������������ʱ�Զ��ͷŻ�����
    m_HttpSessionMap.insert(make_pair(i_iClientSocketFd,i_pWebRtcHttpSession));
    return 0;
}


