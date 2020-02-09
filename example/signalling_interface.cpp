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

/*****************************************************************************
-Fuction		: signalling_interface
-Description	: signalling_interface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
signalling_interface :: signalling_interface()
{
    m_PeerConnectionClient = new peerconnection_client();
    m_iLoginSuccessFlag = 0;


}

/*****************************************************************************
-Fuction		: ~signalling_interface
-Description	: ~signalling_interface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
signalling_interface :: ~signalling_interface()
{
    if(NULL != m_PeerConnectionClient)
        delete m_PeerConnectionClient;
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
int signalling_interface :: GetOfferMsg(char * i_strServerIp, int i_iServerPort, char * i_strSelfName,char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)
{
    int iRet = -1;
    char *pOfferMsg = NULL;
    const char * strOfferMsgFlag = "\"type\" : \"offer\"";

    if(m_iLoginSuccessFlag !=1)
    {
        if(m_PeerConnectionClient->login(i_strServerIp, i_iServerPort,i_strSelfName)>=0)
        {
            m_iLoginSuccessFlag = 1;
            iRet = m_PeerConnectionClient->get_peer_sdp(o_acRecvBuf, o_piRecvLen, i_iRecvBufMaxLen);
            if(iRet >= 0)
            {
                pOfferMsg = strstr(o_acRecvBuf,strOfferMsgFlag);
                if(NULL == pOfferMsg)
                {
                    iRet = -1;
                }
            }
        }
        else
        {
            m_iLoginSuccessFlag = 0;
        }
    }
    else
    {
        iRet = m_PeerConnectionClient->get_peer_sdp(o_acRecvBuf, o_piRecvLen, i_iRecvBufMaxLen);
        if(iRet >= 0)
        {
            pOfferMsg = strstr(o_acRecvBuf,strOfferMsgFlag);
            if(NULL == pOfferMsg)
            {
                iRet = -1;
            }
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
int signalling_interface :: SendAnswerMsg(int i_iPeerId, char * i_acSendBuf, int i_iSendLen)
{
    int iRet = -1;
    iRet = m_PeerConnectionClient->post_sdp_to_peer(i_iPeerId, i_acSendBuf, i_iSendLen);
    return iRet;
}




