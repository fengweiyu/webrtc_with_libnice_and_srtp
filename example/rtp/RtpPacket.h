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


using std::string;

#define IP_MAX_LEN 				(40)
#define RTP_MAX_PACKET_SIZE	((1500-IP_MAX_LEN)/4*4)//MTU (1514-54 ��̫��֡���1514-macͷ14-ipͷ20-tcpͷ20)
#define RTP_MAX_PACKET_NUM	(300)//m_iMaxPacketNum
#define RTP_HEADER_LEN 			(12)


#define RTP_PAYLOAD_H264    96
#define RTP_PAYLOAD_H265    97
#define RTP_PAYLOAD_G711    104
#define RTP_PAYLOAD_VIDEO    106 //�����Ƕ�̬�ģ�������ֱ��ʹ��,webrtc baseline106
#define RTP_PAYLOAD_AUDIO    104 //��������ӳ���

typedef enum
{
	RTP_PACKET_TYPE_H264 = 0,
    RTP_PACKET_TYPE_H265,
    RTP_PACKET_TYPE_G711U,
    RTP_PACKET_TYPE_G711A,
    RTP_PACKET_TYPE_G726,
    RTP_PACKET_TYPE_AAC
        
}E_RtpPacketType;



typedef struct RtpPacketParam
{
    unsigned int    dwSSRC;
    unsigned short  wSeq;
    unsigned int    dwTimestampFreq;
    unsigned int    wPayloadType;
    unsigned int    dwTimestamp;
}T_RtpPacketParam;//��Щ������ÿ��rtp�Ự�ж���һ������Ψһ�ġ�


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
-Description	: NALU�غ����͵�RTP��
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
-Description	: FU_A�غ����͵�RTP��
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
-Description	: NALU�غ����͵�RTP��
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
-Description	: FU_A�غ����͵�RTP��
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
