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
#include "webrtc.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <sys/prctl.h>


static void PrintUsage(char *i_strProcName);
static void * WebRtcProc(void *pArg);


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
    char strSeverIp[20];
    int dwPort;
    WebRTC * pWebRTC=NULL;
    pthread_t tWebRtcID;
    char acOfferMsg[6*1024];
    char acAnswerMsg[6*1024];
    
    if(argc !=3)
    {
        PrintUsage(argv[0]);
        return -1;
    }
    memset(strSeverIp,0,sizeof(strSeverIp));
    strncpy(strSeverIp,argv[1],sizeof(strSeverIp));
    dwPort=atoi(argv[2]);

    pWebRTC = new WebRTC(strSeverIp,dwPort,1);
    if( pthread_create(&tWebRtcID,NULL,WebRtcProc,(void *)pWebRTC) != 0 )
    {
        printf("WebRtcProc pthread_create err\r\n");
    }
    else
    {
        GetOfferMsg();
        pWebRTC->HandleOfferMsg(char * i_strOfferMsg, char * o_strAnswerMsg, int i_iAnswerMaxLen);
        SendAnswerMsg();
        while(1)
        {
            if(GetRtpData())
            {
                break;
            }
            pWebRTC->SendProtectedRtp(char * i_acRtpBuf, int i_iRtpBufLen);
            usleep(40*1000);//25帧率
        }
    }
    delete pWebRTC;
    return 0;
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
    printf("Usage: %s StunIP StunPort\r\n",i_strProcName);
    printf("egg: %s 192.168.0.119 8888\r\n",i_strProcName);
}

/*****************************************************************************
-Fuction        : WebRtcProc
-Description    : WebRtcProc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static void * WebRtcProc(void *pArg)
{
    //设置线程名字,以便让系统知道,
    prctl(15,(unsigned long)"WebRtcProc");
    WebRTC * pWebRTC=NULL;
    
    if(NULL != pArg)
    {
        pWebRTC = (WebRTC *)pArg;
        pWebRTC->Proc();
    }
    return NULL;
}

