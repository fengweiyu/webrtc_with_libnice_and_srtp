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
#ifndef RTP_INTERFACE_H
#define RTP_INTERFACE_H

#define RTP_PACKET_MAX_SIZE	1460//((1500-40)/4*4)//MTU (RTP_MAX_PACKET_SIZE)
#define RTP_PACKET_MAX_NUM	(300)//RTP_MAX_PACKET_NUM

/*****************************************************************************
-Class			: RtpInterface
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpInterface
{
public:
    RtpInterface(char * i_strPath);
	RtpInterface(unsigned char **m_ppPackets,int i_iMaxPacketNum,char *i_strPath);
	~RtpInterface();
    int DeInit(unsigned char **m_ppPackets,int i_iMaxPacketNum);
    int GetSPS_PPS(unsigned char *o_pbSpsBuf,int *o_piSpsBufLen,unsigned char *o_pbPpsBuf,int *o_piPpsBufLen);
    int RemoveH264EmulationBytes(unsigned char *o_pbNaluBuf,int i_iMaxNaluBufLen,unsigned char *i_pbNaluBuf,int i_iNaluLen);
    int GetRtpData(unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum);
    int GetSSRC(unsigned int *o_pdwVideoSSRC,unsigned int *o_pdwAudioSSRC);
    
    int GetRtpPackets(void *m_ptFrame,unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum);
    int IsRtp(char *buf, int size);
    int IsRtcp(char *buf, int size);
private:
	void               *m_pRtp;

    
};


#endif
