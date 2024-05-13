/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FMP4HandleInterface.h
* Description		: 	FMP4HandleInterface operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef FMP4_HANDLE_INTERFACE_H
#define FMP4_HANDLE_INTERFACE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "MediaHandle.h"
#include "FMP4Handle.h"

using std::string;


#define FMP4_MUX_NAME        ".mp4"


/*****************************************************************************
-Class			: FlvHandleInterface
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class FMP4HandleInterface : public MediaHandle
{
public:
    FMP4HandleInterface();
    ~FMP4HandleInterface();
    virtual int Init(char *i_strPath);
    virtual int GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam);
    virtual int GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam);
    virtual int GetMediaInfo(T_MediaInfo *o_ptMediaInfo);
    
    virtual int GetFrame(T_MediaFrameInfo *m_ptFrame);//
    virtual int FrameToContainer(T_MediaFrameInfo *i_ptFrame,E_StreamType i_eStreamType,unsigned char * o_pbBuf, unsigned int i_dwMaxBufLen,int *o_piHeaderOffset=NULL);//
    
    static char *m_strFormatName;
private:
    FMP4Handle m_FMP4Handle;
};












#endif
