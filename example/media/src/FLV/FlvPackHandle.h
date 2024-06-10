/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FlvPackHandle.h
* Description		: 	FlvPackHandle operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef FLV_PACK_HANDLE_H
#define FLV_PACK_HANDLE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdint.h>
#include "MediaHandle.h"
#include "FlvHandle.h"

using std::string;




/*****************************************************************************
-Class			: FlvPackHandle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class FlvPackHandle : public FlvHandle
{
public:
    FlvPackHandle(int i_iEnhancedFlvFlag);
    ~FlvPackHandle();
    int GetMuxData(T_MediaFrameInfo * i_ptFrameInfo,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);
    
private:   
    int CreateHeader(unsigned char* o_pbBuf,unsigned int i_dwMaxLen);
    int CreateTag(T_MediaFrameInfo * i_ptFrameInfo,unsigned char* o_pbBuf,unsigned int i_dwMaxLen);

    int CreateFlvHeader(T_FlvHeader * i_ptFlvHeader,unsigned char* o_pbBuf,unsigned int i_dwMaxLen);
    int CreateFlvTagHeader(T_FlvTagHeader * i_ptFlvTagHeader,unsigned char* o_pbBuf,unsigned int i_dwMaxLen);
    int GenerateVideoData(T_MediaFrameInfo * i_ptFrameInfo,int i_iIsAvcSeqHeader,unsigned char *o_pbVideoData,int i_iMaxVideoData);
    int GenerateAudioData(T_MediaFrameInfo * i_ptAudioInfo,int i_iIsAACSeqHeader,unsigned char *o_pbAudioData,int i_iMaxAudioData);

    int GenerateVideoDataH264(T_MediaFrameInfo * i_ptFrameInfo,int i_iIsAvcSeqHeader,unsigned char *o_pbVideoData,int i_iMaxVideoData);
    int GenerateVideoDataH265(T_MediaFrameInfo * i_ptFrameInfo,int i_iIsAvcSeqHeader,unsigned char *o_pbVideoData,int i_iMaxVideoData);
    int AnnexbToH265Extradata(T_MediaFrameInfo * i_ptFrameInfo,T_FlvH265Extradata *o_ptFlvH265Extradata);
    int VpsToH265Extradata(unsigned char *i_pbVpsData,unsigned short i_wVpsLen,T_FlvH265Extradata *o_ptH265Extradata);
    unsigned char CreateAudioDataTagHeader(T_MediaFrameInfo * i_ptAudioParam);
    int CreateAudioSpecCfgAAC(unsigned int i_dwFrequency,unsigned int i_dwChannels,unsigned char *o_pbAudioData);




    int m_iEnhancedFlvFlag;//0 否，1是
    unsigned char *m_pbFrameBuf;//缓冲区
    int m_iFrameBufMaxLen;//缓冲区总大小
    int m_iAudioSeqHeaderSended;//0 否，1是

    int m_iHeaderCreatedFlag;//flv头已打包过的标志，0 否，1是
    int m_iFindedKeyFrame;//0 否，1是
};












#endif
