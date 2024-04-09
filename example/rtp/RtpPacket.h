/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpPacket.h
* Description		: 	RtpPacket operation center
                        包含多种Rtp打包载荷类型，比如NALU,FU-A等载荷类型
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef RTP_PACKET_H
#define RTP_PACKET_H

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "RtpCommon.h"


/*****************************************************************************
-Class			: RtpPacket
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpPacket
{
public:
    RtpPacket();
    virtual ~RtpPacket();
    int Init(unsigned char **m_ppPackets,int i_iMaxPacketNum);
    int DeInit(unsigned char **m_ppPackets,int i_iMaxPacketNum);
    int GenerateRtpHeader(T_RtpPacketParam *i_ptParam,T_RtpHeader *o_ptRtpHeader);
    int GenerateRtpHeader(T_RtpPacketParam *i_ptParam,int i_iPaddingLen,int i_iMark,unsigned char *o_bRtpHeader);
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);

protected:
	int m_iRtpType;
    int m_iMaxPacketNum;
private:
    RtpPacket *m_pRtpPacket;
};


/*****************************************************************************
-Class			: RtpPacketH264
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpPacketH264 : public RtpPacket
{
public:
    RtpPacketH264();
    virtual ~RtpPacketH264();
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);

protected:
	int m_iRtpVideoType;

private:
    RtpPacketH264 *m_pRtpPacketNALU;
    RtpPacketH264 *m_pRtpPacketFU_A;
};

/*****************************************************************************
-Class			: NALU
-Description	: NALU载荷类型的RTP包
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class H264NALU : public RtpPacketH264
{
public:
    H264NALU();
    virtual ~H264NALU();
    int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);

};

/*****************************************************************************
-Class			: FU_A
-Description	: FU_A载荷类型的RTP包
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class H264FU_A : public RtpPacketH264
{
public:
    H264FU_A();
    virtual ~H264FU_A();
    int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);
    static const unsigned char FU_A_TYPE;
    static const unsigned char FU_A_HEADER_LEN;
};


/*****************************************************************************
-Class			: RtpPacketH264
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpPacketH265 : public RtpPacket
{
public:
    RtpPacketH265();
    virtual ~RtpPacketH265();
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);

protected:
	int m_iRtpVideoType;

private:
    RtpPacketH265 *m_pRtpPacketNALU;
    RtpPacketH265 *m_pRtpPacketFU_A;
};

/*****************************************************************************
-Class			: NALU
-Description	: NALU载荷类型的RTP包
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class H265NALU : public RtpPacketH265
{
public:
    H265NALU();
    virtual ~H265NALU();
    int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);

};

/*****************************************************************************
-Class			: FU_A
-Description	: FU_A载荷类型的RTP包
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class H265FU_A : public RtpPacketH265
{
public:
    H265FU_A();
    virtual ~H265FU_A();
    int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);
    static const unsigned char FU_A_TYPE;
    static const unsigned char FU_A_HEADER_LEN;
};


/*****************************************************************************
-Class			: RtpPacketG711
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpPacketG711 : public RtpPacket
{
public:
    RtpPacketG711();
    virtual ~RtpPacketG711();
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);
private:
    RtpPacketG711 *m_pRtpPacketG711;
};


/*****************************************************************************
-Class			: RtpPacketG726
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpPacketG726 : public RtpPacket
{
public:
    RtpPacketG726();
    virtual ~RtpPacketG726();
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);
private:
    RtpPacketG726 *m_pRtpPacketG726;
};


/*****************************************************************************
-Class			: RtpPacketAAC
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpPacketAAC : public RtpPacket
{
public:
    RtpPacketAAC();
    virtual ~RtpPacketAAC();
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType=0);
private:
    RtpPacketAAC *m_pRtpPacketAAC;
};














#endif
