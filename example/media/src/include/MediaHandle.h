/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	MediaHandle.h
* Description		: 	MediaHandle operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef MEDIA_HANDLE_H
#define MEDIA_HANDLE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>

using std::string;


#define FRAME_BUFFER_MAX_SIZE               (2*1024*1024)   //2m
#define MAX_NALU_CNT_ONE_FRAME              12
#define VIDEO_ENC_PARAM_BUF_MAX_LEN     	64
#define VIDEO_SPS_MAX_SIZE 128
#define VIDEO_PPS_MAX_SIZE 64
#define VIDEO_VPS_MAX_SIZE 256

typedef enum
{
	STREAM_TYPE_UNKNOW = 0,
    STREAM_TYPE_VIDEO_STREAM,
    STREAM_TYPE_AUDIO_STREAM,
    STREAM_TYPE_MUX_STREAM,//包含音视频两路裸流
    STREAM_TYPE_FLV_STREAM,
    STREAM_TYPE_ENHANCED_FLV_STREAM,
    STREAM_TYPE_FMP4_STREAM,
}E_StreamType;

typedef enum
{
	MEDIA_ENCODE_TYPE_UNKNOW = 0,
	MEDIA_ENCODE_TYPE_H264,
    MEDIA_ENCODE_TYPE_H265,
    MEDIA_ENCODE_TYPE_VP8,
    MEDIA_ENCODE_TYPE_VP9,
    MEDIA_ENCODE_TYPE_AAC,
    MEDIA_ENCODE_TYPE_G711A,
    MEDIA_ENCODE_TYPE_G711U,
    MEDIA_ENCODE_TYPE_G726,
    MEDIA_ENCODE_TYPE_MP3,
    MEDIA_ENCODE_TYPE_OPUS,
    MEDIA_ENCODE_TYPE_LPCM,// Linear PCM, platform endian
    MEDIA_ENCODE_TYPE_ADPCM,
    MEDIA_ENCODE_TYPE_LLPCM,// Linear PCM, little endian
}E_MediaEncodeType;

typedef enum
{
	MEDIA_FRAME_TYPE_UNKNOW = 0,
    MEDIA_FRAME_TYPE_VIDEO_I_FRAME,
    MEDIA_FRAME_TYPE_VIDEO_P_FRAME,//inner
    MEDIA_FRAME_TYPE_VIDEO_B_FRAME,
    MEDIA_FRAME_TYPE_AUDIO_FRAME,
        
}E_MediaFrameType;


typedef struct MediaInfo
{
    E_StreamType eStreamType;
    E_MediaEncodeType eVideoEncType;
    unsigned int dwVideoSampleRate;
    E_MediaEncodeType eAudioEncType;
    unsigned int dwAudioSampleRate;
}T_MediaInfo;


typedef struct MediaFrameParam
{
    unsigned char *pbFrameBuf;//缓冲区
    int iFrameBufMaxLen;//缓冲区总大小
    int iFrameBufLen;//缓冲区读到数据的总大小
    int iFrameProcessedLen;
    
	//输出1帧数据结果
    unsigned char *pbFrameStartPos;
    int iFrameLen;
    unsigned int dwNaluCount;
    unsigned int a_dwNaluEndOffset[MAX_NALU_CNT_ONE_FRAME];

    E_MediaFrameType eFrameType;
    int iVideoEncType;
    int iAudioEncType;
    unsigned int dwTimeStamp;
}T_MediaFrameParam;

typedef struct AudioEncodeParam
{
    unsigned int dwChannels;
    unsigned int dwBitsPerSample;
}T_AudioEncodeParam;

typedef struct VideoEncodeParam
{
	unsigned char abSPS[VIDEO_SPS_MAX_SIZE];
	int iSizeOfSPS;
	unsigned char abPPS[VIDEO_PPS_MAX_SIZE];
	int iSizeOfPPS;
	unsigned char abVPS[VIDEO_VPS_MAX_SIZE];
	int iSizeOfVPS;
}T_VideoEncodeParam;


typedef struct MediaFrameBufInfo
{
    unsigned char *pbFrameBuf;//缓冲区
    int iFrameBufMaxLen;//缓冲区总大小
    int iFrameBufLen;//缓冲区读到数据的总大小
    int iFrameProcessedLen;
    
    E_StreamType eStreamType;//裸流的帧数据时，这个需要外部赋值然后传入
    E_MediaEncodeType eEncType;//裸流的帧数据时，这个需要外部赋值然后传入
}T_MediaFrameBufInfo;

typedef struct MediaNaluInfo
{
    unsigned char *pbData;////包含00 00 00 01
    unsigned int dwDataLen;
}T_MediaNaluInfo;

typedef struct MediaFrameInfo
{
    unsigned char *pbFrameBuf;//缓冲区
    int iFrameBufMaxLen;//缓冲区总大小
    int iFrameBufLen;//缓冲区读到数据的总大小
    int iFrameProcessedLen;
    
    //裸流的帧数据(数据流,eStreamType需要被赋值为对应的数据流格式)时，下面5个参数都需要外部赋值然后传入
    E_StreamType eStreamType;//解析文件时,eStreamType外部赋值0(表示不是数据流(是文件)),下面4个参数则由内部赋值
    E_MediaEncodeType eEncType;
    unsigned int dwTimeStamp;//ms
    unsigned int dwSampleRate;//dwSamplesPerSecond
    E_MediaFrameType eFrameType;
    unsigned int dwWidth;//
    unsigned int dwHeight;//

	//输出1帧数据结果
    unsigned char *pbFrameStartPos;//包含00 00 00 01
    int iFrameLen;
    unsigned int dwNaluCount;//包括sps,pps等参数集对应的nalu
    T_MediaNaluInfo atNaluInfo[MAX_NALU_CNT_ONE_FRAME];//存在一帧图像切片成多个(nalu)的情况
    unsigned char bNaluLenSize;//avcc形式存储nalu是的naluLen的大小(所占字节数)
    //unsigned int adwNaluEndOffset[MAX_NALU_CNT_ONE_FRAME];
    T_VideoEncodeParam tVideoEncodeParam;
    T_AudioEncodeParam tAudioEncodeParam;
}T_MediaFrameInfo;

/*****************************************************************************
-Class			: MediaHandle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class MediaHandle
{
public:
    MediaHandle();
    virtual ~MediaHandle();
    virtual int Init(char *i_strPath);
    virtual int Init(E_StreamType i_eStreamType,E_MediaEncodeType i_eVideoEncType,E_MediaEncodeType i_eAudioEncType);
    
    virtual int GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam);
    virtual int GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam);
    virtual int GetMediaInfo(T_MediaInfo *o_ptMediaInfo);
    //一个接口可以取代前面3个接口，包含Init GetNextFrame GetVideoEncParam GetMediaInfo 功能
    virtual int GetFrame(T_MediaFrameInfo *m_ptFrame);
    virtual int FrameToContainer(T_MediaFrameInfo *i_ptFrame,E_StreamType i_eStreamType,unsigned char * o_pbBuf, unsigned int i_dwMaxBufLen,int *o_piHeaderOffset=NULL);//
protected:
	T_MediaInfo m_tMediaInfo;
	
private://也可以考虑直接调用本类方法再调用子类方法
    MediaHandle             *m_pMediaHandle;//默认VideoHandle(裸流时，表示视频裸数据处理)
    MediaHandle             *m_pMediaAudioHandle;//音视频两路裸流时,这个会被赋值表示音频裸数据处理

    MediaHandle             *m_pMediaPackHandle;
	FILE                    *m_pMediaFile;
	//unsigned int 			m_dwFileReadOffset;
	
};









#endif
