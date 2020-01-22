/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpPacket.h
* Description		: 	RtpPacket operation center
                        ��������Rtp����غ����ͣ�����NALU,FU-A���غ�����
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
#include "RtpSession.h"


using std::string;

#define RTP_MAX_PACKET_SIZE	((1500-42)/4*4)//MTU
#define RTP_MAX_PACKET_NUM	(300)



typedef struct RtpHeader
{
#ifdef RTP_BIG_ENDIAN
	unsigned short Version:2;//�汾�ţ�V����������־ʹ�õ�RTP�汾��ռ2λ����ǰЭ��汾��Ϊ2
	unsigned short Pad:1;//���λ��P��������־��ռ1λ�����P=1�����RTP����β���Ͱ������ӵ�����ֽ�
	unsigned short Extend:1;//��չλ��X������չ��־��ռ1λ�����X=1������RTP�̶�ͷ������͸���һ����չͷ��
	unsigned short CsrcCount:4;//CSRC��������CC����CSRC��������ռ4λ��ָʾ�̶�ͷ��������ŵ�CSRC ��ʶ���ĸ���

	unsigned short Mark:1;//���λ��M������ǣ�ռ1λ��һ����ԣ�������Ƶ�����һ֡�Ľ�����������Ƶ����ǻỰ�Ŀ�ʼ
	unsigned short PayloadType:7;//�غ����ͣ�PayloadType���� ��Ч�������ͣ�ռ7λ������˵��RTP��������Ч�غɵ�����
#else //little endian
	unsigned short CsrcCount:4;//CSRC��������CC����CSRC��������ռ4λ��ָʾ�̶�ͷ��������ŵ�CSRC ��ʶ���ĸ���
	unsigned short Extend:1;//��չλ��X������չ��־��ռ1λ�����X=1������RTP�̶�ͷ������͸���һ����չͷ��
	unsigned short Pad:1;//���λ��P��������־��ռ1λ�����P=1�����RTP����β���Ͱ������ӵ�����ֽ�
	unsigned short Version:2;//�汾�ţ�V����������־ʹ�õ�RTP�汾��ռ2λ����ǰЭ��汾��Ϊ2

	unsigned short PayloadType:7;//�غ����ͣ�PayloadType���� ��Ч�������ͣ�ռ7λ������˵��RTP��������Ч�غɵ�����
	unsigned short Mark:1;//���λ��M������ǣ�ռ1λ��һ����ԣ�������Ƶ�����һ֡�Ľ�����������Ƶ����ǻỰ�Ŀ�ʼ
#endif
	unsigned short wSeq;//���кţ�SN����ռ16λ�����ڱ�ʶ�����������͵�RTP���ĵ����кţ�ÿ����һ�����ģ����к���1
	unsigned int dwTimestamp;//ʱ���(Timestamp): ռ32λ����¼�˸ð������ݵĵ�һ���ֽڵĲ���ʱ��
	unsigned int dwSSRC;//ͬ��Դ��ʶ��(SSRC)��ռ32λ�����ڱ�ʶͬ����Դ��ͬ��Դ����ָRTP��������Դ����ͬһ��RTP�Ự�в�����������ͬ��SSRCֵ
}T_RtpHeader;//size 12


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
    ~RtpPacket();
    int GenerateRtpHeader(T_RtpPacketParam *i_ptParam,T_RtpHeader *o_ptRtpHeader);
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iVideoOrAudio=0);
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
    ~RtpPacketH264();
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iVideoOrAudio=0);
private:
    RtpPacketH264 *m_pRtpPacketH264;
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
    ~RtpPacketG711();
    virtual int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iVideoOrAudio=0);
private:
    RtpPacketG711 *m_pRtpPacketG711;
};

/*****************************************************************************
-Class			: NALU
-Description	: NALU�غ����͵�RTP��
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class NALU : public RtpPacketH264
{
public:
    NALU();
    ~NALU();
    int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iVideoOrAudio=0);

};

/*****************************************************************************
-Class			: FU_A
-Description	: FU_A�غ����͵�RTP��
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class FU_A : public RtpPacketH264
{
public:
    FU_A();
    ~FU_A();
    int Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iVideoOrAudio=0);
    static const unsigned char FU_A_TYPE;

};














#endif
