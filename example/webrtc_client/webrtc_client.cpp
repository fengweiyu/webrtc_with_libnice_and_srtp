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

#include "webrtc_client.h"
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
WebRtcClient :: WebRtcClient()
{
    m_pTcpClient = new TcpClient();

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
WebRtcClient :: ~WebRtcClient()
{
    if(NULL != m_pTcpClient)
        delete m_pTcpClient;
}

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
WebRtcClientOffer :: WebRtcClientOffer()
{


}

/*****************************************************************************
-Fuction		: ~peerconnection_client_offer
-Description	: ~peerconnection_client_offer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
WebRtcClientOffer :: ~WebRtcClientOffer()
{

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
int WebRtcClientOffer :: Login(char * i_strServerIp,int i_iServerPort)
{
    int iRet = -1;
    string strIP;
    
    if(NULL==i_strServerIp || i_iServerPort<0)
    {
        printf("WebRtcClientOffer :: login NULL\r\n");
        return iRet;
    }
    strIP.assign(i_strServerIp);
    iRet=m_pTcpClient->Init(&strIP,i_iServerPort);
    return iRet;
}

/*****************************************************************************
-Fuction		: peerconnection_client_answer
-Description	: SendMsg
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int WebRtcClientOffer :: PostSdpMsg(char * i_strSdp)
{
    int iRet = -1;
    iRet=m_pTcpClient->Send(i_strSdp,strlen(i_strSdp));

    return iRet;
}


/*****************************************************************************
-Fuction		: peerconnection_client_answer
-Description	: GetMsg
-Input			: 
-Output 		: 
-Return 		: -1，表示没收到offer 或者收到的不是offer，其他表示peer id
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int WebRtcClientOffer :: GetMsg()
{
    int iRet = -1;
    char acRecvBuf[3*2048];
    int iRecvLen=0;
    char *pCandidate=NULL;
    char acSdpBuf[3*2048];
    
    memset(acRecvBuf,0,sizeof(acRecvBuf));
    if(0==m_pTcpClient->Recv(acRecvBuf,&iRecvLen,sizeof(acRecvBuf)))
    {
        pCandidate = strstr(acRecvBuf,"candidate");
        if(NULL != pCandidate)
        {
            m_Candidate.assign(pCandidate);
            printf("recv candidate:%s\r\n",m_Candidate.c_str());
            if(pCandidate-acRecvBuf>0)
            {
                memset(acSdpBuf,0,sizeof(acSdpBuf));
                memcpy(acSdpBuf,acRecvBuf,pCandidate-acRecvBuf);
                m_SDP.assign(acSdpBuf);
                printf("recv m_SDP:%s\r\n",m_SDP.c_str());
            }
        }
        else
        {
            m_SDP.assign(acRecvBuf);
            printf("recv m_SDP1:%s\r\n",m_SDP.c_str());
        }
    } 
    if(m_Candidate.length()>0 && m_SDP.length()>0)
    {
        iRet =0;
    }
    return iRet;
}



/*****************************************************************************
-Fuction		: peerconnection_client_answer
-Description	: GetMsg
-Input			: 
-Output 		: 
-Return 		: -1，表示没收到offer 或者收到的不是offer，其他表示peer id
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int WebRtcClientOffer :: GetSdpMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)
{
    int iRet = -1;


    memcpy(o_acRecvBuf,m_SDP.c_str(),m_SDP.length());
    
    return iRet;
}



/*****************************************************************************
-Fuction		: peerconnection_client_answer
-Description	: GetMsg
-Input			: 
-Output 		: 
-Return 		: -1，表示没收到offer 或者收到的不是offer，其他表示peer id
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int WebRtcClientOffer :: GetCandidateMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)
{
    int iRet = -1;


    memcpy(o_acRecvBuf,m_Candidate.c_str(),m_Candidate.length());
    
    return iRet;
}


