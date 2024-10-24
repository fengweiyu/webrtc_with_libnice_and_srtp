/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpParse.cpp
* Description		: 	RtpParse operation center
                        包含多种Rtp打包载荷类型，比如NALU,FU-A等载荷类型
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include <stdlib.h>//还是需要.h
#include <stdio.h>
#include <string.h>
#include <iostream>//不加.h,c++新的头文件

#include "Definition.h"
#include "RtpParse.h"
#include "rtp_adapter.h"

#ifndef TRUE
#define TRUE 0
#endif
#ifndef FALSE
#define FALSE -1
#endif


using std::cout;//需要<iostream>
using std::endl;

#define RTP_V(v)	((v >> 30) & 0x03) /* protocol version */
#define RTP_P(v)	((v >> 29) & 0x01) /* padding flag */
#define RTP_X(v)	((v >> 28) & 0x01) /* header extension flag */
#define RTP_CC(v)	((v >> 24) & 0x0F) /* CSRC count */
#define RTP_M(v)	((v >> 23) & 0x01) /* marker bit */
#define RTP_PT(v)	((v >> 16) & 0x7F) /* payload type */
#define RTP_SEQ(v)	((v >> 00) & 0xFFFF) /* sequence number */
#define  Read32BE(ptr,val)     *val = ((unsigned char)ptr[0] << 24) | ((unsigned char)ptr[1] << 16) | ((unsigned char)ptr[2] << 8) | (unsigned char)ptr[3]

/*****************************************************************************
-Fuction		: RtpParse
-Description	: RtpParse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpParse :: RtpParse()
{
    memset(&m_tPacketTypeInfos,0,sizeof(T_RtpPacketTypeInfos));
    m_pAudioFrameBuf = new unsigned char [RTP_AUDIO_FRAME_MAX_LEN];
    memset(m_pAudioFrameBuf,0,RTP_AUDIO_FRAME_MAX_LEN);
    m_iAudioFrameCurLen=0;
    m_pVideoFrameBuf = new unsigned char [RTP_VIDEO_FRAME_MAX_LEN];
    memset(m_pVideoFrameBuf,0,RTP_VIDEO_FRAME_MAX_LEN);
    m_iVideoFrameCurLen=0;
    m_iFrameFlag = 0;
    m_iFrameStartFlag = 0;
    memset(&m_tParseInfos,0,sizeof(T_RtpParseInfos));
    m_iVideoLostPacketFlag = 0;
    m_iAudioLostPacketFlag = 0;
}

/*****************************************************************************
-Fuction		: RtpParse
-Description	: RtpParse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpParse :: ~RtpParse()
{
    if(NULL!= m_pAudioFrameBuf)
    {
        delete[] m_pAudioFrameBuf;
        m_pAudioFrameBuf = NULL;//
    }
    if(NULL!= m_pVideoFrameBuf)
    {
        delete[] m_pVideoFrameBuf;
        m_pVideoFrameBuf = NULL;//
    }
}

/*****************************************************************************
-Fuction		: RtpParse
-Description	: RtpParse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse :: Init(T_RtpPacketTypeInfos i_tPacketTypeInfos)
{
    memcpy(&m_tPacketTypeInfos,&i_tPacketTypeInfos,sizeof(T_RtpPacketTypeInfos));
    return TRUE;
}

/*****************************************************************************
-Fuction		: GenerateRtpHeader
-Description	: GenerateRtpHeader
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse :: ParseRtpHeader(unsigned char *i_pbRtpBuf,int i_iBufLen,T_RtpPacketParam *o_ptParam,int *o_iPadding,int *o_iMark)
{
    int iRet=FALSE;
    unsigned int dwHeader = 0;
    unsigned char *pbRtpBuf= i_pbRtpBuf;
    unsigned char bRtpV= 0;
    unsigned char bRtpX= 0;
    unsigned char bRtpCC= 0;
    unsigned char bRtpPT= 0;
    
    if(NULL == i_pbRtpBuf || NULL == o_ptParam || NULL == o_iPadding || NULL == o_iMark ||i_iBufLen < RTP_HEADER_LEN)
    {
        printf("GenerateRtpHeader err NULL i_bRtpBuf %p,o_ptParam %p i_iBufLen%d\r\n",i_pbRtpBuf,o_ptParam,i_iBufLen);
        return iRet;
    }
    
    Read32BE(pbRtpBuf,&dwHeader);
    bRtpV=RTP_V(dwHeader);
    *o_iPadding = RTP_P(dwHeader);
    bRtpX=RTP_X(dwHeader);
    bRtpCC=RTP_CC(dwHeader);
    *o_iMark = RTP_M(dwHeader);
    bRtpPT= RTP_PT(dwHeader);
    
    o_ptParam->wPayloadType = bRtpPT;
    o_ptParam->wSeq = RTP_SEQ(dwHeader);

    pbRtpBuf+=4;
    Read32BE(pbRtpBuf,&o_ptParam->dwTimestamp);
    pbRtpBuf+=4;
    Read32BE(pbRtpBuf,&o_ptParam->dwSSRC);
    
    iRet=TRUE;
    return iRet;
}

/*****************************************************************************
-Fuction		: Parse
-Description	: Parse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse :: Parse(unsigned char *i_pbPacketBuf,int i_iPacketLen,T_RtpPacketParam *o_ptParam,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iRet=FALSE;
    T_RtpPacketParam tParam;
    int iMark=0;
    int iPad=0;
    int i=0;
    E_RtpPacketType eRtpPacketType = RTP_PACKET_TYPE_UNKNOW;
    unsigned char *pbPacketBuf=NULL;
    int iPacketLen=0;
    int iPadLen=0;
    
	if (!i_pbPacketBuf || i_iPacketLen <= 0 || !o_ptParam|| !o_pbParsedData|| !o_iDataLen)
    {
        cout<<"Parse err NULL"<<endl;
        return iRet;
    }
    memset(&tParam,0,sizeof(T_RtpPacketParam));
    iRet=this->ParseRtpHeader(i_pbPacketBuf,i_iPacketLen,&tParam,&iPad,&iMark);
    if(FALSE == iRet)
    {
        cout<<"i_iRtpPacketType ParseRtpHeader err "<<i_iPacketLen<<endl;
        return iRet;
    }
    pbPacketBuf=i_pbPacketBuf+RTP_HEADER_LEN;
    iPacketLen=i_iPacketLen-RTP_HEADER_LEN;
    if(iPad > 0)
    {
        iPadLen=i_pbPacketBuf[i_iPacketLen-1];
        iPacketLen-=iPadLen;
    }
    
    for(i=0;i<RTP_PACKET_TYPE_MAX;i++)
    {
        if(tParam.wPayloadType == m_tPacketTypeInfos.atTypeInfos[i].iPayload)
        {
            eRtpPacketType = m_tPacketTypeInfos.atTypeInfos[i].ePacketType;
            break;
        }
    }
    tParam.ePacketType=eRtpPacketType;
    
    for(i=0;i<RTP_PACKET_TYPE_MAX;i++)
    {
        if(eRtpPacketType == m_tParseInfos.atParseInfos[i].ePacketType)
        {
            if(tParam.wSeq != m_tParseInfos.atParseInfos[i].wSeq+1)
            {
                RTP_LOGW("eRtpPacketType%d,tParam.wSeq %d!= m_tParseInfos.atParseInfos[i].wSeq%d+1 \r\n",eRtpPacketType,tParam.wSeq,m_tParseInfos.atParseInfos[i].wSeq);
                if(eRtpPacketType == RTP_PACKET_TYPE_H264 || eRtpPacketType == RTP_PACKET_TYPE_H265)
                {
                    m_iVideoLostPacketFlag=1;
                }
                if(eRtpPacketType == RTP_PACKET_TYPE_G711A || eRtpPacketType == RTP_PACKET_TYPE_G711U)
                {
                    m_iAudioLostPacketFlag=1;
                }
            }
            m_tParseInfos.atParseInfos[i].wSeq=tParam.wSeq;
            break;
        }
    }
    if(i>=RTP_PACKET_TYPE_MAX)
    {
        for(i=0;i<RTP_PACKET_TYPE_MAX;i++)
        {
            if(RTP_PACKET_TYPE_UNKNOW == m_tParseInfos.atParseInfos[i].ePacketType)
            {
                m_tParseInfos.atParseInfos[i].ePacketType=eRtpPacketType;
                m_tParseInfos.atParseInfos[i].wSeq=tParam.wSeq;
                break;
            }
        }
    }
    printf("tParam.wSeq %d,wPayloadType%d,ePacketType%d,type h264 %d /h265 %d,iMark %d iPad %d,%#x,%#x \r\n",tParam.wSeq,tParam.wPayloadType,tParam.ePacketType,
    pbPacketBuf[0]&0x1F,((pbPacketBuf[0]) >> 1) & 0x3f,iMark,iPad,pbPacketBuf[0],pbPacketBuf[1]);
    switch (tParam.ePacketType)
    {
        case RTP_PACKET_TYPE_G711U:
        case RTP_PACKET_TYPE_G711A:
        {
            iRet=G711Parse(iMark,pbPacketBuf,iPacketLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            if(*o_iDataLen>0 && m_iAudioLostPacketFlag != 0)
            {
                *o_iDataLen=0;
                printf("m_iAudioLostPacketFlag%d != 0 ,drop audio frame\r\n",m_iAudioLostPacketFlag);
                m_iAudioLostPacketFlag=0;
            }
            break;
        }
        case RTP_PACKET_TYPE_H264:
        {
            iRet=H264Parse(iMark,pbPacketBuf,iPacketLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            if(*o_iDataLen>0 && m_iVideoLostPacketFlag != 0)
            {
                *o_iDataLen=0;
                printf("m_iVideoLostPacketFlag%d != 0 ,drop video frame\r\n",m_iVideoLostPacketFlag);
                m_iVideoLostPacketFlag=0;
            }
            break;
        }
        case RTP_PACKET_TYPE_H265:
        {
            iRet=H265Parse(iMark,pbPacketBuf,iPacketLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            if(*o_iDataLen>0 && m_iVideoLostPacketFlag != 0)
            {
                *o_iDataLen=0;
                printf("H265Parse m_iVideoLostPacketFlag%d != 0 ,drop video frame\r\n",m_iVideoLostPacketFlag);
                m_iVideoLostPacketFlag=0;
            }
            break;
        }
        default :
        {
            printf("tParam.ePacketType err %d %d\r\n",tParam.ePacketType,tParam.wPayloadType);
            break;
        }
    }
    if(FALSE != iRet)
    {
        memcpy(o_ptParam,&tParam,sizeof(T_RtpPacketParam));
    }
    return iRet;
}
/*****************************************************************************
-Fuction		: G711Parse
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::G711Parse(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen)
    {
        printf("G711Parse err %d< %d \r\n",i_iDataMaxLen,i_iBufLen);
        return FALSE;
    }
    //if(1 == i_iMark)
    {//会话开始
        //memcpy(m_pAudioFrameBuf,i_pbRtpBodyBuf,i_iBufLen);
        //m_iAudioFrameCurLen=i_iBufLen;
    }
    //else
    {
        memcpy(m_pAudioFrameBuf+m_iAudioFrameCurLen,i_pbRtpBodyBuf,i_iBufLen);
        m_iAudioFrameCurLen+=i_iBufLen;
        memcpy(o_pbParsedData,m_pAudioFrameBuf,m_iAudioFrameCurLen);
        *o_iDataLen = m_iAudioFrameCurLen;
        m_iAudioFrameCurLen=0;
    }
    return TRUE;
}
/*****************************************************************************
-Fuction		: H264Parse
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::H264Parse(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iRet = FALSE;
    unsigned char bPacketType=0;


	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen)
    {
        printf("H264Parse err %d< %d \r\n",i_iDataMaxLen,i_iBufLen);
        return iRet;
    }

    bPacketType=i_pbRtpBodyBuf[0] & 0x1F;
	switch(bPacketType)
	{
    	case 0: // reserved
    	case 31: // reserved
    	{
            printf("bPacketType reserved %d ,%d\r\n",bPacketType,i_iBufLen);
            iRet = TRUE;// packet discard
            break;
    	}
    	case 24: // STAP-A
        {
            iRet=H264ParseSTAP_A(i_iMark,i_pbRtpBodyBuf,i_iBufLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            break;
        }
    	case 25: // STAP-B
    	case 26: // MTAP16
    	case 27: // MTAP24
        case 29: // FU-B
        {
            printf("bPacketType unsupport %d ,%d\r\n",bPacketType,i_iBufLen);
            break;
        }
    	case 28: // FU-A
    	{
            iRet=H264ParseFU_A(i_iMark,i_pbRtpBodyBuf,i_iBufLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            break;
    	}
    	default: // 1-23 NAL unit
        {
            iRet=H264ParseSingleNalu(i_iMark,i_pbRtpBodyBuf,i_iBufLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            break;
        }
	}
    return iRet;
}
/*****************************************************************************
-Fuction		: H265Parse
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::H265Parse(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iRet = FALSE;
    unsigned char bPacketType=0;


	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen)
    {
        printf("H265Parse err %d< %d \r\n",i_iDataMaxLen,i_iBufLen);
        return iRet;
    }

    bPacketType=((i_pbRtpBodyBuf[0]) >> 1) & 0x3f;
	if (bPacketType > 50)
    {
        printf("bPacketType unsupport %d ,%d\r\n",bPacketType,i_iBufLen);
        return iRet;// packet discard, Unsupported (HEVC) NAL type
    }
    //printf("H265Parse bPacketType %d \r\n",bPacketType);
	switch(bPacketType)
	{
        case 48: // aggregated packet (AP) - with two or more NAL units
        {
            iRet=H265ParseAP(i_iMark,i_pbRtpBodyBuf,i_iBufLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            break;
        }
        case 49: // fragmentation unit (FU)
    	{
            iRet=H265ParseFU(i_iMark,i_pbRtpBodyBuf,i_iBufLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            break;
    	}
        case 32: // video parameter set (VPS)
        case 33: // sequence parameter set (SPS)
        case 34: // picture parameter set (PPS)
        case 39: // supplemental enhancement information (SEI)
        default: // 4.4.1. Single NAL Unit Packets (p24)
        {
            iRet=H265ParseSingleNalu(i_iMark,i_pbRtpBodyBuf,i_iBufLen,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            break;
        }
	}
    return iRet;
}

/*****************************************************************************
-Fuction		: H264ParseSTAP_A
-Description	: 
// 5.7.1. Single-Time Aggregation Packet (STAP) (p23)
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           RTP Header                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|STAP-B NAL HDR |            DON                |  NALU 1 Size  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| NALU 1 Size   | NALU 1 HDR    |         NALU 1 Data           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
:                                                               :
+               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|               | NALU 2 Size                   |   NALU 2 HDR  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            NALU 2 Data                        |
:                                                               :
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :    ...OPTIONAL RTP padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::H264ParseSTAP_A(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iFrameFlag=0;
    unsigned char bStapA=0;
    int iNaluLen=0;
    unsigned char *pbRtpPacket=NULL;
    int iBufLen=0;
    
	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen)
    {
        printf("H264ParseSTAP_A err %d< %d \r\n",i_iDataMaxLen,i_iBufLen);//i_iMark 0
        return FALSE;
    }
    pbRtpPacket=i_pbRtpBodyBuf;
    bStapA=pbRtpPacket[0];//STAP-A头，占用1个字节
    pbRtpPacket++;
    iBufLen=i_iBufLen-1;
    while(iBufLen>2)
    {
        iNaluLen = (pbRtpPacket[0] << 8) | pbRtpPacket[1];//NALU长度 (占用两个字节)
        pbRtpPacket+=2;
        switch(pbRtpPacket[0] & 0x1f)
        {
            case 0x5:
            case 0x1:
                iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
                break;
            default:
                break;
        }
        memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
        m_iVideoFrameCurLen += 3;
        m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
        m_iVideoFrameCurLen++;//add 00 00 00 01
        memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,pbRtpPacket,iNaluLen);
        m_iVideoFrameCurLen+=iNaluLen;
        pbRtpPacket+=iNaluLen;
        iBufLen=i_iBufLen-(pbRtpPacket-i_pbRtpBodyBuf);
    }

    if(0 != iFrameFlag)
    {
        memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
        *o_iDataLen = m_iVideoFrameCurLen;
        m_iVideoFrameCurLen=0;
    }
    return TRUE;
}

/*****************************************************************************
-Fuction		: H264ParseFU_A
-Description	: 
// 5.8. Fragmentation Units (FUs) (p29)
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  FU indicator |   FU header   |              DON              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
|                                                               |
|                          FU payload                           |
|                                                               |
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :   ...OPTIONAL RTP padding     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::H264ParseFU_A(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iFrameFlag=0;
    unsigned char bRtpFuHeader=0;

    
	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen || i_iBufLen<=2)
    {
        printf("H264ParseFU_A err %d< %d,i_iMark %d \r\n",i_iDataMaxLen,i_iBufLen,i_iMark);
        return FALSE;
    }

    
    if(0 == i_iMark)
    {
        bRtpFuHeader=i_pbRtpBodyBuf[1];
        if(bRtpFuHeader&0x80)
        {//分包开始
            memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
            m_iVideoFrameCurLen += 3;
            m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
            m_iVideoFrameCurLen++;//add 00 00 00 01
            m_pVideoFrameBuf[m_iVideoFrameCurLen]=(i_pbRtpBodyBuf[0]/*indicator*/ & 0xE0) | (bRtpFuHeader & 0x1F);
            switch(m_pVideoFrameBuf[m_iVideoFrameCurLen] & 0x1f)
            {
                case 0x5:
                case 0x1:
                    m_iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
                    break;
                default:
                    break;
            }
            m_iVideoFrameCurLen++;//add NAL unit type byte
            memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf+2,i_iBufLen-2);
            m_iVideoFrameCurLen+=i_iBufLen-2;
        }
        else if(bRtpFuHeader&0x40)
        {//分包结束
            memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf+2,i_iBufLen-2);
            m_iVideoFrameCurLen+=i_iBufLen-2;
            if(0 != m_iFrameFlag)
            {
                memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
                *o_iDataLen = m_iVideoFrameCurLen;
                m_iVideoFrameCurLen=0;
                m_iFrameFlag=0;
            }
        }
        else
        {
            memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf+2,i_iBufLen-2);
            m_iVideoFrameCurLen+=i_iBufLen-2;
        }
    }
    else
    {//分包结束
        memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf+2,i_iBufLen-2);
        m_iVideoFrameCurLen+=i_iBufLen-2;
        if(0 != m_iFrameFlag)
        {
            memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
            *o_iDataLen = m_iVideoFrameCurLen;
            m_iVideoFrameCurLen=0;
            m_iFrameFlag=0;
        }
    }

    return TRUE;
}

/*****************************************************************************
-Fuction		: H264ParseSingleNalu
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::H264ParseSingleNalu(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iFrameFlag=0;
	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen/*||!i_iMark*/)//webrtc客户端下发过来的会是0
    {
        printf("H264ParseSingleNalu err %d< %d,i_iMark %d \r\n",i_iDataMaxLen,i_iBufLen,i_iMark);//i_iMark 0
        return FALSE;
    }
    switch(i_pbRtpBodyBuf[0] & 0x1f)
    {
        case 0x5:
        case 0x1:
        {
            if(0 == m_iFrameStartFlag)
            {
                memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
                m_iVideoFrameCurLen += 3;
                m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
                m_iVideoFrameCurLen++;//add 00 00 00 01
                m_iFrameStartFlag=1;
            }
        }
        break;
        case 0x7:
        case 0x8:
        {
            memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
            m_iVideoFrameCurLen += 3;
            m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
            m_iVideoFrameCurLen++;//add 00 00 00 01
            break;
        }
        default:
            break;
    }
    memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf,i_iBufLen);
    m_iVideoFrameCurLen+=i_iBufLen;

    if(0 != i_iMark)
    {
        memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
        *o_iDataLen = m_iVideoFrameCurLen;
        m_iVideoFrameCurLen=0;
        m_iFrameStartFlag=0;
    }
    return TRUE;
    

	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen||!i_iMark)//webrtc客户端下发过来的会是0
    {
        printf("H264ParseSingleNalu err %d< %d,i_iMark %d \r\n",i_iDataMaxLen,i_iBufLen,i_iMark);//i_iMark 0
        return FALSE;
    }
    switch(i_pbRtpBodyBuf[0] & 0x1f)
    {
        case 0x5:
        case 0x1:
            iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
            break;
        default:
            break;
    }
    memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
    m_iVideoFrameCurLen += 3;
    m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
    m_iVideoFrameCurLen++;//add 00 00 00 01
    memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf,i_iBufLen);
    m_iVideoFrameCurLen+=i_iBufLen;

    if(0 != iFrameFlag)
    {
        memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
        *o_iDataLen = m_iVideoFrameCurLen;
        m_iVideoFrameCurLen=0;
    }
    return TRUE;
}

/*****************************************************************************
-Fuction		: H265ParseAP
-Description	: 在SDP中sprop-max-don-diff = 0时,DONL可以省略;当 0<sprop-max-don-diff<=32767时，DONL不能省略
// 4.4.2. Aggregation Packets (APs) (p25)
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          RTP Header                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      PayloadHdr (Type=48)     |           NALU 1 DONL         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           NALU 1 Size         |            NALU 1 HDR         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                         NALU 1 Data . . .                     |
|                                                               |
+     . . .     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|               |  NALU 2 DOND  |            NALU 2 Size        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          NALU 2 HDR           |                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+            NALU 2 Data        |
|                                                               |
|         . . .                 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :    ...OPTIONAL RTP padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::H265ParseAP(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iFrameFlag=0;
    unsigned char bStapA=0;
    int iNaluLen=0;
    unsigned char *pbRtpPacket=NULL;
    int iBufLen=0;
    unsigned char bNaluType = 0;//取nalu类型


	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen)
    {
        printf("H265ParseAP err %d< %d \r\n",i_iDataMaxLen,i_iBufLen);//i_iMark 0
        return FALSE;
    }
    pbRtpPacket=i_pbRtpBodyBuf+2;//AP头，占用2个字节 PayloadHdr
    iBufLen=i_iBufLen-2;
    while(iBufLen>2)
    {
        //pbRtpPacket+=2;// 实测没有DON
        //iBufLen-=2;//所以注释
        
        iNaluLen = (pbRtpPacket[0] << 8) | pbRtpPacket[1];//NALU长度 (占用两个字节)
        pbRtpPacket+=2;
        iBufLen-=2;
        bNaluType = (pbRtpPacket[0] & 0x7E)>>1;//取nalu类型
        //printf("H265ParseAP bPacketType %d %#x,%d,%d\r\n",bNaluType,pbRtpPacket[0],iNaluLen,iBufLen);
        if(bNaluType >= 0 && bNaluType <= 9)// p slice 片
        {
            iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
        }
        else if(bNaluType >= 16 && bNaluType <= 21)// IRAP 等同于i帧
        {
            iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
        }
        memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
        m_iVideoFrameCurLen += 3;
        m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
        m_iVideoFrameCurLen++;//add 00 00 00 01
        memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,pbRtpPacket,iNaluLen);
        m_iVideoFrameCurLen+=iNaluLen;
        pbRtpPacket+=iNaluLen;
        iBufLen-=iNaluLen;
    }

    if(0 != iFrameFlag && 0 != i_iMark)//i_iMark=1才结束，应对帧切片的情况，帧切片结束Mark=1
    {//从而让返回的帧数据包含一帧中的多个切片nalu
        memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
        *o_iDataLen = m_iVideoFrameCurLen;
        m_iVideoFrameCurLen=0;
    }
    return TRUE;
}

/*****************************************************************************
-Fuction		: H265ParseFU
-Description	: 
// 4.4.3. Fragmentation Units (p29)
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     PayloadHdr (Type=49)      |    FU header  |  DONL (cond)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
|  DONL (cond)  |                                               |
|-+-+-+-+-+-+-+-+                                               |
|                           FU payload                          |
|                                                               |
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :    ...OPTIONAL RTP padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|   FuType  |
+---------------+
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::H265ParseFU(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iFrameFlag=0;
    unsigned char bRtpFuHeader=0;
    unsigned char bNaluType=0;
    
	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen || i_iBufLen<=3)
    {
        printf("H265ParseFU err %d< %d,i_iMark %d \r\n",i_iDataMaxLen,i_iBufLen,i_iMark);
        return FALSE;
    }
    
    if(0 == i_iMark)
    {
        bRtpFuHeader=i_pbRtpBodyBuf[2];
        if(bRtpFuHeader&0x80)
        {//分包开始
            memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
            m_iVideoFrameCurLen += 3;
            m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
            m_iVideoFrameCurLen++;//add 00 00 00 01
            m_pVideoFrameBuf[m_iVideoFrameCurLen] = ((bRtpFuHeader & 0x3F) << 1) | (i_pbRtpBodyBuf[0] & 0x81); // replace NAL Unit Type Bits
            bNaluType=(m_pVideoFrameBuf[m_iVideoFrameCurLen] & 0x7E)>>1;
            //printf("H265ParseFU bPacketType %d ,%#x\r\n",bNaluType,m_pVideoFrameBuf[m_iVideoFrameCurLen]);
            if(bNaluType >= 0 && bNaluType <= 9)// p slice 片
            {
                m_iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
            }
            else if(bNaluType >= 16 && bNaluType <= 21)// IRAP 等同于i帧
            {
                m_iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
            }
            m_iVideoFrameCurLen++;//add NAL unit type byte
            m_pVideoFrameBuf[m_iVideoFrameCurLen] =i_pbRtpBodyBuf[1];
            m_iVideoFrameCurLen++;//add NAL unit type byte

            memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf+3,i_iBufLen-3);
            m_iVideoFrameCurLen+=i_iBufLen-3;
        }
        else if(bRtpFuHeader&0x40)
        {//分包结束
            memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf+3,i_iBufLen-3);
            m_iVideoFrameCurLen+=i_iBufLen-3;
            if(0 != m_iFrameFlag)
            {
                //memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
                //*o_iDataLen = m_iVideoFrameCurLen;//i_iMark=1才结束，应对帧切片的情况，帧切片结束Mark=1
                //m_iVideoFrameCurLen=0;//从而让返回的帧数据包含一帧中的多个切片nalu
                m_iFrameFlag=0;
            }
        }
        else
        {
            memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf+3,i_iBufLen-3);
            m_iVideoFrameCurLen+=i_iBufLen-3;
        }
    }
    else
    {//分包结束
        memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf+3,i_iBufLen-3);
        m_iVideoFrameCurLen+=i_iBufLen-3;
        if(0 != m_iFrameFlag)
        {
            memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
            *o_iDataLen = m_iVideoFrameCurLen;
            m_iVideoFrameCurLen=0;
            m_iFrameFlag=0;
        }
    }

    return TRUE;
}

/*****************************************************************************
-Fuction		: H265ParseSingleNalu
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::H265ParseSingleNalu(int i_iMark,unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iFrameFlag=0;

    
	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen/*||!i_iMark*/)//webrtc客户端下发过来的会是0
    {
        printf("H265ParseSingleNalu err %d< %d,i_iMark %d \r\n",i_iDataMaxLen,i_iBufLen,i_iMark);//i_iMark 0
        return FALSE;
    }
    unsigned char bNaluType = (i_pbRtpBodyBuf[0] & 0x7E)>>1;//取nalu类型
    if(bNaluType >= 0 && bNaluType <= 9)// p slice 片
    {
        //if(0 == m_iFrameStartFlag)
        {//一帧切片成多个nalu单元(前面的片i_iMark都是0)，切片的每个nalu也要加开始码
            memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
            m_iVideoFrameCurLen += 3;
            m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
            m_iVideoFrameCurLen++;//add 00 00 00 01
            //m_iFrameStartFlag=1;
        }
    }
    else if(bNaluType >= 16 && bNaluType <= 21)// IRAP 等同于i帧
    {
        //if(0 == m_iFrameStartFlag)
        {//一帧切片成多个nalu单元(前面的片i_iMark都是0)，切片的每个nalu也要加开始码
            memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
            m_iVideoFrameCurLen += 3;
            m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
            m_iVideoFrameCurLen++;//add 00 00 00 01
            //m_iFrameStartFlag=1;
        }
    }
    else if(bNaluType == 32)//VPS
    {
        memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
        m_iVideoFrameCurLen += 3;
        m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
        m_iVideoFrameCurLen++;//add 00 00 00 01
    }
    else if(bNaluType == 33)//SPS
    {
        memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
        m_iVideoFrameCurLen += 3;
        m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
        m_iVideoFrameCurLen++;//add 00 00 00 01
    }
    else if(bNaluType == 34)//PPS
    {
        memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
        m_iVideoFrameCurLen += 3;
        m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
        m_iVideoFrameCurLen++;//add 00 00 00 01
    }
    memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf,i_iBufLen);
    m_iVideoFrameCurLen+=i_iBufLen;

    if(0 != i_iMark)
    {
        memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
        *o_iDataLen = m_iVideoFrameCurLen;
        m_iVideoFrameCurLen=0;
        m_iFrameStartFlag=0;
    }
    return TRUE;
    

	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen||!i_iMark)//webrtc客户端下发过来的会是0
    {
        printf("H265ParseSingleNalu err %d< %d,i_iMark %d \r\n",i_iDataMaxLen,i_iBufLen,i_iMark);//i_iMark 0
        return FALSE;
    }
    if(bNaluType >= 0 && bNaluType <= 9)// p slice 片
    {
        iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
    }
    else if(bNaluType >= 16 && bNaluType <= 21)// IRAP 等同于i帧
    {
        iFrameFlag = 1;//i b p 的nalu才表示一帧的结束
    }
    memset(m_pVideoFrameBuf+m_iVideoFrameCurLen,0,3);
    m_iVideoFrameCurLen += 3;
    m_pVideoFrameBuf[m_iVideoFrameCurLen]=1;
    m_iVideoFrameCurLen++;//add 00 00 00 01
    memcpy(m_pVideoFrameBuf+m_iVideoFrameCurLen,i_pbRtpBodyBuf,i_iBufLen);
    m_iVideoFrameCurLen+=i_iBufLen;

    if(0 != iFrameFlag)
    {
        memcpy(o_pbParsedData,m_pVideoFrameBuf,m_iVideoFrameCurLen);
        *o_iDataLen = m_iVideoFrameCurLen;
        m_iVideoFrameCurLen=0;
    }
    return TRUE;
}

