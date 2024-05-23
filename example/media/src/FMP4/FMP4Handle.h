/*****************************************************************************
* Copyright (C) 2023-2028 HANGZHOU JFTECH CO., LTD. All rights reserved.
------------------------------------------------------------------------------
* File Module       :   FMP4Handle.h
* Description       :   FMP4Handle operation center
* Created           :   2023.11.21.
* Author            :   Yu Weifeng
* Function List     :   
* Last Modified     :   
* History           :   
******************************************************************************/
#ifndef FMP4_HANDLE_H
#define FMP4_HANDLE_H

#include "FMP4.h"

#define FMP4_SPS_MAX_SIZE 128 //VIDEO_SPS_MAX_SIZE
#define FMP4_PPS_MAX_SIZE 64 //VIDEO_PPS_MAX_SIZE
#define FMP4_VPS_MAX_SIZE 256 //VIDEO_VPS_MAX_SIZE


typedef struct Fmp4HevcDecoderConfigurationRecord
{
    unsigned char  configurationVersion;    // 1-only
    unsigned char  general_profile_space;   // 2bit,[0,3]
    unsigned char  general_tier_flag;       // 1bit,[0,1]
    unsigned char  general_profile_idc; // 5bit,[0,31]
    unsigned int general_profile_compatibility_flags;//uint32_t
    uint64_t general_constraint_indicator_flags;//uint64_t
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
}T_Fmp4H265ExtraData;

typedef struct Fmp4NaluInfo
{
    unsigned char *pbData;////pbFrameStartPos的偏移地址，包含00 00 00 01
    unsigned int dwDataLen;
}T_Fmp4NaluInfo;

typedef struct Fmp4AnnexbVideoEncParam
{
    unsigned int dwWidth;//
    unsigned int dwHeight;//
	unsigned char abSPS[FMP4_SPS_MAX_SIZE];
	int iSizeOfSPS;
	unsigned char abPPS[FMP4_PPS_MAX_SIZE];
	int iSizeOfPPS;
	unsigned char abVPS[FMP4_VPS_MAX_SIZE];
	int iSizeOfVPS;
}T_Fmp4AnnexbVideoEncParam;

typedef struct Fmp4AnnexbFrameInfo
{
    unsigned char *pbFrameStartPos;//包含00 00 00 01 //音频帧包含aac 7字节头，ADTS头
    int iFrameLen;

    E_FMP4_FRAME_TYPE eFrameType;
    E_FMP4_ENC_TYPE eEncType;
    uint64_t ddwTimeStamp;//ms
    uint64_t ddwSampleRate;//dwSamplesPerSecond
    
    unsigned int dwNaluCount;//包括sps,pps等参数集对应的nalu
    T_Fmp4NaluInfo atNaluInfo[12];//存在一帧图像切片成多个(nalu)的情况
    T_Fmp4AnnexbVideoEncParam tVideoEncParam;
    T_Fmp4AudioEncParam tAudioEncParam;
}T_Fmp4AnnexbFrameInfo;

/*****************************************************************************
-Class          : FMP4
-Description    : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4Handle
{
public:
    FMP4Handle();
    virtual ~FMP4Handle();

    
    int GetMuxData(T_Fmp4AnnexbFrameInfo *i_ptFmp4FrameInfo,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen,int *o_piHeaderOffset=NULL,int i_iForcePack=0);
    int GetMuxHeader(unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);

private:
    int SaveFrame(T_Fmp4AnnexbFrameInfo *i_ptFmp4FrameInfo);
    int DelAllFrame();
    
    int CreateAudioSpecCfgAAC(unsigned int i_dwFrequency,unsigned int i_dwChannels,unsigned char *o_pbAudioData);
    int GenerateVideoExtraData(E_FMP4_ENC_TYPE eEncType,T_Fmp4AnnexbVideoEncParam *i_ptVideoEncParam,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);
    int GenerateH264ExtraData(T_Fmp4AnnexbVideoEncParam *i_ptVideoEncParam,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);
    int GenerateH265ExtraData(T_Fmp4AnnexbVideoEncParam *i_ptVideoEncParam,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);
    int AnnexbToH265Extradata(T_Fmp4AnnexbVideoEncParam *i_ptVideoEncParam,T_Fmp4H265ExtraData *o_ptH265Extradata);
    int VpsToH265Extradata(unsigned char *i_pbVpsData,unsigned short i_wVpsLen,T_Fmp4H265ExtraData *o_ptH265Extradata);
    int SpsToH265Extradata(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_Fmp4H265ExtraData *o_ptH265Extradata);
    int DecodeEBSP(unsigned char* nalu, int bytes, unsigned char* sodb);
    int HevcProfileTierLevel(unsigned char* nalu, int bytes, unsigned char maxNumSubLayersMinus1,T_Fmp4H265ExtraData* hevc);
    unsigned int H264ReadBitByUE(unsigned char* data, int bytes, int* offset);
        
    unsigned char *m_pbMediaData;
    int m_iCurMediaDataLen;

    unsigned char *m_pbFmp4Header;
    int m_iFmp4HeaderLen;
    int m_iFragSeq;//fmp4片段序列号
    int m_iHeaderCreatedFlag;//fmp4头已打包过的标志，0 否，1是
    FMP4 m_FMP4; 
    list<T_Fmp4FrameInfo> m_FMP4MediaList;
    int m_iFindedKeyFrame;//0 否，1是
};



















#endif
