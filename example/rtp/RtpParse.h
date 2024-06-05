/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpParse.h
* Description		: 	RtpParse operation center
                        包含多种Rtp打包载荷类型，比如NALU,FU-A等载荷类型
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

#define RTP_AUDIO_FRAME_MAX_LEN 			(20*1024)
#define RTP_VIDEO_FRAME_MAX_LEN 			(2*1024*1024)

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
    int Init(T_RtpPacketTypeInfos i_tPacketTypeInfos);
    virtual int Parse(unsigned char *i_pbPacketBuf,int i_iPacketLen,T_RtpPacketParam *o_ptParam,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen);

protected:
	T_RtpPacketTypeInfos m_tPacketTypeInfos;
private:
    int ParseRtpHeader(unsigned char *i_pbRtpBuf,int i_iBufLen,T_RtpPacketParam *o_ptParam,int *o_iPadding,int *o_iMark);
    int G711Parse(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen);
    int H264Parse(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen);
    int H264ParseSTAP_A(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen);
    int H264ParseFU_A(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen);
    int H264ParseSingleNalu(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen);

    unsigned char *m_pAudioFrameBuf;
    int m_iAudioFrameCurLen;
    unsigned char *m_pVideoFrameBuf;
    int m_iVideoFrameCurLen;
    int m_iFrameFlag;
};














#endif
