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
#include <stdlib.h>//还是需要.h
#include <stdio.h>
#include <string.h>
#include <iostream>//不加.h,c++新的头文件
#include "Definition.h"
#include "MediaHandle.h"
#include "rtp_adapter.h"

using std::cout;//需要<iostream>
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
    
    m_pVideoRtpSession = new RtpSession(RTP_PAYLOAD_VIDEO,0);//i_dwSampleRate 暂时用不上先填0 tMediaInfo.dwVideoSampleRate

    iRet=m_RtpPacket.Init(m_ppPackets, i_iMaxPacketNum);
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
Rtp :: Rtp()
{
    m_pMediaHandle = NULL;
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
    }
    if(NULL !=m_pVideoRtpSession)
    {
        delete m_pVideoRtpSession;
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
    return m_RtpPacket.DeInit(m_ppPackets, i_iMaxPacketNum);
}

/*****************************************************************************
-Fuction		: RtpInterface::Init
-Description	: 
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

    m_pVideoRtpSession = new RtpSession(RTP_PAYLOAD_VIDEO,0);//i_dwSampleRate 暂时用不上先填0 tMediaInfo.dwVideoSampleRate

    iRet=m_RtpPacket.Init(m_ppPackets, i_iMaxPacketNum);
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
-Description	: RtpInterface
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,其他表示rtp包个数
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
    tRtpPacketParam.dwTimestamp += dwDiffTimestamp;//这样做的目的是让rtp的时间戳从0开始，
    m_dwLastTimestamp = m_ptMediaFrameParam->dwTimeStamp;//不然也可以直接用m_tMediaFrameParam.dwTimeStamp
    
    pbNaluStartPos = m_ptMediaFrameParam->pbFrameStartPos;
    dwNaluOffset = 0;
    iPacketNum = 0;

    /*if(1== m_ptMediaFrameParam->dwNaluCount && FRAME_TYPE_VIDEO_I_FRAME == m_ptMediaFrameParam->eFrameType)
    {//修正i帧没有pps以及sps情况，前面的参数集不一定适用后面的i帧
        T_VideoEncodeParam tVideoEncodeParam;
        iRet=m_pMediaHandle->GetVideoEncParam(&tVideoEncodeParam);
        iPacketNum+=m_RtpPacket.Packet(&tRtpPacketParam,tVideoEncodeParam.abSPS,tVideoEncodeParam.iSizeOfSPS,&o_ppbPacketBuf[iPacketNum],&o_aiEveryPacketLen[iPacketNum]);
        m_pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
        iPacketNum+=m_RtpPacket.Packet(&tRtpPacketParam,tVideoEncodeParam.abPPS,tVideoEncodeParam.iSizeOfPPS,&o_ppbPacketBuf[iPacketNum],&o_aiEveryPacketLen[iPacketNum]);
        m_pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
    }*/
    
    for(i=0;i<m_ptMediaFrameParam->dwNaluCount;i++)
    {
        iPacketNum+=m_RtpPacket.Packet(&tRtpPacketParam,pbNaluStartPos,m_ptMediaFrameParam->a_dwNaluEndOffset[i]-dwNaluOffset,&o_ppbPacketBuf[iPacketNum],&o_aiEveryPacketLen[iPacketNum]);
        m_pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
        if(iPacketNum<=0 || iPacketNum>i_iPacketBufMaxNum)
        {
            cout<<"m_pRtpPacket->Packet err"<<iPacketNum<<endl;
            iPacketNum =-1;
            return iPacketNum;
        }

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
unsigned int Rtp::GetSSRC()
{
    T_RtpPacketParam tRtpPacketParam;
    
    memset(&tRtpPacketParam,0,sizeof(T_RtpPacketParam));
    if(NULL !=m_pVideoRtpSession)
    {
        m_pVideoRtpSession->GetRtpPacketParam(&tRtpPacketParam);
    }
    
	return tRtpPacketParam.dwSSRC;
}

/*****************************************************************************
-Fuction		: GetRtpPackets
-Description	: GetRtpPackets
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,其他表示rtp包个数
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
    
    if(NULL == o_ppbPacketBuf ||NULL == o_aiEveryPacketLen ||NULL == m_pMediaHandle )
    {
        RTP_LOGE("GetRtpPackets NULL\r\n");
        return iPacketNum;
    }
    
    iRet=m_pMediaHandle->GetFrame(m_ptFrame);
    if(FALSE == iRet)
    {
        RTP_LOGE("GetFrame err \r\n");
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
        dwDiffTimestamp = m_ptFrame->dwTimeStamp - m_dwLastTimestamp;
    }
    tRtpPacketParam.dwTimestamp += dwDiffTimestamp;//这样做的目的是让rtp的时间戳从0开始，
    m_dwLastTimestamp = m_ptFrame->dwTimeStamp;//不然也可以直接用m_tMediaFrameParam.dwTimeStamp
    
    pbNaluStartPos = m_ptFrame->pbFrameStartPos;
    dwNaluOffset = 0;
    iPacketNum = 0;
    if (m_ptFrame->dwNaluCount > MAX_NALU_CNT_ONE_FRAME)
    {
        RTP_LOGE("m_ptFrame->dwNaluCount err %d ,%d\r\n",m_ptFrame->dwNaluCount,m_ptFrame->iFrameLen);
        return iPacketNum;
    }
    //RTP_LOGI("m_ptMediaFrameParam->dwNaluCount %d iFrameLen %d dwTimeStamp%d\r\n",m_ptFrame->dwNaluCount,m_ptFrame->iFrameLen,m_ptFrame->dwTimeStamp);
    for(i=0;i<m_ptFrame->dwNaluCount;i++)
    {
        iPacketNum+=m_RtpPacket.Packet(&tRtpPacketParam,pbNaluStartPos,m_ptFrame->adwNaluEndOffset[i]-dwNaluOffset,&o_ppbPacketBuf[iPacketNum],&o_aiEveryPacketLen[iPacketNum]);
        m_pVideoRtpSession->SetRtpPacketParam(&tRtpPacketParam);
        if(iPacketNum<=0 || iPacketNum>i_iPacketBufMaxNum)
        {
            RTP_LOGE("m_pRtpPacket->Packet  err %d \r\n",iPacketNum);
            iPacketNum = -1;
            return iPacketNum;
        }

        pbNaluStartPos = m_ptFrame->pbFrameStartPos +m_ptFrame->adwNaluEndOffset[i];
        dwNaluOffset =m_ptFrame->adwNaluEndOffset[i];
    }
    RTP_LOGI("iPacketNum %d ,m_ptMediaFrameParam->dwNaluCount %d eFrameType %d iFrameLen %d dwTimeStamp%d\r\n",iPacketNum,m_ptFrame->dwNaluCount,
    m_ptFrame->eFrameType,m_ptFrame->iFrameLen,m_ptFrame->dwTimeStamp);
    return iPacketNum;
}

