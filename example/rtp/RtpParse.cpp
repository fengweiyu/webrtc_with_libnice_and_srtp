/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpParse.cpp
* Description		: 	RtpParse operation center
                        包含多种Rtp打包载荷类型，比如NALU,FU-A等载荷类型
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include <stdlib.h>//还是需要.h
#include <stdio.h>
#include <string.h>
#include <iostream>//不加.h,c++新的头文件

#include "Definition.h"
#include "RtpParse.h"

#ifndef TRUE
#define TRUE 0
#endif
#ifndef FALSE
#define FALSE -1
#endif


using std::cout;//需要<iostream>
using std::endl;

/*****************************************************************************
-Fuction		: RtpParse
-Description	: RtpParse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpParse :: RtpParse()
{
    memset(&m_tPacketTypeInfos,0,sizeof(T_RtpPacketTypeInfos));

}

/*****************************************************************************
-Fuction		: RtpParse
-Description	: RtpParse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpParse :: ~RtpParse()
{
}

/*****************************************************************************
-Fuction		: RtpParse
-Description	: RtpParse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse :: Init(T_RtpPacketTypeInfos i_tPacketTypeInfos)
{
    memcpy(&m_tPacketTypeInfos,&i_tPacketTypeInfos,sizeof(T_RtpPacketTypeInfos));
    return TRUE;
}

/*****************************************************************************
-Fuction		: GenerateRtpHeader
-Description	: GenerateRtpHeader
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse :: ParseRtpHeader(unsigned char *i_pbRtpBuf,int i_iBufLen,T_RtpPacketParam *o_ptParam,int *o_iPaddingLen,int *o_iMark)
{
    int iRet=FALSE;
    RtpHeader *pHeader = NULL;
    
    if(NULL == i_pbRtpBuf || NULL == o_ptParam || NULL == o_iPaddingLen || NULL == o_iMark ||i_iBufLen < RTP_HEADER_LEN)
    {
        printf("GenerateRtpHeader err NULL i_bRtpBuf %p,o_ptParam %p i_iBufLen%d\r\n",i_pbRtpBuf,o_ptParam,i_iBufLen);
        return iRet;
    }
    pHeader = (RtpHeader *)i_pbRtpBuf;
    *o_iPaddingLen = pHeader->Pad;
    *o_iMark = pHeader->Mark;
    o_ptParam->wPayloadType = pHeader->PayloadType;
    o_ptParam->wSeq = pHeader->wSeq;
    o_ptParam->dwTimestamp = pHeader->dwTimestamp;
    o_ptParam->dwSSRC = pHeader->dwSSRC;
    iRet=TRUE;
    return iRet;
}

/*****************************************************************************
-Fuction		: Parse
-Description	: Parse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse :: Parse(unsigned char *i_pbPacketBuf,int i_iPacketLen,T_RtpPacketParam *o_ptParam,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
    int iRet=FALSE;
    T_RtpPacketParam tParam;
    int iMark=0;
    int iPad=0;
    int i=0;
    E_RtpPacketType eRtpPacketType = RTP_PACKET_TYPE_UNKNOW;
    
	if (!i_pbPacketBuf || i_iPacketLen <= 0 || !o_ptParam|| !o_pbParsedData|| !o_iDataLen)
    {
        cout<<"Parse err NULL"<<endl;
        return iRet;
    }
    memset(&tParam,0,sizeof(T_RtpPacketParam));
    iRet=this->ParseRtpHeader(i_pbPacketBuf,i_iPacketLen,&tParam,&iPad,&iMark);
    if(FALSE == iRet)
    {
        cout<<"i_iRtpPacketType ParseRtpHeader err "<<i_iPacketLen<<endl;
        return iRet;
    }
    for(i=0;i<RTP_PACKET_TYPE_MAX;i++)
    {
        if(tParam.wPayloadType == m_tPacketTypeInfos.atTypeInfos[i].iPayload)
        {
            eRtpPacketType = m_tPacketTypeInfos.atTypeInfos[i].ePacketType;
            break;
        }
    }
    tParam.ePacketType=eRtpPacketType;
    switch (tParam.ePacketType)
    {
        case RTP_PACKET_TYPE_G711U:
        case RTP_PACKET_TYPE_G711A:
        {
            iRet=G711Parse(i_pbPacketBuf+RTP_HEADER_LEN,i_iPacketLen-RTP_HEADER_LEN,o_pbParsedData,o_iDataLen,i_iDataMaxLen);
            break;
        }
        default :
        {
            printf("tParam.ePacketType err %d %d\r\n",tParam.ePacketType,tParam.wPayloadType);
            break;
        }
    }
    if(FALSE != iRet)
    {
        memcpy(o_ptParam,&tParam,sizeof(T_RtpPacketParam));
    }
    return iRet;
}
/*****************************************************************************
-Fuction		: Parse
-Description	: Parse
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpParse ::G711Parse(unsigned char *i_pbRtpBodyBuf,int i_iBufLen,unsigned char *o_pbParsedData,int *o_iDataLen,int i_iDataMaxLen)
{
	if (!i_pbRtpBodyBuf || i_iDataMaxLen < i_iBufLen || !o_pbParsedData|| !o_iDataLen)
    {
        printf("G711Parse err %d< %d \r\n",i_iDataMaxLen,i_iBufLen);
        return FALSE;
    }
    memcpy(o_pbParsedData,i_pbRtpBodyBuf,i_iBufLen);
    *o_iDataLen = i_iBufLen;
    return TRUE;
}

