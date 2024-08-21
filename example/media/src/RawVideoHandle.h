/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	VideoHandle.h
* Description		: 	VideoHandle operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef RAW_VIDEO_HANDLE_H
#define RAW_VIDEO_HANDLE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "MediaHandle.h"

using std::string;


#define VIDEO_ENC_FORMAT_H264_NAME        ".h264"
#define VIDEO_ENC_FORMAT_H265_NAME        ".h265"

typedef struct RawHevcDecoderConfigurationRecord
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
}T_RawH265Extradata;
/*****************************************************************************
-Class			: RawVideoHandle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RawVideoHandle
{
public:
    RawVideoHandle();
    ~RawVideoHandle();
    int SpsToH264Resolution(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_RawH265Extradata *o_ptH265Extradata);
    int SpsToH265Extradata(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_RawH265Extradata *o_ptH265Extradata);
    int DecodeEBSP(unsigned char* nalu, int bytes, unsigned char* sodb);
    int HevcProfileTierLevel(unsigned char* nalu, int bytes, unsigned char maxNumSubLayersMinus1,T_RawH265Extradata* hevc);
    unsigned int H264ReadBitByUE(unsigned char* data, int bytes, int* offset);
private:
};

/*****************************************************************************
-Class			: H264Handle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class H264Handle : public MediaHandle
{
public:
    H264Handle();
    ~H264Handle();
    virtual int Init(char *i_strPath);
    virtual int GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam);
    virtual int GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam);
    virtual int GetMediaInfo(T_MediaInfo *o_ptMediaInfo);
    
    virtual int GetFrame(T_MediaFrameInfo *m_ptFrame);//
    
    static char *m_strVideoFormatName;
private:
    int SetH264NaluData(unsigned char i_bNaluType,unsigned char i_bStartCodeLen,unsigned char *i_pbNaluData,int i_iNaluDataLen,T_MediaFrameInfo *m_ptFrame);

    int RemoveH264EmulationBytes(unsigned char *o_pbNaluBuf,int i_iMaxNaluBufLen,unsigned char *i_pbNaluBuf,int i_iNaluLen);
	T_VideoEncodeParam      m_tVideoEncodeParam;

	RawVideoHandle m_oRawVideoHandle;
};



/*****************************************************************************
-Class			: H264Handle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class H265Handle : public MediaHandle
{
public:
    H265Handle();
    ~H265Handle();
    virtual int Init(char *i_strPath);
    virtual int GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam);
    virtual int GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam);
    virtual int GetMediaInfo(T_MediaInfo *o_ptMediaInfo);
    
    virtual int GetFrame(T_MediaFrameInfo *m_ptFrame);//
    
    static char *m_strVideoFormatName;
private:
    int SetH265NaluData(unsigned char i_bNaluType,unsigned char i_bStartCodeLen,unsigned char *i_pbNaluData,int i_iNaluDataLen,T_MediaFrameInfo *m_ptFrame);

	T_VideoEncodeParam      m_tVideoEncodeParam;
	
	RawVideoHandle m_oRawVideoHandle;
};



/*****************************************************************************
-Class			: VP9Handle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class VP9Handle : public MediaHandle
{
public:
    VP9Handle();
    ~VP9Handle();
    //int Init(char *i_strPath);


    //static char *m_strVideoFormatName;
private:
    T_VideoEncodeParam      m_tVideoEncodeParam;
};









#endif
