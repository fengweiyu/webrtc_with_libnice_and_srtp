/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	TsPack.h
* Description		: 	TsPack operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef TS_PACK_H
#define TS_PACK_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <list>
#include <stdint.h>
#include "MediaHandle.h"

using std::string;
using std::list;



/*****************************************************************************
-Class			: TsPack
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class TsPack 
{
public:
    TsPack();
    ~TsPack();
    int GetMuxData(T_MediaFrameInfo * i_ptFrameInfo,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen,int i_iForcePack=0);
    int GetDurationAndPTS(int64_t *o_ddwSegmentDuration,int64_t *o_ddwSegmentPTS);
    
private:   
    int SaveFrame(T_MediaFrameInfo * i_ptFrameInfo);
    int DelAllFrame();

    int FrameToTS(T_MediaFrameInfo * i_ptFrameInfo,int i_iVideoStreamType,int i_iAudioStreamType,int i_iEnableAudio,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);
    int GetTsHeader(unsigned int i_dwPID, unsigned char i_bPayloadUnitStartIndicator, unsigned char i_bAdaptationFieldControl,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);
    int GetTsAdaptationField(int i_iLenPES,unsigned int i_dwTimeStamp,unsigned char i_bFlagPCR,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);

    int GeneratePAT(unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);
    int GeneratePMT(unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen,int i_iVideoStreamType,int i_iAudioStreamType,int i_iEnableAudio);
    int GeneratePES(T_MediaFrameInfo * i_ptFrameInfo,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);
    int GetPesHeader(T_MediaFrameInfo * i_ptFrameInfo,int i_iFrameLen,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen);

    unsigned int CalcCrc32(unsigned char * i_pbData,unsigned int i_dwDataLen);



	unsigned char m_bContinuityCounterPAT;//连续性计数器,各类型的TS  包之间是独立的关系
	unsigned char m_bContinuityCounterPMT;//表示有多少个 pat包，几个pmt包 ，几个MP3 包，几个 h264包，0x00 - 0x0f ，然后折回到0x00                               
	unsigned char m_bContinuityCounterVideo;
	unsigned char m_bContinuityCounterAudio;

    unsigned char *m_pbBufPES;//缓冲区
    unsigned char *m_pbMediaData;
    int m_iCurMediaDataLen;
    list<T_MediaFrameInfo> m_MediaList;
    int m_iFindedKeyFrame;//0 否，1是

    int64_t m_ddwSegmentPTS;
    int64_t m_ddwSegmentDuration;
};












#endif
