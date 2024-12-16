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
#include <stdint.h>
#include <string>
#include <list>
#include "MediaHandle.h"
#include "FlvHandle.h"

using std::string;
using std::list;



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
    int SaveFrame(T_MediaFrameInfo *i_ptFrameInfo);
    int GetMediaFrame(T_MediaFrameInfo *o_ptFrameInfo);
        
    int CreateHeader(unsigned char* o_pbBuf,unsigned int i_dwMaxLen,unsigned char i_bVideo,unsigned char i_bAudio);
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




    int m_iEnhancedFlvFlag;//0 ��1��
    unsigned char *m_pbFrameBuf;//������
    int m_iFrameBufMaxLen;//�������ܴ�С
    int m_iAudioSeqHeaderSended;//0 ��1��

    int m_iHeaderCreatedFlag;//flvͷ�Ѵ�����ı�־��0 ��1��
    int m_iFindedKeyFrame;//0 ��1��

    unsigned char *m_pbMediaData;
    int m_iCurMediaDataLen;
    list<T_MediaFrameInfo> m_MediaFrameList;
    int m_iMediaFramePrasedFlag;//����������ϱ�־��0 ��1��
};












#endif
