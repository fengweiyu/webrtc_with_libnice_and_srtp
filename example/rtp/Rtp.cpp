/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Rtp.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "Rtp.h"
#include <stdlib.h>//������Ҫ.h
#include <stdio.h>
#include <string.h>
#include <iostream>//����.h,c++�µ�ͷ�ļ�
#include "Definition.h"
#include "MediaHandle.h"

using std::cout;//��Ҫ<iostream>
using std::endl;

/*****************************************************************************
-Fuction		: Rtp
-Description	: Rtp
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Rtp :: Rtp(unsigned char **m_ppPackets,int i_iMaxPacketNum,char *i_strPath)
{
    int iRet=FALSE;
    
    m_pMediaHandle = NULL;
    m_dwLastTimestamp = 0;
    m_ptMediaFrameParam = NULL;

    m_pMediaHandle=new MediaHandle();
    if(NULL !=m_pMediaHandle && NULL !=i_strPath)
    {
        m_pMediaHandle->Init(i_strPath);
    }
    
    m_pVideoRtpSession = new RtpSession(RTP_PAYLOAD_VIDEO,0);//i_dwSampleRate ��ʱ�ò�������0 tMediaInfo.dwVideoSampleRate
    m_pAudioRtpSession = new RtpSession(RTP_PAYLOAD_G711A,0);
    
    iRet=m_VideoRtpPacket.Init(m_ppPackets, i_iMaxPacketNum);
    if(FALSE == iRet)
    {
        RTP_LOGE("m_pRtpPacket->Init NULL\r\n");
    }
}

/*****************************************************************************
-Fuction		: Rtp
-Description	: Rtp
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Rtp :: Rtp(char *i_strPath)
{
    int iRet=FALSE;
    
    m_pMediaHandle = NULL;
    m_dwLastTimestamp = 0;
    m_ptMediaFrameParam = NULL;

    m_pMediaHandle=new MediaHandle();
    if(NULL !=m_pMediaHandle && NULL !=i_strPath)
    {
        m_pMediaHandle->Init(i_strPath);
    }
    m_pVideoRtpSession = NULL;
    m_pAudioRtpSession = NULL;
}

/*****************************************************************************
-Fuction		: Rtp
-Description	: Rtp
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Rtp :: Rtp()
{
    m_pMediaHandle = NULL;
    m_pVideoRtpSession = NULL;
    m_pAudioRtpSession = NULL;
    m_dwLastTimestamp = 0;
    m_ptMediaFrameParam = NULL;
}

/*****************************************************************************
-Fuction		: ~Rtp
-Description	: ~Rtp
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Rtp :: ~Rtp()
{
    if(NULL !=m_pMediaHandle)
    {
        delete m_pMediaHandle;
        m_pMediaHandle = NULL;
    }
    if(NULL !=m_pVideoRtpSession)
    {
        delete m_pVideoRtpSession;
        m_pVideoRtpSession = NULL;
    }
    if(NULL !=m_pAudioRtpSession)
    {
        delete m_pAudioRtpSession;
        m_pAudioRtpSession = NULL;
    }
}


/*****************************************************************************
-Fuction		: ~Rtp
-Description	: ~Rtp
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp :: DeInit(unsigned char **m_ppPackets,int i_iMaxPacketNum)
{
    if(NULL!= m_ptMediaFrameParam && NULL!= m_ptMediaFrameParam->pbFrameBuf)
    {
        delete m_ptMediaFrameParam->pbFrameBuf;
    }
    if(NULL!= m_ptMediaFrameParam)
    {
        delete (T_MediaFrameParam *)m_ptMediaFrameParam;
    }
    return m_VideoRtpPacket.DeInit(m_ppPackets, i_iMaxPacketNum);
}

/*****************************************************************************
-Fuction		: RtpInterface::Init
-Description	: �ݲ�ʹ��
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::Init(unsigned char **m_ppPackets,int i_iMaxPacketNum,char *i_strPath)
{
    int iRet=FALSE;
    
    if(NULL == i_strPath)
    {
        cout<<"Init NULL"<<endl;
    }
    else
    {
        m_pMediaHandle=new MediaHandle();
        if(NULL !=m_pMediaHandle)
            iRet=m_pMediaHandle->Init(i_strPath);
    }

    m_pVideoRtpSession = new RtpSession(RTP_PAYLOAD_VIDEO,0);//i_dwSampleRate ��ʱ�ò�������0 tMediaInfo.dwVideoSampleRate

    iRet=m_VideoRtpPacket.Init(m_ppPackets, i_iMaxPacketNum);
    if(FALSE == iRet)
    {
        cout<<"m_pRtpPacket->Init NULL"<<endl;
        return iRet;
    }
    m_ptMediaFrameParam = new T_MediaFrameParam();
    if(NULL == m_ptMediaFrameParam)
    {
        cout<<"m_ptMediaFrameParam malloc NULL"<<endl;
        return iRet;
    }
    memset((T_MediaFrameParam *)m_ptMediaFrameParam,0,sizeof(T_MediaFrameParam));
    m_ptMediaFrameParam->pbFrameBuf = new unsigned char[FRAME_BUFFER_MAX_SIZE];
    if(NULL == m_ptMediaFrameParam->pbFrameBuf)
    {
        cout<<"pbFrameBuf malloc NULL"<<endl;
        delete (T_MediaFrameParam *)m_ptMediaFrameParam;
        return iRet;
    }
	return iRet;
}

/*****************************************************************************
-Fuction		: RtpInterface
-Description	: ֻ����Ƶ���
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,������ʾrtp������
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp :: GetRtpData(unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum)
{
    int iPacketNum = -1;
    int i;
    int iRet = FALSE;
    T_RtpPacketParam tRtpPacketParam;
    unsigned int dwDiffTimestamp = 0;
    unsigned int dwNaluOffset = 0;
    unsigned char *pbNaluStartPos=NULL;
    
    if(NULL == o_ppbPacketBuf ||NULL == o_aiEveryPacketLen ||NULL == m_pMediaHandle )
    {
        cout<<"GetRtpData NULL"<<endl;
        return iPacketNum;
    }
    
    m_ptMediaFrameParam->eFrameType = MEDIA_FRAME_TYPE_UNKNOW;
    memset(m_ptMediaFrameParam->pbFrameBuf,0,FRAME_BUFFER_MAX_SIZE);
    m_ptMediaFrameParam->iFrameBufMaxLen = FRAME_BUFFER_MAX_SIZE;
    iRet=m_pMediaHandle->GetNextFrame(m_ptMediaFrameParam);
    if(FALSE == iRet)
    {
        cout<<"m_MediaHandle.GetNextFrame err"<<endl;
        return iPacketNum;
    }

    memset(&tRtpPacketParam,0,sizeof(T_RtpPacketParam));
    m_pVideoRtpSession->GetRtpPacketParam(&tRtpPacketParam);
    if (0 == m_dwLastTimestamp)
    {
        dwDiffTimestamp = 0;
    }
    else
    {
        dwDiffTimestamp = m_ptMediaFrameParam->dwTimeStamp - m_dwLastTimestamp;
    }
    tRtpPacketParam.dwTimestamp += dwDiffTimestamp;//��������Ŀ������rtp��ʱ�����0��ʼ��
    m_dwLastTimestamp = m_ptMediaFrameParam->dwTimeStamp;//��ȻҲ����ֱ����m_tMediaFrameParam.dwTimeStamp
    
    pbNaluStartPos = m_ptMediaFrameParam->pbFrameStartPos;
    dwNaluOffset = 0;
    iPacketNum = 0;

    /*if(1== m_ptMediaFrameParam->dwNaluCount && FRAME_TYPE_VIDEO_I_FRAME == m_ptMediaFrameParam->eFrameType)
    {//����i֡û��pps�Լ�sps�����ǰ��Ĳ�������һ�����ú����i֡
        T_VideoEncodeParam tVideoEncodeParam;
        iRet=m_pMediaHandle->GetVideoEncParam(&tVideoEncodeParam);
        iPacketNum+=m_RtpPacket.Packet(&tRtpPacketParam,tVideoEncodeParam.abSPS,tVideoEncodeParam.iSizeOfSPS,&o_ppbPacketBuf[iPacketNum],&o_aiEveryPacketLen[iPacketNum]);
        m_pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
        iPacketNum+=m_RtpPacket.Packet(&tRtpPacketParam,tVideoEncodeParam.abPPS,tVideoEncodeParam.iSizeOfPPS,&o_ppbPacketBuf[iPacketNum],&o_aiEveryPacketLen[iPacketNum]);
        m_pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
    }*/
    
    for(i=0;i<m_ptMediaFrameParam->dwNaluCount;i++)
    {
        iRet=m_VideoRtpPacket.Packet(&tRtpPacketParam,pbNaluStartPos,
        m_ptMediaFrameParam->a_dwNaluEndOffset[i]-dwNaluOffset,&o_ppbPacketBuf[iPacketNum],i_iPacketBufMaxNum-iPacketNum,&o_aiEveryPacketLen[iPacketNum]);
        m_pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
        if(iRet<=0 || iPacketNum+iRet>i_iPacketBufMaxNum)
        {
            cout<<"m_pRtpPacket->Packet err"<<iPacketNum<<endl;
            iPacketNum =-1;
            return iPacketNum;
        }
        iPacketNum+=iRet;
        pbNaluStartPos = m_ptMediaFrameParam->pbFrameStartPos +m_ptMediaFrameParam->a_dwNaluEndOffset[i];
        dwNaluOffset =m_ptMediaFrameParam->a_dwNaluEndOffset[i];
    }

    cout<<"m_ptMediaFrameParam)->dwNaluCount"<<m_ptMediaFrameParam->dwNaluCount<<" eFrameType "<<m_ptMediaFrameParam->eFrameType<<endl;
    return iPacketNum;
}

/*****************************************************************************
-Fuction		: VideoHandle::GetSPS_PPS
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::GetSPS_PPS(unsigned char *o_pbSpsBuf,int *o_piSpsBufLen,unsigned char *o_pbPpsBuf,int *o_piPpsBufLen)
{
    int iRet=FALSE;
    T_VideoEncodeParam tVideoEncodeParam;
    
    if(NULL !=m_pMediaHandle)
    {
        memset(&tVideoEncodeParam,0,sizeof(T_VideoEncodeParam));
        iRet=m_pMediaHandle->GetVideoEncParam(&tVideoEncodeParam);
        memcpy(o_pbSpsBuf,tVideoEncodeParam.abSPS,tVideoEncodeParam.iSizeOfSPS);
        *o_piSpsBufLen = tVideoEncodeParam.iSizeOfSPS;
        memcpy(o_pbPpsBuf,tVideoEncodeParam.abPPS,tVideoEncodeParam.iSizeOfPPS);
        *o_piPpsBufLen = tVideoEncodeParam.iSizeOfPPS;
    }
    if(0 == tVideoEncodeParam.iSizeOfSPS ||0 == tVideoEncodeParam.iSizeOfPPS)
    {
        iRet=FALSE;
    }
	return iRet;
}

/*****************************************************************************
-Fuction		: VideoHandle::GetSPS_PPS
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::GetSSRC(unsigned int *o_pdwVideoSSRC,unsigned int *o_pdwAudioSSRC)
{
    int iRet = -1;
    T_RtpPacketParam tRtpPacketParam;

    if((NULL != o_pdwVideoSSRC&&NULL == m_pVideoRtpSession) ||(NULL != o_pdwAudioSSRC&&NULL == m_pAudioRtpSession))
    {
        RTP_LOGE("GetSSRC m_pVideoRtpSession m_pAudioRtpSessionNULL%p,%p,%p,%p,\r\n",o_pdwVideoSSRC,m_pVideoRtpSession,o_pdwAudioSSRC,m_pAudioRtpSession);
        return iRet;
    }
    if(NULL != o_pdwVideoSSRC)
    {
        memset(&tRtpPacketParam,0,sizeof(T_RtpPacketParam));
        if(NULL !=m_pVideoRtpSession)
        {
            iRet=m_pVideoRtpSession->GetRtpPacketParam(&tRtpPacketParam);
        }
        *o_pdwVideoSSRC = tRtpPacketParam.dwSSRC;
    }
    if(NULL != o_pdwAudioSSRC)
    {
        memset(&tRtpPacketParam,0,sizeof(T_RtpPacketParam));
        if(NULL !=m_pAudioRtpSession)
        {
            iRet|=m_pAudioRtpSession->GetRtpPacketParam(&tRtpPacketParam);
        }
        *o_pdwAudioSSRC = tRtpPacketParam.dwSSRC;
    }
	return iRet;
}


/*****************************************************************************
-Fuction		: Rtp
-Description	: Rtp
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp :: SetRtpTypeInfo(T_RtpMediaInfo *i_ptRtpMediaInfo)
{
    int iRet=FALSE;
    T_RtpPacketTypeInfos tRtpPacketTypeInfos;
	E_RtpPacketType ePacketType=RTP_PACKET_TYPE_UNKNOW;
	
    if(NULL == i_ptRtpMediaInfo)
    {
        RTP_LOGE("i_ptRtpMediaInfo NULL err %p\r\n",i_ptRtpMediaInfo);
        return iRet;
    }
    m_pVideoRtpSession = new RtpSession(i_ptRtpMediaInfo->iVideoPayload,0);//i_dwSampleRate ��ʱ�ò�������0 tMediaInfo.dwVideoSampleRate
    m_pAudioRtpSession = new RtpSession(i_ptRtpMediaInfo->iAudioPayload,0);

    
    memset(&tRtpPacketTypeInfos,0,sizeof(T_RtpPacketTypeInfos));
    tRtpPacketTypeInfos.atTypeInfos[0].iPayload=i_ptRtpMediaInfo->iVideoPayload;
    switch(i_ptRtpMediaInfo->iVideoEnc)
    {
        case MEDIA_ENCODE_TYPE_H264:
        {
            ePacketType = RTP_PACKET_TYPE_H264;
            break;
        }
        case MEDIA_ENCODE_TYPE_H265:
        {
            ePacketType = RTP_PACKET_TYPE_H265;
            break;
        }
        default :
        {
            RTP_LOGE("iVideoEnc eEncType err %d\r\n",i_ptRtpMediaInfo->iVideoEnc);
            return iRet;
        }
    }
    tRtpPacketTypeInfos.atTypeInfos[0].ePacketType=ePacketType;
    ePacketType=RTP_PACKET_TYPE_UNKNOW;
    tRtpPacketTypeInfos.atTypeInfos[1].iPayload=i_ptRtpMediaInfo->iAudioPayload;
    switch(i_ptRtpMediaInfo->iAudioEnc)
    {
        case MEDIA_ENCODE_TYPE_AAC:
        {
            ePacketType = RTP_PACKET_TYPE_AAC;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711A:
        {
            ePacketType = RTP_PACKET_TYPE_G711A;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711U:
        {
            ePacketType = RTP_PACKET_TYPE_G711U;
            break;
        }
        default :
        {
            RTP_LOGE("iAudioEnc eEncType err %d\r\n",i_ptRtpMediaInfo->iAudioEnc);
            return iRet;
        }
    }
    tRtpPacketTypeInfos.atTypeInfos[1].ePacketType=ePacketType;
    return m_RtpParse.Init(tRtpPacketTypeInfos);
}

/*****************************************************************************
-Fuction		: GetFrame
-Description	: GetFrame������ParseRtpPacket�������޸�ptFrame->iFrameBufLen��
ptFrame->iFrameProcessedLen��ص��߼���ɹر�(��,����alexa��Ƶ���ȷ��͹���ֻ��80���ȵ�����)
�����յĲ���֡��������Ϊ������һ֡�Ĺ���(��֡-->һ֡)
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp :: GetFrame(T_MediaFrameInfo *m_ptFrame)
{
    int iRet = -1;

    if(NULL == m_ptFrame)
    {
        RTP_LOGE("GetFrame NULL\r\n");
        return -1;
    }
    if(STREAM_TYPE_MUX_STREAM != m_ptFrame->eStreamType)
    {//Ŀǰ�ⲿֻ��STREAM_TYPE_MUX_STREAM�����鱨��,�������Թر�������Ҳ��������߼�
        //return m_pMediaHandle->GetFrame(m_ptFrame);//���������ⲿ����̫��
    }
    m_ptFrame->iFrameProcessedLen=0;//���ParseRtpPacket�����е�o_ptFrame->iFrameBufLen,
    iRet = m_pMediaHandle->GetFrame(m_ptFrame);//����ʵ��ÿ�ν���֡���ݵ�һ����,Ȼ�������һ����������Ҫ���֡,��:
    m_ptFrame->iFrameBufLen -= m_ptFrame->iFrameProcessedLen;//����һ֡ 10 ms 80���ȵ�֡�ϲ���160��֡��
    if(m_ptFrame->iFrameProcessedLen>0 && m_ptFrame->iFrameBufLen>0)//ÿ�ζ���Ҫ������
    {//���ִ������Ҫע�⿴��־����������ֵ����(���)
        RTP_LOGW("Rtp::GetFrame warn %d,eEncType%d,eFrameType%d,dwNaluCount%d,iFrameLen%d,%d,%d\r\n",m_ptFrame->eStreamType,m_ptFrame->eEncType,
        m_ptFrame->eFrameType,m_ptFrame->dwNaluCount,m_ptFrame->iFrameLen,m_ptFrame->iFrameProcessedLen,m_ptFrame->iFrameBufLen);
        m_ptFrame->iFrameBufLen=0;//ʣ�������Ҫ��������������������ʹ��ptFrame->pbFrameBuf+ptFrame->iFrameBufLen�ᱣ������������ݼ�������Ӷ�����(����)�Ĵ������
        //return 0;//���������Ѿ��������ɷ���0
    }//����7�ֽڵ����nalu������֡���ݺ���(00 00 00 01 0c xx xx),��ɽ���if�����У�������ݿ�ֱ�Ӷ���
    return iRet;
}

/*****************************************************************************
-Fuction		: GetRtpPackets
-Description	: GetRtpPackets
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,������ʾrtp������
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp :: GetRtpPackets(T_MediaFrameInfo *m_ptFrame,unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum)
{
    int iPacketNum = -1;
    int i;
    int iRet = FALSE;
    T_RtpPacketParam tRtpPacketParam;
    unsigned int dwDiffTimestamp = 0;
    unsigned int dwNaluOffset = 0;
    unsigned char *pbNaluStartPos=NULL;
    int iRtpPacketType = RTP_PACKET_TYPE_H264;
    
    if(NULL == m_ptFrame ||NULL == o_ppbPacketBuf ||NULL == o_aiEveryPacketLen ||NULL == m_pMediaHandle )
    {
        RTP_LOGE("GetRtpPackets NULL\r\n");
        return iPacketNum;
    }
    if(NULL == m_pVideoRtpSession ||NULL == m_pAudioRtpSession)
    {
        RTP_LOGE("GetRtpPackets m_pVideoRtpSession m_pAudioRtpSession NULL %p,%p\r\n",m_pVideoRtpSession,m_pAudioRtpSession);
        return iPacketNum;
    }
    switch(m_ptFrame->eEncType)
    {
        case MEDIA_ENCODE_TYPE_H264:
        {
            iRtpPacketType = RTP_PACKET_TYPE_H264;
            break;
        }
        case MEDIA_ENCODE_TYPE_H265:
        {
            iRtpPacketType = RTP_PACKET_TYPE_H265;
            break;
        }
        case MEDIA_ENCODE_TYPE_AAC:
        {
            iRtpPacketType = RTP_PACKET_TYPE_AAC;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711A:
        {
            iRtpPacketType = RTP_PACKET_TYPE_G711A;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711U:
        {
            iRtpPacketType = RTP_PACKET_TYPE_G711U;
            break;
        }
        default :
        {
            RTP_LOGE("m_ptFrame->eEncType err %d\r\n",m_ptFrame->eEncType);
            return iPacketNum;
        }
    }
    iPacketNum = 0;
    do
    {
        memset(&tRtpPacketParam,0,sizeof(T_RtpPacketParam));
        m_pAudioRtpSession->GetRtpPacketParam(&tRtpPacketParam);
        tRtpPacketParam.dwTimestamp = m_ptFrame->dwTimeStamp*m_ptFrame->dwSampleRate/1000;
        if(MEDIA_FRAME_TYPE_AUDIO_FRAME == m_ptFrame->eFrameType)
        {
            iRet=m_AudioRtpPacket.Packet(&tRtpPacketParam,m_ptFrame->pbFrameStartPos,m_ptFrame->iFrameLen,
            &o_ppbPacketBuf[iPacketNum],i_iPacketBufMaxNum-iPacketNum,&o_aiEveryPacketLen[iPacketNum],iRtpPacketType);
            m_pAudioRtpSession->SetRtpPacketParam(&tRtpPacketParam);
            if(iRet<=0 || iPacketNum+iRet>i_iPacketBufMaxNum)
            {
                RTP_LOGE("m_AudioRtpPacket->Packet err %d %d %d\r\n",iRet,iPacketNum,i_iPacketBufMaxNum);
                iPacketNum = -1;
                break;
            }
            iPacketNum+=iRet;
            break;
        }
        memset(&tRtpPacketParam,0,sizeof(T_RtpPacketParam));
        m_pVideoRtpSession->GetRtpPacketParam(&tRtpPacketParam);
        if (0 == m_dwLastTimestamp)
        {
            dwDiffTimestamp = 0;
        }
        else
        {
            dwDiffTimestamp = m_ptFrame->dwTimeStamp - m_dwLastTimestamp;
        }////ʱ����ĵ�λ��1/VIDEO_H264_SAMPLE_RATE(s),Ƶ�ʵĵ���
        m_dwLastTimestamp = m_ptFrame->dwTimeStamp;//��ȻҲ����ֱ����m_tMediaFrameParam.dwTimeStamp
        //tRtpPacketParam.dwTimestamp += dwDiffTimestamp*m_ptFrame->dwSampleRate/1000;//��������Ŀ������rtp��ʱ�����0��ʼ��
        tRtpPacketParam.dwTimestamp = m_ptFrame->dwTimeStamp*m_ptFrame->dwSampleRate/1000;//ʱ������ⲿ����
        pbNaluStartPos = m_ptFrame->pbFrameStartPos;
        dwNaluOffset = 0;
        if (m_ptFrame->dwNaluCount > MAX_NALU_CNT_ONE_FRAME)
        {
            RTP_LOGE("m_ptFrame->dwNaluCount err %d ,%d\r\n",m_ptFrame->dwNaluCount,m_ptFrame->iFrameLen);
            break;
        }
        //RTP_LOGI("m_ptMediaFrameParam->dwNaluCount %d iFrameLen %d dwTimeStamp%d\r\n",m_ptFrame->dwNaluCount,m_ptFrame->iFrameLen,m_ptFrame->dwTimeStamp);
        for(i=0;i<m_ptFrame->dwNaluCount;i++)
        {
            iRet=m_VideoRtpPacket.Packet(&tRtpPacketParam,m_ptFrame->atNaluInfo[i].pbData,m_ptFrame->atNaluInfo[i].dwDataLen,
            &o_ppbPacketBuf[iPacketNum],i_iPacketBufMaxNum-iPacketNum,&o_aiEveryPacketLen[iPacketNum],iRtpPacketType);
            m_pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
            if(iRet<=0 || iPacketNum+iRet>i_iPacketBufMaxNum)
            {
                RTP_LOGE("m_VideoRtpPacket->Packet err %d %d %d\r\n",iRet,iPacketNum,i_iPacketBufMaxNum);
                iPacketNum = -1;
                break;
            }
            iPacketNum+=iRet;
            //pbNaluStartPos = m_ptFrame->pbFrameStartPos +m_ptFrame->adwNaluEndOffset[i];
            //dwNaluOffset =m_ptFrame->adwNaluEndOffset[i];
        }
    }while(0);

    //RTP_LOGI("iPacketNum %d ,m_ptMediaFrameParam->dwNaluCount %d eFrameType %d iFrameLen %d dwTimeStamp%d\r\n",iPacketNum,m_ptFrame->dwNaluCount,
    //m_ptFrame->eFrameType,m_ptFrame->iFrameLen,m_ptFrame->dwTimeStamp);
    return iPacketNum;
}
/*****************************************************************************
-Fuction		: ParseRtpPacket
-Description	: GetFrame������ParseRtpPacket�������޸�ptFrame->iFrameBufLen��
ptFrame->iFrameProcessedLen��ص��߼���ɹر�(��,����alexa��Ƶ���ȷ��͹���ֻ��80���ȵ�����)
�����յĲ���֡��������Ϊ������һ֡�Ĺ���(��֡-->һ֡)
-Input			: 
-Output 		: 
-Return 		: <0 err,0 need more data,>0 success
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::ParseRtpPacket(unsigned char *i_pbPacketBuf,int i_iPacketLen,T_MediaFrameInfo *o_ptFrame)
{
    int iRet = -1;
    T_RtpPacketParam tParam;
    int iFrameBufLen=0;
    unsigned char bNaluType=0;
    E_MediaFrameType eLastFrameType=MEDIA_FRAME_TYPE_UNKNOW;
    int iFrameReaminLen=0;
    
    if(NULL == i_pbPacketBuf ||NULL == o_ptFrame)
    {
        RTP_LOGE("GetRtpPackets NULL%d\r\n",i_iPacketLen);
        return iRet;
    }
    
    memset(&tParam,0,sizeof(T_RtpPacketParam));
    //o_ptFrame->pbFrameStartPos=o_ptFrame->pbFrameBuf;
    //o_ptFrame->iFrameLen=0;
    if(o_ptFrame->iFrameBufLen>0)
    {//����ж�֡��Ҫ�ϲ�Ϊ(����Ҫ���)һ֡,
        eLastFrameType=o_ptFrame->eFrameType;//Ҫ�ж�ǰ��֡��ͬһ���͵�֡
        iFrameReaminLen=o_ptFrame->iFrameBufLen;//����rtp������������⣬����rtp�������ڲ�ȥ����,�Ա�֤���ص�֡��������һ֡��������
    }
    iRet=m_RtpParse.Parse(i_pbPacketBuf,i_iPacketLen,&tParam,o_ptFrame->pbFrameBuf+o_ptFrame->iFrameBufLen,&iFrameBufLen,o_ptFrame->iFrameBufMaxLen);
    if(iRet<0)
    {
        RTP_LOGE("m_RtpParse.Parse err%d\r\n",iRet);
        return iRet;
    }
    if(iFrameBufLen<=0)
    {
        return 0;//0 need more data
    }
    o_ptFrame->iFrameBufLen+=iFrameBufLen;//����һ֡ 10 ms 80���ȵ�֡�ϲ���160��֡��(����㴦��Ҳ����)
    switch (tParam.ePacketType)
    {
        case RTP_PACKET_TYPE_G711A:
        {
            o_ptFrame->eEncType=MEDIA_ENCODE_TYPE_G711A;
            o_ptFrame->dwTimeStamp=tParam.dwTimestamp;///8;//*1000/8000�ȹ̶��������Ż�Ϊ�ɱ�(��㴦��)
            o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_AUDIO_FRAME;
            iRet = iFrameBufLen;
            break;
        }
        case RTP_PACKET_TYPE_G711U:
        {
            o_ptFrame->eEncType=MEDIA_ENCODE_TYPE_G711U;
            o_ptFrame->dwTimeStamp=tParam.dwTimestamp;///8;
            o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_AUDIO_FRAME;
            iRet = iFrameBufLen;
            break;
        }
        case RTP_PACKET_TYPE_H264:
        {
            o_ptFrame->eEncType=MEDIA_ENCODE_TYPE_H264;
            o_ptFrame->dwTimeStamp=tParam.dwTimestamp;///90;//*1000/90000 ��㴦��
            switch(o_ptFrame->pbFrameBuf[4] & 0x1f)
            {
                case 0x5:
                case 0x7: //sps
                case 0x8://pps
                {
                    o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
                    break;
                }
                case 0x1:
                {
                    o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_P_FRAME;
                    break;
                }
                default:
                {
                    RTP_LOGE("o_ptFrame->pbFrameBuf[4] %x\r\n",o_ptFrame->pbFrameBuf[4]);
                    o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_UNKNOW;
                    break;
                }
            }
            iRet = iFrameBufLen;
            break;
        }
        case RTP_PACKET_TYPE_H265:
        {
            o_ptFrame->eEncType=MEDIA_ENCODE_TYPE_H265;
            o_ptFrame->dwTimeStamp=tParam.dwTimestamp;///90;//*1000/90000 ��㴦��
            bNaluType=(o_ptFrame->pbFrameBuf[4] & 0x7E)>>1;
            if(bNaluType >= 0 && bNaluType <= 9)// p slice Ƭ
            {
                o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_P_FRAME;
            }
            else if(bNaluType >= 16 && bNaluType <= 21)// IRAP ��ͬ��i֡
            {
                o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
            }
            else if(bNaluType == 32)//VPS
            {
                o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
            }
            else if(bNaluType == 33)//SPS
            {
                o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
            }
            else if(bNaluType == 34)//PPS
            {
                o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
            }
            else
            {
                RTP_LOGE("o_ptFrame->pbFrameBuf[4] %x\r\n",o_ptFrame->pbFrameBuf[4]);
                o_ptFrame->eFrameType=MEDIA_FRAME_TYPE_UNKNOW;
            }
            //RTP_LOGD("RTP_PACKET_TYPE_H265 %d iFrameBufLen %d,%#x\r\n",bNaluType,iFrameBufLen,o_ptFrame->pbFrameBuf[4]);
            iRet = iFrameBufLen;
            break;
        }
        default :
        {
            iRet = iFrameBufLen;
            RTP_LOGE("ParseRtpPacket.ePacketType err %d,%d\r\n",tParam.ePacketType,iFrameBufLen);
            break;
        }
    }
    if(iFrameReaminLen>0)
    {
        if((MEDIA_FRAME_TYPE_AUDIO_FRAME==eLastFrameType && eLastFrameType!=o_ptFrame->eFrameType)
        ||((MEDIA_FRAME_TYPE_VIDEO_I_FRAME==eLastFrameType||MEDIA_FRAME_TYPE_VIDEO_P_FRAME==eLastFrameType||MEDIA_FRAME_TYPE_VIDEO_B_FRAME==eLastFrameType)&&
        (MEDIA_FRAME_TYPE_VIDEO_I_FRAME!=o_ptFrame->eFrameType&&MEDIA_FRAME_TYPE_VIDEO_P_FRAME!=o_ptFrame->eFrameType&&MEDIA_FRAME_TYPE_VIDEO_B_FRAME!=o_ptFrame->eFrameType))
        ||(MEDIA_FRAME_TYPE_UNKNOW!=eLastFrameType))
        {
            RTP_LOGW("eLastFrameType!=o_ptFrame->eFrameType warn %d,%d,iFrameReaminLen%d\r\n",eLastFrameType,o_ptFrame->eFrameType,iFrameReaminLen);
            o_ptFrame->iFrameBufLen-=iFrameReaminLen;//����ǰ����Ҫ�����֡
            memmove(o_ptFrame->pbFrameBuf,o_ptFrame->pbFrameBuf+iFrameReaminLen,o_ptFrame->iFrameBufLen);
        }
    }
    return iRet;
}

/*****************************************************************************
-Fuction		: Rtp
-Description	: //0 �� ,1 ��
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::IsRtp(char *buf, int size) 
{
    if (size < 2) 
    {
        return 0;
    }
    RtpHeader *header = (RtpHeader *)buf;
    //RTP_LOGI("IsRtp header->PayloadType %d\r\n",header->PayloadType);
    if(((header->PayloadType < 64) || (header->PayloadType >= 96)) && header->Version == 2)
        return 1;
    return 0;
}

/*****************************************************************************
-Fuction		: Rtp
-Description	: //0 �� ,1 ��
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::IsRtcp(char *buf, int size) 
{
    if (size < 2) 
    {
        return 0;
    }
    RtpHeader *header = (RtpHeader *)buf;
    //RTP_LOGI("IsRtcp header->PayloadType %d\r\n",header->PayloadType);
    if((header->PayloadType >= 64) && (header->PayloadType < 96))
        return 1;
    return 0;
}

