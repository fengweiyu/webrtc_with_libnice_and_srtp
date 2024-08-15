/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	TsInterface.cpp
* Description		: 	TsInterface operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "MediaAdapter.h"
#include "TsInterface.h"
#include <string.h>
#include <iostream>


using std::cout;//需要<iostream>
using std::endl;



char * TsInterface::m_strFormatName=(char *)TS_MUX_NAME;
/*****************************************************************************
-Fuction		: FlvHandleInterface
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TsInterface::TsInterface()
{
    m_pPack=NULL;
}
/*****************************************************************************
-Fuction		: ~TsInterface
-Description	: ~TsInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TsInterface::~TsInterface()
{
    if(NULL!= m_pPack)
    {
        delete m_pPack;
    }
}


/*****************************************************************************
-Fuction		: FlvHandleInterface::Init
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TsInterface::Init(char *i_strPath)
{
    int iRet=FALSE;
	return TRUE;
}

/*****************************************************************************
-Fuction		: FlvHandleInterface::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TsInterface::GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam)
{
    int iRet=FALSE;

	return TRUE;
}
/*****************************************************************************
-Fuction		: FlvHandleInterface::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TsInterface::GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam)
{
    int iRet=FALSE;

	return TRUE;
}
/*****************************************************************************
-Fuction		: FlvHandleInterface::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TsInterface::GetMediaInfo(T_MediaInfo *o_ptMediaInfo)
{
    int iRet=FALSE;

	return TRUE;
}

/*****************************************************************************
-Fuction        : GetFrame
-Description    : m_ptFrame->iFrameBufLen 必须大于等于一帧数据大小否则会失败
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsInterface::GetFrame(T_MediaFrameInfo *m_ptFrame)
{
    int iRet = -1;
    unsigned char *pcFrameData = NULL;
    int iRemainDataLen = 0;
    int iProcessedLen = 0;

    
    if(NULL == m_ptFrame || NULL == m_ptFrame->pbFrameBuf ||m_ptFrame->iFrameBufLen <= 4)
    {
        MH_LOGE("GetFrame NULL %d\r\n", m_ptFrame->iFrameBufLen);
        return iRet;
    }
	
    
	return TRUE;
}

/*****************************************************************************
-Fuction        : FrameToContainer
-Description    : m_ptFrame->iFrameBufLen 必须大于等于一帧数据大小否则会失败
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsInterface::FrameToContainer(T_MediaFrameInfo *i_ptFrame,E_StreamType i_eStreamType,unsigned char * o_pbBuf, unsigned int i_dwMaxBufLen,int *o_piHeaderOffset)
{
    int iRet=FALSE;
    int iEnhancedFlvFlag=0;
    
    if(NULL == i_ptFrame ||NULL == o_pbBuf ||NULL == i_ptFrame->pbFrameStartPos ||i_ptFrame->iFrameLen <= 0)
    {
        MH_LOGE("FrameToContainer err NULL\r\n");
        return iRet;
    }

    if(NULL== m_pPack)
    {
        m_pPack=new TsPack();
    }
    return m_pPack->GetMuxData(i_ptFrame,o_pbBuf,i_dwMaxBufLen);
}



