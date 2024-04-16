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
#include "RtpParse.h"
#include "RtpSession.h"
#include "MediaHandle.h"
#include "rtp_adapter.h"

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
    int GetSSRC(unsigned int *o_pdwVideoSSRC,unsigned int *o_pdwAudioSSRC);
    
    int SetRtpTypeInfo(T_RtpMediaInfo *i_ptRtpMediaInfo);
    int GetFrame(T_MediaFrameInfo *m_ptFrame);
    int GetRtpPackets(T_MediaFrameInfo *m_ptFrame,unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum);
    int ParseRtpPacket(unsigned char *i_pbPacketBuf,int i_iPacketLen,T_MediaFrameInfo *o_ptFrame);
    int IsRtp(char *buf, int size);
    int IsRtcp(char *buf, int size);
private:
    MediaHandle             *m_pMediaHandle;
	RtpPacket               m_VideoRtpPacket;//m_RtpPacket
	RtpPacket               m_AudioRtpPacket;
    T_MediaFrameParam       *m_ptMediaFrameParam;
    RtpSession              *m_pVideoRtpSession;
    RtpSession              *m_pAudioRtpSession;
    
	RtpParse                m_RtpParse;
	
    unsigned int 	        m_dwLastTimestamp;//Á÷¿ØÐèÒª
};


#endif
