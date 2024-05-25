/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FlvHandleInterface.cpp
* Description		: 	FlvHandleInterface operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "MediaAdapter.h"
#include "FlvHandleInterface.h"
#include <string.h>
#include <iostream>


using std::cout;//需要<iostream>
using std::endl;



char * FlvHandleInterface::m_strFormatName=(char *)FLV_MUX_NAME;
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
FlvHandleInterface::FlvHandleInterface()
{
    m_pFlvHandle = new FlvHandle();
}
/*****************************************************************************
-Fuction		: ~FlvHandleInterface
-Description	: ~FlvHandleInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
FlvHandleInterface::~FlvHandleInterface()
{
    if(NULL!= m_pFlvHandle)
    {
        delete m_pFlvHandle;
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
int FlvHandleInterface::Init(char *i_strPath)
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
int FlvHandleInterface::GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam)
{
    int iRet=FALSE;

	return iRet;
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
int FlvHandleInterface::GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam)
{
    int iRet=FALSE;

    return iRet;
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
int FlvHandleInterface::GetMediaInfo(T_MediaInfo *o_ptMediaInfo)
{
    int iRet=FALSE;

    return iRet;
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
int FlvHandleInterface::GetFrame(T_MediaFrameInfo *m_ptFrame)
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
	
	pcFrameData = m_ptFrame->pbFrameBuf;
	iRemainDataLen = m_ptFrame->iFrameBufLen;
	//while(iRemainDataLen>0&&iRet>0&&0==m_ptFrame->iFrameLen);
    m_ptFrame->dwNaluCount = 0;
    m_ptFrame->iFrameLen = 0;
    do
    {
        iRet=m_pFlvHandle->GetFrameData(iProcessedLen,m_ptFrame);
        if(iRet <= 0)
        {
            MH_LOGE("m_pFlvHandle->GetFrameData err %d %d\r\n",iProcessedLen,iRet);
            return -1;
        }
        iProcessedLen+=iRet;
        iRemainDataLen-=iRet;
    }while(iRemainDataLen>0&&0==m_ptFrame->iFrameLen);
        
	if(0 != m_ptFrame->iFrameLen)
	{
        m_ptFrame->iFrameProcessedLen += iProcessedLen;
	}
    return 0;
}



