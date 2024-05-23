/*****************************************************************************
* Copyright (C) 2023-2028 HANGZHOU JFTECH CO., LTD. All rights reserved.
------------------------------------------------------------------------------
* File Module       :   FMP4Handle.h
* Description       :   FMP4Handle operation center
                        FMP4封装处理(默认是缓存一个GOP再打包)
                        
* Created           :   2023.11.21.
* Author            :   Yu Weifeng
* Function List     :   
* Last Modified     :   
* History           :   
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "FMP4Handle.h"


#define FMP4_MEDIA_BUF_MAX_LEN (3*1024*1024)
#define FMP4_HEADER_BUF_MAX_LEN (3*1024)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define BIT(ptr, off) (((ptr)[(off) / 8] >> (7 - ((off) % 8))) & 0x01)


/*****************************************************************************
-Fuction        : FMP4Handle
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
FMP4Handle::FMP4Handle()
{
    m_iHeaderCreatedFlag = 0;
    m_iFragSeq = 0;
    m_FMP4MediaList.clear();
    m_pbMediaData = new unsigned char[FMP4_MEDIA_BUF_MAX_LEN];
    m_iCurMediaDataLen = 0;
    m_pbFmp4Header = new unsigned char[FMP4_HEADER_BUF_MAX_LEN];
    m_iFmp4HeaderLen = 0;
    m_iFindedKeyFrame = 0;
}


/*****************************************************************************
-Fuction        : FMP4Handle
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
FMP4Handle::~FMP4Handle()
{
    if(m_FMP4MediaList.size()>0)
    {
        DelAllFrame();
    }
    if(NULL != m_pbMediaData)
    {
        delete[] m_pbMediaData;
        m_pbMediaData = NULL;
        m_iCurMediaDataLen = 0;
    }
    if(NULL != m_pbFmp4Header)
    {
        delete[] m_pbFmp4Header;
        m_pbFmp4Header = NULL;
        m_iFmp4HeaderLen = 0;
    }
}


/*****************************************************************************
-Fuction        : FMP4Handle
-Description    : 
i_iForcePack=0，默认收到i帧打包，=1强制打包
o_piHeaderOffset=NULL，默认头部信息(ftyp ,moov)不打包到缓冲区，否则打包进去并输出长度
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::GetMuxData(T_Fmp4AnnexbFrameInfo *i_ptFmp4FrameInfo,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen,int *o_piHeaderOffset,int i_iForcePack)
{
    int iRet = -1;
    int iDataLen = 0;
    
    if(NULL == i_ptFmp4FrameInfo ||NULL == i_ptFmp4FrameInfo->pbFrameStartPos ||NULL == o_pbBuf)
    {
        FMP4_LOGE("GetMuxData err NULL\r\n");
        return iRet;
    }
    if(m_iFindedKeyFrame==0&&i_ptFmp4FrameInfo->eFrameType!=FMP4_VIDEO_KEY_FRAME)
    {
        FMP4_LOGW("Skip frame:%d\r\n",i_ptFmp4FrameInfo->eFrameType);//内部打包开始时间使用第一个i帧为参考基准时间
        return 0;//所以第一帧必须i帧，其余帧要过滤掉
    }
    SaveFrame(i_ptFmp4FrameInfo);
    if(m_iFindedKeyFrame!=0 && m_FMP4MediaList.size()>1 && (i_ptFmp4FrameInfo->eFrameType==FMP4_VIDEO_KEY_FRAME ||0 != i_iForcePack))
    {
        if(0==m_iHeaderCreatedFlag)
        {
            m_iFmp4HeaderLen=m_FMP4.CreateHeader(&m_FMP4MediaList,m_pbFmp4Header,FMP4_HEADER_BUF_MAX_LEN);
            m_iHeaderCreatedFlag=1;
            if(NULL != o_piHeaderOffset)
            {
                memcpy(o_pbBuf,m_pbFmp4Header,m_iFmp4HeaderLen);
                iDataLen = m_iFmp4HeaderLen;
                *o_piHeaderOffset = iDataLen;
            }
        }
        iDataLen+=m_FMP4.CreateSegment(&m_FMP4MediaList,++m_iFragSeq,o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
        DelAllFrame();
        SaveFrame(i_ptFmp4FrameInfo);
    }
    if(i_ptFmp4FrameInfo->eFrameType==FMP4_VIDEO_KEY_FRAME)
    {
        m_iFindedKeyFrame=1;
    }
    return iDataLen;
}


/*****************************************************************************
-Fuction        : FMP4Handle
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::GetMuxHeader(unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    
    if(NULL == o_pbBuf)
    {
        FMP4_LOGE("GetMuxHeader err NULL\r\n");
        return iRet;
    }
    if(0 == m_iFmp4HeaderLen)
    {
        FMP4_LOGW("GetMuxHeader m_iFmp4HeaderLen NULL\r\n");
        return 0;
    }
    if(m_iFmp4HeaderLen > i_dwMaxBufLen)
    {
        FMP4_LOGW("GetMuxHeader i_dwMaxBufLen err%d,%d\r\n",m_iFmp4HeaderLen,i_dwMaxBufLen);
        return iRet;
    }
    memcpy(o_pbBuf,m_pbFmp4Header,m_iFmp4HeaderLen);
    iDataLen = m_iFmp4HeaderLen;
    return iDataLen;
}




/*****************************************************************************
-Fuction        : FMP4Handle
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::SaveFrame(T_Fmp4AnnexbFrameInfo *i_ptFmp4FrameInfo)
{
    int iRet = -1;
    int i = 0;
    T_Fmp4FrameInfo tFmp4FrameInfo;
    unsigned char* pbVideoData = NULL;
    unsigned char *pbNaluBuf=NULL;
    int iNaluLen=0;
    
    if(NULL == i_ptFmp4FrameInfo ||NULL == i_ptFmp4FrameInfo->pbFrameStartPos ||i_ptFmp4FrameInfo->iFrameLen <= 0)
    {
        FMP4_LOGE("SaveFrame err NULL\r\n");
        return iRet;
    }
    if(i_ptFmp4FrameInfo->iFrameLen + m_iCurMediaDataLen > FMP4_MEDIA_BUF_MAX_LEN)
    {
        FMP4_LOGE("SaveFrame err %d,%d\r\n",i_ptFmp4FrameInfo->iFrameLen,m_iCurMediaDataLen);
        return iRet;
    }
    memset(&tFmp4FrameInfo,0,sizeof(T_Fmp4FrameInfo));
    tFmp4FrameInfo.eFrameType=i_ptFmp4FrameInfo->eFrameType;
    tFmp4FrameInfo.eEncType = i_ptFmp4FrameInfo->eEncType;
    tFmp4FrameInfo.ddwTimeStamp=i_ptFmp4FrameInfo->ddwTimeStamp;
    tFmp4FrameInfo.ddwSampleRate=i_ptFmp4FrameInfo->ddwSampleRate;
    
    tFmp4FrameInfo.pbFrameStartPos = m_pbMediaData+m_iCurMediaDataLen;
    switch(tFmp4FrameInfo.eEncType)
    {
        case FMP4_ENC_H264 :
        case FMP4_ENC_H265 :
        {
            pbVideoData = tFmp4FrameInfo.pbFrameStartPos;
            for(i=0;i<(int)i_ptFmp4FrameInfo->dwNaluCount;i++)
            {
                pbNaluBuf=i_ptFmp4FrameInfo->atNaluInfo[i].pbData;
                iNaluLen=i_ptFmp4FrameInfo->atNaluInfo[i].dwDataLen;
                //drop 0001
                if (pbNaluBuf[0] == 0 && pbNaluBuf[1] == 0 && pbNaluBuf[2] == 1) 
                {
                    pbNaluBuf   += 3;
                    iNaluLen    -= 3;
                }
                if (pbNaluBuf[0] == 0 && pbNaluBuf[1] == 0 && pbNaluBuf[2] == 0 && pbNaluBuf[3] == 1) 
                {
                    pbNaluBuf   += 4;
                    iNaluLen    -= 4;
                }
                Write32BE(pbVideoData,iNaluLen);//这个长度指不带00 00 00 01的数据长度,
                pbVideoData += sizeof(iNaluLen);//也就是全裸数据的长度
                memcpy(pbVideoData,pbNaluBuf,iNaluLen);
                pbVideoData += iNaluLen;
            }
            tFmp4FrameInfo.iFrameLen = pbVideoData-tFmp4FrameInfo.pbFrameStartPos;
            tFmp4FrameInfo.dwNaluLenSize = 4;
            m_iCurMediaDataLen+=tFmp4FrameInfo.iFrameLen;
            if (tFmp4FrameInfo.eFrameType == FMP4_VIDEO_KEY_FRAME) 
            {
                tFmp4FrameInfo.iEncExtraDataLen=GenerateVideoExtraData(tFmp4FrameInfo.eEncType,&i_ptFmp4FrameInfo->tVideoEncParam,tFmp4FrameInfo.abEncExtraData,sizeof(tFmp4FrameInfo.abEncExtraData));
            }
            break;
        }
        case FMP4_ENC_AAC :
        {
            memcpy(tFmp4FrameInfo.pbFrameStartPos,i_ptFmp4FrameInfo->pbFrameStartPos+7,i_ptFmp4FrameInfo->iFrameLen-7);
            tFmp4FrameInfo.iFrameLen = i_ptFmp4FrameInfo->iFrameLen-7;
            m_iCurMediaDataLen+=tFmp4FrameInfo.iFrameLen;
            tFmp4FrameInfo.iEncExtraDataLen=CreateAudioSpecCfgAAC(i_ptFmp4FrameInfo->ddwSampleRate,i_ptFmp4FrameInfo->tAudioEncParam.dwChannels,tFmp4FrameInfo.abEncExtraData);
            break;
        }
		case FMP4_ENC_G711A:
		case FMP4_ENC_G711U:
        {
            memcpy(tFmp4FrameInfo.pbFrameStartPos,i_ptFmp4FrameInfo->pbFrameStartPos,i_ptFmp4FrameInfo->iFrameLen);
            tFmp4FrameInfo.iFrameLen = i_ptFmp4FrameInfo->iFrameLen;
            m_iCurMediaDataLen+=tFmp4FrameInfo.iFrameLen;
            break;
        }
        default :
        {
            FMP4_LOGE("SaveFrame tFmp4FrameInfo->eEncType err%d\r\n",tFmp4FrameInfo.eEncType);
            return iRet;
        }
    }
    tFmp4FrameInfo.tVideoEncParam.dwHeight= i_ptFmp4FrameInfo->tVideoEncParam.dwHeight;
    tFmp4FrameInfo.tVideoEncParam.dwWidth= i_ptFmp4FrameInfo->tVideoEncParam.dwWidth;
    tFmp4FrameInfo.tAudioEncParam.dwChannels= i_ptFmp4FrameInfo->tAudioEncParam.dwChannels;
    tFmp4FrameInfo.tAudioEncParam.dwBitsPerSample= i_ptFmp4FrameInfo->tAudioEncParam.dwBitsPerSample;

    m_FMP4MediaList.push_back(tFmp4FrameInfo);
    return 0;
}





/*****************************************************************************
-Fuction        : FMP4Handle
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::DelAllFrame()
{
    m_FMP4MediaList.clear();
    m_iCurMediaDataLen = 0;
    return 0;
}




/*****************************************************************************
-Fuction        : CreateAudioDataTagHeader
-Description    : CreateAudioDataTagHeader
-Input          : 
-Output         : 
-Return         : iVideoDataLen must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::CreateAudioSpecCfgAAC(unsigned int i_dwFrequency,unsigned int i_dwChannels,unsigned char *o_pbAudioData)
{
    int iAudioSpecCfgLen = 0;
    unsigned char bProfile = 1;
    unsigned char bAudioObjectType = 0;
    unsigned char bChannelConfiguration = 0;//声道模式0 = 单声道1 = 双声道（立体声）
    unsigned char bSamplingFrequencyIndex = 0;
    int i = 0;
    ///索引表  
    static unsigned int const s_adwSamplingFrequencyTable[16] = {
      96000, 88200, 64000, 48000,
      44100, 32000, 24000, 22050,
      16000, 12000, 11025, 8000,
      7350,  0,     0,      0
    };

    bProfile = 1;
    bAudioObjectType = bProfile + 1;  ///其中profile=1;  
    for (i = 0; i < 16; i++)
    {
        if (s_adwSamplingFrequencyTable[i] == i_dwFrequency)
        {
            bSamplingFrequencyIndex = (unsigned char)i;
            break;
        }
    }
    if(i_dwChannels>0)
        bChannelConfiguration = (unsigned char)(i_dwChannels-1);//声道=通道-1
    else
        bChannelConfiguration=0;
    o_pbAudioData[0] = (bAudioObjectType << 3) | (bSamplingFrequencyIndex >> 1);
    o_pbAudioData[1] = (bSamplingFrequencyIndex << 7) | (bChannelConfiguration << 3);
    iAudioSpecCfgLen = 2;

    return iAudioSpecCfgLen;
}


/*****************************************************************************
-Fuction        : GenerateH264ExtraData
-Description    : AVC sequence header or extradata
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::GenerateVideoExtraData(E_FMP4_ENC_TYPE eEncType,T_Fmp4AnnexbVideoEncParam *i_ptVideoEncParam,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iDataLen = 0;
    
    if(NULL == i_ptVideoEncParam || NULL == o_pbBuf )
    {
		FMP4_LOGE("GenerateVideoExtraData NULL %d,%d \r\n", i_dwMaxBufLen, (11 + i_ptVideoEncParam->iSizeOfSPS + i_ptVideoEncParam->iSizeOfPPS));
        return iDataLen;
    }
    switch(eEncType)
    {
        case FMP4_ENC_H264 :
        {
            iDataLen=GenerateH264ExtraData(i_ptVideoEncParam,o_pbBuf,i_dwMaxBufLen);
            break;
        }
        case FMP4_ENC_H265 :
        {
            iDataLen=GenerateH265ExtraData(i_ptVideoEncParam,o_pbBuf,i_dwMaxBufLen);
            break;
        }
        default :
        {
            FMP4_LOGE("GenerateVideoExtraData ->eEncType err%d\r\n",eEncType);
            break;
        }
    }
    return iDataLen;
}





/*****************************************************************************
-Fuction        : GenerateH264ExtraData
-Description    : AVC sequence header or extradata
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::GenerateH264ExtraData(T_Fmp4AnnexbVideoEncParam *i_ptVideoEncParam,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iDataLen = 0;
    unsigned char* pbData = NULL;
    unsigned short wLen = 0;
    unsigned char* pbVideoData = NULL;
    
    if(NULL == i_ptVideoEncParam || NULL == o_pbBuf ||i_ptVideoEncParam->iSizeOfSPS <=0 ||
    i_dwMaxBufLen < (11+i_ptVideoEncParam->iSizeOfSPS+i_ptVideoEncParam->iSizeOfPPS))
    {
        FMP4_LOGE("GenerateH264ExtraData NULL %d,%d \r\n", i_dwMaxBufLen,(11+i_ptVideoEncParam->iSizeOfSPS+i_ptVideoEncParam->iSizeOfPPS));
        return iDataLen;
    }
    pbData = o_pbBuf;
    pbData[iDataLen] = 0x1;////AVC sequence header or extradata版本号 1
    iDataLen += 1;
    memcpy(&pbData[iDataLen], &i_ptVideoEncParam->abSPS[1], 3);
    iDataLen += 3;
    pbData[iDataLen] = 0xff;//nalu size 4 字节
    iDataLen += 1;
    
    pbData[iDataLen] = 0xe1;//SPS 个数 =1
    iDataLen += 1;
    wLen = (unsigned short)i_ptVideoEncParam->iSizeOfSPS;
    pbVideoData = &pbData[iDataLen];
    Write16BE(pbVideoData,wLen);
    iDataLen += sizeof(wLen);
    memcpy(&pbData[iDataLen], i_ptVideoEncParam->abSPS,wLen);
    iDataLen += wLen;
    
    pbData[iDataLen] = 0x1;//PPS 个数 =1
    iDataLen += 1;
    wLen = (unsigned short)i_ptVideoEncParam->iSizeOfPPS;
    pbVideoData = &pbData[iDataLen];
    Write16BE(pbVideoData,wLen);
    iDataLen += sizeof(wLen);
    memcpy(&pbData[iDataLen], i_ptVideoEncParam->abPPS,wLen);
    iDataLen += wLen;
    
    return iDataLen;
}


/*****************************************************************************
-Fuction        : GenerateH265ExtraData
-Description    : 
23字节解析可参考SrsRawHEVCStream::mux_sequence_header,需要通过解析vps,sps才能得出这个HEVC extradata
或者参考media server中的hevc_profile_tier_level调用
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::GenerateH265ExtraData(T_Fmp4AnnexbVideoEncParam *i_ptVideoEncParam,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iVideoDataLen = 0;
    unsigned char* pbVideoData = NULL;
    unsigned short wLen = 0;
    T_Fmp4H265ExtraData tFmp4H265ExtraData;
    
    if(NULL == i_ptVideoEncParam || NULL == o_pbBuf ||i_ptVideoEncParam->iSizeOfSPS <=0 ||
    i_dwMaxBufLen < (23+i_ptVideoEncParam->iSizeOfSPS+i_ptVideoEncParam->iSizeOfPPS+i_ptVideoEncParam->iSizeOfVPS))
    {
        FMP4_LOGE("GenerateH264ExtraData NULL %d,%d \r\n", i_dwMaxBufLen,(23+i_ptVideoEncParam->iSizeOfSPS+i_ptVideoEncParam->iSizeOfPPS));
        return iVideoDataLen;
    }

    memset(&tFmp4H265ExtraData,0,sizeof(T_Fmp4H265ExtraData));
    if(0 != AnnexbToH265Extradata(i_ptVideoEncParam,&tFmp4H265ExtraData))
    {
        FMP4_LOGE("AnnexbToH265Extradata err %d\r\n", iVideoDataLen);
        return iVideoDataLen;
    }
    // HEVCDecoderConfigurationRecord
    // ISO/IEC 14496-15:2017
    // 8.3.3.1.2 Syntax
    pbVideoData = o_pbBuf;
    *pbVideoData = tFmp4H265ExtraData.configurationVersion;
    pbVideoData++;
    // general_profile_space + general_tier_flag + general_profile_idc
    *pbVideoData = ((tFmp4H265ExtraData.general_profile_space & 0x03) << 6) | ((tFmp4H265ExtraData.general_tier_flag & 0x01) << 5) | (tFmp4H265ExtraData.general_profile_idc & 0x1F);
    pbVideoData++;
    
    // general_profile_compatibility_flags
    Write32BE(pbVideoData, tFmp4H265ExtraData.general_profile_compatibility_flags);
    pbVideoData += sizeof(tFmp4H265ExtraData.general_profile_compatibility_flags);
    // general_constraint_indicator_flags
    Write32BE(pbVideoData, (unsigned int)(tFmp4H265ExtraData.general_constraint_indicator_flags >> 16));
    pbVideoData += sizeof(unsigned int);
    Write16BE(pbVideoData, (unsigned short)tFmp4H265ExtraData.general_constraint_indicator_flags);
    pbVideoData += sizeof(unsigned short);
    // general_level_idc
    *pbVideoData = tFmp4H265ExtraData.general_level_idc;
    pbVideoData++;
    // min_spatial_segmentation_idc
    Write16BE(pbVideoData, 0xF000 | tFmp4H265ExtraData.min_spatial_segmentation_idc);
    pbVideoData += sizeof(unsigned short);
    *pbVideoData = 0xFC | tFmp4H265ExtraData.parallelismType;
    pbVideoData++;
    *pbVideoData = 0xFC | tFmp4H265ExtraData.chromaFormat;
    pbVideoData++;
    *pbVideoData = 0xF8 | tFmp4H265ExtraData.bitDepthLumaMinus8;
    pbVideoData++;
    *pbVideoData = 0xF8 | tFmp4H265ExtraData.bitDepthChromaMinus8;
    pbVideoData++;
    Write16BE(pbVideoData,tFmp4H265ExtraData.avgFrameRate);
    pbVideoData += sizeof(unsigned short);
    *pbVideoData = (tFmp4H265ExtraData.constantFrameRate << 6) | ((tFmp4H265ExtraData.numTemporalLayers & 0x07) << 3) | ((tFmp4H265ExtraData.temporalIdNested & 0x01) << 2) | (tFmp4H265ExtraData.lengthSizeMinusOne & 0x03);
    pbVideoData++;
    *pbVideoData = tFmp4H265ExtraData.numOfArrays;
    pbVideoData++;
         
    // numOfVideoParameterSets, always 1
    char numOfVideoParameterSets[2] = { 0x00,0x01 };
  // vps
    // nal_type
    *pbVideoData = (i_ptVideoEncParam->abVPS[0] >> 1) & 0x3f;
    pbVideoData++;
    // numOfVideoParameterSets, always 1
    memcpy(pbVideoData, numOfVideoParameterSets, sizeof(numOfVideoParameterSets));
    pbVideoData += sizeof(numOfVideoParameterSets);
    // videoParameterSetLength
    Write16BE(pbVideoData,(unsigned short)i_ptVideoEncParam->iSizeOfVPS);
    pbVideoData += sizeof(unsigned short);
    // videoParameterSetNALUnit
    memcpy(pbVideoData,i_ptVideoEncParam->abVPS,i_ptVideoEncParam->iSizeOfVPS);
    pbVideoData += i_ptVideoEncParam->iSizeOfVPS;
 // sps
    // nal_type
    *pbVideoData = (i_ptVideoEncParam->abSPS[0] >> 1) & 0x3f;
    pbVideoData++;
    // numOfVideoParameterSets, always 1
    memcpy(pbVideoData, numOfVideoParameterSets, sizeof(numOfVideoParameterSets));
    pbVideoData += sizeof(numOfVideoParameterSets);
    // videoParameterSetLength
    Write16BE(pbVideoData,(unsigned short)i_ptVideoEncParam->iSizeOfSPS);
    pbVideoData += sizeof(unsigned short);
    // videoParameterSetNALUnit
    memcpy(pbVideoData,i_ptVideoEncParam->abSPS,i_ptVideoEncParam->iSizeOfSPS);
    pbVideoData += i_ptVideoEncParam->iSizeOfSPS;
// pps
    // nal_type
    *pbVideoData = (i_ptVideoEncParam->abPPS[0] >> 1) & 0x3f;
    pbVideoData++;
    // numOfVideoParameterSets, always 1
    memcpy(pbVideoData, numOfVideoParameterSets, sizeof(numOfVideoParameterSets));
    pbVideoData += sizeof(numOfVideoParameterSets);
    // videoParameterSetLength
    Write16BE(pbVideoData,(unsigned short)i_ptVideoEncParam->iSizeOfPPS);
    pbVideoData += sizeof(unsigned short);
    // videoParameterSetNALUnit
    memcpy(pbVideoData,i_ptVideoEncParam->abPPS,i_ptVideoEncParam->iSizeOfPPS);
    pbVideoData += i_ptVideoEncParam->iSizeOfPPS;

    iVideoDataLen = pbVideoData - o_pbBuf;
    return iVideoDataLen;
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
int FMP4Handle::AnnexbToH265Extradata(T_Fmp4AnnexbVideoEncParam *i_ptVideoEncParam,T_Fmp4H265ExtraData *o_ptH265Extradata)
{
    int iRet = -1;
    unsigned short wLen = 0;
    
    if(NULL == i_ptVideoEncParam || NULL == o_ptH265Extradata)
    {
        FMP4_LOGE("AnnexbToH265Extradata NULL %d \r\n", iRet);
        return iRet;
    }
    o_ptH265Extradata->configurationVersion = 1;
    o_ptH265Extradata->lengthSizeMinusOne = 3; // 4 bytes
    o_ptH265Extradata->numOfArrays = 3; // numOfArrays, default 3
    wLen = (unsigned short)i_ptVideoEncParam->iSizeOfVPS;
    iRet = VpsToH265Extradata(i_ptVideoEncParam->abVPS,wLen,o_ptH265Extradata);
    wLen = (unsigned short)i_ptVideoEncParam->iSizeOfSPS;
    iRet |= SpsToH265Extradata(i_ptVideoEncParam->abSPS,wLen,o_ptH265Extradata);
    return iRet;
}

/*****************************************************************************
-Fuction        : VpsToH265Extradata
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::VpsToH265Extradata(unsigned char *i_pbVpsData,unsigned short i_wVpsLen,T_Fmp4H265ExtraData *o_ptH265Extradata)
{
    int iRet = -1;
    unsigned char abSodbVPS[FMP4_VPS_MAX_SIZE];
    int iSodbLen = 0;
    unsigned char vps_max_sub_layers_minus1;
    unsigned char vps_temporal_id_nesting_flag;
    
    if(NULL == i_pbVpsData || NULL == o_ptH265Extradata)
    {
        FMP4_LOGE("VpsToH265Extradata NULL %d \r\n", i_wVpsLen);
        return iRet;
    }
    memset(abSodbVPS,0,sizeof(abSodbVPS));
    iSodbLen = DecodeEBSP(i_pbVpsData, i_wVpsLen, abSodbVPS);
    if (iSodbLen < 16 + 2)
    {
        FMP4_LOGE("VpsToH265Extradata iSodbLen err %d \r\n", iSodbLen);
        return iRet;
    }
    vps_max_sub_layers_minus1 = (abSodbVPS[3] >> 1) & 0x07;
    vps_temporal_id_nesting_flag = abSodbVPS[3] & 0x01;
    o_ptH265Extradata->numTemporalLayers = MAX(o_ptH265Extradata->numTemporalLayers, vps_max_sub_layers_minus1 + 1);
    o_ptH265Extradata->temporalIdNested = (o_ptH265Extradata->temporalIdNested || vps_temporal_id_nesting_flag) ? 1 : 0;
    iRet = HevcProfileTierLevel(abSodbVPS + 6, iSodbLen - 6, vps_max_sub_layers_minus1, o_ptH265Extradata);
    if(iRet < 0)
    {
        FMP4_LOGE("HevcProfileTierLevel err %d \r\n", i_wVpsLen);
        return iRet;
    }
    return 0;
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
int FMP4Handle::SpsToH265Extradata(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_Fmp4H265ExtraData *o_ptH265Extradata)
{
    int iRet = -1;
    unsigned char abSodbSPS[FMP4_SPS_MAX_SIZE];
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
        FMP4_LOGE("SpsToH265Extradata NULL %d \r\n", i_wSpsLen);
        return iRet;
    }
    memset(abSodbSPS,0,sizeof(abSodbSPS));
    iSodbLen = DecodeEBSP(i_pbSpsData, i_wSpsLen, abSodbSPS);
    if (iSodbLen < 12+3)
    {
        FMP4_LOGE("iSodbLen err %d \r\n", iSodbLen);
        return iRet;
    }
    sps_max_sub_layers_minus1 = (abSodbSPS[2] >> 1) & 0x07;
    sps_temporal_id_nesting_flag = abSodbSPS[2] & 0x01;
    n = HevcProfileTierLevel(abSodbSPS + 3, iSodbLen - 3, sps_max_sub_layers_minus1, o_ptH265Extradata);
    if (n <= 0)
    {
        FMP4_LOGE("HevcProfileTierLevel err %d \r\n", n);
        return iRet;
    }
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
-Description    : 脱壳操作
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::DecodeEBSP(unsigned char* nalu, int bytes, unsigned char* sodb)
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
-Fuction        : hevc_profile_tier_level
-Description    : 脱壳操作
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4Handle::HevcProfileTierLevel(unsigned char* nalu, int bytes, unsigned char maxNumSubLayersMinus1,T_Fmp4H265ExtraData* hevc)
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
-Fuction        : H264ReadBitByUE 读字节数据
-Description    : 指数哥伦布编码，ue(v)的解码
第一步：每次读取一个比特，如果是0就继续读，直至读到1为止，
此时读取比特的个数即leadingZeroBits
第二步：从第一步读到的比特1后，再顺序读leadingZeroBits       
个比特
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
unsigned int FMP4Handle::H264ReadBitByUE(unsigned char* data, int bytes, int* offset)
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


