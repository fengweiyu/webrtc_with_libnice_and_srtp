/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       peerconnection_client.c
* Description           : 	

暂时只做answer端，offer端后续再加

* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/

#include "peerconnection_client.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************
-Fuction		: peerconnection_client
-Description	: peerconnection_client
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
peerconnection_client :: peerconnection_client()
{
    m_pHttpClient = new HttpClient();
    m_iMyId = -1;
    m_iPeerId = -1;

}

/*****************************************************************************
-Fuction		: ~peerconnection_client
-Description	: ~peerconnection_client
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
peerconnection_client :: ~peerconnection_client()
{
    if(NULL != m_pHttpClient)
        delete m_pHttpClient;
}


/*****************************************************************************
-Fuction		: login
-Description	: login
-Input			: 
-Output 		: 
-Return 		: //peer_id
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int peerconnection_client :: login(char * i_strServerIp,int i_iServerPort,char * i_strSelfName)
{
    int iRet = -1;
    char strURL[128];
    char acRecvBuf[2048];
    int iRecvLen=0;
    char strPeerName[128];
    char *pPeerName = NULL;
    char *pSelfName = NULL;
    //int iConnectedFlag =0;

    if(NULL==i_strServerIp || i_iServerPort<0 || NULL==i_strSelfName)
    {
        printf("peerconnection_client :: login NULL\r\n");
        return iRet;
    }

    m_pHttpClient->Init(i_strServerIp,i_iServerPort);
    memset(strURL,0,sizeof(strURL));
    snprintf(strURL,sizeof(strURL),"/sign_in?%s",i_strSelfName);
    if(0==m_pHttpClient->Send(HTTP_METHOD_GET,strURL,NULL,0))
    {
        memset(acRecvBuf,0,sizeof(acRecvBuf));
        if(0==m_pHttpClient->RecvBody(acRecvBuf,&iRecvLen,sizeof(acRecvBuf)))
        {//选择对方id，eg:
            //ywf@ywf-PC,3,1
            //Administrator@MQR7X7EPYJYF6P9,1,1        
            pSelfName = strstr(acRecvBuf,i_strSelfName);
            if(NULL != pSelfName)
            {
                //sscanf(acRecvBuf+strlen(i_strSelfName),",%[1-9]%[^,]",strMyId);
                m_iMyId = atoi(pSelfName+strlen(i_strSelfName)+1);
                printf("peer list:\r\n");
                printf("%s",acRecvBuf);
                iRet =0;
                /* offer端后续再做
                printf("connect peer name:");
                memset(strPeerName,0,sizeof(strPeerName));
                scanf("%s",strPeerName);
                pPeerName = strstr(acRecvBuf,strPeerName);
                if(NULL != pPeerName)
                {
                    iRet = atoi(pPeerName+strlen(i_strSelfName)+1);//return peer id
                }
                else
                {
                    printf("input peer name unmatched(%s),please do it again",strPeerName);
                }*/
            }
        }
    }
    return iRet;
}



/*****************************************************************************
-Fuction		: get_peer_sdp
-Description	: 不一定返回有offer ,上层需判断是否是offer消息
-Input			: 
-Output 		: 
-Return 		: -1，对方还没有回应，其他表示peer id
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int peerconnection_client :: get_peer_sdp(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen)
{
    int iRet = -1;
    char strURL[128];
    char acRecvBuf[4*1024];
    int iRecvLen=0;
    const char * strHttpBodyFlag = "\r\n\r\n";
    const char * strPeerIdFlag = "Pragma: ";
    char *pPeerId = NULL;
    char *pBody = NULL;
    
    if(NULL==o_acRecvBuf || i_iRecvBufMaxLen<0 || NULL==o_piRecvLen)
    {
        printf("peerconnection_client :: get_peer_sdp NULL\r\n");
        return iRet;
    }
    memset(strURL,0,sizeof(strURL));
    snprintf(strURL,sizeof(strURL),"/wait?peer_id=%d",m_iMyId);
    if(0==m_pHttpClient->Send(HTTP_METHOD_GET,strURL,NULL,0))
    {
        if(0==m_pHttpClient->Recv(acRecvBuf,&iRecvLen,sizeof(acRecvBuf)))
        {
            pBody = strstr(acRecvBuf,strHttpBodyFlag);
            if(NULL != pBody && i_iRecvBufMaxLen>=iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag)))
            {
                *o_piRecvLen = iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag));
                memcpy(o_acRecvBuf,pBody+strlen(strHttpBodyFlag),*o_piRecvLen);
                pPeerId = strstr(acRecvBuf,strPeerIdFlag);
                if(NULL != pPeerId)
                {
                    iRet = atoi(pPeerId+strlen(pPeerId));
                }
            }
        }
    }
    return iRet;
}




/*****************************************************************************
-Fuction		: post_sdp_to_peer
-Description	: post_sdp_to_peer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int peerconnection_client :: post_sdp_to_peer(int i_iPeerId,char * i_acSendBuf,int i_iSendLen)
{
    int iRet = -1;
    char strURL[128];
    char acRecvBuf[2048];
    int iRecvLen=0;
    
    if(NULL==i_acSendBuf || i_iSendLen<0 || i_iPeerId<0)
    {
        printf("peerconnection_client :: post_sdp_to_peer NULL\r\n");
        return iRet;
    }
    memset(strURL,0,sizeof(strURL));
    snprintf(strURL,sizeof(strURL),"/message?peer_id=%d&to=%d",m_iMyId,i_iPeerId);
    if(0==m_pHttpClient->Send(HTTP_METHOD_POST,strURL,i_acSendBuf,i_iSendLen,HTTP_CONTENT_TYPE_TEXT))
    {
        memset(acRecvBuf,0,sizeof(acRecvBuf));
        if(0==m_pHttpClient->RecvBody(acRecvBuf,&iRecvLen,sizeof(acRecvBuf)))
        {
            iRet = 0;
        }
    }
    return iRet;
}
















