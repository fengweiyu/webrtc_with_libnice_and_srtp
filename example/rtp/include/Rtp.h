/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Rtp.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef RTP_H
#define RTP_H

#include "VideoHandle.h"
#include "RtpPacket.h"

/*****************************************************************************
-Class			: RtpInterface
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class Rtp
{
public:
	Rtp();
	~Rtp();
    int Init(char *i_strPath);

	
    int GetRtpData(unsigned char *o_pbRtpBuf,int *o_iRtpBufSize,int i_iRtpBufMaxSize);


private:
    VideoHandle             *m_pVideoHandle;
	RtpPacket               *m_pRtpPacket;

    
};


#endif
