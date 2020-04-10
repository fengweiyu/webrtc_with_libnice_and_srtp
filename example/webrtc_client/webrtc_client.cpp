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

    if(NULL==i_strServerIp || i_iServerPort<0)
    {
        printf("WebRtcClientOffer :: login NULL\r\n");
        return iRet;
    }

    iRet=m_pTcpClient->Init(i_strServerIp,i_iServerPort);
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


    memset(acRecvBuf,0,sizeof(acRecvBuf));
    if(0==m_pTcpClient->Recv(acRecvBuf,&iRecvLen,sizeof(acRecvBuf)))
    {
        if(NULL != strstr(acRecvBuf,"candidate"))
        {
            m_Candidate.assign(acRecvBuf);
        }
        else
        {
            m_SDP.assign(acRecvBuf);
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


