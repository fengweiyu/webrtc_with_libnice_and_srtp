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

#include "RtpPacket.h"
#include "RtpSession.h"
#include "MediaHandle.h"

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
	Rtp(unsigned char **m_ppPackets,int i_iMaxPacketNum,char *i_strPath);
    Rtp(char *i_strPath);
	Rtp();
	~Rtp();
    int Init(unsigned char **m_ppPackets,int i_iMaxPacketNum,char *i_strPath);
    int DeInit(unsigned char **m_ppPackets,int i_iMaxPacketNum);
    int GetSPS_PPS(unsigned char *o_pbSpsBuf,int *o_piSpsBufLen,unsigned char *o_pbPpsBuf,int *o_piPpsBufLen);
    int GetRtpData(unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum);
    unsigned int GetSSRC();

    int GetRtpPackets(T_MediaFrameInfo *m_ptFrame,unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum);
private:
    MediaHandle             *m_pMediaHandle;
	RtpPacket               m_RtpPacket;
	RtpPacket               m_AudioRtpPacket;
    T_MediaFrameParam       *m_ptMediaFrameParam;
    RtpSession              *m_pVideoRtpSession;
    
    unsigned int 	        m_dwLastTimestamp;//Á÷¿ØÐèÒª
};


#endif
