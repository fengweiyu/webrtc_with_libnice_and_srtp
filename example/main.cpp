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

#include <pthread.h>
#include <sys/prctl.h>

#include "webrtc.h"
#include "signalling_interface.h"
#include "rtp_interface.h"

#define WEBRTC_RTP_MAX_PACKET_NUM	(300)
#define WEBRTC_RTP_MAX_PACKET_SIZE	((1500-42)/4*4)//MTU

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
    RtpInterface *pRtpInterface=NULL;
    SignalingInterface *pSignalingInf=NULL;
    int iRecvLen=0;
    int iPeerId = -1;

    
    if(argc !=5)
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
        pSignalingInf = new SignalingInterface();
        pRtpInterface = new RtpInterface(argv[4]);
        int iPacketNum=0;
        unsigned char *ppbPacketBuf[WEBRTC_RTP_MAX_PACKET_NUM]={0};
        int aiEveryPacketLen[WEBRTC_RTP_MAX_PACKET_NUM]={0};
        int i=0;
        for(i=0;i<WEBRTC_RTP_MAX_PACKET_NUM;i++)
        {
            ppbPacketBuf[i]=(unsigned char *)malloc(WEBRTC_RTP_MAX_PACKET_SIZE);
            memset(ppbPacketBuf[i],0,WEBRTC_RTP_MAX_PACKET_SIZE);
        }
        
        while(1)
        {
            memset(acOfferMsg,0,sizeof(acOfferMsg));
            iPeerId = pSignalingInf->GetOfferMsg(strSeverIp,dwPort,argv[3],acOfferMsg,&iRecvLen,sizeof(acOfferMsg));
            if(iPeerId>=0)
            {
                memset(acAnswerMsg,0,sizeof(acAnswerMsg));
                pWebRTC->HandleOfferMsg(acOfferMsg,acAnswerMsg,sizeof(acAnswerMsg));
                pSignalingInf->SendAnswerMsg(iPeerId,acAnswerMsg,strlen(acAnswerMsg));
            }
            iPacketNum = pRtpInterface->GetRtpData(&ppbPacketBuf,aiEveryPacketLen,WEBRTC_RTP_MAX_PACKET_SIZE,WEBRTC_RTP_MAX_PACKET_NUM);
            if(iPacketNum > 0)
            {
                for(i=0;i<iPacketNum;i++)
                {
                    pWebRTC->SendProtectedRtp(ppbPacketBuf[i], aiEveryPacketLen[i]);
                }
                iPacketNum = -1;
            }
            usleep(40*1000);//25帧率
        }
        for(i=0;i<WEBRTC_RTP_MAX_PACKET_NUM;i++)
            free(ppbPacketBuf[i]);
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
    printf("Usage: %s StunIP StunPort SelfName VideoFile\r\n",i_strProcName);
    printf("egg: %s 192.168.0.119 8888 ywf singl.h264\r\n",i_strProcName);
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

