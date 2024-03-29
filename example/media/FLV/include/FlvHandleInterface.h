/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FlvHandleInterface.h
* Description		: 	FlvHandleInterface operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef FLV_HANDLE_INTERFACE_H
#define FLV_HANDLE_INTERFACE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "MediaHandle.h"
#include "FlvHandle.h"

using std::string;


#define FLV_MUX_NAME        ".flv"


/*****************************************************************************
-Class			: FlvHandleInterface
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class FlvHandleInterface : public MediaHandle
{
public:
    FlvHandleInterface();
    ~FlvHandleInterface();
    virtual int Init(char *i_strPath);
    virtual int GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam);
    virtual int GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam);
    virtual int GetMediaInfo(T_MediaInfo *o_ptMediaInfo);
    
    virtual int GetFrame(T_MediaFrameInfo *m_ptFrame);//
    
    static char *m_strFormatName;
private:
    FlvHandle *m_pFlvHandle;
};












#endif
