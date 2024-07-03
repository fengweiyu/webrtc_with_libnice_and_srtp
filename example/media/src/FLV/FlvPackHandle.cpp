/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FlvPackHandle.cpp
* Description		: 	FlvPackHandle operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "MediaAdapter.h"
#include "FlvPackHandle.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>


using std::cout;//需要<iostream>
using std::endl;

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define Write16BE(p,value) \
do{ \
    p[0] = (unsigned char)((value >> 8) & 0xFF);  \
    p[1] = (unsigned char)((value) & 0xFF);    \
}while(0)

#define Write24BE(ptr,val) \
do{ \
    ptr[0] = (unsigned char)((val >> 16) & 0xFF); \
    ptr[1] = (unsigned char)((val >> 8) & 0xFF); \
    ptr[2] = (unsigned char)((val) & 0xFF);  \
}while(0)

#define Write32BE(p,value) \
do{ \
    p[0] = (unsigned char)((value >> 24) & 0xFF); \
    p[1] = (unsigned char)((value >> 16) & 0xFF); \
    p[2] = (unsigned char)((value >> 8) & 0xFF);  \
    p[3] = (unsigned char)((value) & 0xFF);    \
}while(0)



/*****************************************************************************
-Fuction		: FlvPackHandle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
FlvPackHandle::FlvPackHandle(int i_iEnhancedFlvFlag)
{
    m_pbFrameBuf = new unsigned char [FLV_FRAME_BUF_MAX_LEN];
    m_iFrameBufMaxLen = FLV_FRAME_BUF_MAX_LEN;
    m_iAudioSeqHeaderSended=0;
    
    m_iHeaderCreatedFlag = 0;
    m_iFindedKeyFrame = 0;
    m_iEnhancedFlvFlag=i_iEnhancedFlvFlag;
}
/*****************************************************************************
-Fuction		: ~FlvPackHandle
-Description	: ~FlvPackHandle
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
FlvPackHandle::~FlvPackHandle()
{
    if(NULL!= m_pbFrameBuf)
    {
        delete[] m_pbFrameBuf;
    }
    m_iFrameBufMaxLen = 0;
}


/*****************************************************************************
-Fuction        : GetMuxData
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvPackHandle::GetMuxData(T_MediaFrameInfo * i_ptFrameInfo,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    
    if(NULL == i_ptFrameInfo ||NULL == o_pbBuf)
    {
        MH_LOGE("GetMuxData err NULL\r\n");
        return iRet;
    }
    if(m_iFindedKeyFrame==0&&i_ptFrameInfo->eFrameType!=MEDIA_FRAME_TYPE_VIDEO_I_FRAME)
    {
        //MH_LOGW("Skip frame:%d\r\n",i_ptFrameInfo->eFrameType);//内部打包开始时间使用第一个i帧为参考基准时间
        //return 0;//所以第一帧必须i帧，其余帧要过滤掉
    }
    if(i_ptFrameInfo->eFrameType==MEDIA_FRAME_TYPE_VIDEO_I_FRAME)
    {
        m_iFindedKeyFrame=1;
    }
    if(0==m_iHeaderCreatedFlag /*&& i_ptFrameInfo->eFrameType==MEDIA_FRAME_TYPE_VIDEO_I_FRAME*/)
    {
        iRet=this->CreateHeader(o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
        if(iRet <= 0)
        {
            MH_LOGE("CreateHeader err %d \r\n",iRet);
            return -1;
        }
        iDataLen+=iRet;
        m_iHeaderCreatedFlag=1;
    }
    iRet=this->CreateTag(i_ptFrameInfo,o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
    if(iRet <= 0)
    {
        MH_LOGE("CreateTag err %d \r\n",iRet);
        return -1;
    }
    iDataLen+=iRet;
    return iDataLen;
}

/*****************************************************************************
-Fuction        : CreateHeader
-Description    : ParseFlvHeader
-Input          : 
-Output         : 
-Return         :  must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvPackHandle::CreateHeader(unsigned char* o_pbBuf,unsigned int i_dwMaxLen)
{
    int iRet = -1;
    int iLen = 0;
    T_FlvHeader tFlvHeader;
    unsigned int dwPreviousTagSize0=0;
    
	if (i_dwMaxLen < FLV_HEADER_LEN+sizeof(dwPreviousTagSize0) ||NULL == o_pbBuf)
	{
        MH_LOGE("CreateHeader NULL %d \r\n", i_dwMaxLen);
		return -1;
	}
	memset(&tFlvHeader,0,sizeof(T_FlvHeader));
	memcpy(tFlvHeader.FLV,"FLV",sizeof(tFlvHeader.FLV));
	tFlvHeader.bVersion = 1;
	tFlvHeader.bAudio = 1;
	tFlvHeader.bVideo = 1;
	tFlvHeader.dwOffset= FLV_HEADER_LEN;
	iLen=CreateFlvHeader(&tFlvHeader,o_pbBuf,i_dwMaxLen);
	memcpy(o_pbBuf+iLen,&dwPreviousTagSize0,sizeof(dwPreviousTagSize0));
	iLen+=sizeof(dwPreviousTagSize0);
	
	return iLen;
}

/*****************************************************************************
-Fuction        : CreateTag
-Description    : FLV_TAG_SCRIPT_TYPE 需要amf模块后续再加
-Input          : 
-Output         : 
-Return         :  must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvPackHandle::CreateTag(T_MediaFrameInfo * i_ptFrameInfo,unsigned char* o_pbBuf,unsigned int i_dwMaxLen)
{
    int iRet = -1;
    int iLen = 0;
    T_FlvTagHeader tFlvTagHeader;
    unsigned char bType=0;
    unsigned int dwPreviousTagSize=0;

    
	if (i_dwMaxLen < FLV_TAG_HEADER_LEN+FLV_PRE_TAG_LEN ||NULL == i_ptFrameInfo||NULL == o_pbBuf)
	{
        MH_LOGE("CreateTag NULL %d \r\n", i_dwMaxLen);
		return -1;
	}
    switch(i_ptFrameInfo->eFrameType)
    {
        case MEDIA_FRAME_TYPE_VIDEO_I_FRAME :
        {
            bType = FLV_TAG_VIDEO_TYPE;
            iLen = this->GenerateVideoData(i_ptFrameInfo, 1,m_pbFrameBuf,m_iFrameBufMaxLen);
            if(iLen <= 0)
            {
                MH_LOGE("GenerateVideoData err %d \r\n",iLen);
                return -1;
            }
            memset(&tFlvTagHeader,0,sizeof(T_FlvTagHeader));
            tFlvTagHeader.bFilter= 0;
            tFlvTagHeader.bType= bType;
            tFlvTagHeader.dwSize= iLen;
            tFlvTagHeader.dwTimestamp= i_ptFrameInfo->dwTimeStamp;
            tFlvTagHeader.dwStreamId= 0;// StreamID Always 0
            iLen = this->CreateFlvTagHeader(&tFlvTagHeader,o_pbBuf,i_dwMaxLen);
            if(iLen <= 0)
            {
                MH_LOGE("CreateFlvTagHeader err %d \r\n",iLen);
                return -1;
            }
            memcpy(o_pbBuf+iLen,m_pbFrameBuf,tFlvTagHeader.dwSize);
            iLen+=tFlvTagHeader.dwSize;
            Write32BE((o_pbBuf+iLen),iLen);//dwPreviousTagSize
            iLen+=sizeof(iLen);
            iRet =this->GenerateVideoData(i_ptFrameInfo,0,m_pbFrameBuf,m_iFrameBufMaxLen);
            if(iRet <= 0)
            {
                MH_LOGE("GenerateVideoData 0 err %d \r\n",iLen);
                return -1;
            }
            break;
        }
        case MEDIA_FRAME_TYPE_VIDEO_P_FRAME :
        case MEDIA_FRAME_TYPE_VIDEO_B_FRAME :
        {
            bType = FLV_TAG_VIDEO_TYPE;
            iRet =this->GenerateVideoData(i_ptFrameInfo,0,m_pbFrameBuf,m_iFrameBufMaxLen);
            if(iRet <= 0)
            {
                MH_LOGE("GenerateVideoData P 0 err %d \r\n",iLen);
                return -1;
            }
            break;
        }
        case MEDIA_FRAME_TYPE_AUDIO_FRAME :
        {
            bType = FLV_TAG_AUDIO_TYPE;
            if(MEDIA_ENCODE_TYPE_AAC==i_ptFrameInfo->eEncType&&0==m_iAudioSeqHeaderSended)
            {
                iLen = this->GenerateAudioData(i_ptFrameInfo, 1,m_pbFrameBuf,m_iFrameBufMaxLen);
                if(iLen <= 0)
                {
                    MH_LOGE("GenerateAudioData err %d \r\n",iLen);
                    return -1;
                }
                memset(&tFlvTagHeader,0,sizeof(T_FlvTagHeader));
                tFlvTagHeader.bFilter= 0;
                tFlvTagHeader.bType= bType;
                tFlvTagHeader.dwSize= iLen;
                tFlvTagHeader.dwTimestamp= i_ptFrameInfo->dwTimeStamp;
                tFlvTagHeader.dwStreamId= 0;// StreamID Always 0
                iLen = this->CreateFlvTagHeader(&tFlvTagHeader,o_pbBuf,i_dwMaxLen);
                if(iLen <= 0)
                {
                    MH_LOGE("CreateFlvTagHeader err %d \r\n",iLen);
                    return -1;
                }
                memcpy(o_pbBuf+iLen,m_pbFrameBuf,tFlvTagHeader.dwSize);
                iLen+=tFlvTagHeader.dwSize;
                Write32BE((o_pbBuf+iLen),iLen);//dwPreviousTagSize
                iLen+=sizeof(iLen);
                
                m_iAudioSeqHeaderSended=1;
            }
            iRet =this->GenerateAudioData(i_ptFrameInfo,0,m_pbFrameBuf,m_iFrameBufMaxLen);
            if(iRet <= 0)
            {
                MH_LOGE("GenerateAudioData err %d \r\n",iLen);
                return -1;
            }
            break;
        }
        default :
        {
            MH_LOGE("FrameToContainer i_ptFrame->eFrameType err%d\r\n",i_ptFrameInfo->eFrameType);
            return iRet;
        }
    }
    memset(&tFlvTagHeader,0,sizeof(T_FlvTagHeader));
    tFlvTagHeader.bFilter= 0;
    tFlvTagHeader.bType= bType;
    tFlvTagHeader.dwSize= iRet;
    tFlvTagHeader.dwTimestamp= i_ptFrameInfo->dwTimeStamp;
    tFlvTagHeader.dwStreamId= 0;// StreamID Always 0
    iRet = this->CreateFlvTagHeader(&tFlvTagHeader,o_pbBuf+iLen,i_dwMaxLen-iLen);
    if(iRet <= 0)
    {
        MH_LOGE("CreateFlvTagHeader a err %d \r\n",iLen);
        return -1;
    }
    iLen+=iRet;
    memcpy(o_pbBuf+iLen,m_pbFrameBuf,tFlvTagHeader.dwSize);
    iLen+=tFlvTagHeader.dwSize;
    Write32BE((o_pbBuf+iLen),(iRet+tFlvTagHeader.dwSize));//dwPreviousTagSize
    iLen+=sizeof(iLen);

	return iLen;
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
int FlvPackHandle::CreateFlvHeader(T_FlvHeader * i_ptFlvHeader,unsigned char* o_pbBuf,unsigned int i_dwMaxLen)
{
    int iRet = -1;
    int iLen = 0;
    
	if (i_dwMaxLen < FLV_HEADER_LEN || NULL == i_ptFlvHeader ||NULL == o_pbBuf)
	{
        MH_LOGE("CreateFlvHeader NULL %d \r\n", i_dwMaxLen);
		return iLen;
	}
	
	memcpy(&o_pbBuf[iLen],i_ptFlvHeader->FLV,sizeof(i_ptFlvHeader->FLV));
	iLen+=sizeof(i_ptFlvHeader->FLV);
	o_pbBuf[iLen] = i_ptFlvHeader->bVersion;
	iLen++;
	o_pbBuf[iLen]=0;
	o_pbBuf[iLen]|=i_ptFlvHeader->bAudio>0?4:0;
	o_pbBuf[iLen]|=i_ptFlvHeader->bVideo>0?1:0;
	iLen++;
	Write32BE((o_pbBuf + iLen),i_ptFlvHeader->dwOffset);
	iLen+=4;
	return iLen;
}


/*****************************************************************************
-Fuction        : CreateFlvTagHeader
-Description    : CreateFlvTagHeader
-Input          : 
-Output         : 
-Return         :  must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvPackHandle::CreateFlvTagHeader(T_FlvTagHeader * i_ptFlvTagHeader,unsigned char* o_pbBuf,unsigned int i_dwMaxLen)
{
    int iRet = -1;
    int iLen = 0;
    
	if (i_dwMaxLen < FLV_TAG_HEADER_LEN || NULL == i_ptFlvTagHeader ||NULL == o_pbBuf)
	{
        MH_LOGE("CreateFlvTagHeader NULL %d \r\n", i_dwMaxLen);
		return iLen;
	}

	// TagType
	o_pbBuf[iLen] = (i_ptFlvTagHeader->bFilter&0xE0)|(i_ptFlvTagHeader->bType&0x1F);
	iLen++;
	
	// DataSize
	Write24BE((o_pbBuf + iLen),i_ptFlvTagHeader->dwSize);
	iLen+=3;

	// TimestampExtended | Timestamp
	Write24BE((o_pbBuf + iLen),i_ptFlvTagHeader->dwTimestamp);
	iLen+=3;
	o_pbBuf[iLen] = (i_ptFlvTagHeader->dwTimestamp >> 24) & 0xff;//TimestampExtended
	iLen++;

	// StreamID Always 0
	Write24BE((o_pbBuf + iLen),i_ptFlvTagHeader->dwStreamId);
	iLen+=3;

	return iLen;
}

/*****************************************************************************
-Fuction        : GenerateVideoData
-Description    : GenerateVideoData
-Input          : 
-Output         : 
-Return         : iVideoDataLen must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvPackHandle::GenerateVideoData(T_MediaFrameInfo * i_ptFrameInfo,int i_iIsAvcSeqHeader,unsigned char *o_pbVideoData,int i_iMaxVideoData)
{
    int iVideoDataLen = 0;

    if(NULL == i_ptFrameInfo || NULL == o_pbVideoData)
    {
        MH_LOGE("GenerateVideoData NULL %d\r\n", i_iMaxVideoData);
        return iVideoDataLen;
    }
    if(MEDIA_ENCODE_TYPE_UNKNOW == i_ptFrameInfo->eEncType ||MEDIA_FRAME_TYPE_UNKNOW == i_ptFrameInfo->eFrameType||MEDIA_FRAME_TYPE_AUDIO_FRAME == i_ptFrameInfo->eFrameType)
    {
        MH_LOGE("GenerateVideoData RTMP_UNKNOW_FRAME %d\r\n", i_ptFrameInfo->eFrameType);
        return iVideoDataLen;
    }
    if(MEDIA_ENCODE_TYPE_H264 == i_ptFrameInfo->eEncType)
    {
        iVideoDataLen = GenerateVideoDataH264(i_ptFrameInfo,i_iIsAvcSeqHeader,o_pbVideoData,i_iMaxVideoData);
    }
    if(MEDIA_ENCODE_TYPE_H265 == i_ptFrameInfo->eEncType)
    {
        iVideoDataLen = GenerateVideoDataH265(i_ptFrameInfo,i_iIsAvcSeqHeader,o_pbVideoData,i_iMaxVideoData);
    }

    return iVideoDataLen;
}

/*****************************************************************************
-Fuction        : GenerateVideoData
-Description    : GenerateVideoData
-Input          : 
-Output         : 
-Return         : iVideoDataLen must > 0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvPackHandle::GenerateAudioData(T_MediaFrameInfo * i_ptAudioInfo,int i_iIsAACSeqHeader,unsigned char *o_pbAudioData,int i_iMaxAudioData)
{
    int iAudioDataLen = 0;
    unsigned char bAudioTagHeader = 0;

    
    if(NULL == i_ptAudioInfo || NULL == o_pbAudioData)
    {
        MH_LOGE("GenerateAudioData NULL %d\r\n", i_iMaxAudioData);
        return iAudioDataLen;
    }
    if(MEDIA_ENCODE_TYPE_UNKNOW == i_ptAudioInfo->eEncType)
    {
        MH_LOGE("FlvPackHandle MEDIA_ENCODE_TYPE_UNKNOW %d\r\n",i_ptAudioInfo->eEncType);
        return iAudioDataLen;
    }
    //tag Header 1 byte
    bAudioTagHeader = CreateAudioDataTagHeader(i_ptAudioInfo);
    o_pbAudioData[iAudioDataLen] = bAudioTagHeader;
    iAudioDataLen += 1;

    if(MEDIA_ENCODE_TYPE_AAC == i_ptAudioInfo->eEncType)
    {
        //tag Body(AAC packet type) 1 byte
        if(0 == i_iIsAACSeqHeader)
        {
            o_pbAudioData[iAudioDataLen] = 0x1;
        }
        else
        {
            o_pbAudioData[iAudioDataLen] = 0x0;//AvcSeqHeader
        }
        iAudioDataLen += 1;
        //tag Body(AAC SeqHeader or AAC Raw)
        if(0 == i_iIsAACSeqHeader)
        {
            memcpy(&o_pbAudioData[iAudioDataLen], i_ptAudioInfo->pbFrameStartPos+7,i_ptAudioInfo->iFrameLen-7);
            iAudioDataLen += i_ptAudioInfo->iFrameLen-7;
        }
        else
        {
            iAudioDataLen += CreateAudioSpecCfgAAC(i_ptAudioInfo->dwSampleRate,i_ptAudioInfo->tAudioEncodeParam.dwChannels,&o_pbAudioData[iAudioDataLen]);
        }
    }
    else
    {
        memcpy(&o_pbAudioData[iAudioDataLen], i_ptAudioInfo->pbFrameStartPos,i_ptAudioInfo->iFrameLen);
        iAudioDataLen += i_ptAudioInfo->iFrameLen;
    }
    return iAudioDataLen;
}

/*****************************************************************************
-Fuction        : GenerateVideoData
-Description    : GenerateVideoMsgBody
-Input          : i_iIsAvcSeqHeader 0 nalu ,1 AvcSeqHeader
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvPackHandle::GenerateVideoDataH264(T_MediaFrameInfo * i_ptFrameInfo,int i_iIsAvcSeqHeader,unsigned char *o_pbVideoData,int i_iMaxVideoData)
{
    int iVideoDataLen = 0;
    unsigned char* pbVideoData = NULL;

    if(NULL == i_ptFrameInfo || NULL == o_pbVideoData ||
    i_iMaxVideoData < (int)(5+11 +(i_iIsAvcSeqHeader?i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS+i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS: i_ptFrameInfo->iFrameLen)))
    {
        MH_LOGE("GenerateVideoData NULL %d\r\n", i_iMaxVideoData);
        return iVideoDataLen;
    }
    if(MEDIA_ENCODE_TYPE_H264 != i_ptFrameInfo->eEncType ||MEDIA_FRAME_TYPE_UNKNOW == i_ptFrameInfo->eFrameType || 
    (MEDIA_FRAME_TYPE_VIDEO_I_FRAME == i_ptFrameInfo->eFrameType && 
    0 != i_iIsAvcSeqHeader && (i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS<= 0 ||i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS<= 0)))
    {
        MH_LOGE("GenerateVideoData MEDIA_FRAME_TYPE_UNKNOW %d\r\n", i_ptFrameInfo->eFrameType);
        return iVideoDataLen;
    }
    //tag Header 1 byte
    if(MEDIA_FRAME_TYPE_VIDEO_I_FRAME == i_ptFrameInfo->eFrameType)
    {
        o_pbVideoData[iVideoDataLen] = 0x17;
    }
    else
    {
        o_pbVideoData[iVideoDataLen] = 0x27;
    }
    iVideoDataLen += 1;
    //tag Body(AVCC Header) 4 byte
    if(0 == i_iIsAvcSeqHeader)
    {
        o_pbVideoData[iVideoDataLen] = 0x1;
    }
    else
    {
        o_pbVideoData[iVideoDataLen] = 0x0;//AvcSeqHeader
    }
    iVideoDataLen += 1;
    memset(&o_pbVideoData[iVideoDataLen],0,3);
    iVideoDataLen += 3;
    
    //tag Body(AVCC Body)
    if(0 == i_iIsAvcSeqHeader)
    {
        int i=0;
#if 0        
        const char *test = "{\"test\":123}";
        char seih[] = {6,5};
        unsigned char pInfoUUID[16] = {0x4A, 0x46, 0x55, 0x55, 0x49, 0x44, 0x49, 0x4E, 0x46, 0x4F, 0x46, 0x52, 0x41, 0x4D, 0x00, 0x0F}; 
        char sei[1024];
        int len = 0;
        memcpy(&sei[len],seih,sizeof(seih));
        len+=sizeof(seih);
        sei[len] = sizeof(pInfoUUID)+strlen(test);
        len++;
        memcpy(&sei[len],pInfoUUID,sizeof(pInfoUUID));
        len+=sizeof(pInfoUUID);
        memcpy(&sei[len],test,strlen(test));
        len+=strlen(test);
        if(len%2 == 0)
        {
            sei[len] = 0x00;
            sei[len+1] = 0x80;
            len+=2;
        }
        else
        {
            sei[len] = 0x80;
            len+=1;
        }
        pbVideoData = &o_pbVideoData[iVideoDataLen];
        Write32BE(pbVideoData,len);
        iVideoDataLen += sizeof(len);
        memcpy(&o_pbVideoData[iVideoDataLen],sei,len);
        iVideoDataLen +=len;
#endif
        for(i=0;i<(int)i_ptFrameInfo->dwNaluCount;i++)
        {
            unsigned char *pbNaluData=i_ptFrameInfo->atNaluInfo[i].pbData;
            unsigned int dwDataLen=i_ptFrameInfo->atNaluInfo[i].dwDataLen;
            int iStartCodeLen=4;
            if (dwDataLen >= 3 && pbNaluData[0] == 0 && pbNaluData[1] == 0 && pbNaluData[2] == 1)
            {
                iStartCodeLen=3;
            }
            else if (dwDataLen >= 4 && pbNaluData[0] == 0 && pbNaluData[1] == 0 && pbNaluData[2] == 0 && pbNaluData[3] == 1)
            {
                iStartCodeLen=4;
            }
            pbNaluData+=iStartCodeLen;
            dwDataLen-=iStartCodeLen;
            pbVideoData = &o_pbVideoData[iVideoDataLen];
            Write32BE(pbVideoData,dwDataLen);
            iVideoDataLen += sizeof(dwDataLen);
            memcpy(&o_pbVideoData[iVideoDataLen],pbNaluData,dwDataLen);
            iVideoDataLen += dwDataLen;
        }
    }
    else
    {
        o_pbVideoData[iVideoDataLen] = 0x1;////AVC sequence header or extradata版本号 1
        iVideoDataLen += 1;
        memcpy(&o_pbVideoData[iVideoDataLen], &i_ptFrameInfo->tVideoEncodeParam.abSPS[1], 3);
        iVideoDataLen += 3;
        o_pbVideoData[iVideoDataLen] = 0xff;//nalu size 4 字节
        iVideoDataLen += 1;
        o_pbVideoData[iVideoDataLen] = 0xe1;//SPS 个数 =1
        iVideoDataLen += 1;
        pbVideoData = &o_pbVideoData[iVideoDataLen];
        Write16BE(pbVideoData, i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS);
        iVideoDataLen += 2;
        memcpy(&o_pbVideoData[iVideoDataLen], i_ptFrameInfo->tVideoEncodeParam.abSPS,i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS);
        iVideoDataLen += i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS;
        o_pbVideoData[iVideoDataLen] = 0x1;//PPS 个数 =1
        iVideoDataLen += 1;
        pbVideoData = &o_pbVideoData[iVideoDataLen];
        Write16BE(pbVideoData, i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS);
        iVideoDataLen += 2;
        memcpy(&o_pbVideoData[iVideoDataLen], i_ptFrameInfo->tVideoEncodeParam.abPPS,i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS);
        iVideoDataLen += i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS;
    }
    
    return iVideoDataLen;
}

/*****************************************************************************
-Fuction        : GenerateVideoData
-Description    : GenerateVideoMsgBody
-Input          : i_iIsAvcSeqHeader 0 nalu ,1 AvcSeqHeader
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FlvPackHandle::GenerateVideoDataH265(T_MediaFrameInfo * i_ptFrameInfo,int i_iIsAvcSeqHeader,unsigned char *o_pbVideoData,int i_iMaxVideoData)
{
    int iVideoDataLen = 0;
    T_FlvH265Extradata tRtmpH265Extradata;
    unsigned char* pbVideoData = NULL;

    if(NULL == i_ptFrameInfo || NULL == o_pbVideoData ||
    i_iMaxVideoData < (int)(5+23 +(i_iIsAvcSeqHeader?i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS+i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS+i_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS: i_ptFrameInfo->iFrameLen)))
    {
        MH_LOGE("GenerateVideoData NULL %d\r\n", i_iMaxVideoData);
        return iVideoDataLen;
    }
    if(MEDIA_ENCODE_TYPE_H265 != i_ptFrameInfo->eEncType ||MEDIA_FRAME_TYPE_UNKNOW == i_ptFrameInfo->eFrameType || 
    (MEDIA_FRAME_TYPE_VIDEO_I_FRAME == i_ptFrameInfo->eFrameType && 0 != i_iIsAvcSeqHeader 
    && (i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS<= 0 ||i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS<= 0)))
    {
        MH_LOGE("GenerateVideoData MEDIA_FRAME_TYPE_UNKNOW %d\r\n", i_ptFrameInfo->eFrameType);
        return iVideoDataLen;
    }
    pbVideoData = o_pbVideoData;
    //tag Header 1 byte
    if(0 == m_iEnhancedFlvFlag)
    {
        if(MEDIA_FRAME_TYPE_VIDEO_I_FRAME == i_ptFrameInfo->eFrameType)
        {
            *pbVideoData = 0x1c;
        }
        else
        {
            *pbVideoData = 0x2c;//微信小程序验证h265这种定义是可以的
        }
        pbVideoData += 1;
        //tag Body(AVCC Header) 4 byte
        if(0 == i_iIsAvcSeqHeader)
        {
            *pbVideoData = 0x1;
        }
        else
        {
            *pbVideoData = 0x0;//AvcSeqHeader
        }
        pbVideoData += 1;
        memset(pbVideoData,0,3);
        pbVideoData += 3;
    }
    else
    {
        unsigned char eFlvPacketType=FLV_PACKET_TYPE_CODED_FRAMES_X;
        *pbVideoData = 0x80;
        if(MEDIA_FRAME_TYPE_VIDEO_I_FRAME == i_ptFrameInfo->eFrameType)
        {
            *pbVideoData |= 0x10;
        }
        else
        {
            *pbVideoData |= 0x20;///1 = key frame, 2 = inter frame
        }
        if (0 != i_iIsAvcSeqHeader)//如果PacketType是PacketTypeSequenceStart，表示后续H265的数据内容是DecoderConfigurationRecord，也就是常说的sequence header;
        {
            eFlvPacketType=FLV_PACKET_TYPE_SEQUENCE_START;
        }
        *pbVideoData |= eFlvPacketType;
        pbVideoData += 1;
        Write32BE(pbVideoData,FLV_VIDEO_ENC_H265);
        pbVideoData+=4;
    }
    
    //tag Body(AVCC Body)
    if(0 == i_iIsAvcSeqHeader)
    {
        int i=0;
        for(i=0;i<(int)i_ptFrameInfo->dwNaluCount;i++)
        {
            unsigned char *pbNaluData=i_ptFrameInfo->atNaluInfo[i].pbData;
            unsigned int dwDataLen=i_ptFrameInfo->atNaluInfo[i].dwDataLen;
            int iStartCodeLen=4;
            if (dwDataLen >= 3 && pbNaluData[0] == 0 && pbNaluData[1] == 0 && pbNaluData[2] == 1)
            {
                iStartCodeLen=3;
            }
            else if (dwDataLen >= 4 && pbNaluData[0] == 0 && pbNaluData[1] == 0 && pbNaluData[2] == 0 && pbNaluData[3] == 1)
            {
                iStartCodeLen=4;
            }
            pbNaluData+=iStartCodeLen;
            dwDataLen-=iStartCodeLen;
            Write32BE(pbVideoData,dwDataLen);
            pbVideoData += sizeof(dwDataLen);
            memcpy(pbVideoData, pbNaluData,dwDataLen);
            pbVideoData +=dwDataLen;
        }
    }
    else//23字节解析可参考SrsRawHEVCStream::mux_sequence_header,需要通过解析vps,sps才能得出这个HEVC extradata
    {//或者参考media server中的hevc_profile_tier_level调用
        memset(&tRtmpH265Extradata,0,sizeof(T_FlvH265Extradata));
        if(0 != AnnexbToH265Extradata(i_ptFrameInfo,&tRtmpH265Extradata))
        {
            MH_LOGE("AnnexbToH265Extradata err %d\r\n", iVideoDataLen);
            iVideoDataLen = 0;
            return -1;
        }
        // HEVCDecoderConfigurationRecord
        // ISO/IEC 14496-15:2017
        // 8.3.3.1.2 Syntax
        *pbVideoData = tRtmpH265Extradata.configurationVersion;
        pbVideoData++;
        // general_profile_space + general_tier_flag + general_profile_idc
        *pbVideoData = ((tRtmpH265Extradata.general_profile_space & 0x03) << 6) | ((tRtmpH265Extradata.general_tier_flag & 0x01) << 5) | (tRtmpH265Extradata.general_profile_idc & 0x1F);
        pbVideoData++;

        // general_profile_compatibility_flags
        Write32BE(pbVideoData, tRtmpH265Extradata.general_profile_compatibility_flags);
        pbVideoData += sizeof(tRtmpH265Extradata.general_profile_compatibility_flags);
        // general_constraint_indicator_flags
        Write32BE(pbVideoData, (unsigned int)(tRtmpH265Extradata.general_constraint_indicator_flags >> 16));
        pbVideoData += sizeof(unsigned int);
        Write16BE(pbVideoData, (unsigned short)tRtmpH265Extradata.general_constraint_indicator_flags);
        pbVideoData += sizeof(unsigned short);
        // general_level_idc
        *pbVideoData = tRtmpH265Extradata.general_level_idc;
        pbVideoData++;
        // min_spatial_segmentation_idc
        Write16BE(pbVideoData, 0xF000 | tRtmpH265Extradata.min_spatial_segmentation_idc);
        pbVideoData += sizeof(unsigned short);
        *pbVideoData = 0xFC | tRtmpH265Extradata.parallelismType;
        pbVideoData++;
        *pbVideoData = 0xFC | tRtmpH265Extradata.chromaFormat;
        pbVideoData++;
        *pbVideoData = 0xF8 | tRtmpH265Extradata.bitDepthLumaMinus8;
        pbVideoData++;
        *pbVideoData = 0xF8 | tRtmpH265Extradata.bitDepthChromaMinus8;
        pbVideoData++;
        Write16BE(pbVideoData,tRtmpH265Extradata.avgFrameRate);
        pbVideoData += sizeof(unsigned short);
        *pbVideoData = (tRtmpH265Extradata.constantFrameRate << 6) | ((tRtmpH265Extradata.numTemporalLayers & 0x07) << 3) | ((tRtmpH265Extradata.temporalIdNested & 0x01) << 2) | (tRtmpH265Extradata.lengthSizeMinusOne & 0x03);
        pbVideoData++;
        *pbVideoData = tRtmpH265Extradata.numOfArrays;
        pbVideoData++;
        
        // numOfVideoParameterSets, always 1
        char numOfVideoParameterSets[2] = { 0x00,0x01 };
     // vps
        // nal_type
        *pbVideoData = (i_ptFrameInfo->tVideoEncodeParam.abVPS[0] >> 1) & 0x3f;
        pbVideoData++;
        // numOfVideoParameterSets, always 1
        memcpy(pbVideoData, numOfVideoParameterSets, sizeof(numOfVideoParameterSets));
        pbVideoData += sizeof(numOfVideoParameterSets);
        // videoParameterSetLength
        Write16BE(pbVideoData,i_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS);
        pbVideoData += sizeof(unsigned short);
        // videoParameterSetNALUnit
        memcpy(pbVideoData,i_ptFrameInfo->tVideoEncodeParam.abVPS,i_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS);
        pbVideoData += i_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS;
    // sps
       // nal_type
        *pbVideoData = (i_ptFrameInfo->tVideoEncodeParam.abSPS[0] >> 1) & 0x3f;
       pbVideoData++;
       // numOfVideoParameterSets, always 1
       memcpy(pbVideoData, numOfVideoParameterSets, sizeof(numOfVideoParameterSets));
       pbVideoData += sizeof(numOfVideoParameterSets);
       // videoParameterSetLength
       Write16BE(pbVideoData,i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS);
       pbVideoData += sizeof(unsigned short);
       // videoParameterSetNALUnit
       memcpy(pbVideoData,i_ptFrameInfo->tVideoEncodeParam.abSPS,i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS);
       pbVideoData += i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS;
   // pps
      // nal_type
       *pbVideoData = (i_ptFrameInfo->tVideoEncodeParam.abPPS[0] >> 1) & 0x3f;
       pbVideoData++;
      // numOfVideoParameterSets, always 1
      memcpy(pbVideoData, numOfVideoParameterSets, sizeof(numOfVideoParameterSets));
      pbVideoData += sizeof(numOfVideoParameterSets);
      // videoParameterSetLength
      Write16BE(pbVideoData,i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS);
      pbVideoData += sizeof(unsigned short);
      // videoParameterSetNALUnit
      memcpy(pbVideoData,i_ptFrameInfo->tVideoEncodeParam.abPPS,i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS);
      pbVideoData += i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS;
    }
    iVideoDataLen = pbVideoData - o_pbVideoData;
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
int FlvPackHandle::AnnexbToH265Extradata(T_MediaFrameInfo * i_ptFrameInfo,T_FlvH265Extradata *o_ptFlvH265Extradata)
{
    int iRet = -1;
    if(NULL == i_ptFrameInfo || NULL == o_ptFlvH265Extradata)
    {
        MH_LOGE("AnnexbToH265Extradata NULL %d \r\n", iRet);
        return iRet;
    }
    o_ptFlvH265Extradata->configurationVersion = 1;
    o_ptFlvH265Extradata->lengthSizeMinusOne = 3; // 4 bytes
    o_ptFlvH265Extradata->numOfArrays = 3; // numOfArrays, default 3
    iRet = VpsToH265Extradata(i_ptFrameInfo->tVideoEncodeParam.abVPS,(unsigned short)i_ptFrameInfo->tVideoEncodeParam.iSizeOfVPS,o_ptFlvH265Extradata);
    iRet |= FlvHandle::SpsToH265Extradata(i_ptFrameInfo->tVideoEncodeParam.abSPS,(unsigned short)i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS,o_ptFlvH265Extradata);
    return iRet;
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
int FlvPackHandle::VpsToH265Extradata(unsigned char *i_pbVpsData,unsigned short i_wVpsLen,T_FlvH265Extradata *o_ptH265Extradata)
{
    int iRet = -1;
    unsigned char abSodbVPS[VIDEO_VPS_MAX_SIZE];
    int iSodbLen = 0;
    unsigned char vps_max_sub_layers_minus1;
    unsigned char vps_temporal_id_nesting_flag;
    
    if(NULL == i_pbVpsData || NULL == o_ptH265Extradata)
    {
        MH_LOGE("VpsToH265Extradata NULL %d \r\n", i_wVpsLen);
        return iRet;
    }
    memset(abSodbVPS,0,sizeof(abSodbVPS));
    iSodbLen = DecodeEBSP(i_pbVpsData, i_wVpsLen, abSodbVPS);
    if (iSodbLen < 16 + 2)
        return iRet;
    vps_max_sub_layers_minus1 = (abSodbVPS[3] >> 1) & 0x07;
    vps_temporal_id_nesting_flag = abSodbVPS[3] & 0x01;
    o_ptH265Extradata->numTemporalLayers = MAX(o_ptH265Extradata->numTemporalLayers, vps_max_sub_layers_minus1 + 1);
    o_ptH265Extradata->temporalIdNested = (o_ptH265Extradata->temporalIdNested || vps_temporal_id_nesting_flag) ? 1 : 0;
    iRet = HevcProfileTierLevel(abSodbVPS + 6, iSodbLen - 6, vps_max_sub_layers_minus1, o_ptH265Extradata);
    if(iRet < 0)
    {
        MH_LOGE("HevcProfileTierLevel err %d \r\n", i_wVpsLen);
        return iRet;
    }
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
unsigned char FlvPackHandle::CreateAudioDataTagHeader(T_MediaFrameInfo * i_ptAudioParam)
{
    unsigned char bAudioTagHeader = 0;
    unsigned char bEncType = 0;
    unsigned char bSampleRate = 3;// 0-5.5kHz;1-11kHz;2-22kHz;3-44kHz(AAC都是3)
    unsigned char bSendBit = 0b00;
    unsigned char bChannels = 1;
    
    switch(i_ptAudioParam->eEncType)
    {
        case MEDIA_ENCODE_TYPE_MP3:
        {
            bEncType = 2;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711A:
        {
            bEncType = 7;
            break;
        }
        case MEDIA_ENCODE_TYPE_G711U:
        {
            bEncType = 8;
            break;
        }
        case MEDIA_ENCODE_TYPE_AAC:
        default:
        {
            bEncType = 10;
            break;
        }
    }
    switch (i_ptAudioParam->dwSampleRate)
    {
        case 5500:
        {
            bSampleRate = 0;
            break;
        }
        case 11000:
        {
            bSampleRate = 1;
            break;
        }
        case 22000:
        {
            bSampleRate = 2;
            break;
        }
        case 44000:
        default:
        {
            bSampleRate = 3;
            break;
        }
    }
    bSendBit = i_ptAudioParam->tAudioEncodeParam.dwBitsPerSample == 16 ? 0b01 : 0b00;

    if (i_ptAudioParam->eEncType != MEDIA_ENCODE_TYPE_AAC)
    {
        bChannels = i_ptAudioParam->tAudioEncodeParam.dwChannels > 1 ? 1 : 0;
    }
    else
    {
        bChannels = 1;
    }
    
    bAudioTagHeader = bEncType << 4;
    bAudioTagHeader |= (bSampleRate << 2);
    bAudioTagHeader |= (bSendBit << 1);
    bAudioTagHeader |= bChannels;

    return bAudioTagHeader;
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
int FlvPackHandle::CreateAudioSpecCfgAAC(unsigned int i_dwFrequency,unsigned int i_dwChannels,unsigned char *o_pbAudioData)
{
    int iAudioSpecCfgLen = 0;
    unsigned char bProfile = 1;
    unsigned char bAudioObjectType = 0;
    unsigned char bChannelConfiguration = 0;
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
    bChannelConfiguration = (unsigned char)i_dwChannels;
    
    o_pbAudioData[0] = (bAudioObjectType << 3) | (bSamplingFrequencyIndex >> 1);
    o_pbAudioData[1] = (bSamplingFrequencyIndex << 7) | (bChannelConfiguration << 3);
    iAudioSpecCfgLen = 2;

    return iAudioSpecCfgLen;
}

