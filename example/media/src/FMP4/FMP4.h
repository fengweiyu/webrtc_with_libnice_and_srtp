/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       :   FMP4.h
* Description       :   FMP4 operation center
* Created           :   2023.11.21.
* Author            :   Yu Weifeng
* Function List     :   
* Last Modified     :   
* History           :   
******************************************************************************/
#ifndef FMP4_H
#define FMP4_H

#include <stdint.h>
#include <string>
#include <list>
#include <map>
#include "MediaAdapter.h"

using std::string;
using std::list;
using std::map;

#define  FMP4_LOGW     MH_LOGW
#define  FMP4_LOGE     MH_LOGE
#define  FMP4_LOGD     MH_LOGD
#define  FMP4_LOGI     MH_LOGI

#define FMP4_MAX_TRAK_NUM 15

#define Write16BE(p,val) \
do{ \
    p[0] = (unsigned char)((val >> 8) & 0xFF);  \
    p[1] = (unsigned char)((val) & 0xFF);    \
}while(0)

#define Write32BE(p,val) \
do{ \
    p[0] = (unsigned char)((val >> 24) & 0xFF); \
    p[1] = (unsigned char)((val >> 16) & 0xFF); \
    p[2] = (unsigned char)((val >> 8) & 0xFF);  \
    p[3] = (unsigned char)((val) & 0xFF);    \
}while(0)

typedef enum
{
	FMP4_STREAM_TYPE_UNKNOW = 0,
    FMP4_STREAM_TYPE_VIDEO_STREAM,
    FMP4_STREAM_TYPE_AUDIO_STREAM,
    FMP4_STREAM_TYPE_MUX_STREAM,//混合流,包含音视频两路流(两个轨道)
}E_Fmp4StreamType;

typedef enum
{
    FMP4_UNKNOW_FRAME = 0,
    FMP4_VIDEO_KEY_FRAME,
    FMP4_VIDEO_INNER_FRAME,
    FMP4_AUDIO_FRAME,
}E_FMP4_FRAME_TYPE;

typedef enum
{
    FMP4_UNKNOW_ENC_TYPE = 0,
    FMP4_ENC_H264,
    FMP4_ENC_H265,
    FMP4_ENC_H266,
    FMP4_ENC_VP8,
    FMP4_ENC_VP9,// https://www.webmproject.org/vp9/mp4/
    FMP4_ENC_VP10,
    FMP4_ENC_AV1,// https://aomediacodec.github.io/av1-isobmff 
    FMP4_ENC_VC1,
    FMP4_ENC_JPEG,
    FMP4_ENC_PNG,
    FMP4_ENC_G711A,// G711 A-law
    FMP4_ENC_G711U,// G711 mu-law
    FMP4_ENC_AAC,
    FMP4_ENC_OPUS,
    FMP4_ENC_LPCM,// Linear PCM, platform endian
    FMP4_ENC_ADPCM,
    FMP4_ENC_MP3,
    FMP4_ENC_LLPCM,// Linear PCM, little endian
}E_FMP4_ENC_TYPE;
typedef struct Fmp4VideoEncParam
{
    unsigned int dwWidth;//
    unsigned int dwHeight;//
}T_Fmp4VideoEncParam;
typedef struct Fmp4AudioEncParam
{
    unsigned int dwChannels;
    unsigned int dwBitsPerSample;
}T_Fmp4AudioEncParam;

typedef struct Fmp4FrameInfo
{
    unsigned char *pbFrameStartPos;//avcc格式 :长度+数据(不带0001)
    int iFrameLen;
    unsigned int dwNaluLenSize;//avcc形式存储nalu是的naluLen的大小(所占字节数。默认4字节)

    E_FMP4_FRAME_TYPE eFrameType;
    E_FMP4_ENC_TYPE eEncType;
    uint64_t ddwTimeStamp;//ms
    uint64_t ddwSampleRate;//dwSamplesPerSecond
    
    T_Fmp4VideoEncParam tVideoEncParam;
    T_Fmp4AudioEncParam tAudioEncParam;
	unsigned char abEncExtraData[512];
	int iEncExtraDataLen;
}T_Fmp4FrameInfo;

typedef struct Fmp4MediaInfo
{
    T_Fmp4FrameInfo *aptFrame;
    int iFrameNum;
}T_Fmp4MediaInfo;

/*****************************************************************************
-Class          : FMP4
-Description    : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4
{
public:
    FMP4(E_Fmp4StreamType eFmp4StreamType);//具体什么流信息,在拿到帧信息之前就知道,
    FMP4();//后续可以考虑放到设置帧信息的接口里去设置轨道个数，但可能会造成moov与moof不一致
    virtual ~FMP4();

    
    int CreateHeader(list<T_Fmp4FrameInfo> * i_pFMP4Media,unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen);
    int CreateSegment(list<T_Fmp4FrameInfo> * i_pFMP4Media,unsigned int i_dwSeqNum,unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen);


private:
	list<T_Fmp4FrameInfo> * m_pFMP4Media;
	
    //T_Fmp4MediaInfo m_tMedia;
    unsigned int m_adwTrakHandlerType[FMP4_MAX_TRAK_NUM];
    int m_iCurTrakNum;
    int m_iFindFirstFrame;//0 否1是
    unsigned int m_dwSegmentBaseDecTime;
};



















#endif
