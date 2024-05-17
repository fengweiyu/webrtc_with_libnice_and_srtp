/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       main.cpp
* Description           : 	    
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>

#include "MediaHandle.h"
#define MEDIA_BUF_MAX_LEN	(2*1024*1024) 
#define MEDIA_FILE_BUF_MAX_LEN	(6*1024*1024) 

static int proc(const char * i_strSrcFilePath,const char *i_strDstFilePath);
static void PrintUsage(char *i_strProcName);

/*****************************************************************************
-Fuction        : main
-Description    : main
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int main(int argc, char* argv[]) 
{
    int iRet = -1;
    
    if(argc !=3)
    {
        PrintUsage(argv[0]);
        return proc("H264AAC.flv","H264AAC.mp4");//proc("H264G711A.flv","H264G711A.mp4");H264AAC
    }
    return proc(argv[1],argv[2]);
}
/*****************************************************************************
-Fuction        : main
-Description    : main
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static int proc(const char * i_strSrcFilePath,const char *i_strDstFilePath)
{
    MediaHandle cMediaHandle;
    T_MediaFrameInfo m_tFileFrameInfo;
    unsigned char * pbFileBuf;
	FILE  *pMediaFile=NULL;
	int iRet = -1,iWriteLen=0;
	int iHeaderLen=0;
	
    cMediaHandle.Init((char *)i_strSrcFilePath);
    do
    {
        memset(&m_tFileFrameInfo,0,sizeof(T_MediaFrameInfo));

        pbFileBuf = new unsigned char [MEDIA_FILE_BUF_MAX_LEN];
        if(NULL == pbFileBuf)
        {
            printf("NULL == pbFileBuf err\r\n");
            break;
        } 
        pMediaFile = fopen(i_strDstFilePath,"wb");//
        if(NULL == pMediaFile)
        {
            printf("fopen %s err\r\n",i_strDstFilePath);
            break;
        } 
        
        m_tFileFrameInfo.pbFrameBuf = new unsigned char [MEDIA_BUF_MAX_LEN];
        if(NULL == m_tFileFrameInfo.pbFrameBuf)
        {
            printf("NULL == m_tFileFrameInfo.pbFrameBuf err\r\n");
            break;
        } 
        m_tFileFrameInfo.iFrameBufMaxLen = MEDIA_BUF_MAX_LEN;

        while(1)
        {
            m_tFileFrameInfo.iFrameLen = 0;
            cMediaHandle.GetFrame(&m_tFileFrameInfo);
            if(m_tFileFrameInfo.iFrameLen <= 0)
            {
                printf("m_tFileFrameInfo.iFrameLen <= 0 %s\r\n",i_strDstFilePath);
                break;
            } 
            iWriteLen = cMediaHandle.FrameToContainer(&m_tFileFrameInfo,STREAM_TYPE_FMP4_STREAM,pbFileBuf,MEDIA_FILE_BUF_MAX_LEN,&iHeaderLen);
            if(iWriteLen < 0)
            {
                printf("FrameToContainer err iWriteLen %d\r\n",iWriteLen);
                break;
            }
            if(iWriteLen == 0)
            {
                continue;
            }
            iRet = fwrite(pbFileBuf, 1,iWriteLen, pMediaFile);
            if(iRet != iWriteLen)
            {
                printf("fwrite err %d iWriteLen%d\r\n",iRet,iWriteLen);
                break;
            }
            //break;
        }
    }while(0);
    
    
    if(NULL != m_tFileFrameInfo.pbFrameBuf)
    {
        delete [] m_tFileFrameInfo.pbFrameBuf;
    } 
    if(NULL != pbFileBuf)
    {
        delete [] pbFileBuf;
    } 
    if(NULL != pMediaFile)
    {
        fclose(pMediaFile);//fseek(m_pMediaFile,0,SEEK_SET); 
    } 
    return iRet;
}
/*****************************************************************************
-Fuction        : PrintUsage
-Description    : PrintUsage
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static void PrintUsage(char *i_strProcName)
{
    printf("Usage: %s inputFile outputFile\r\n",i_strProcName);
    //printf("eg: %s 9112 77.72.169.210 3478\r\n",i_strProcName);
    printf("run default args: %s H264G711A.flv H264G711A.mp4\r\n",i_strProcName);
}

