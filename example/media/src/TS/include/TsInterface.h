/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	TsInterface.h
* Description		: 	TsInterface operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef TS_INTERFACE_H
#define TS_INTERFACE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "MediaHandle.h"
#include "TsPack.h"

using std::string;


#define TS_MUX_NAME        ".ts"


/*****************************************************************************
-Class			: TsInterface
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class TsInterface : public MediaHandle
{
public:
    TsInterface();
    ~TsInterface();
    virtual int Init(char *i_strPath);
    virtual int GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam);
    virtual int GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam);
    virtual int GetMediaInfo(T_MediaInfo *o_ptMediaInfo);
    
    virtual int GetFrame(T_MediaFrameInfo *m_ptFrame);//
    virtual int FrameToContainer(T_MediaFrameInfo *i_ptFrame,E_StreamType i_eStreamType,unsigned char * o_pbBuf, unsigned int i_dwMaxBufLen,int *o_piHeaderOffset=NULL);//

    static char *m_strFormatName;
private:
    TsPack *m_pPack;
};












#endif
