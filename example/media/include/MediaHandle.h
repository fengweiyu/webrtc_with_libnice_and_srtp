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

typedef enum
{
	STREAM_TYPE_UNKNOW = 0,
    STREAM_TYPE_VIDEO_STREAM,
    STREAM_TYPE_AUDIO_STREAM,
    STREAM_TYPE_MUX_STREAM,//��������Ƶ��·����
    STREAM_TYPE_FLV_STREAM,
}E_StreamType;

typedef enum
{
	MEDIA_ENCODE_TYPE_UNKNOW = 0,
	MEDIA_ENCODE_TYPE_H264,
    MEDIA_ENCODE_TYPE_H265,
    MEDIA_ENCODE_TYPE_VP8,
    MEDIA_ENCODE_TYPE_VP9,
    MEDIA_ENCODE_TYPE_AAC,
    MEDIA_ENCODE_TYPE_G711U,
    MEDIA_ENCODE_TYPE_G711A,
    MEDIA_ENCODE_TYPE_G726,
}E_MediaEncodeType;

typedef enum
{
	MEDIA_FRAME_TYPE_UNKNOW = 0,
    MEDIA_FRAME_TYPE_VIDEO_I_FRAME,
    MEDIA_FRAME_TYPE_VIDEO_P_FRAME,
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
    unsigned char *pbFrameBuf;//������
    int iFrameBufMaxLen;//�������ܴ�С
    int iFrameBufLen;//�������������ݵ��ܴ�С
    int iFrameProcessedLen;
    
	//���1֡���ݽ��
    unsigned char *pbFrameStartPos;
    int iFrameLen;
    unsigned int dwNaluCount;
    unsigned int a_dwNaluEndOffset[MAX_NALU_CNT_ONE_FRAME];

    E_MediaFrameType eFrameType;
    int iVideoEncType;
    int iAudioEncType;
    unsigned int dwTimeStamp;
}T_MediaFrameParam;


typedef struct VideoEncodeParam
{
	unsigned char abSPS[VIDEO_ENC_PARAM_BUF_MAX_LEN];
	int iSizeOfSPS;
	unsigned char abPPS[VIDEO_ENC_PARAM_BUF_MAX_LEN];
	int iSizeOfPPS;
	unsigned char abVPS[VIDEO_ENC_PARAM_BUF_MAX_LEN];
	int iSizeOfVPS;
}T_VideoEncodeParam;


typedef struct MediaFrameBufInfo
{
    unsigned char *pbFrameBuf;//������
    int iFrameBufMaxLen;//�������ܴ�С
    int iFrameBufLen;//�������������ݵ��ܴ�С
    int iFrameProcessedLen;
    
    E_StreamType eStreamType;//������֡����ʱ�������Ҫ�ⲿ��ֵȻ����
    E_MediaEncodeType eEncType;//������֡����ʱ�������Ҫ�ⲿ��ֵȻ����
}T_MediaFrameBufInfo;

typedef struct MediaFrameInfo
{
    unsigned char *pbFrameBuf;//������
    int iFrameBufMaxLen;//�������ܴ�С
    int iFrameBufLen;//�������������ݵ��ܴ�С
    int iFrameProcessedLen;
    
    E_StreamType eStreamType;//������֡����ʱ�������Ҫ�ⲿ��ֵȻ����
    E_MediaEncodeType eEncType;//������֡����ʱ�������Ҫ�ⲿ��ֵȻ����
    unsigned int dwTimeStamp;//������֡����ʱ������ⲿ�ḳֵȻ����
    unsigned int dwSampleRate;//�ڲ��жϵ���Ϊ0���򲻻���ȥ��ֵ
    E_MediaFrameType eFrameType;

	//���1֡���ݽ��
    unsigned char *pbFrameStartPos;
    int iFrameLen;
    unsigned int dwNaluCount;//����sps,pps�Ȳ�������Ӧ��nalu
    unsigned int adwNaluEndOffset[MAX_NALU_CNT_ONE_FRAME];
    T_VideoEncodeParam tVideoEncodeParam;
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
    //һ���ӿڿ���ȡ��ǰ��3���ӿڣ�����Init GetNextFrame GetVideoEncParam GetMediaInfo ����
    virtual int GetFrame(T_MediaFrameInfo *m_ptFrame);//
protected:
	T_MediaInfo m_tMediaInfo;
	
private:
    MediaHandle             *m_pMediaHandle;//Ĭ��VideoHandle(����ʱ����ʾ��Ƶ�����ݴ���)
    MediaHandle             *m_pMediaAudioHandle;//����Ƶ��·����ʱ,����ᱻ��ֵ��ʾ��Ƶ�����ݴ���
    
	FILE                    *m_pMediaFile;
	//unsigned int 			m_dwFileReadOffset;
	
};









#endif
