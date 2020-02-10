/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       rtp_interface.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "rtp_interface.h"

/*****************************************************************************
-Fuction		: RtpInterface
-Description	: RtpInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpInterface :: RtpInterface(char * i_strPath)
{
    m_pRtp = new Rtp();
    m_pRtp->Init(i_strPath);
}

/*****************************************************************************
-Fuction		: ~RtpInterface
-Description	: ~RtpInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpInterface :: ~RtpInterface()
{
    if(NULL !=m_pRtp)
    {

        delete m_pRtp;
    }
}

/*****************************************************************************
-Fuction		: GetRtpData
-Description	: GetRtpData
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,其他表示rtp包个数
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface :: GetRtpData(unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iEveryRtpBufMaxSize,int i_iPacketBufMaxNum)
{
    return m_pRtp->GetRtpData(o_ppbPacketBuf,o_aiEveryPacketLen,i_iEveryRtpBufMaxSize,i_iPacketBufMaxNum);
}



