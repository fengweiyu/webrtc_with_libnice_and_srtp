/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       main.cpp
* Description           : 	    stun:stun.oss.aliyuncs.com:3478 stun.voipbuster.com:3478
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/prctl.h>
#include "webrtc_server_interface.h"

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
    
    int dwServerPort=9112;
    char strStunIp[20];//"77.72.169.210"
    int dwStunPort=3478;
    
    if(argc !=4)
    {
        PrintUsage(argv[0]);
        snprintf(strStunIp,sizeof(strStunIp),"%s","77.72.169.210");//stun.voipbuster.com:3478
    }
    else
    {
        dwServerPort=atoi(argv[1]);
        memset(strStunIp,0,sizeof(strStunIp));
        snprintf(strStunIp,sizeof(strStunIp),"%s",argv[2]);
        dwStunPort=atoi(argv[3]);
    }
    WebRtcServerInf *pWebRtcServer = new WebRtcServerInf(dwServerPort);
    iRet=pWebRtcServer->Proc(strStunIp,dwStunPort);//×èÈû
    
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
    printf("Usage: %s ServerPort StunIP StunPort\r\n",i_strProcName);
    //printf("eg: %s 9112 77.72.169.210 3478\r\n",i_strProcName);
    printf("run default args: %s 9112 77.72.169.210 3478\r\n",i_strProcName);
}

