/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FlvHandle.h
* Description		: 	FlvHandle operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef FLV_HANDLE_H
#define FLV_HANDLE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdint.h>
#include "MediaHandle.h"

using std::string;


//#define FLV_MUX_NAME        ".flv"


#define FLV_HEADER_LEN		    9	// DataOffset included
#define FLV_PRE_TAG_LEN	        4	// previous tag size
#define FLV_TAG_HEADER_LEN	    11	// StreamID included
#define FLV_TAG_AUDIO_TYPE		8
#define FLV_TAG_VIDEO_TYPE		9
#define FLV_TAG_SCRIPT_TYPE		18

typedef struct FlvHeader
{
	unsigned char FLV[3];
	unsigned char bVersion;
	unsigned char bAudio;
	unsigned char bVideo;
	unsigned int dwOffset; // data offset
}T_FlvHeader;
typedef struct FlvTagHeader
{
	unsigned char bFilter; // 0-No pre-processing required
	unsigned char bType; // 8-audio, 9-video, 18-script data
	unsigned int dwSize; // data size
	unsigned int dwTimestamp;
	unsigned int dwStreamId;
}T_FlvTagHeader;

typedef struct FlvTag
{
	T_FlvTagHeader tTagHeader; 
	unsigned char *pbTagData;
	unsigned int dwDataMaxLen; //如果等于0,则pbTagData会被赋值为输入缓存数据的偏移地址
	unsigned int dwDataCurLen; //否则，会把数据拷贝到pbTagData指向的缓存
}T_FlvTag;

typedef struct FlvHevcDecoderConfigurationRecord
{
    unsigned char  configurationVersion;    // 1-only
    unsigned char  general_profile_space;   // 2bit,[0,3]
    unsigned char  general_tier_flag;       // 1bit,[0,1]
    unsigned char  general_profile_idc; // 5bit,[0,31]
    unsigned int general_profile_compatibility_flags;//uint32_t
    uint64_t general_constraint_indicator_flags;//uint64_t , uint64
    unsigned char  general_level_idc;
    unsigned short min_spatial_segmentation_idc;
    unsigned char  parallelismType;     // 2bit,[0,3]
    unsigned char  chromaFormat;            // 2bit,[0,3]
    unsigned char  bitDepthLumaMinus8;  // 3bit,[0,7]
    unsigned char  bitDepthChromaMinus8;    // 3bit,[0,7]
    unsigned short avgFrameRate;
    unsigned char  constantFrameRate;       // 2bit,[0,3]
    unsigned char  numTemporalLayers;       // 3bit,[0,7]
    unsigned char  temporalIdNested;        // 1bit,[0,1]
    unsigned char  lengthSizeMinusOne;  // 2bit,[0,3]

    unsigned char  numOfArrays;

    unsigned int pic_width;
    unsigned int pic_height;
}T_FlvH265Extradata;


/*****************************************************************************
-Class			: H264Handle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class FlvHandle
{
public:
    FlvHandle();
    ~FlvHandle();
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
    int SpsToH265Extradata(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_FlvH265Extradata *o_ptH265Extradata);
    int DecodeEBSP(unsigned char* nalu, int bytes, unsigned char* sodb);
    unsigned int H264ReadBitByUE(unsigned char* data, int bytes, int* offset);
    int HevcProfileTierLevel(unsigned char* nalu, int bytes, unsigned char maxNumSubLayersMinus1,T_FlvH265Extradata* hevc);
    int ParseFlvTagHeader(unsigned char* i_pbBuf,unsigned int i_dwLen,T_FlvTagHeader * o_ptFlvTagHeader);
    int ParseFlvHeader(unsigned char* i_pbBuf,unsigned int i_dwLen,T_FlvHeader * o_ptFlvHeader);
    
    //static char *m_strFormatName;
private:
    unsigned char *m_pbFrameBuf;//缓冲区
    int m_iFrameBufMaxLen;//缓冲区总大小
};












#endif
