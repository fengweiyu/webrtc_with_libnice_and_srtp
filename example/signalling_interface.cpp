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
SignalingInterface :: SignalingInterface(int i_iIsAnswer)
{
    m_iLoginSuccessFlag = 0;
    if(0 == i_iIsAnswer)
    {
        m_PeerConnectionClient = new peerconnection_client_offer();
    }
    else
    {
        m_PeerConnectionClient = new peerconnection_client_answer();
    }

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
    return m_PeerConnectionClient->Login(i_strServerIp, i_iServerPort,i_strSelfName);
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
int SignalingInterface :: GetMsg(char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)
{
    return m_PeerConnectionClient->GetMsg(o_acRecvBuf, o_piRecvLen, i_iRecvBufMaxLen);
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
    return m_PeerConnectionClient->GetCandidateMsg(o_acRecvBuf, o_piRecvLen, i_iRecvBufMaxLen);
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
int SignalingInterface :: SendMsg(int i_iPeerId, char * i_acSendBuf, int i_iSendLen,char * o_acRecvBuf, int * o_piRecvLen, int i_iRecvBufMaxLen)
{
    return m_PeerConnectionClient->SendMsg(i_iPeerId, i_acSendBuf, i_iSendLen,o_acRecvBuf,o_piRecvLen,i_iRecvBufMaxLen);
}




