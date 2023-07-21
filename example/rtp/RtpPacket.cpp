/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpPacket.cpp
* Description		: 	RtpPacket operation center
                        ��������Rtp����غ����ͣ�����NALU,FU-A���غ�����
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include <stdlib.h>//������Ҫ.h
#include <stdio.h>
#include <string.h>
#include <iostream>//����.h,c++�µ�ͷ�ļ�

#include "Definition.h"
#include "RtpPacket.h"

#ifndef TRUE
#define TRUE 0
#endif
#ifndef FALSE
#define FALSE -1
#endif


using std::cout;//��Ҫ<iostream>
using std::endl;

/*****************************************************************************
-Fuction		: RtpPacket
-Description	: RtpPacket
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacket :: RtpPacket()
{
    m_pRtpPacket = NULL;
    m_iRtpType = 0;

}

/*****************************************************************************
-Fuction		: RtpPacket
-Description	: RtpPacket
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacket :: ~RtpPacket()
{
    if(m_pRtpPacket != NULL)
        delete m_pRtpPacket;
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
int RtpPacket :: GenerateRtpHeader(T_RtpPacketParam *i_ptParam,T_RtpHeader *o_ptRtpHeader)
{
    int iRet=FALSE;
    if(NULL == i_ptParam || NULL == o_ptRtpHeader)
    {
        cout<<"GenerateRtpHeader err NULL"<<endl;
    }
    else
    {
        T_RtpHeader tRtpHeader;
        memset(&tRtpHeader,0,sizeof(T_RtpHeader));
        tRtpHeader.Version=2;
        tRtpHeader.Pad=0;
        tRtpHeader.Extend=0;
        tRtpHeader.CsrcCount=0;
        tRtpHeader.Mark=0;
        tRtpHeader.PayloadType=i_ptParam->wPayloadType;
        tRtpHeader.wSeq=i_ptParam->wSeq++;
        tRtpHeader.dwTimestamp=i_ptParam->dwTimestamp;
        tRtpHeader.dwSSRC=i_ptParam->dwSSRC;

        memcpy(o_ptRtpHeader,&tRtpHeader,sizeof(T_RtpHeader));
        iRet=TRUE;
    }
    return iRet;
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
int RtpPacket :: GenerateRtpHeader(T_RtpPacketParam *i_ptParam,int i_iPaddingLen,int i_iMark,unsigned char *o_bRtpHeader)
{
    int iRet=FALSE;
    if(NULL == i_ptParam || NULL == o_bRtpHeader || RTP_MAX_PACKET_SIZE <= RTP_HEADER_LEN)
    {
        cout<<"GenerateRtpHeader err NULL"<<endl;
    }
    else
    {
        o_bRtpHeader[0] = 0x80 | //v=2
                        (i_iPaddingLen > 0 ? 0x20:0) | //�Ƿ������
                        0 | 
                        0; //CSRC count = 0
        o_bRtpHeader[1] = (unsigned char)(((i_iMark & 1) << 7) | (i_ptParam->wPayloadType & 0x7f));//1bit markbit, 7 bits pt
        o_bRtpHeader[2] =(unsigned char)(i_ptParam->wSeq >> 8);
        o_bRtpHeader[3] =(unsigned char)(i_ptParam->wSeq & 0xff);
        
        o_bRtpHeader[4] =(unsigned char)(i_ptParam->dwTimestamp >> 24);
        o_bRtpHeader[5] =(unsigned char)(i_ptParam->dwTimestamp >> 16);
        o_bRtpHeader[6] =(unsigned char)(i_ptParam->dwTimestamp >> 8);
        o_bRtpHeader[7] =(unsigned char)(i_ptParam->dwTimestamp & 0xff);

        o_bRtpHeader[8] =(unsigned char)(i_ptParam->dwSSRC >> 24);
        o_bRtpHeader[9] =(unsigned char)(i_ptParam->dwSSRC >> 16);
        o_bRtpHeader[10] =(unsigned char)(i_ptParam->dwSSRC >> 8);
        o_bRtpHeader[11] =(unsigned char)(i_ptParam->dwSSRC & 0xff);
        
        i_ptParam->wSeq ++;

        iRet=TRUE;
    }
    return iRet;
}

/*****************************************************************************
-Fuction		: RtpPacket
-Description	: RtpPacket
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpPacket :: Init(unsigned char **m_ppPackets,int i_iMaxPacketNum)
{
    int i=0;
    int iRet=FALSE;
    if(NULL == m_ppPackets)
    {
        cout<<"RtpPacket Init NULL"<<endl;
        return iRet;
    }
    for(i =0;i<i_iMaxPacketNum;i++)
    {
        m_ppPackets[i] = new unsigned char[RTP_MAX_PACKET_SIZE];
        if(NULL == m_ppPackets[i])
        {
            DeInit(m_ppPackets,i_iMaxPacketNum);
            return iRet;
        }
        memset(m_ppPackets[i],0,RTP_MAX_PACKET_SIZE);//��ʼ�������棬���nalu�ֱ�һ��
    }
    return TRUE;
}

/*****************************************************************************
-Fuction		: RtpPacket
-Description	: RtpPacket
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpPacket :: DeInit(unsigned char **m_ppPackets,int i_iMaxPacketNum)
{
    int i=0;
    int iRet=FALSE;
    if(NULL == m_ppPackets)
    {
        cout<<"RtpPacket DeInit NULL"<<endl;
        return iRet;
    }
    for(i =0;i<i_iMaxPacketNum;i++)
    {
        if(NULL != m_ppPackets[i])
        {
            delete [] m_ppPackets[i];
            m_ppPackets[i] = NULL;
        }
    }
    return TRUE;
}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen i_iVideoOrAudio=0 Ĭ����Ƶ��1��Ƶ
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpPacket :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
	if (!i_pbFrameBuf || i_iFrameLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
        return iRet;
    }
    
    switch(i_iRtpPacketType)
    {
        case RTP_PACKET_TYPE_H264:
        {
            if(NULL != m_pRtpPacket)
            {
                if(m_pRtpPacket->m_iRtpType != i_iRtpPacketType)
                {
                    delete m_pRtpPacket;
                    m_pRtpPacket = NULL;
                    m_pRtpPacket = new RtpPacketH264();
                }
            }
            else
            {
                m_pRtpPacket = new RtpPacketH264();
            }
            break;
        }
        case RTP_PACKET_TYPE_H265:
        {
            if(NULL != m_pRtpPacket)
            {
                if(m_pRtpPacket->m_iRtpType != i_iRtpPacketType)
                {
                    delete m_pRtpPacket;
                    m_pRtpPacket = NULL;
                    m_pRtpPacket = new RtpPacketH265();
                }
            }
            else
            {
                m_pRtpPacket = new RtpPacketH265();
            }
            break;
        }
        case RTP_PACKET_TYPE_AAC:
        {
            if(NULL != m_pRtpPacket)
            {
                if(m_pRtpPacket->m_iRtpType != i_iRtpPacketType)
                {
                    delete m_pRtpPacket;
                    m_pRtpPacket = NULL;
                    m_pRtpPacket = new RtpPacketAAC();
                }
            }
            else
            {
                m_pRtpPacket = new RtpPacketAAC();
            }
            break;
        }
        case RTP_PACKET_TYPE_G711U:
        case RTP_PACKET_TYPE_G711A:
        {
            if(NULL != m_pRtpPacket)
            {
                if(m_pRtpPacket->m_iRtpType != i_iRtpPacketType)
                {
                    delete m_pRtpPacket;
                    m_pRtpPacket = NULL;
                    m_pRtpPacket = new RtpPacketG711();
                }
            }
            else
            {
                m_pRtpPacket = new RtpPacketG711();
            }
            break;
        }
        default:
        {
            cout<<"i_iRtpPacketType err "<<i_iRtpPacketType<<endl;
            break;
        }
    }
    if(NULL == m_pRtpPacket)
    {
        cout<<"i_iRtpPacketType new err "<<i_iRtpPacketType<<endl;
    }
    else
    {
        iRet=m_pRtpPacket->Packet(i_ptParam,i_pbFrameBuf,i_iFrameLen,o_ppPackets,o_aiEveryPacketLen);
    }
    return iRet;
}
/*****************************************************************************
-Fuction		: RtpPacketH264
-Description	: RtpPacketH264
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketH264 :: RtpPacketH264()
{
    m_pRtpPacketNALU = NULL;
    m_pRtpPacketFU_A = NULL;
    m_iRtpType = RTP_PACKET_TYPE_H264;
    m_iRtpVideoType = 0;
}

/*****************************************************************************
-Fuction		: RtpPacketH264
-Description	: ~RtpPacketH264
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketH264 :: ~RtpPacketH264()
{
    if(m_pRtpPacketNALU != NULL)
        delete m_pRtpPacketNALU;
    if(m_pRtpPacketFU_A != NULL)
        delete m_pRtpPacketFU_A;
}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen i_iVideoOrAudio=0 Ĭ����Ƶ��1��Ƶ
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpPacketH264 :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    unsigned char *pbNaluBuf=i_pbNaluBuf;
    int iNaluLen=i_iNaluLen;
	if (!i_pbNaluBuf || i_iNaluLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
    }
    else
    {
        //drop 0001
        if (pbNaluBuf[0] == 0 && pbNaluBuf[1] == 0 && pbNaluBuf[2] == 1) 
        {
            pbNaluBuf   += 3;
            iNaluLen    -= 3;
        }
        if (pbNaluBuf[0] == 0 && pbNaluBuf[1] == 0 && pbNaluBuf[2] == 0 && pbNaluBuf[3] == 1) 
        {
            pbNaluBuf   += 4;
            iNaluLen    -= 4;
        }
        
        if((unsigned int)iNaluLen <=RTP_MAX_PACKET_SIZE- sizeof(T_RtpHeader))
        {//����NAL����Ԫ
            if(NULL == m_pRtpPacketNALU)
            {
                m_pRtpPacketNALU = new H264NALU();
            }
            if(NULL != m_pRtpPacketNALU)
            {
                iRet=m_pRtpPacketNALU->Packet(i_ptParam,pbNaluBuf,iNaluLen,o_ppPackets,o_aiEveryPacketLen);
            }
        }
        else
        {//��Ƭ��Ԫ��FU-A��
            if(NULL == m_pRtpPacketFU_A)
            {
                m_pRtpPacketFU_A = new H264FU_A();
            }
            if(NULL != m_pRtpPacketFU_A)
            {
                iRet=m_pRtpPacketFU_A->Packet(i_ptParam,pbNaluBuf,iNaluLen,o_ppPackets,o_aiEveryPacketLen);
            }
        }
    }
    return iRet;
}

/*****************************************************************************
-Fuction		: NALU
-Description	: NALU
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H264NALU :: H264NALU()
{


}

/*****************************************************************************
-Fuction		: NALU
-Description	: ~NALU
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H264NALU :: ~H264NALU()
{


}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H264NALU :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    int iPackNum=0;
    int iMark = 0;
    int iPaddingLen = 0;
    
	if (!i_pbNaluBuf || i_iNaluLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
    }
    else
    {
        switch(i_pbNaluBuf[0] & 0x1f)
        {
            case 0x5:
            case 0x1:
                iMark = 1;//i b p ��nalu�ű�ʾһ֡�Ľ���
                break;
            default:
                break;
        }
        if((i_iNaluLen%4)>0)
        {
            iPaddingLen = 4 -(i_iNaluLen%4);//4�ֽڶ���
        }
        RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
        memcpy(o_ppPackets[iPackNum]+RTP_HEADER_LEN,i_pbNaluBuf,i_iNaluLen);
        o_aiEveryPacketLen[iPackNum]=i_iNaluLen+RTP_HEADER_LEN;
        if(iPaddingLen>0)
        {
            //������λ�����λ�����һλ��ʾ��䳤��
            o_aiEveryPacketLen[iPackNum]+=iPaddingLen;
            o_ppPackets[iPackNum][o_aiEveryPacketLen[iPackNum]-1] = iPaddingLen;
        }
        iPackNum++;
        iRet=iPackNum;        
    }
    return iRet;
}

const unsigned char H264FU_A::FU_A_TYPE=28;
const unsigned char H264FU_A::FU_A_HEADER_LEN=2;
/*****************************************************************************
-Fuction		: FU_A
-Description	: FU_A
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H264FU_A :: H264FU_A()
{
    m_iRtpVideoType = FU_A_TYPE;

}

/*****************************************************************************
-Fuction		: FU_A
-Description	: ~FU_A
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H264FU_A :: ~H264FU_A()
{


}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H264FU_A :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    int iPackNum=0;
    unsigned char *pbNaluBuf=i_pbNaluBuf;
    int iNaluLen=i_iNaluLen;
    int iPaddingLen=0;
    
	if (!i_pbNaluBuf || i_iNaluLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
        return iRet;
    }

    unsigned char bNaluHeader=i_pbNaluBuf[0];
    int iMark = 0;
    while (iNaluLen> 0 && iPackNum < RTP_MAX_PACKET_NUM) 
    {
        iMark = 0;
        if (iPackNum == 0) 
        {
            pbNaluBuf ++; //drop nalu header��һ���ֽڵ�FRT������������в�����(ԭʼ��)nalu header
            iNaluLen --;
        } 
        else if ((unsigned int)iNaluLen <= RTP_MAX_PACKET_SIZE- sizeof(T_RtpHeader)-FU_A_HEADER_LEN) //iNaluLen�Ѿ�--��
        {//NALU Payload������1�ֽڵ�FU indicator  ��1�ֽڵ�   FU header����
            iMark = 1;//���һ��
        }

        if (iPackNum == 0) 
        {
            RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
            o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 0] = (bNaluHeader & 0xe0) | FU_A_TYPE;//FU indicator,��5λ
            o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 1] = (bNaluHeader & 0x1f);//FU indicator����3λ:S E R ,R: 1 bit ����λ��������Ϊ0
            o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 1] |= 0x80; //S: 1 bit �����ó�1,��ʼλָʾ��ƬNAL��Ԫ�Ŀ�ʼ
            memcpy(o_ppPackets[iPackNum] + RTP_HEADER_LEN + FU_A_HEADER_LEN, pbNaluBuf, RTP_MAX_PACKET_SIZE- RTP_HEADER_LEN-FU_A_HEADER_LEN);
            o_aiEveryPacketLen[iPackNum] = RTP_MAX_PACKET_SIZE;

            pbNaluBuf += RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN - FU_A_HEADER_LEN;
            iNaluLen -= RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN - FU_A_HEADER_LEN;
        }
        else
        {
            if (iMark) 
            {
                if(((RTP_HEADER_LEN+FU_A_HEADER_LEN+iNaluLen)%4)>0)
                {
                    iPaddingLen = 4 -((RTP_HEADER_LEN+FU_A_HEADER_LEN+iNaluLen)%4);//4�ֽڶ���
                }
                RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 0] = (bNaluHeader & 0x60) | FU_A_TYPE;//FU indicator,��5λ
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 1] = (bNaluHeader & 0x1f);//FU indicator����3λ:S E R ,R: 1 bit ����λ��������Ϊ0
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 1] |= 0x40; //E: 1 bit �����ó�1, ����λָʾ��ƬNAL��Ԫ�Ľ���
            
                memcpy(o_ppPackets[iPackNum] + RTP_HEADER_LEN + FU_A_HEADER_LEN, pbNaluBuf, iNaluLen);
                o_aiEveryPacketLen[iPackNum] = RTP_HEADER_LEN+ FU_A_HEADER_LEN + iNaluLen;
                if(iPaddingLen>0)
                {
                    o_aiEveryPacketLen[iPackNum]+=iPaddingLen;
                    o_ppPackets[iPackNum][o_aiEveryPacketLen[iPackNum]-1] = iPaddingLen;
                }
                pbNaluBuf += iNaluLen;
                iNaluLen -= iNaluLen;
            } 
            else 
            {
                RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 0] = (bNaluHeader & 0x60) | FU_A_TYPE;//FU indicator,��5λ
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 1] = (bNaluHeader & 0x1f);//FU indicator����3λ:S E R ,R: 1 bit ����λ��������Ϊ0
                memcpy(o_ppPackets[iPackNum] + RTP_HEADER_LEN +FU_A_HEADER_LEN, pbNaluBuf, RTP_MAX_PACKET_SIZE- RTP_HEADER_LEN-FU_A_HEADER_LEN);
                o_aiEveryPacketLen[iPackNum] = RTP_MAX_PACKET_SIZE;
                
                pbNaluBuf += RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN -FU_A_HEADER_LEN;
                iNaluLen -= RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN - FU_A_HEADER_LEN;
            }
        }
        iPackNum++;
    }
    iRet=iPackNum;        

    return iRet;
}

/*****************************************************************************
-Fuction		: RtpPacketH264
-Description	: RtpPacketH264
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketH265 :: RtpPacketH265()
{
    m_pRtpPacketNALU = NULL;
    m_pRtpPacketFU_A = NULL;
    m_iRtpType = RTP_PACKET_TYPE_H265;
    m_iRtpVideoType = 0;
}

/*****************************************************************************
-Fuction		: RtpPacketH264
-Description	: ~RtpPacketH264
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketH265 :: ~RtpPacketH265()
{
    if(m_pRtpPacketNALU != NULL)
        delete m_pRtpPacketNALU;
    if(m_pRtpPacketFU_A != NULL)
        delete m_pRtpPacketFU_A;
}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen i_iVideoOrAudio=0 Ĭ����Ƶ��1��Ƶ
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpPacketH265 :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    unsigned char *pbNaluBuf=i_pbNaluBuf;
    int iNaluLen=i_iNaluLen;
	if (!i_pbNaluBuf || i_iNaluLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
    }
    else
    {
        //drop 0001
        if (pbNaluBuf[0] == 0 && pbNaluBuf[1] == 0 && pbNaluBuf[2] == 0 && pbNaluBuf[3] == 1) 
        {
            pbNaluBuf   += 4;
            iNaluLen    -= 4;
        }
        
        if((unsigned int)iNaluLen <=RTP_MAX_PACKET_SIZE- sizeof(T_RtpHeader))
        {//����NAL����Ԫ
            if(NULL == m_pRtpPacketNALU)
            {
                m_pRtpPacketNALU = new H265NALU();
            }
            if(NULL != m_pRtpPacketNALU)
            {
                iRet=m_pRtpPacketNALU->Packet(i_ptParam,pbNaluBuf,iNaluLen,o_ppPackets,o_aiEveryPacketLen);
            }
        }
        else
        {//��Ƭ��Ԫ��FU-A��
            if(NULL == m_pRtpPacketFU_A)
            {
                m_pRtpPacketFU_A = new H265FU_A();
            }
            if(NULL != m_pRtpPacketFU_A)
            {
                iRet=m_pRtpPacketFU_A->Packet(i_ptParam,pbNaluBuf,iNaluLen,o_ppPackets,o_aiEveryPacketLen);
            }
        }
    }
    return iRet;
}
/*****************************************************************************
-Fuction		: NALU
-Description	: NALU
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H265NALU :: H265NALU()
{

}

/*****************************************************************************
-Fuction		: NALU
-Description	: ~NALU
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H265NALU :: ~H265NALU()
{


}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H265NALU :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    int iPackNum=0;
    int iMark = 0;
    int iPaddingLen = 0;
    unsigned char bNaluType = 0;
    
    if (!i_pbNaluBuf || i_iNaluLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
    }
    else
    {
        bNaluType = (i_pbNaluBuf[0] & 0x7E)>>1;//ȡnalu����
        if(bNaluType >= 0 && bNaluType <= 9)// p slice Ƭ
        {
            iMark = 1;//i p b nalu�ű�ʾһ֡�Ľ���
        }
        else if(bNaluType >= 16 && bNaluType <= 21)// IRAP ��ͬ��i֡
        {
            iMark = 1;//i p b nalu�ű�ʾһ֡�Ľ���
        }
        
        if((i_iNaluLen%4)>0)
        {
            iPaddingLen = 4 -(i_iNaluLen%4);//4�ֽڶ���
        }
        RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
        memcpy(o_ppPackets[iPackNum]+RTP_HEADER_LEN,i_pbNaluBuf,i_iNaluLen);
        o_aiEveryPacketLen[iPackNum]=i_iNaluLen+RTP_HEADER_LEN;
        if(iPaddingLen>0)
        {
            //������λ�����λ�����һλ��ʾ��䳤��
            o_aiEveryPacketLen[iPackNum]+=iPaddingLen;
            o_ppPackets[iPackNum][o_aiEveryPacketLen[iPackNum]-1] = iPaddingLen;
        }
        iPackNum++;
        iRet=iPackNum;        
    }
    return iRet;
}


const unsigned char H265FU_A::FU_A_TYPE=49;
const unsigned char H265FU_A::FU_A_HEADER_LEN=3;
/*****************************************************************************
-Fuction		: FU_A
-Description	: FU_A
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H265FU_A :: H265FU_A()
{
    m_iRtpVideoType = FU_A_TYPE;

}

/*****************************************************************************
-Fuction		: FU_A
-Description	: ~FU_A
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H265FU_A :: ~H265FU_A()
{


}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H265FU_A :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbNaluBuf,int i_iNaluLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    int iPackNum=0;
    unsigned char *pbNaluBuf=i_pbNaluBuf;
    int iNaluLen=i_iNaluLen;
    int iPaddingLen=0;
    
    if (!i_pbNaluBuf || i_iNaluLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
        return iRet;
    }

    unsigned char bNaluHeader1=i_pbNaluBuf[0];
    unsigned char bNaluHeader2=i_pbNaluBuf[1];
    int iMark = 0;
    while (iNaluLen> 0 && iPackNum < RTP_MAX_PACKET_NUM) 
    {
        iMark = 0;
        if (iPackNum == 0) 
        {
            pbNaluBuf +=2; //drop nalu header�����ֽڣ�����������в�����(ԭʼ��)nalu header
            iNaluLen -=2;
        } 
        else if ((unsigned int)iNaluLen <= RTP_MAX_PACKET_SIZE- RTP_HEADER_LEN-FU_A_HEADER_LEN) //iNaluLen�Ѿ�--��
        {//NALU Payload������1�ֽڵ�FU indicator  ��1�ֽڵ�   FU header����
            iMark = 1;//���һ��
        }

        if (iPackNum == 0) 
        {
            RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
            o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 0] = (bNaluHeader1 & 0x81) | (FU_A_TYPE<<1);//�޸�typeֵΪ49(& 0x81�����ԭ�е�ռλ����)
            o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 1] = bNaluHeader2;//����
            o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 2] = (bNaluHeader1>>1) & 0x3F; //nalu type
            o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 2] |= 0x80; //S: 1 bit �����ó�1,��ʼλָʾ��ƬNAL��Ԫ�Ŀ�ʼ
            memcpy(o_ppPackets[iPackNum] + RTP_HEADER_LEN + FU_A_HEADER_LEN, pbNaluBuf, RTP_MAX_PACKET_SIZE- RTP_HEADER_LEN-FU_A_HEADER_LEN);
            o_aiEveryPacketLen[iPackNum] = RTP_MAX_PACKET_SIZE;

            pbNaluBuf += RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN - FU_A_HEADER_LEN;
            iNaluLen -= RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN - FU_A_HEADER_LEN;
        }
        else
        {
            if (iMark) 
            {
                if(((RTP_HEADER_LEN+FU_A_HEADER_LEN+iNaluLen)%4)>0)
                {
                    iPaddingLen = 4 -((RTP_HEADER_LEN+FU_A_HEADER_LEN+iNaluLen)%4);//4�ֽڶ���
                }
                RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 0] = (bNaluHeader1 & 0x81) | (FU_A_TYPE<<1);//�޸�typeֵΪ49(& 0x81�����ԭ�е�ռλ����)
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 1] = bNaluHeader2;//����
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 2] = (bNaluHeader1>>1) & 0x3F; //nalu type
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 2] |= 0x40; //E: 1 bit �����ó�1, ����λָʾ��ƬNAL��Ԫ�Ľ���
            
                memcpy(o_ppPackets[iPackNum] + RTP_HEADER_LEN + FU_A_HEADER_LEN, pbNaluBuf, iNaluLen);
                o_aiEveryPacketLen[iPackNum] = RTP_HEADER_LEN+ FU_A_HEADER_LEN + iNaluLen;
                if(iPaddingLen>0)
                {
                    o_aiEveryPacketLen[iPackNum]+=iPaddingLen;
                    o_ppPackets[iPackNum][o_aiEveryPacketLen[iPackNum]-1] = iPaddingLen;
                }
                pbNaluBuf += iNaluLen;
                iNaluLen -= iNaluLen;
            } 
            else 
            {
                RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 0] = (bNaluHeader1 & 0x81) | (FU_A_TYPE<<1);//�޸�typeֵΪ49(& 0x81�����ԭ�е�ռλ����)
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 1] = bNaluHeader2;//����
                o_ppPackets[iPackNum][sizeof(T_RtpHeader) + 2] = (bNaluHeader1>>1) & 0x3F; //nalu type
                memcpy(o_ppPackets[iPackNum] + RTP_HEADER_LEN + FU_A_HEADER_LEN, pbNaluBuf, RTP_MAX_PACKET_SIZE- RTP_HEADER_LEN-FU_A_HEADER_LEN);
                o_aiEveryPacketLen[iPackNum] = RTP_MAX_PACKET_SIZE;
                
                pbNaluBuf += RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN - FU_A_HEADER_LEN;
                iNaluLen -= RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN - FU_A_HEADER_LEN;
            }
        }
        iPackNum++;
    }
    iRet=iPackNum;        

    return iRet;
}


/*****************************************************************************
-Fuction		: RtpPacketG711
-Description	: RtpPacketG711
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketG711 :: RtpPacketG711()
{
    m_pRtpPacketG711 = NULL;
    m_iRtpType = RTP_PACKET_TYPE_G711U;
}

/*****************************************************************************
-Fuction		: RtpPacketH264
-Description	: ~RtpPacketH264
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketG711 :: ~RtpPacketG711()
{
    if(m_pRtpPacketG711 != NULL)
        delete m_pRtpPacketG711;

}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen i_iVideoOrAudio=0 Ĭ����Ƶ��1��Ƶ
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpPacketG711 :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    unsigned char *pbFrameBuf=i_pbFrameBuf;
    int iFrameLen=i_iFrameLen;
    int iPackNum=0;
    int iMark = 0;
    int iPaddingLen = 0;
    
	if (!i_pbFrameBuf || i_iFrameLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
    }
    else
    {
        while (iFrameLen> 0 && iPackNum < RTP_MAX_PACKET_NUM) 
        {
            if ((unsigned int)iFrameLen <= RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN) 
            {
                iMark = 1;
                if(((iFrameLen+RTP_HEADER_LEN)%4)>0)
                {
                    iPaddingLen = 4 -((iFrameLen+RTP_HEADER_LEN)%4);//4�ֽڶ���
                }
                RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
                memcpy(o_ppPackets[iPackNum]+RTP_HEADER_LEN,pbFrameBuf,iFrameLen);
                o_aiEveryPacketLen[iPackNum]=iFrameLen+RTP_HEADER_LEN;
                if(iPaddingLen>0)
                {
                    //������λ�����λ���һλ��ʾ����ֶεĳ���
                    o_aiEveryPacketLen[iPackNum]+=iPaddingLen;
                    o_ppPackets[iPackNum][o_aiEveryPacketLen[iPackNum]-1] = iPaddingLen;
                }
                pbFrameBuf += iFrameLen;
                iFrameLen -= iFrameLen;
            } 
            else 
            {
                RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
                memcpy(o_ppPackets[iPackNum]+RTP_HEADER_LEN,pbFrameBuf, RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN);
                o_aiEveryPacketLen[iPackNum]= RTP_MAX_PACKET_SIZE;
                pbFrameBuf +=  RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN;
                iFrameLen -=  RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN;
            }
            iPackNum++;
        }
        iRet=iPackNum;        
    }
    return iRet;
}
/*****************************************************************************
-Fuction		: RtpPacketG726
-Description	: RtpPacketG726
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketG726 :: RtpPacketG726()
{
    m_pRtpPacketG726 = NULL;
    m_iRtpType = RTP_PACKET_TYPE_G726;

}

/*****************************************************************************
-Fuction		: RtpPacketH264
-Description	: ~RtpPacketH264
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketG726 :: ~RtpPacketG726()
{
    if(m_pRtpPacketG726 != NULL)
        delete m_pRtpPacketG726;
}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen i_iVideoOrAudio=0 Ĭ����Ƶ��1��Ƶ
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpPacketG726 :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    unsigned char *pbFrameBuf=i_pbFrameBuf;
    int iFrameLen=i_iFrameLen;
    int iPackNum=0;
    int iMark = 0;
    int iPaddingLen = 0;
    
    if (!i_pbFrameBuf || i_iFrameLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
    }
    else
    {
        while (iFrameLen> 0 && iPackNum < RTP_MAX_PACKET_NUM) 
        {
            if ((unsigned int)iFrameLen <= RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN) 
            {
                iMark = 1;
                if(((iFrameLen+RTP_HEADER_LEN)%4)>0)
                {
                    iPaddingLen = 4 -((iFrameLen+RTP_HEADER_LEN)%4);//4�ֽڶ���
                }
                RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
                memcpy(o_ppPackets[iPackNum]+RTP_HEADER_LEN,pbFrameBuf,iFrameLen);
                o_aiEveryPacketLen[iPackNum]=iFrameLen+RTP_HEADER_LEN;
                if(iPaddingLen>0)
                {
                    //������λ�����λ���һλ��ʾ����ֶεĳ���
                    o_aiEveryPacketLen[iPackNum]+=iPaddingLen;
                    o_ppPackets[iPackNum][o_aiEveryPacketLen[iPackNum]-1] = iPaddingLen;
                }
                pbFrameBuf += iFrameLen;
                iFrameLen -= iFrameLen;
            } 
            else 
            {
                RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
                memcpy(o_ppPackets[iPackNum]+RTP_HEADER_LEN,pbFrameBuf, RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN);
                o_aiEveryPacketLen[iPackNum]= RTP_MAX_PACKET_SIZE;
                pbFrameBuf +=  RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN;
                iFrameLen -=  RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN;
            }
            iPackNum++;
        }
        iRet=iPackNum;        
    }
    return iRet;
}

/*****************************************************************************
-Fuction		: RtpPacketAAC
-Description	: RtpPacketAAC
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketAAC :: RtpPacketAAC()
{
    m_pRtpPacketAAC = NULL;
    m_iRtpType = RTP_PACKET_TYPE_AAC;
}

/*****************************************************************************
-Fuction		: RtpPacketAAC
-Description	: ~RtpPacketAAC
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpPacketAAC :: ~RtpPacketAAC()
{
    if(m_pRtpPacketAAC != NULL)
        delete m_pRtpPacketAAC;

}

/*****************************************************************************
-Fuction		: Packet
-Description	: Packet
-Input			: i_ptParam i_pbNaluBuf i_iNaluLen i_iVideoOrAudio=0 Ĭ����Ƶ��1��Ƶ
-Output 		: ��ά����o_ppPackets�Ŷ�������ݣ�����o_aiEveryPacketLen��ÿ���ĳ���
-Return 		: ������ĿPacketsNum
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpPacketAAC :: Packet(T_RtpPacketParam *i_ptParam,unsigned char *i_pbFrameBuf,int i_iFrameLen,unsigned char **o_ppPackets,int *o_aiEveryPacketLen,int i_iRtpPacketType)
{
    int iRet=FALSE;
    unsigned char *pbFrameBuf=i_pbFrameBuf;
    int iFrameLen=i_iFrameLen;
    int iPackNum=0;
    int iMark = 0;
    int iPaddingLen = 0;
    
    if (!i_pbFrameBuf || i_iFrameLen <= 0 || !o_ppPackets || !o_aiEveryPacketLen)
    {
        cout<<"Packet err NULL"<<endl;
        return iRet;
    }
    
    pbFrameBuf += 7;//aac Ҫƫ��7�ֽ�ͷ��������
    iFrameLen-=7;
    while (iFrameLen> 0 && iPackNum < RTP_MAX_PACKET_NUM) 
    {
        if ((unsigned int)iFrameLen <= RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN-4) 
        {
            iMark = 1;
            if(((iFrameLen+RTP_HEADER_LEN+4)%4)>0)
            {
                iPaddingLen = 4 -((iFrameLen+RTP_HEADER_LEN+4)%4);//4�ֽڶ���
            }
            RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
            o_ppPackets[iPackNum][RTP_HEADER_LEN] = 0x00;
            o_ppPackets[iPackNum][RTP_HEADER_LEN+1] = 0x10;
            o_ppPackets[iPackNum][RTP_HEADER_LEN+2] = (unsigned char)((iFrameLen & 0x1FE0)>>5);//ȡ���ȵĸ�8λ
            o_ppPackets[iPackNum][RTP_HEADER_LEN+3] = (unsigned char)((iFrameLen & 0x1F)<<3);//ȡ���ȵĵ�5λ����13λ��aac data�ĳ���
            
            memcpy(o_ppPackets[iPackNum]+RTP_HEADER_LEN+4,pbFrameBuf,iFrameLen);
            o_aiEveryPacketLen[iPackNum]=iFrameLen+RTP_HEADER_LEN+4;
            if(iPaddingLen>0)
            {
                //������λ�����λ���һλ��ʾ����ֶεĳ���
                o_aiEveryPacketLen[iPackNum]+=iPaddingLen;
                o_ppPackets[iPackNum][o_aiEveryPacketLen[iPackNum]-1] = iPaddingLen;
            }
            pbFrameBuf += iFrameLen;
            iFrameLen -= iFrameLen;
        } 
        else 
        {
            RtpPacket :: GenerateRtpHeader(i_ptParam,iPaddingLen,iMark,o_ppPackets[iPackNum]);
            o_ppPackets[iPackNum][RTP_HEADER_LEN] = 0x00;
            o_ppPackets[iPackNum][RTP_HEADER_LEN+1] = 0x10;
            o_ppPackets[iPackNum][RTP_HEADER_LEN+2] = (unsigned char)((iFrameLen & 0x1FE0)>>5);//ȡ���ȵĸ�8λ
            o_ppPackets[iPackNum][RTP_HEADER_LEN+3] = (unsigned char)((iFrameLen & 0x1F)<<3);//ȡ���ȵĵ�5λ����13λ��aac data�ĳ���
            
            memcpy(o_ppPackets[iPackNum]+RTP_HEADER_LEN+4,pbFrameBuf, RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN-4);
            o_aiEveryPacketLen[iPackNum]= RTP_MAX_PACKET_SIZE;
            pbFrameBuf +=  RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN;
            iFrameLen -=  RTP_MAX_PACKET_SIZE - RTP_HEADER_LEN;
        }
        iPackNum++;
    }
    iRet=iPackNum;        

    return iRet;
}




