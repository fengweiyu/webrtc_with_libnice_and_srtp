/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FMP4HandleInterface.cpp
* Description		: 	FMP4HandleInterface operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "MediaAdapter.h"
#include "FMP4HandleInterface.h"
#include <string.h>
#include <iostream>


using std::cout;//需要<iostream>
using std::endl;



char * FMP4HandleInterface::m_strFormatName=(char *)FMP4_MUX_NAME;
/*****************************************************************************
-Fuction		: FlvHandleInterface
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
FMP4HandleInterface::FMP4HandleInterface()
{
}
/*****************************************************************************
-Fuction		: ~FlvHandleInterface
-Description	: ~FlvHandleInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
FMP4HandleInterface::~FMP4HandleInterface()
{
}


/*****************************************************************************
-Fuction		: FlvHandleInterface::Init
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int FMP4HandleInterface::Init(char *i_strPath)
{
    int iRet=FALSE;
	return 0;
}

/*****************************************************************************
-Fuction		: FlvHandleInterface::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int FMP4HandleInterface::GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam)
{
    int iRet=FALSE;

	return iRet;
}
/*****************************************************************************
-Fuction		: FlvHandleInterface::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int FMP4HandleInterface::GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam)
{
    int iRet=FALSE;

    return iRet;
}
/*****************************************************************************
-Fuction		: FlvHandleInterface::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int FMP4HandleInterface::GetMediaInfo(T_MediaInfo *o_ptMediaInfo)
{
    int iRet=FALSE;

    return iRet;
}

/*****************************************************************************
-Fuction        : GetFrame
-Description    : m_ptFrame->iFrameBufLen 必须大于等于一帧数据大小否则会失败
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4HandleInterface::GetFrame(T_MediaFrameInfo *m_ptFrame)
{
    int iRet=FALSE;

    return iRet;
}

/*****************************************************************************
-Fuction        : GetFrame
-Description    : m_ptFrame->iFrameBufLen 必须大于等于一帧数据大小否则会失败
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4HandleInterface::FrameToContainer(T_MediaFrameInfo *i_ptFrame,E_StreamType i_eStreamType,unsigned char * o_pbBuf, unsigned int i_dwMaxBufLen,int *o_piHeaderOffset)
{
    int iRet=FALSE;
    T_Fmp4AnnexbFrameInfo tFmp4FrameInfo;

    if(NULL == i_ptFrame ||NULL == o_pbBuf ||NULL == i_ptFrame->pbFrameStartPos ||i_ptFrame->iFrameLen <= 0)
    {
        FMP4_LOGE("FrameToContainer err NULL\r\n");
        return iRet;
    }

    memset(&tFmp4FrameInfo,0,sizeof(T_Fmp4AnnexbFrameInfo));
    tFmp4FrameInfo.pbFrameStartPos=i_ptFrame->pbFrameStartPos;
    tFmp4FrameInfo.iFrameLen = i_ptFrame->iFrameLen;
    switch(i_ptFrame->eFrameType)
    {
        case MEDIA_FRAME_TYPE_VIDEO_I_FRAME :
        {
            tFmp4FrameInfo.eFrameType = FMP4_VIDEO_KEY_FRAME;
            break;
        }
        case MEDIA_FRAME_TYPE_VIDEO_P_FRAME :
        case MEDIA_FRAME_TYPE_VIDEO_B_FRAME :
        {
            tFmp4FrameInfo.eFrameType = FMP4_VIDEO_INNER_FRAME;
            break;
        }
        case MEDIA_FRAME_TYPE_AUDIO_FRAME :
        {
            tFmp4FrameInfo.eFrameType = FMP4_AUDIO_FRAME;
            break;
        }
        default :
        {
            FMP4_LOGE("FrameToContainer i_ptFrame->eFrameType err%d\r\n",i_ptFrame->eFrameType);
            return iRet;
        }
    }
    switch(i_ptFrame->eEncType)
    {
        case MEDIA_ENCODE_TYPE_H264 :
        {
            tFmp4FrameInfo.eEncType = FMP4_ENC_H264;
            break;
        }
        case MEDIA_ENCODE_TYPE_H265 :
        {
            tFmp4FrameInfo.eEncType = FMP4_ENC_H265;
            break;
        }
        case MEDIA_ENCODE_TYPE_AAC :
        {
            tFmp4FrameInfo.eEncType = FMP4_ENC_AAC;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711A :
        {
            tFmp4FrameInfo.eEncType = FMP4_ENC_G711A;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711U :
        {
            tFmp4FrameInfo.eEncType = FMP4_ENC_G711U;
            break;
        }
        default :
        {
            FMP4_LOGE("FrameToContainer i_ptFrame->eEncType err%d\r\n",i_ptFrame->eEncType);
            return iRet;
        }
    }
    tFmp4FrameInfo.ddwTimeStamp=i_ptFrame->dwTimeStamp;
    tFmp4FrameInfo.ddwSampleRate=i_ptFrame->dwSampleRate;
    
    tFmp4FrameInfo.dwNaluCount=i_ptFrame->dwNaluCount;
    memcpy(tFmp4FrameInfo.atNaluInfo,i_ptFrame->atNaluInfo,sizeof(tFmp4FrameInfo.atNaluInfo));//
    tFmp4FrameInfo.tVideoEncParam.dwHeight= i_ptFrame->dwHeight;
    tFmp4FrameInfo.tVideoEncParam.dwWidth= i_ptFrame->dwWidth;
    tFmp4FrameInfo.tVideoEncParam.iSizeOfSPS= i_ptFrame->tVideoEncodeParam.iSizeOfSPS;
    tFmp4FrameInfo.tVideoEncParam.iSizeOfPPS= i_ptFrame->tVideoEncodeParam.iSizeOfPPS;
    tFmp4FrameInfo.tVideoEncParam.iSizeOfVPS= i_ptFrame->tVideoEncodeParam.iSizeOfVPS;
    memcpy(tFmp4FrameInfo.tVideoEncParam.abSPS,i_ptFrame->tVideoEncodeParam.abSPS,sizeof(tFmp4FrameInfo.tVideoEncParam.abSPS));//
    memcpy(tFmp4FrameInfo.tVideoEncParam.abPPS,i_ptFrame->tVideoEncodeParam.abPPS,sizeof(tFmp4FrameInfo.tVideoEncParam.abPPS));//
    memcpy(tFmp4FrameInfo.tVideoEncParam.abVPS,i_ptFrame->tVideoEncodeParam.abVPS,sizeof(tFmp4FrameInfo.tVideoEncParam.abVPS));//
    tFmp4FrameInfo.tAudioEncParam.dwChannels= i_ptFrame->tAudioEncodeParam.dwChannels;
    tFmp4FrameInfo.tAudioEncParam.dwBitsPerSample= i_ptFrame->tAudioEncodeParam.dwBitsPerSample;
    
    return m_FMP4Handle.GetMuxData(&tFmp4FrameInfo,o_pbBuf,i_dwMaxBufLen,o_piHeaderOffset);
}



