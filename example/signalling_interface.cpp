/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       signalling_interface.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "signalling_interface.h"
#include <stdlib.h>//还是需要.h
#include <stdio.h>
#include <string.h>

/*****************************************************************************
-Fuction		: SignalingInterface
-Description	: SignalingInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
SignalingInterface :: SignalingInterface()
{
    m_PeerConnectionClient = new peerconnection_client();
    m_iLoginSuccessFlag = 0;


}

/*****************************************************************************
-Fuction		: ~SignalingInterface
-Description	: ~SignalingInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
SignalingInterface :: ~SignalingInterface()
{
    if(NULL != m_PeerConnectionClient)
        delete m_PeerConnectionClient;
}

/*****************************************************************************
-Fuction		: Login
-Description	: 对于没有login的信令服务器可以填空函数
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int SignalingInterface :: Login(char * i_strServerIp, int i_iServerPort, char * i_strSelfName)
{
    int iRet=-1;
    
    if(m_PeerConnectionClient->login(i_strServerIp, i_iServerPort,i_strSelfName)>=0)
    {
        iRet=0;
    }

    return iRet;
}


/*****************************************************************************
-Fuction		: signalling_interface
-Description	: GetOfferMsg
-Input			: 
-Output 		: 
-Return 		: -1，表示没收到offer 或者收到的不是offer，其他表示peer id
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int SignalingInterface :: GetOfferMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)
{
    int iRet = -1;
    char *pOfferMsg = NULL;
    const char * strOfferMsgFlag = "\"type\" : \"offer\"";

    iRet = m_PeerConnectionClient->get_peer_sdp(o_acRecvBuf, o_piRecvLen, i_iRecvBufMaxLen);
    if(iRet >= 0)
    {
        pOfferMsg = strstr(o_acRecvBuf,strOfferMsgFlag);
        if(NULL == pOfferMsg)
        {
            printf("m_PeerConnectionClient->GetOfferMsg err:%d\r\n",iRet);
            iRet = -1;
        }
    }
    return iRet;
}


/*****************************************************************************
-Fuction		: signalling_interface
-Description	: GetCandidateMsg
-Input			: 
-Output 		: 
-Return 		: -1，表示没收到offer 或者收到的不是offer，其他表示peer id
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int SignalingInterface :: GetCandidateMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)
{
    int iRet = -1;
    char *pCandidateMsg = NULL;
    const char * strCandidateMsgFlag = "\"candidate\" : \"candidate:";

    iRet = m_PeerConnectionClient->get_peer_sdp(o_acRecvBuf, o_piRecvLen, i_iRecvBufMaxLen);
    if(iRet >= 0)
    {
        pCandidateMsg = strstr(o_acRecvBuf,strCandidateMsgFlag);
        if(NULL == pCandidateMsg)
        {
            printf("m_PeerConnectionClient->GetCandidateMsg err:%d\r\n",iRet);
            iRet = -1;
        }
    }
    return iRet;
}


/*****************************************************************************
-Fuction		: signalling_interface
-Description	: SendAnswerMsg
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int SignalingInterface :: SendAnswerMsg(int i_iPeerId, char * i_acSendBuf, int i_iSendLen)
{
    int iRet = -1;
    iRet = m_PeerConnectionClient->post_sdp_to_peer(i_iPeerId, i_acSendBuf, i_iSendLen);
    return iRet;
}




