/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpCommon.h
* Description		: 	RtpPacket operation center
                        ��������Rtp����غ����ͣ�����NALU,FU-A���غ�����
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef RTP_COMMON_H
#define RTP_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string>


using std::string;

#define IP_MAX_LEN 				(40)
#define SRTP_MAX_LEN 				(128+16)
#define RTP_MAX_PACKET_SIZE	(1460-SRTP_MAX_LEN)//((1500-IP_MAX_LEN)/4*4)//MTU (1514-54 ��̫��֡���1514-macͷ14-ipͷ20-tcpͷ20)
#define RTP_MAX_PACKET_NUM	(300)//m_iMaxPacketNum�޷����ݵ����µ�������,���ݲ�ʹ��
#define RTP_HEADER_LEN 			(12)


#define RTP_PAYLOAD_H264    96
#define RTP_PAYLOAD_H265    97
#define RTP_PAYLOAD_G711A    8//a=rtpmap:8 PCMA/8000 webrtc
#define RTP_PAYLOAD_G711U    0//a=rtpmap:0 PCMU/8000 webrtc
#define RTP_PAYLOAD_OPUS   111//a=rtpmap:111 opus/48000/2 webrtc
#define RTP_PAYLOAD_VIDEO    106 //�����Ƕ�̬�ģ�������ֱ��ʹ��,webrtc baseline106
#define RTP_PAYLOAD_AUDIO    104 //��������ӳ���

typedef enum
{
	RTP_PACKET_TYPE_UNKNOW = 0,
	RTP_PACKET_TYPE_H264,
    RTP_PACKET_TYPE_H265,
    RTP_PACKET_TYPE_G711U,
    RTP_PACKET_TYPE_G711A,
    RTP_PACKET_TYPE_G726,
    RTP_PACKET_TYPE_AAC,
    
    RTP_PACKET_TYPE_MAX//��ʵ�ʺ���
}E_RtpPacketType;

typedef struct RtpPacketTypeInfo
{
    int iPayload;
	E_RtpPacketType ePacketType;
}T_RtpPacketTypeInfo;
typedef struct RtpPacketTypeInfos
{
	T_RtpPacketTypeInfo atTypeInfos[RTP_PACKET_TYPE_MAX];
}T_RtpPacketTypeInfos;

typedef struct RtpPacketParam
{
    unsigned int    dwSSRC;
    unsigned short  wSeq;
    unsigned int    dwTimestampFreq;
    unsigned int    wPayloadType;
    unsigned int    dwTimestamp;//ʱ����ĵ�λ��1/VIDEO_H264_SAMPLE_RATE(s),Ƶ�ʵĵ���
    E_RtpPacketType  ePacketType;   
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
}T_RtpHeader;//size 12 ��ͬƽ̨sizeof(T_RtpHeader)��С���ܲ�ͬ,������RTP_HEADER_LEN����











#endif
