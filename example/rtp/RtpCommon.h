/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpCommon.h
* Description		: 	RtpPacket operation center
                        包含多种Rtp打包载荷类型，比如NALU,FU-A等载荷类型
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
#define RTP_MAX_PACKET_SIZE	(1460-SRTP_MAX_LEN)//((1500-IP_MAX_LEN)/4*4)//MTU (1514-54 以太网帧最大1514-mac头14-ip头20-tcp头20)
#define RTP_MAX_PACKET_NUM	(300)//m_iMaxPacketNum无法传递到底下的类里面,故暂不使用
#define RTP_HEADER_LEN 			(12)


#define RTP_PAYLOAD_H264    96
#define RTP_PAYLOAD_H265    97
#define RTP_PAYLOAD_G711A    8//a=rtpmap:8 PCMA/8000 webrtc
#define RTP_PAYLOAD_G711U    0//a=rtpmap:0 PCMU/8000 webrtc
#define RTP_PAYLOAD_OPUS   111//a=rtpmap:111 opus/48000/2 webrtc
#define RTP_PAYLOAD_VIDEO    106 //由于是动态的，所以先直接使用,webrtc baseline106
#define RTP_PAYLOAD_AUDIO    104 //后续做成映射表

typedef enum
{
	RTP_PACKET_TYPE_UNKNOW = 0,
	RTP_PACKET_TYPE_H264,
    RTP_PACKET_TYPE_H265,
    RTP_PACKET_TYPE_G711U,
    RTP_PACKET_TYPE_G711A,
    RTP_PACKET_TYPE_G726,
    RTP_PACKET_TYPE_AAC,
    
    RTP_PACKET_TYPE_MAX//无实际含义
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
    unsigned int    dwTimestamp;//时间戳的单位是1/VIDEO_H264_SAMPLE_RATE(s),频率的倒数
    E_RtpPacketType  ePacketType;   
}T_RtpPacketParam;//这些参数在每个rtp会话中都不一样，即唯一的。


typedef struct RtpHeader
{
#ifdef RTP_BIG_ENDIAN
	unsigned short Version:2;//版本号（V）：用来标志使用的RTP版本，占2位，当前协议版本号为2
	unsigned short Pad:1;//填充位（P）：填充标志，占1位，如果P=1，则该RTP包的尾部就包含附加的填充字节
	unsigned short Extend:1;//扩展位（X）：扩展标志，占1位，如果X=1，则在RTP固定头部后面就跟有一个扩展头部
	unsigned short CsrcCount:4;//CSRC计数器（CC）：CSRC计数器，占4位，指示固定头部后面跟着的CSRC 标识符的个数

	unsigned short Mark:1;//标记位（M）：标记，占1位，一般而言，对于视频，标记一帧的结束；对于音频，标记会话的开始
	unsigned short PayloadType:7;//载荷类型（PayloadType）： 有效荷载类型，占7位，用于说明RTP报文中有效载荷的类型
#else //little endian
	unsigned short CsrcCount:4;//CSRC计数器（CC）：CSRC计数器，占4位，指示固定头部后面跟着的CSRC 标识符的个数
	unsigned short Extend:1;//扩展位（X）：扩展标志，占1位，如果X=1，则在RTP固定头部后面就跟有一个扩展头部
	unsigned short Pad:1;//填充位（P）：填充标志，占1位，如果P=1，则该RTP包的尾部就包含附加的填充字节
	unsigned short Version:2;//版本号（V）：用来标志使用的RTP版本，占2位，当前协议版本号为2

	unsigned short PayloadType:7;//载荷类型（PayloadType）： 有效荷载类型，占7位，用于说明RTP报文中有效载荷的类型
	unsigned short Mark:1;//标记位（M）：标记，占1位，一般而言，对于视频，标记一帧的结束；对于音频，标记会话的开始
#endif
	unsigned short wSeq;//序列号（SN）：占16位，用于标识发送者所发送的RTP报文的序列号，每发送一个报文，序列号增1
	unsigned int dwTimestamp;//时间戳(Timestamp): 占32位，记录了该包中数据的第一个字节的采样时刻
	unsigned int dwSSRC;//同步源标识符(SSRC)：占32位，用于标识同步信源，同步源就是指RTP包流的来源。在同一个RTP会话中不能有两个相同的SSRC值
}T_RtpHeader;//size 12 不同平台sizeof(T_RtpHeader)大小可能不同,所以用RTP_HEADER_LEN代替











#endif
