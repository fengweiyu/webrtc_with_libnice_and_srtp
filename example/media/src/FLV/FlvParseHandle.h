/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FlvParseHandle.h
* Description		: 	FlvParseHandle operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef FLV_PARSE_HANDLE_H
#define FLV_PARSE_HANDLE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdint.h>
#include "MediaHandle.h"
#include "FlvHandle.h"

using std::string;


//#define FLV_MUX_NAME        ".flv"


typedef struct FlvTag
{
	T_FlvTagHeader tTagHeader; 
	unsigned char *pbTagData;
	unsigned int dwDataMaxLen; //如果等于0,则pbTagData会被赋值为输入缓存数据的偏移地址
	unsigned int dwDataCurLen; //否则，会把数据拷贝到pbTagData指向的缓存
}T_FlvTag;


/*****************************************************************************
-Class			: FlvParseHandle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class FlvParseHandle : public FlvHandle
{
public:
    FlvParseHandle();
    ~FlvParseHandle();
    int GetFrameData(int i_iDataOffset,T_MediaFrameInfo *m_ptFrame);
private:   
    int GetAudioData(unsigned char *i_pbAudioTag,int i_iTagLen,T_MediaFrameInfo *m_ptFrame,unsigned char *o_pbAudioData,int i_iAudioDataMaxLen);
    int GetVideoData(unsigned char *i_pbVideoTag,int i_iTagLen,T_MediaFrameInfo * m_ptFrameInfo,unsigned char *o_pbVideoData,int i_iVideoDataMaxLen);
    int FlvReadHeader(unsigned char* i_pbBuf,unsigned int i_dwLen);
    int FlvReadTag(unsigned char* i_pbBuf,unsigned int i_dwLen,T_FlvTag * o_ptFlvTag);
    int GetVideoDataNalu(unsigned char *i_pbVideoData,int i_iDataLen,T_MediaFrameInfo * m_ptFrameInfo,unsigned char *o_pbVideoData,int i_iVideoDataMaxLen);
    
    unsigned char ParseAudioDataTagHeader(unsigned char i_bAudioTagHeader,T_MediaFrameInfo *m_ptFrame);
    int AddAdtsHeader(unsigned int i_dwSampleRate,unsigned int i_dwChannels,int i_iAudioRawDataLen,unsigned char *o_pbAudioData,int i_iDataMaxLen);
    int SpsToH264Resolution(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_FlvH265Extradata *o_ptH265Extradata);
    int ParseFlvTagHeader(unsigned char* i_pbBuf,unsigned int i_dwLen,T_FlvTagHeader * o_ptFlvTagHeader);
    int ParseFlvHeader(unsigned char* i_pbBuf,unsigned int i_dwLen,T_FlvHeader * o_ptFlvHeader);
    
    //static char *m_strFormatName;
private:
    unsigned char *m_pbFrameBuf;//缓冲区
    int m_iFrameBufMaxLen;//缓冲区总大小

    int m_iParseStarted;//0 否，1是
};












#endif
