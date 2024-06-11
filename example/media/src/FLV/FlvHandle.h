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


#define FLV_TO_U32(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#define FLV_VIDEO_ENC_AV1	FLV_TO_U32('a', 'v', '0', '1') //×¢Òâ²»ÊÇavc(h264)
#define FLV_VIDEO_ENC_VP9	FLV_TO_U32('v', 'p', '0', '9') 
#define FLV_VIDEO_ENC_H265	FLV_TO_U32('h', 'v', 'c', '1') //HEVC

typedef enum
{
    FLV_PACKET_TYPE_SEQUENCE_START=0,
    FLV_PACKET_TYPE_CODED_FRAMES,
    FLV_PACKET_TYPE_SEQUENCE_END,
    FLV_PACKET_TYPE_CODED_FRAMES_X,
    FLV_PACKET_TYPE_METADATA,
    FLV_PACKET_TYPE_MPEG2_TS_SEQUENCE_START,
}E_FlvPacketType;


//#define FLV_MUX_NAME        ".flv"
#define BIT(ptr, off) (((ptr)[(off) / 8] >> (7 - ((off) % 8))) & 0x01)


#define FLV_FRAME_BUF_MAX_LEN (1024*1024)


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
-Class			: FlvHandle
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
    int SpsToH265Extradata(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_FlvH265Extradata *o_ptH265Extradata);
    int DecodeEBSP(unsigned char* nalu, int bytes, unsigned char* sodb);
    int HevcProfileTierLevel(unsigned char* nalu, int bytes, unsigned char maxNumSubLayersMinus1,T_FlvH265Extradata* hevc);
    unsigned int H264ReadBitByUE(unsigned char* data, int bytes, int* offset);
private:
};












#endif
