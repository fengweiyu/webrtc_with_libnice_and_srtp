/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FlvHandle.cpp
* Description		: 	FlvHandle operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "MediaAdapter.h"
#include "FlvHandle.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>


using std::cout;//��Ҫ<iostream>
using std::endl;

#define  Read16BE(ptr,val)     *val = ((unsigned char)ptr[0] << 8) | (unsigned char)ptr[1]
#define  Read24BE(ptr,val)     *val = ((unsigned char)ptr[0] << 16) | ((unsigned char)ptr[1] << 8) | (unsigned char)ptr[2]
#define  Read32BE(ptr,val)     *val = ((unsigned char)ptr[0] << 24) | ((unsigned char)ptr[1] << 16) | ((unsigned char)ptr[2] << 8) | (unsigned char)ptr[3]
#define  Read32LE(ptr,val)     *val = (unsigned char)ptr[0] | ((unsigned char)ptr[1] << 8) | ((unsigned char)ptr[2] << 16) | ((unsigned char)ptr[3] << 24)
#define BIT(ptr, off) (((ptr)[(off) / 8] >> (7 - ((off) % 8))) & 0x01)


#define FLV_FRAME_BUF_MAX_LEN (1024*1024)

#define FLV_H264_SAMPLE_RATE 90000
#define FLV_H265_SAMPLE_RATE 90000

#define FLV_TO_U32(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#define FLV_VIDEO_ENC_AV1	FLV_TO_U32('a', 'v', '0', '1') //ע�ⲻ��avc(h264)
#define FLV_VIDEO_ENC_VP9	FLV_TO_U32('v', 'p', '0', '9') 
#define FLV_VIDEO_ENC_H265	FLV_TO_U32('h', 'v', 'c', '1') //HEVC

#define PACKET_TYPE_SEQUENCE_START 0
typedef enum
{
    FLV_PACKET_TYPE_SEQUENCE_START=0,
    FLV_PACKET_TYPE_CODED_FRAMES,
    FLV_PACKET_TYPE_SEQUENCE_END,
    FLV_PACKET_TYPE_CODED_FRAMES_X,
    FLV_PACKET_TYPE_METADATA,
    FLV_PACKET_TYPE_MPEG2_TS_SEQUENCE_START,
}E_FlvPacketType;

/*****************************************************************************
-Fuction		: H264Handle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
FlvHandle::FlvHandle()
{
    m_pbFrameBuf = new unsigned char [FLV_FRAME_BUF_MAX_LEN];
    m_iFrameBufMaxLen = FLV_FRAME_BUF_MAX_LEN;
}
/*****************************************************************************
-Fuction		: ~H264Handle
-Description	: ~H264Handle
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
FlvHandle::~FlvHandle()
{
    if(NULL!= m_pbFrameBuf)
    {
        delete[] m_pbFrameBuf;
    }
    m_iFrameBufMaxLen = 0;
}

/*****************************************************************************
-Fuction        : GetFrameData
-Description    : m_ptFrame->iFrameBufLen ������ڵ���һ֡���ݴ�С�����ʧ��
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::GetFrameData(int i_iDataOffset,T_MediaFrameInfo *m_ptFrame)
{
    int iRet = -1;
    int iProcessedLen = 0;
    T_FlvTag tFlvTag;

    
    if(NULL == m_ptFrame || NULL == m_ptFrame->pbFrameBuf ||m_ptFrame->iFrameBufLen <= FLV_TAG_HEADER_LEN)
    {
        MH_LOGE("GetFrameData NULL %d\r\n", m_ptFrame->iFrameBufLen);
        return iRet;
    }
	
    if(m_ptFrame->iFrameProcessedLen <= 0 && 0==i_iDataOffset)
    {
        iProcessedLen=this->FlvReadHeader(m_ptFrame->pbFrameBuf+i_iDataOffset,m_ptFrame->iFrameBufLen-i_iDataOffset);
        if(iProcessedLen <= 0)
        {
            MH_LOGE("FlvReadHeader err %d\r\n",iProcessedLen);
            return iRet;
        }
        return iProcessedLen;//���ص�����Ա�i_iDataOffsetֵ�õ����
    }
    memset(&tFlvTag,0,sizeof(T_FlvTag));
    iRet = this->FlvReadTag(m_ptFrame->pbFrameBuf+i_iDataOffset,m_ptFrame->iFrameBufLen-i_iDataOffset,&tFlvTag);
    if(iRet <= 0)
    {
        MH_LOGE("FlvReadTag err %d\r\n",iRet);
        return iRet;
    }
    iProcessedLen+=iRet;
    switch(tFlvTag.tTagHeader.bType)
    {
        case FLV_TAG_VIDEO_TYPE :
        {
            //demux
            iRet=this->GetVideoData(tFlvTag.pbTagData,tFlvTag.dwDataCurLen,m_ptFrame,m_pbFrameBuf,m_iFrameBufMaxLen);
            if(iRet < 0)
            {
                MH_LOGE("GetVideoData err %d \r\n",iRet);
            }
            else if(iRet == 0)
            {
                MH_LOGD("GetVideoData need more data %d \r\n",iRet);
            }
            else
            {
                if(m_ptFrame->eEncType == MEDIA_ENCODE_TYPE_H264)
                {
                    m_ptFrame->dwSampleRate = FLV_H264_SAMPLE_RATE;
                }
                else if(m_ptFrame->eEncType == MEDIA_ENCODE_TYPE_H265)
                {
                    m_ptFrame->dwSampleRate = FLV_H265_SAMPLE_RATE;
                }
                m_ptFrame->dwTimeStamp = tFlvTag.tTagHeader.dwTimestamp;
                m_ptFrame->pbFrameStartPos = m_pbFrameBuf;
                m_ptFrame->iFrameLen = iRet;
            }
            //m_dwFileTimestamp = tFlvTagHeader.dwTimestamp+m_dwFileBaseTimestamp;//����Ƶʱ���Ϊ��׼
            break;
        }
        case FLV_TAG_AUDIO_TYPE :
        {
            //demux
            iRet = this->GetAudioData(tFlvTag.pbTagData,tFlvTag.dwDataCurLen,m_ptFrame,m_pbFrameBuf,m_iFrameBufMaxLen);
            if(iRet < 0)
            {
                MH_LOGE("GetAudioData err %d \r\n",iRet);
            }
            else if(iRet == 0)
            {
                MH_LOGD("GetAudioData need more data %d \r\n",iRet);
            }
            else
            {
                m_ptFrame->eFrameType = MEDIA_FRAME_TYPE_AUDIO_FRAME;
                m_ptFrame->dwTimeStamp = tFlvTag.tTagHeader.dwTimestamp;
                m_ptFrame->pbFrameStartPos = m_pbFrameBuf;
                m_ptFrame->iFrameLen = iRet;
            }
            break;
        }
        case FLV_TAG_SCRIPT_TYPE :
        {
            break;
        }
        default :
        {
            break;
        }
    }    
    if(iRet < 0)
    {
        return iRet;
    }
    return iProcessedLen;
}

/*****************************************************************************
-Fuction        : GetAudioData
-Description    : FLV��ʽ��Ƶ���ݽ���
-Input          : 
-Output         : 
-Return         : ��֧�ֵı����ʽ����-1��ֻ������ͷ����0�����������򷵻س���
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::GetAudioData(unsigned char *i_pbAudioTag,int i_iTagLen,T_MediaFrameInfo *m_ptFrame,unsigned char *o_pbAudioData,int i_iAudioDataMaxLen)
{
    int iAudioDataLen = 0;
    unsigned char bIsAACSeqHeader = 0;
    int iProcessedLen = 0;
    
    if(NULL == i_pbAudioTag || NULL == m_ptFrame || NULL == o_pbAudioData)
    {
        MH_LOGE("GetAudioData NULL %d\r\n", i_iTagLen);
        return -1;
    }
    //tag Header 1 byte
    ParseAudioDataTagHeader(i_pbAudioTag[iProcessedLen],m_ptFrame);
    if(MEDIA_ENCODE_TYPE_UNKNOW == m_ptFrame->eEncType ||MEDIA_ENCODE_TYPE_OPUS == m_ptFrame->eEncType)
    {
        MH_LOGE("RTMP_UNKNOW_ENC_TYPE %d\r\n", m_ptFrame->eEncType);//OPUS�ݲ�֧��,��������
        return -1;
    }
    iProcessedLen++;

    if(MEDIA_ENCODE_TYPE_AAC == m_ptFrame->eEncType)
    {
        bIsAACSeqHeader = i_pbAudioTag[iProcessedLen] == 0 ? 1:0;//tag Body(AAC packet type) 1 byte
        iProcessedLen++;
        //tag Body(AAC SeqHeader or AAC Raw)
        if(0 == bIsAACSeqHeader)
        {
            iAudioDataLen += AddAdtsHeader(m_ptFrame->dwSampleRate,m_ptFrame->tAudioEncodeParam.dwChannels,
            i_iTagLen-iProcessedLen,o_pbAudioData,i_iAudioDataMaxLen);//mp4���ܴ�(avcc)
            memcpy(o_pbAudioData+iAudioDataLen, &i_pbAudioTag[iProcessedLen],i_iTagLen-iProcessedLen);
            iAudioDataLen +=i_iTagLen-iProcessedLen;
        }
        else
        {
            unsigned char bProfile = 0; // 0-NULL, 1-AAC Main, 2-AAC LC, 2-AAC SSR, 3-AAC LTP
            unsigned char bSamplingFreqIndex = 0; // 0-96000, 1-88200, 2-64000, 3-48000, 4-44100, 5-32000, 6-24000, 7-22050, 8-16000, 9-12000, 10-11025, 11-8000, 12-7350, 13/14-reserved, 15-frequency is written explictly
            unsigned char bChannelConf = 0; // 0-AOT, 1-1channel,front-center, 2-2channels, front-left/right, 3-3channels: front center/left/right, 4-4channels: front-center/left/right, back-center, 5-5channels: front center/left/right, back-left/right, 6-6channels: front center/left/right, back left/right LFE-channel, 7-8channels
            unsigned int dwSamplingFrequency = 0;  // codec frequency, valid only in decode
            unsigned char bChannel = 0; 
            static const unsigned int s_adwSamplingFreq[13] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350 };
            static const unsigned char s_abChannels[8] = { 0, 1, 2, 3, 4, 5, 6, 8 };
            
            bProfile = (i_pbAudioTag[iProcessedLen] >> 3) & 0x1F;
            bSamplingFreqIndex = ((i_pbAudioTag[iProcessedLen] & 0x7) << 1) | ((i_pbAudioTag[iProcessedLen+1] >> 7) & 0x01);
            bChannelConf = (i_pbAudioTag[iProcessedLen+1] >> 3) & 0x0F;
            if(bSamplingFreqIndex < 13)
                dwSamplingFrequency = s_adwSamplingFreq[bSamplingFreqIndex];
            if(bChannelConf < 8)
                bChannel = s_abChannels[bChannelConf];

            m_ptFrame->dwSampleRate = dwSamplingFrequency;
            m_ptFrame->tAudioEncodeParam.dwChannels = bChannel;
        }
        
    }
    else
    {
        memcpy(o_pbAudioData,&i_pbAudioTag[iProcessedLen],i_iTagLen-iProcessedLen);
        iAudioDataLen += i_iTagLen-iProcessedLen;
    }
    return iAudioDataLen;
}


/*****************************************************************************
-Fuction        : GetVideoData
-Description    : FLV��ʽ��Ƶ���ݽ���
-Input          : 
-Output         : m_ptFrameInfo->atNaluInfo[i].pbDataָ����ڴ���o_pbVideoData����o_pbVideoData��annex-b��ʽ����
-Return         : ��֧�ֵı����ʽ����-1��ֻ������ͷ����0�����������򷵻س���
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::GetVideoData(unsigned char *i_pbVideoTag,int i_iTagLen,T_MediaFrameInfo * m_ptFrameInfo,unsigned char *o_pbVideoData,int i_iVideoDataMaxLen)
{
    int iVideoDataLen = 0;
    unsigned char bIsAvcSeqHeader = 0;
    int iProcessedLen = 0;
    int i = 0,j = 0;
    unsigned int dwNaluLen = 0;
    unsigned char bNaluType = 0;
    unsigned char bIsExHeader = 0;
    unsigned char * pbVideoTagHeader=NULL;
    unsigned char bFrameType = 0;
    unsigned char bCodecId = 0;
    unsigned char bPacketType = 0;
    unsigned int dwFourCC = 0;
    int cts;        /// video composition time(PTS - DTS), AVC/HEVC/AV1 only

    
    if(NULL == i_pbVideoTag || NULL == m_ptFrameInfo || NULL == o_pbVideoData)
    {
        MH_LOGE("GetVideoData NULL %d\r\n", i_iTagLen);
        return -1;
    }
    //tag Header 1 byte
    pbVideoTagHeader= &i_pbVideoTag[iProcessedLen];
    bIsExHeader = ((i_pbVideoTag[iProcessedLen]>>7)&0x01);
    if(0 == bIsExHeader)
    {//���bIsExHeaderδʹ�ܣ� ����ѭ֮ǰ��rtmp/flv��ͳ�淶
        bFrameType = ((i_pbVideoTag[iProcessedLen]>>4)&0x0f);////1 = key frame, 2 = inter frame
        bCodecId = (i_pbVideoTag[iProcessedLen]&0x0f);//4bits��codecId��H.264��ֵΪ7.
        iProcessedLen++;
        
        bIsAvcSeqHeader = i_pbVideoTag[iProcessedLen] == 0 ? 1:0;//tag Body(AVC packet type) 1 byte
        iProcessedLen++;
        //tag Body(AVC SeqHeader or AVC Raw)
        cts = (i_pbVideoTag[iProcessedLen] << 16) | (i_pbVideoTag[iProcessedLen+1] << 8) | i_pbVideoTag[iProcessedLen+2];
        cts = (cts + 0xFF800000) ^ 0xFF800000; // signed 24-integer
        iProcessedLen+=3;
        
        switch(pbVideoTagHeader[0])
        {
            case 0x17:
            {
                m_ptFrameInfo->eFrameType = MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
                m_ptFrameInfo->eEncType = MEDIA_ENCODE_TYPE_H264;
                break;
            }
            case 0x27:
            {
                m_ptFrameInfo->eFrameType = MEDIA_FRAME_TYPE_VIDEO_P_FRAME;//RTMP_VIDEO_INNER_FRAME;
                m_ptFrameInfo->eEncType = MEDIA_ENCODE_TYPE_H264;
                break;
            }
            case 0x1c:
            {
                m_ptFrameInfo->eFrameType = MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
                m_ptFrameInfo->eEncType = MEDIA_ENCODE_TYPE_H265;
                break;
            }
            case 0x2c:
            {
                m_ptFrameInfo->eFrameType = MEDIA_FRAME_TYPE_VIDEO_P_FRAME;//RTMP_VIDEO_INNER_FRAME;
                m_ptFrameInfo->eEncType = MEDIA_ENCODE_TYPE_H265;
                break;
            }
            default:
            {
                m_ptFrameInfo->eFrameType = MEDIA_FRAME_TYPE_UNKNOW;
                m_ptFrameInfo->eEncType = MEDIA_ENCODE_TYPE_UNKNOW;
                break;
            }
        }
    }
    else
    {// IsExHeaderʹ�ܣ���ʾEnhanced-Rtmp��ʽʹ��
        bFrameType = ((i_pbVideoTag[iProcessedLen]>>4)&0x07);////1 = key frame, 2 = inter frame
        bPacketType = (i_pbVideoTag[iProcessedLen]&0x0f);// 0 = PacketTypeSequenceStart // 1 = PacketTypeCodedFrames
                                                   // 2 = PacketTypeSequenceEnd // 3 = PacketTypeCodedFramesX
                                                   // 4 = PacketTypeMetadata // 5 = PacketTypeMPEG2TSSequenceStart
        iProcessedLen++;
        Read32BE((i_pbVideoTag + iProcessedLen),&dwFourCC);
        iProcessedLen+=4;

        if (bPacketType == FLV_PACKET_TYPE_SEQUENCE_START)//���PacketType��PacketTypeSequenceStart����ʾ����H265������������DecoderConfigurationRecord��Ҳ���ǳ�˵��sequence header;
        {
            bIsAvcSeqHeader=1;
        }
        else if (bPacketType == FLV_PACKET_TYPE_CODED_FRAMES || bPacketType == FLV_PACKET_TYPE_CODED_FRAMES_X)
        {
            if (bPacketType == FLV_PACKET_TYPE_CODED_FRAMES)
            {
                //���PacketType��PacketTypeCodedFrames������pts��dts�в�ֵ
                cts = (i_pbVideoTag[iProcessedLen] << 16) | (i_pbVideoTag[iProcessedLen+1] << 8) | i_pbVideoTag[iProcessedLen+2];//24bits����ʾpts��dts�Ĳ�ֵ
                cts = (cts + 0xFF800000) ^ 0xFF800000; // signed 24-integer
                iProcessedLen+=3;
            }
            else //���PacketType��PacketTypeCodedFramesX
            {
                //��CompositionTime����ʡ3�ֽڵĿռ�
            }
            //�����������H265����
        }
        if(dwFourCC != FLV_VIDEO_ENC_H265)
        {
            MH_LOGE("dwFourCC err %x\r\n",dwFourCC);
            m_ptFrameInfo->eEncType = MEDIA_ENCODE_TYPE_UNKNOW;
        }
        else
        {
            m_ptFrameInfo->eEncType = MEDIA_ENCODE_TYPE_H265;
        }
        switch(bFrameType)
        {
            case 1:
            {
                m_ptFrameInfo->eFrameType = MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
                break;
            }
            case 2:
            {
                m_ptFrameInfo->eFrameType = MEDIA_FRAME_TYPE_VIDEO_P_FRAME;//RTMP_VIDEO_INNER_FRAME;
                break;
            }
            default:
            {
                m_ptFrameInfo->eFrameType = MEDIA_FRAME_TYPE_UNKNOW;
                break;
            }
        }
    }

    
    if(MEDIA_ENCODE_TYPE_UNKNOW == m_ptFrameInfo->eEncType)
    {
        MH_LOGE("m_ptFrameInfo->eEncType err %d\r\n",i_pbVideoTag[iProcessedLen]);
        return -1;
    }

    if(MEDIA_ENCODE_TYPE_H264 == m_ptFrameInfo->eEncType)
    {
        if(0 == bIsAvcSeqHeader)
        {
            T_FlvH265Extradata tH265Extradata;
            memset(&tH265Extradata,0,sizeof(T_FlvH265Extradata));
            SpsToH264Resolution(m_ptFrameInfo->tVideoEncodeParam.abSPS,m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS,&tH265Extradata);
            m_ptFrameInfo->dwWidth = tH265Extradata.pic_width;
            m_ptFrameInfo->dwHeight = tH265Extradata.pic_height;
        
            iVideoDataLen=GetVideoDataNalu(&i_pbVideoTag[iProcessedLen],i_iTagLen-iProcessedLen,m_ptFrameInfo,o_pbVideoData,i_iVideoDataMaxLen);
        }
        else
        {
            if(1 != i_pbVideoTag[iProcessedLen])
            {
                MH_LOGE("i_pbVideoTag->ver err %d\r\n",i_pbVideoTag[iProcessedLen]);
                return -1;
            }
            iProcessedLen++;
            unsigned char bProfile;
            unsigned char bCompatibility; // constraint_set[0-5]_flag
            unsigned char bLevel;
            unsigned char bNaluLenSize; // NALUnitLength = (lengthSizeMinusOne + 1), default 4(0x03+1)
            unsigned char bSpsNum;
            unsigned char bPpsNum;
            bProfile = i_pbVideoTag[iProcessedLen];
            iProcessedLen++;
            bCompatibility = i_pbVideoTag[iProcessedLen];
            iProcessedLen++;
            bLevel = i_pbVideoTag[iProcessedLen];
            iProcessedLen++;
            bNaluLenSize = (i_pbVideoTag[iProcessedLen] & 0x03) + 1;
            iProcessedLen++;
            bSpsNum = i_pbVideoTag[iProcessedLen] & 0x1F;
            iProcessedLen++;
            for(i = 0;i<bSpsNum;i++)
            {
                Read16BE((i_pbVideoTag+iProcessedLen),&m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS);
                iProcessedLen+=2;
                if(m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS <= sizeof(m_ptFrameInfo->tVideoEncodeParam.abSPS))
                {
                    memcpy(m_ptFrameInfo->tVideoEncodeParam.abSPS,&i_pbVideoTag[iProcessedLen],m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS);
                }
                iProcessedLen+=m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS;
            }
            bPpsNum = i_pbVideoTag[iProcessedLen];
            iProcessedLen++;
            for(i = 0;i<bPpsNum;i++)
            {
                Read16BE((i_pbVideoTag+iProcessedLen),&m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS);
                iProcessedLen+=2;
                if(m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS <= sizeof(m_ptFrameInfo->tVideoEncodeParam.abPPS))
                {
                    memcpy(m_ptFrameInfo->tVideoEncodeParam.abPPS,&i_pbVideoTag[iProcessedLen],m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS);
                }
                iProcessedLen+=m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS;
            }
            m_ptFrameInfo->bNaluLenSize = bNaluLenSize;
        }
    }
    else if(MEDIA_ENCODE_TYPE_H265 == m_ptFrameInfo->eEncType)
    {                
        T_FlvH265Extradata tH265Extradata;
        if(0 == bIsAvcSeqHeader)
        {
            memset(&tH265Extradata,0,sizeof(T_FlvH265Extradata));
            SpsToH265Extradata(m_ptFrameInfo->tVideoEncodeParam.abSPS,m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS,&tH265Extradata);
            m_ptFrameInfo->dwWidth = tH265Extradata.pic_width;
            m_ptFrameInfo->dwHeight = tH265Extradata.pic_height;
            iVideoDataLen=GetVideoDataNalu(&i_pbVideoTag[iProcessedLen],i_iTagLen-iProcessedLen,m_ptFrameInfo,o_pbVideoData,i_iVideoDataMaxLen);
        }
        else
        {
            unsigned char *pbVideoData = &i_pbVideoTag[iProcessedLen];
            unsigned char numOfArrays;
            unsigned char nalutype;
            unsigned char *pbVideoParams;
            unsigned short numOfVideoParameterSets,lenOfVideoParameterSets;
            
            memset(&tH265Extradata,0,sizeof(T_FlvH265Extradata));
            tH265Extradata.configurationVersion = pbVideoData[0];
            if(1 != tH265Extradata.configurationVersion)
            {
                MH_LOGE("i_pbVideoTag->ver err %d\r\n",tH265Extradata.configurationVersion);
                return -1;
            }
            
            tH265Extradata.general_profile_space = (pbVideoData[1] >> 6) & 0x03;
            tH265Extradata.general_tier_flag = (pbVideoData[1] >> 5) & 0x01;
            tH265Extradata.general_profile_idc = pbVideoData[1] & 0x1F;
            tH265Extradata.general_profile_compatibility_flags = (pbVideoData[2] << 24) | (pbVideoData[3] << 16) | (pbVideoData[4] << 8) | pbVideoData[5];
            tH265Extradata.general_constraint_indicator_flags = ((unsigned int)pbVideoData[6] << 24) | ((unsigned int)pbVideoData[7] << 16) | ((unsigned int)pbVideoData[8] << 8) | (unsigned int)pbVideoData[9];
            tH265Extradata.general_constraint_indicator_flags = (tH265Extradata.general_constraint_indicator_flags << 16) | (((uint64_t)pbVideoData[10]) << 8) | pbVideoData[11];
            tH265Extradata.general_level_idc = pbVideoData[12];
            tH265Extradata.min_spatial_segmentation_idc = ((pbVideoData[13] & 0x0F) << 8) | pbVideoData[14];
            tH265Extradata.parallelismType = pbVideoData[15] & 0x03;
            tH265Extradata.chromaFormat = pbVideoData[16] & 0x03;
            tH265Extradata.bitDepthLumaMinus8 = pbVideoData[17] & 0x07;
            tH265Extradata.bitDepthChromaMinus8 = pbVideoData[18] & 0x07;
            tH265Extradata.avgFrameRate = (pbVideoData[19] << 8) | pbVideoData[20];
            tH265Extradata.constantFrameRate = (pbVideoData[21] >> 6) & 0x03;
            tH265Extradata.numTemporalLayers = (pbVideoData[21] >> 3) & 0x07;
            tH265Extradata.temporalIdNested = (pbVideoData[21] >> 2) & 0x01;
            tH265Extradata.lengthSizeMinusOne = pbVideoData[21] & 0x03;
            numOfArrays = pbVideoData[22];

            pbVideoParams = &pbVideoData[23];
            for (i = 0; i < numOfArrays; i++)
            {
                nalutype = pbVideoParams[0];//ȡ�������Ѿ��������
                numOfVideoParameterSets = (pbVideoParams[1] << 8) | pbVideoParams[2];
                pbVideoParams += 3;
                for (j = 0; j < numOfVideoParameterSets; j++)
                {
                    lenOfVideoParameterSets = (pbVideoParams[0] << 8) | pbVideoParams[1];
                    switch(nalutype)
                    {
                        case 32:
                        {//VPS
                            m_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS= lenOfVideoParameterSets;//pbVideoParams[2]��û����ģ�ԭʼ��
                            memcpy(m_ptFrameInfo->tVideoEncodeParam.abVPS,&pbVideoParams[2],m_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS);
                            break;//bNaluType = (pbVideoParams[2] & 0x7E)>>1;//ȡnalu����
                        }
                        case 33:
                        {//SPS
                            m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS= lenOfVideoParameterSets;
                            memcpy(m_ptFrameInfo->tVideoEncodeParam.abSPS,&pbVideoParams[2],m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS);
                            break;
                        }
                        case 34:
                        {//PPS
                            m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS = lenOfVideoParameterSets;
                            memcpy(m_ptFrameInfo->tVideoEncodeParam.abPPS,&pbVideoParams[2],m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS);
                            break;
                        }
                        case 39:
                        {//sei
                            break;
                        }
                        default:
                        {
                            MH_LOGE("nalutype & 0x3F err %d\r\n",nalutype & 0x3F);
                            break;
                        }

                    }
                    tH265Extradata.numOfArrays++;
                    pbVideoParams += 2 + lenOfVideoParameterSets;
                }
            }
            m_ptFrameInfo->bNaluLenSize = tH265Extradata.lengthSizeMinusOne+1;
        }
    }
    else
    {
        iVideoDataLen = -1;
    }
    return iVideoDataLen;
}


/*****************************************************************************
-Fuction        : GetVideoDataNalu
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::GetVideoDataNalu(unsigned char *i_pbVideoData,int i_iDataLen,T_MediaFrameInfo * m_ptFrameInfo,unsigned char *o_pbVideoData,int i_iVideoDataMaxLen)
{
    int iVideoDataLen = 0;
    int iProcessedLen = 0;
    int i = 0,j = 0;
    unsigned int dwNaluLen = 0;
    //unsigned char bNaluType = 0;
    
    m_ptFrameInfo->dwNaluCount=0;
    
#if 0 //avcc������ʹ��aud
    memset(&o_pbVideoData[iVideoDataLen],0x00,3);
    iVideoDataLen+=3;
    o_pbVideoData[iVideoDataLen]=2;//����
    iVideoDataLen++;
    o_pbVideoData[iVideoDataLen]=9;
    iVideoDataLen++;
    if(m_ptRtmpFrameInfo->eFrameType == RTMP_VIDEO_KEY_FRAME)
        o_pbVideoData[iVideoDataLen]=0x10;
    else
        o_pbVideoData[iVideoDataLen]=0x30;
    iVideoDataLen++;
#endif
    if(m_ptFrameInfo->eFrameType == MEDIA_FRAME_TYPE_VIDEO_I_FRAME)//��ffmpeg h264_mp4toannexbһ��
    {//ֻ�ǵ�һ��naluǰ��,һ��tag data ��nalu������һ����
        if(m_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS > 0)
        {
            m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].pbData = &o_pbVideoData[iVideoDataLen];
            
            memset(&o_pbVideoData[iVideoDataLen],0x00,3);
            iVideoDataLen+=3;
            o_pbVideoData[iVideoDataLen]=1;
            iVideoDataLen++;
            memcpy(&o_pbVideoData[iVideoDataLen], m_ptFrameInfo->tVideoEncodeParam.abVPS,m_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS);
            iVideoDataLen += m_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS;
            
            m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].dwDataLen = &o_pbVideoData[iVideoDataLen]-m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].pbData;
            m_ptFrameInfo->dwNaluCount++;
        }
        if(m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS > 0)
        {
            m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].pbData = &o_pbVideoData[iVideoDataLen];
            
            memset(&o_pbVideoData[iVideoDataLen],0x00,3);
            iVideoDataLen+=3;
            o_pbVideoData[iVideoDataLen]=1;
            iVideoDataLen++;
            memcpy(&o_pbVideoData[iVideoDataLen], m_ptFrameInfo->tVideoEncodeParam.abSPS,m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS);
            iVideoDataLen += m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS;
            
            m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].dwDataLen = &o_pbVideoData[iVideoDataLen]-m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].pbData;
            m_ptFrameInfo->dwNaluCount++;
        }
        if(m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS > 0)
        {
            m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].pbData = &o_pbVideoData[iVideoDataLen];
        
            memset(&o_pbVideoData[iVideoDataLen],0x00,3);
            iVideoDataLen+=3;
            o_pbVideoData[iVideoDataLen]=1;
            iVideoDataLen++;
            memcpy(&o_pbVideoData[iVideoDataLen],m_ptFrameInfo->tVideoEncodeParam.abPPS,m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS);
            iVideoDataLen += m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS;
            
            m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].dwDataLen = &o_pbVideoData[iVideoDataLen]-m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].pbData;
            m_ptFrameInfo->dwNaluCount++;
        }
    }
    for(i=0;i<(int)(sizeof(m_ptFrameInfo->atNaluInfo)/sizeof(T_MediaNaluInfo))&& iProcessedLen < i_iDataLen;i++)//flv tag data���ж��nalu size+nalu data
    {
#if 0 //avcc������ʹ��
        dwNaluLen = 0;
        for (j = 0; j < m_ptRtmpFrameInfo->bNaluLenSize; j++)
            dwNaluLen = (dwNaluLen << 8) + i_pbVideoData[iProcessedLen+j];
        
        if((0x67 == i_pbVideoData[iProcessedLen+4]) ||(0x06 == i_pbVideoData[iProcessedLen+4]) ||
        (0x68 == i_pbVideoData[iProcessedLen+4]))
        {//�Ѿ��ӹ������ˣ����ټ�
            iProcessedLen += dwNaluLen+4;
            continue;
        }
        memcpy(&o_pbVideoData[iVideoDataLen], &i_pbVideoData[iProcessedLen],dwNaluLen+4);
        iProcessedLen+=dwNaluLen+4;
        iVideoDataLen += dwNaluLen+4;
        continue;
#endif
        m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].pbData = &o_pbVideoData[iVideoDataLen];
        
        memset(&o_pbVideoData[iVideoDataLen],0x00,3);
        iVideoDataLen+=3;
        o_pbVideoData[iVideoDataLen]=1;
        iVideoDataLen++;
        
        //while(iProcessedLen < i_iTagLen)//����ֻ֧��data��ֻ��һ��nalu�����
        {
            dwNaluLen = 0;
            for (j = 0; j < m_ptFrameInfo->bNaluLenSize; j++)
                dwNaluLen = (dwNaluLen << 8) + i_pbVideoData[iProcessedLen+j];
            iProcessedLen+= m_ptFrameInfo->bNaluLenSize;
            
            if((0x67 == i_pbVideoData[iProcessedLen] && m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS > 0) ||
            (0x68 == i_pbVideoData[iProcessedLen] && m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS > 0) ||
            (0x40 == i_pbVideoData[iProcessedLen] && m_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS > 0) ||
            (0x42 == i_pbVideoData[iProcessedLen] && m_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS > 0) ||
            (0x44 == i_pbVideoData[iProcessedLen] && m_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS > 0))//�Ѿ��ӹ������ˣ����ټ�
            {//�����Ż�Ϊȡnalu���ͣ���ʱֱ��ʹ��ֵ�ж�
                iProcessedLen += dwNaluLen;//bNaluType = (i_pbVideoData[iProcessedLen] & 0x7E)>>1;//h265ȡnalu����20 32 33 34
                iVideoDataLen -= m_ptFrameInfo->bNaluLenSize;//bNaluType = pcNaluStartPos[3] & 0x1f;;//h264ȡnalu����7 8 1 5
                continue;
            }

            //if(bNaluType != i_pbVideoTag[iProcessedLen])//����ֻ��һ��nalu�����
            {
                //bNaluType = i_pbVideoTag[iProcessedLen];//
            }
            //else//���������޷�������
            {
                //iProcessedLen+=1;//ƫ��nalu type NaluSliceHeader
                //dwNaluLen-=1;//
            }
            
            memcpy(&o_pbVideoData[iVideoDataLen], &i_pbVideoData[iProcessedLen],dwNaluLen);
            iVideoDataLen += dwNaluLen;
            iProcessedLen+=dwNaluLen;
        }
        
        m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].dwDataLen = &o_pbVideoData[iVideoDataLen]-m_ptFrameInfo->atNaluInfo[m_ptFrameInfo->dwNaluCount].pbData;
        m_ptFrameInfo->dwNaluCount++;
    }
    return iVideoDataLen;
}

/*****************************************************************************
-Fuction        : ParseAudioDataTagHeader
-Description    : ParseAudioDataTagHeader
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
unsigned char FlvHandle::ParseAudioDataTagHeader(unsigned char i_bAudioTagHeader,T_MediaFrameInfo *m_ptFrame)
{
    unsigned char bAudioTagHeader = 0;
    unsigned char bEncType = 0;
    unsigned char bSampleRateIndex = 3;// 0-5.5kHz;1-11kHz;2-22kHz;3-44.1kHz(AAC����3)
    unsigned char bSampleBits = 0;//0b00 win �ᱨ��;// audio sample bits: 0-8 bit samples, 1-16-bit samples
    unsigned char bChannels = 1;// audio channel count: 0-Mono sound, 1-Stereo sound
    ///������  
    static unsigned int const s_adwSampleRateTable[16] = {
      5500, 11000, 22000, 44100,
    };

	bEncType = (i_bAudioTagHeader & 0xF0) >> 4;
	bSampleRateIndex = (i_bAudioTagHeader & 0x0C) >> 2;
	bSampleBits = (i_bAudioTagHeader & 0x02) >> 1;
	bChannels = i_bAudioTagHeader & 0x01;//����ģʽ0 = ������1 = ˫��������������
	
    m_ptFrame->tAudioEncodeParam.dwChannels = bChannels+1;//ͨ��=����+1
    m_ptFrame->tAudioEncodeParam.dwBitsPerSample = bSampleBits == 1 ? 16 : 8;//0b01 win �ᱨ��
    m_ptFrame->dwSampleRate= 44100;
    if(bSampleRateIndex < 4)
        m_ptFrame->dwSampleRate = s_adwSampleRateTable[bSampleRateIndex];
    switch(bEncType)
    {
        case 2:
        {
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_MP3;
            break;
        }
        case 7:
        {
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_G711A;
            break;
        }
        case 8:
        {
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_G711U;
            break;
        }
        case 13:
        {
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_OPUS;
            break;
        }
        case 0:
        {
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_LPCM;
            break;
        }
        case 1:
        {
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_ADPCM;
            break;
        }
        case 3:
        {
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_LLPCM;
            break;
        }
        case 10:
        default:
        {
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_AAC;
            break;
        }
    }
    return 0;
}

/*****************************************************************************
-Fuction        : AddAdtsHeader
-Description    : AddAdtsHeader
-Input          : 
-Output         : 
-Return         : iVideoDataLen must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::AddAdtsHeader(unsigned int i_dwSampleRate,unsigned int i_dwChannels,int i_iAudioRawDataLen,unsigned char *o_pbAudioData,int i_iDataMaxLen)
{
    int iLen = 0;
    int iAllLen = i_iAudioRawDataLen + 7;
    int iSampleIndex = 0x04;
    
    if (NULL == o_pbAudioData || i_iDataMaxLen < 7) 
    {
        MH_LOGE("AddAdtsHeader NULL %d \r\n", i_iDataMaxLen);
        return iLen;
    }

    switch (i_dwSampleRate) 
    {
        case 96000:
            iSampleIndex = 0x00;
            break;
        case 88200:
            iSampleIndex = 0x01;
            break;
        case 64000:
            iSampleIndex = 0x02;
            break;
        case 48000:
            iSampleIndex = 0x03;
            break;
        case 44100:
            iSampleIndex = 0x04;
            break;
        case 32000:
            iSampleIndex = 0x05;
            break;
        case 24000:
            iSampleIndex = 0x06;
            break;
        case 22050:
            iSampleIndex = 0x07;
            break;
        case 16000:
            iSampleIndex = 0x08;
            break;
        case 12000:
            iSampleIndex = 0x09;
            break;
        case 11025:
            iSampleIndex = 0x0a;
            break;
        case 8000:
            iSampleIndex = 0x0b;
            break;
        case 7350:
            iSampleIndex = 0x0c;
            break;
        default:
            break;
    }

    o_pbAudioData[0] = 0xFF;
    o_pbAudioData[1] = 0xF1;
    o_pbAudioData[2] = 0x40 | (iSampleIndex << 2) | (i_dwChannels >> 2);
    o_pbAudioData[3] = ((i_dwChannels & 0x03) << 6) | (iAllLen >> 11);
    o_pbAudioData[4] = (iAllLen >> 3) & 0xFF;
    o_pbAudioData[5] = ((iAllLen << 5) & 0xFF) | 0x1F;
    o_pbAudioData[6] = 0xFC;

    iLen = 7;
    return iLen;
}

/*****************************************************************************
-Fuction        : RtmpMediaHandle
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::SpsToH264Resolution(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_FlvH265Extradata *o_ptH265Extradata)
{
    int iRet = -1;
    unsigned char abSodbSPS[VIDEO_SPS_MAX_SIZE];
    int iSodbLen = 0;
    
    unsigned char id;
    unsigned char profile_idc;
    unsigned char level_idc;
    unsigned char constraint_set_flags = 0;
    unsigned char chroma_format_idc;
    unsigned char bit_depth_luma;
    unsigned char frame_mbs_only_flag;
    unsigned char seq_scaling_matrix_present_flag,seq_scaling_list_present_flag;
    unsigned char frame_cropping_flag;
    int delta_scale, lastScale = 8, nextScale = 8;
    int sizeOfScalingList;
    int iBit;//ƫ�Ƶڼ�λ
    int i ,j,num_ref_frames_in_pic_order_cnt_cycle;
    unsigned int pic_order_cnt_type;
    unsigned int pic_width_in_mbs_minus1,pic_height_in_map_units_minus1;
    unsigned int frame_crop_left_offset,frame_crop_right_offset,frame_crop_top_offset,frame_crop_bottom_offset;

    
    if(NULL == i_pbSpsData || NULL == o_ptH265Extradata || 0 >= i_wSpsLen)
    {
        MH_LOGE("SpsToH265Extradata NULL %d \r\n", i_wSpsLen);
        return iRet;
    }
    memset(abSodbSPS,0,sizeof(abSodbSPS));
    iSodbLen = DecodeEBSP(i_pbSpsData, i_wSpsLen, abSodbSPS);
    if (iSodbLen < 12+3)
        return iRet;
    profile_idc = abSodbSPS[1] ;
    iBit = 2*8;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 0; // constraint_set0_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 1; // constraint_set1_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 2; // constraint_set2_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 3; // constraint_set3_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 4; // constraint_set4_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 5; // constraint_set5_flag
    iBit++;
    iBit+=2;
    level_idc = abSodbSPS[3] ;
    iBit+=8;
    id=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
    if (profile_idc == 100 || profile_idc == 110 ||profile_idc == 122 || profile_idc == 244 || profile_idc ==  44 ||profile_idc == 83 || 
    profile_idc == 86 || profile_idc == 118 ||profile_idc == 128 || profile_idc == 138 || profile_idc == 139 ||profile_idc == 134) 
    {
        chroma_format_idc=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
        if (chroma_format_idc == 3) 
        {
            iBit++; // separate_colour_plane_flag
        }
        bit_depth_luma = H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit) + 8;
        H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // bit_depth_chroma_minus8
        iBit++; // qpprime_y_zero_transform_bypass_flag
        seq_scaling_matrix_present_flag = BIT(abSodbSPS, iBit);// seq_scaling_matrix_present_flag
        iBit++;
        if (seq_scaling_matrix_present_flag) 
        { 
            for (i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); i++) 
            {
                seq_scaling_list_present_flag = BIT(abSodbSPS, iBit); // seq_scaling_list_present_flag
                iBit++;
                if (!seq_scaling_list_present_flag)
                    continue;
                lastScale = 8;
                nextScale = 8;
                sizeOfScalingList = i < 6 ? 16 : 64;
                for (j = 0; j < sizeOfScalingList; j++) 
                {
                    if (nextScale != 0) 
                    {
                        delta_scale = H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
                        nextScale = (lastScale + delta_scale) & 0xff;
                    }
                    lastScale = nextScale == 0 ? lastScale : nextScale;
                }
            }
        }
    }
    else
    {
        chroma_format_idc = 1;
        bit_depth_luma = 8;
    }
    H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // log2_max_frame_num_minus4
    pic_order_cnt_type = H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
    if (pic_order_cnt_type == 0) 
    {
        H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // log2_max_pic_order_cnt_lsb_minus4
    } 
    else if (pic_order_cnt_type == 1) 
    {
        iBit++;    // delta_pic_order_always_zero
        H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // offset_for_non_ref_pic
        H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // offset_for_top_to_bottom_field
        num_ref_frames_in_pic_order_cnt_cycle = (int)H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
        for (i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
            H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // offset_for_ref_frame
    }
    H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // max_num_ref_frames
    iBit++; // gaps_in_frame_num_value_allowed_flag
    pic_width_in_mbs_minus1=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // pic_width_in_mbs_minus1
    pic_height_in_map_units_minus1=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // pic_height_in_map_units_minus1
    frame_mbs_only_flag = BIT(abSodbSPS, iBit);
    iBit++;
    if (!frame_mbs_only_flag)
        iBit++; // mb_adaptive_frame_field_flag
    iBit++; // direct_8x8_inference_flag
    frame_cropping_flag=BIT(abSodbSPS, iBit);// frame_cropping_flag
    iBit++;
    if (frame_cropping_flag) 
    { 
        frame_crop_left_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // frame_crop_left_offset
        frame_crop_right_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // frame_crop_right_offset
        frame_crop_top_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // frame_crop_top_offset
        frame_crop_bottom_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // frame_crop_bottom_offset
    }
    
    // ��߼��㹫ʽ
    o_ptH265Extradata->pic_width = (pic_width_in_mbs_minus1+1) * 16;
    o_ptH265Extradata->pic_height = (2 - frame_mbs_only_flag)* (pic_height_in_map_units_minus1 +1) * 16;
    if(frame_cropping_flag)
    {
        unsigned int crop_unit_x;
        unsigned int crop_unit_y;
        if (0 == chroma_format_idc) // monochrome
        {
            crop_unit_x = 1;
            crop_unit_y = 2 - frame_mbs_only_flag;
        }
        else if (1 == chroma_format_idc) // 4:2:0
        {
            crop_unit_x = 2;
            crop_unit_y = 2 * (2 - frame_mbs_only_flag);
        }
        else if (2 == chroma_format_idc) // 4:2:2
        {
            crop_unit_x = 2;
            crop_unit_y = 2 - frame_mbs_only_flag;
        }
        else // 3 == sps.chroma_format_idc   // 4:4:4
        {
            crop_unit_x = 1;
            crop_unit_y = 2 -frame_mbs_only_flag;
        }
        
        o_ptH265Extradata->pic_width -= crop_unit_x * (frame_crop_left_offset + frame_crop_right_offset);
        o_ptH265Extradata->pic_height -= crop_unit_y * (frame_crop_top_offset + frame_crop_bottom_offset);
    }

    return 0;
}

/*****************************************************************************
-Fuction        : SpsToH265Extradata
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::SpsToH265Extradata(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_FlvH265Extradata *o_ptH265Extradata)
{
    int iRet = -1;
    unsigned char abSodbSPS[VIDEO_SPS_MAX_SIZE];
    int iSodbLen = 0;
    unsigned char sps;
    unsigned char sps_max_sub_layers_minus1;
    unsigned char sps_temporal_id_nesting_flag;
    unsigned char conformance_window_flag;
    int n;
    unsigned int pic_width_in_luma_samples;
    unsigned int pic_height_in_luma_samples;
    unsigned int conf_win_left_offset;
    unsigned int conf_win_right_offset;
    unsigned int conf_win_top_offset;
    unsigned int conf_win_bottom_offset;
    unsigned int sub_width,sub_height;
    unsigned char separate_colour_plane_flag = 0;
    
    if(NULL == i_pbSpsData || NULL == o_ptH265Extradata || 0 >= i_wSpsLen)
    {
        MH_LOGE("SpsToH265Extradata NULL %d \r\n", i_wSpsLen);
        return iRet;
    }
    memset(abSodbSPS,0,sizeof(abSodbSPS));
    iSodbLen = DecodeEBSP(i_pbSpsData, i_wSpsLen, abSodbSPS);
    if (iSodbLen < 12+3)
        return iRet;
    sps_max_sub_layers_minus1 = (abSodbSPS[2] >> 1) & 0x07;
    sps_temporal_id_nesting_flag = abSodbSPS[2] & 0x01;
    n = HevcProfileTierLevel(abSodbSPS + 3, iSodbLen - 3, sps_max_sub_layers_minus1, o_ptH265Extradata);
    if (n <= 0)
        return iRet;
    n = (n + 3) * 8;
    
    sps = (unsigned char)H264ReadBitByUE(abSodbSPS, iSodbLen, &n);
    o_ptH265Extradata->chromaFormat = (unsigned char)H264ReadBitByUE(abSodbSPS, iSodbLen, &n);
    if (3 == o_ptH265Extradata->chromaFormat)
    {
        separate_colour_plane_flag=BIT(abSodbSPS, n);
        n++;
    }
    pic_width_in_luma_samples=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // pic_width_in_luma_samples
    pic_height_in_luma_samples=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // pic_height_in_luma_samples
    conformance_window_flag = BIT(abSodbSPS, n); 
    n++; // conformance_window_flag
    if (conformance_window_flag)
    {
        conf_win_left_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // conf_win_left_offset
        conf_win_right_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // conf_win_right_offset
        conf_win_top_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // conf_win_top_offset
        conf_win_bottom_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // conf_win_bottom_offset
    }
    o_ptH265Extradata->bitDepthLumaMinus8 = (unsigned char)H264ReadBitByUE(abSodbSPS, iSodbLen, &n);
    o_ptH265Extradata->bitDepthChromaMinus8 = (unsigned char)H264ReadBitByUE(abSodbSPS, iSodbLen, &n);
    
    o_ptH265Extradata->pic_width = pic_width_in_luma_samples;
    o_ptH265Extradata->pic_height = pic_height_in_luma_samples;
    if (conformance_window_flag)
    {
        sub_width=((1==o_ptH265Extradata->chromaFormat)||(2 == o_ptH265Extradata->chromaFormat))&&(0==separate_colour_plane_flag)?2:1;
        sub_height=(1==o_ptH265Extradata->chromaFormat)&& (0 == separate_colour_plane_flag)?2:1;
        o_ptH265Extradata->pic_width -= (sub_width*conf_win_right_offset + sub_width*conf_win_left_offset);
        o_ptH265Extradata->pic_height -= (sub_height*conf_win_bottom_offset + sub_height*conf_win_top_offset);
    }
    return 0;
}

/*****************************************************************************
-Fuction        : DecodeEBSP
-Description    : �ѿǲ���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::DecodeEBSP(unsigned char* nalu, int bytes, unsigned char* sodb)
{
    int i, j;
    for (j = i = 0; i < bytes; i++)
    {
        if (i + 2 < bytes && 0 == nalu[i] && 0 == nalu[i + 1] && 0x03 == nalu[i + 2])
        {
            sodb[j++] = nalu[i];
            sodb[j++] = nalu[i + 1];
            i += 2;
        }
        else
        {
            sodb[j++] = nalu[i];
        }
    }
    return j;
}

/*****************************************************************************
-Fuction        : H264ReadBitByUE ���ֽ�����
-Description    : ָ�����ײ����룬ue(v)�Ľ���
��һ����ÿ�ζ�ȡһ�����أ������0�ͼ�������ֱ������1Ϊֹ��
��ʱ��ȡ���صĸ�����leadingZeroBits
�ڶ������ӵ�һ�������ı���1����˳���leadingZeroBits       
������
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
unsigned int FlvHandle::H264ReadBitByUE(unsigned char* data, int bytes, int* offset)
{
    int bit, i;
    int leadingZeroBits = -1;

    for (bit = 0; !bit && *offset / 8 < bytes; ++leadingZeroBits)
    {
        bit = (data[*offset / 8] >> (7 - (*offset % 8))) & 0x01;
        ++*offset;
    }

    bit = 0;
    //assert(leadingZeroBits < 32);

    for (i = 0; i < leadingZeroBits && *offset / 8 < bytes; i++)
    {
        bit = (bit << 1) | ((data[*offset / 8] >> (7 - (*offset % 8))) & 0x01);
        ++*offset;
    }

    return (unsigned int)((1 << leadingZeroBits) - 1 + bit);
}

/*****************************************************************************
-Fuction        : hevc_profile_tier_level
-Description    : �ѿǲ���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::HevcProfileTierLevel(unsigned char* nalu, int bytes, unsigned char maxNumSubLayersMinus1,T_FlvH265Extradata* hevc)
{
    int n;
    unsigned char i;
    unsigned char sub_layer_profile_present_flag[8];
    unsigned char sub_layer_level_present_flag[8];

    if (bytes < 12)
        return -1;

    hevc->general_profile_space = (nalu[0] >> 6) & 0x03;
    hevc->general_tier_flag = (nalu[0] >> 5) & 0x01;
    hevc->general_profile_idc = nalu[0] & 0x1f;

    hevc->general_profile_compatibility_flags = 0;
    hevc->general_profile_compatibility_flags |= nalu[1] << 24;
    hevc->general_profile_compatibility_flags |= nalu[2] << 16;
    hevc->general_profile_compatibility_flags |= nalu[3] << 8;
    hevc->general_profile_compatibility_flags |= nalu[4];

    hevc->general_constraint_indicator_flags = 0;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[5]) << 40;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[6]) << 32;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[7]) << 24;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[8]) << 16;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[9]) << 8;
    hevc->general_constraint_indicator_flags |= nalu[10];

    hevc->general_level_idc = nalu[11];
    if (maxNumSubLayersMinus1 < 1)
        return 12;

    if (bytes < 14)
        return -1; // error

    for (i = 0; i < maxNumSubLayersMinus1; i++)
    {
        sub_layer_profile_present_flag[i] = BIT(nalu, 12 * 8 + i * 2);
        sub_layer_level_present_flag[i] = BIT(nalu, 12 * 8 + i * 2 + 1);
    }

    n = 12 + 2;
    for (i = 0; i < maxNumSubLayersMinus1; i++)
    {
        if(sub_layer_profile_present_flag[i])
            n += 11;
        if (sub_layer_level_present_flag[i])
            n += 1;
    }

    return bytes < n ? n : -1;
}


/*****************************************************************************
-Fuction        : FlvReadHeader
-Description    : FlvReadHeader
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::FlvReadHeader(unsigned char* i_pbBuf,unsigned int i_dwLen)
{
    unsigned int dwPreviousTagSize0;
    T_FlvHeader tFlvHeader;

	if (i_dwLen < FLV_HEADER_LEN ||NULL == i_pbBuf)
	{
        MH_LOGE("FlvReadHeader NULL %d \r\n", i_dwLen);
		return -1;
	}
	memset(&tFlvHeader,0,sizeof(T_FlvHeader));
	if(FLV_HEADER_LEN != ParseFlvHeader(i_pbBuf,i_dwLen,&tFlvHeader))
    {
        MH_LOGE("FlvReadHeader err %d \r\n", i_dwLen);
        return -1;
    }
	if (tFlvHeader.dwOffset < FLV_HEADER_LEN ||tFlvHeader.dwOffset > FLV_HEADER_LEN + 4096)
	{
        MH_LOGE("FlvReadHeader bOffset err %d \r\n", tFlvHeader.dwOffset);
		return -1;
	}

	// PreviousTagSize0
	Read32BE((i_pbBuf + tFlvHeader.dwOffset),&dwPreviousTagSize0);
	if(0 != dwPreviousTagSize0)
    {
        MH_LOGE("FlvReadHeader dwPreviousTagSize0 err %d \r\n", dwPreviousTagSize0);
        return -1;
    }
	return tFlvHeader.dwOffset+FLV_PRE_TAG_LEN;
}

/*****************************************************************************
-Fuction        : ParseFlvHeader
-Description    : ParseFlvHeader
-Input          : 
-Output         : 
-Return         :  must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::FlvReadTag(unsigned char* i_pbBuf,unsigned int i_dwLen,T_FlvTag * o_ptFlvTag)
{
	unsigned int dwProcessedLen = 0;
	T_FlvTagHeader tFlvTagHeader;
    unsigned int dwPreviousTagSize;

    if (i_dwLen < FLV_TAG_HEADER_LEN ||NULL == i_pbBuf ||NULL == o_ptFlvTag|| (0 != o_ptFlvTag->dwDataMaxLen&&NULL == o_ptFlvTag->pbTagData))
    {
        MH_LOGE("FlvReadTagHeader NULL %d %d \r\n", i_dwLen,o_ptFlvTag->dwDataMaxLen);
        return -1;
    }
    memset(&tFlvTagHeader,0,sizeof(T_FlvTagHeader));
	if (FLV_TAG_HEADER_LEN != ParseFlvTagHeader(i_pbBuf, i_dwLen, &tFlvTagHeader))
    {
        MH_LOGE("FlvReadTagHeader err %d \r\n", i_dwLen);
        return -1;
    }

	if (i_dwLen < tFlvTagHeader.dwSize+FLV_TAG_HEADER_LEN|| (0 != o_ptFlvTag->dwDataMaxLen&&o_ptFlvTag->dwDataMaxLen < tFlvTagHeader.dwSize))
    {
        MH_LOGE("FlvReadTagHeader err i_dwLen%d,dwSize%d,dwDataMaxLen%d \r\n", i_dwLen,tFlvTagHeader.dwSize,o_ptFlvTag->dwDataMaxLen);
        return -1;
    }
	if(0 != tFlvTagHeader.dwStreamId)
    {
        MH_LOGE("FlvReadTagHeader dwStreamId err %d \r\n", tFlvTagHeader.dwStreamId);// StreamID Always 0
        return -1;
    }
	// PreviousTagSizeN
	Read32BE((i_pbBuf + FLV_TAG_HEADER_LEN+tFlvTagHeader.dwSize),&dwPreviousTagSize);
	if(FLV_TAG_HEADER_LEN+tFlvTagHeader.dwSize != dwPreviousTagSize)
    {
        MH_LOGE("FlvReadTagHeader dwPreviousTagSize err %d,%d,%d \r\n",FLV_TAG_HEADER_LEN,tFlvTagHeader.dwSize, dwPreviousTagSize);
        return -1;
    }
    
    memcpy(&o_ptFlvTag->tTagHeader,&tFlvTagHeader,sizeof(T_FlvTagHeader));
    if(0 != o_ptFlvTag->dwDataMaxLen)
        memcpy(o_ptFlvTag->pbTagData,i_pbBuf+FLV_TAG_HEADER_LEN,tFlvTagHeader.dwSize);
    else
        o_ptFlvTag->pbTagData = i_pbBuf+FLV_TAG_HEADER_LEN;
    o_ptFlvTag->dwDataCurLen = tFlvTagHeader.dwSize;
    
    dwProcessedLen = FLV_TAG_HEADER_LEN+tFlvTagHeader.dwSize+sizeof(dwPreviousTagSize);
    return dwProcessedLen;
}


/*****************************************************************************
-Fuction        : ParseFlvHeader
-Description    : ParseFlvHeader
-Input          : 
-Output         : 
-Return         :  must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::ParseFlvTagHeader(unsigned char* i_pbBuf,unsigned int i_dwLen,T_FlvTagHeader * o_ptFlvTagHeader)
{
    if (i_dwLen < FLV_TAG_HEADER_LEN ||NULL == i_pbBuf ||NULL == o_ptFlvTagHeader)
    {
        MH_LOGE("ParseFlvTagHeader NULL %d \r\n", i_dwLen);
        return -1;
    }

	// TagType
	o_ptFlvTagHeader->bType = i_pbBuf[0] & 0x1F;
	o_ptFlvTagHeader->bFilter = (i_pbBuf[0] >> 5) & 0x01;
    if (FLV_TAG_AUDIO_TYPE != o_ptFlvTagHeader->bType && FLV_TAG_VIDEO_TYPE != o_ptFlvTagHeader->bType && 
    FLV_TAG_SCRIPT_TYPE != o_ptFlvTagHeader->bType)
    {
        MH_LOGE("ParseFlvTagHeader err %d \r\n", o_ptFlvTagHeader->bType);
        return -1;
    }

	// DataSize
	o_ptFlvTagHeader->dwSize= ((unsigned int)i_pbBuf[1] << 16) | ((unsigned int)i_pbBuf[2] << 8) | i_pbBuf[3];

	// TimestampExtended | Timestamp
	o_ptFlvTagHeader->dwTimestamp= ((unsigned int)i_pbBuf[4] << 16) | ((unsigned int)i_pbBuf[5] << 8) | i_pbBuf[6] | ((unsigned int)i_pbBuf[7] << 24);

	// StreamID Always 0
	o_ptFlvTagHeader->dwStreamId= ((unsigned int)i_pbBuf[8] << 16) | ((unsigned int)i_pbBuf[9] << 8) | i_pbBuf[10];
	//assert(0 == tag->streamId);

	return FLV_TAG_HEADER_LEN;
}

/*****************************************************************************
-Fuction        : ParseFlvHeader
-Description    : ParseFlvHeader
-Input          : 
-Output         : 
-Return         :  must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvHandle::ParseFlvHeader(unsigned char* i_pbBuf,unsigned int i_dwLen,T_FlvHeader * o_ptFlvHeader)
{
	if (i_dwLen < FLV_HEADER_LEN || 'F' != i_pbBuf[0] || 'L' != i_pbBuf[1] || 'V' != i_pbBuf[2])
	{
        MH_LOGE("ParseFlvHeader NULL %d \r\n", i_dwLen);
		return -1;
	}
	if (0x00 != (i_pbBuf[4] & 0xF8) || 0x00 != (i_pbBuf[4] & 0x20))
	{
        MH_LOGE("ParseFlvHeader err %d \r\n", i_pbBuf[4]);//Ԥ��Ϊ0
		return -1;
	}
	
	o_ptFlvHeader->FLV[0] = i_pbBuf[0];
	o_ptFlvHeader->FLV[1] = i_pbBuf[1];
	o_ptFlvHeader->FLV[2] = i_pbBuf[2];
	o_ptFlvHeader->bVersion = i_pbBuf[3];
	o_ptFlvHeader->bAudio = (i_pbBuf[4] >> 2) & 0x01;
	o_ptFlvHeader->bVideo = i_pbBuf[4] & 0x01;
	Read32BE((i_pbBuf + 5),&o_ptFlvHeader->dwOffset);

	return FLV_HEADER_LEN;
}

