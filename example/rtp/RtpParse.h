/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpParse.h
* Description		: 	RtpParse operation center
                        ��������Rtp����غ����ͣ�����NALU,FU-A���غ�����
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef RTP_PARSE_H
#define RTP_PARSE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "RtpCommon.h"


/*****************************************************************************
-Class			: RtpParse
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpParse
{
public:
    RtpParse();
    virtual ~RtpParse();

    virtual int Parse(unsigned char *i_pbPacketBuf,int i_iPacketLen,T_RtpPacketParam *o_ptParam,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen);

protected:
	int m_iRtpType;
private:
    int ParseRtpHeader(unsigned char *i_pbRtpBuf,int i_iBufLen,T_RtpPacketParam *o_ptParam,int *o_iPaddingLen,int *o_iMark);
    int G711Parse(unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen);
};














#endif