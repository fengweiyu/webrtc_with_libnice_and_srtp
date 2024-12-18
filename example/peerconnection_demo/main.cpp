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
#if 0

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

#define WEBRTC_RTP_PAYLOAD_H264 102 //webrtc中是102
#define WEBRTC_VIDEO_ENCODE_FORMAT_NAME "H264"
#define WEBRTC_H264_TIMESTAMP_FREQUENCY 90000

typedef enum WebRtcOfferStatus
{
    WEBRTC_OFFER_INIT,
    WEBRTC_OFFER_SEND_OFFER,
    WEBRTC_OFFER_HANDLE_ANSWER,
    WEBRTC_OFFER_HANDLE_CANDIDATE,
    WEBRTC_OFFER_SEND_READY,//等待通道建立
    WEBRTC_OFFER_SEND_RTP,
    WEBRTC_OFFER_EXIT,
}E_WebRtcOfferStatus;

typedef enum WebRtcAnswerStatus
{
    WEBRTC_ANSWER_INIT,
    WEBRTC_ANSWER_HANDLE_OFFER,
    WEBRTC_ANSWER_HANDLE_CANDIDATE,
    WEBRTC_ANSWER_SEND_READY,//等待通道建立
    WEBRTC_ANSWER_SEND_RTP,
    WEBRTC_ANSWER_EXIT,
}E_WebRtcAnswerStatus;

static void PrintUsage(char *i_strProcName);
static void * WebRtcProc(void *pArg);
static int OfferProc(WebRTC * i_pWebRTC,char * i_strServerIp, int i_iServerPort, char * i_strSelfName,char *i_strVideoPath);
static int AnswerProc(WebRTC * i_pWebRTC,char * i_strServerIp, int i_iServerPort, char * i_strSelfName,char *i_strVideoPath);

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

    int iRet = -1;
    
    if(argc !=6)
    {
        PrintUsage(argv[0]);
        return -1;
    }
    memset(strSeverIp,0,sizeof(strSeverIp));
    strncpy(strSeverIp,argv[1],sizeof(strSeverIp));
    dwPort=atoi(argv[2]);
    
    //简单点，又因为没有ui，所以offer 和answer不能共存
    if(0 == strcmp(argv[5],"offer"))
    {
        pWebRTC = new WebRtcOffer(strSeverIp,dwPort,ICE_CONTROLLING_ROLE);
    }
    else if(0 == strcmp(argv[5],"answer"))
    {
        pWebRTC = new WebRtcAnswer(strSeverIp,dwPort,ICE_CONTROLLING_ROLE);
    }
    else
    {
        PrintUsage(argv[0]);
        return -1;
    }
    if( pthread_create(&tWebRtcID,NULL,WebRtcProc,(void *)pWebRTC) != 0 )
    {
        printf("WebRtcProc pthread_create err\r\n");
    }
    else
    {
        if(0 == strcmp(argv[5],"offer"))
        {
            iRet =OfferProc(pWebRTC,strSeverIp,dwPort,argv[3],argv[4]);
        }
        else
        {
            iRet =AnswerProc(pWebRTC,strSeverIp,dwPort,argv[3],argv[4]);
        }
    }
    delete pWebRTC;
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
    printf("Usage: %s StunIP StunPort SelfName VideoFile offer/answer\r\n",i_strProcName);
    printf("eg: %s 192.168.0.181 8888 ywf sintel.h264 answer\r\n",i_strProcName);
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

/*****************************************************************************
-Fuction        : AnswerProc
-Description    : SendAnswerProc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static int OfferProc(WebRTC * i_pWebRTC,char * i_strServerIp, int i_iServerPort, char * i_strSelfName,char *i_strVideoPath)
{
    char acGetMsg[6*1024];
    T_VideoInfo tVideoInfo;
    char acOfferMsg[6*1024];
    RtpInterface *pRtpInterface=NULL;
    SignalingInterface *pSignalingInf=NULL;
    int iRecvLen=0;
    int iPeerId = -1;
    int iRet=0;

    pSignalingInf = new SignalingInterface(0);
    pRtpInterface = new RtpInterface(i_strVideoPath);
    int iPacketNum=0;
    unsigned char *ppbPacketBuf[WEBRTC_RTP_MAX_PACKET_NUM]={0};
    int aiEveryPacketLen[WEBRTC_RTP_MAX_PACKET_NUM]={0};
    int i=0;
    for(i=0;i<WEBRTC_RTP_MAX_PACKET_NUM;i++)
    {
        ppbPacketBuf[i]=(unsigned char *)malloc(WEBRTC_RTP_MAX_PACKET_SIZE);//
        memset(ppbPacketBuf[i],0,WEBRTC_RTP_MAX_PACKET_SIZE);
    }
    E_WebRtcOfferStatus eWebRtcStatus=WEBRTC_OFFER_INIT;
    while(1)
    {
        switch(eWebRtcStatus)
        {
            case WEBRTC_OFFER_INIT:
            {
                iPacketNum = pRtpInterface->GetRtpData(ppbPacketBuf,aiEveryPacketLen,WEBRTC_RTP_MAX_PACKET_SIZE,WEBRTC_RTP_MAX_PACKET_NUM);//第一次会失败
                iPeerId = pSignalingInf->Login(i_strServerIp,i_iServerPort,i_strSelfName);
                if(iPeerId >= 0)
                {
                    eWebRtcStatus=WEBRTC_OFFER_SEND_OFFER;
                }
                else
                {
                    printf("pSignalingInf->Login err\r\n");
                }
                break;
            }
            case WEBRTC_OFFER_SEND_OFFER:
            {
                memset(&tVideoInfo,0,sizeof(tVideoInfo));
                snprintf(tVideoInfo.strFormatName,sizeof(tVideoInfo.strFormatName),"%s",WEBRTC_VIDEO_ENCODE_FORMAT_NAME);
                tVideoInfo.dwTimestampFrequency=WEBRTC_H264_TIMESTAMP_FREQUENCY;  
                tVideoInfo.bRtpPayloadType=WEBRTC_RTP_PAYLOAD_H264;
                tVideoInfo.wPortNumForSDP=9;
                tVideoInfo.iMediaID=0;
                memset(acOfferMsg,0,sizeof(acOfferMsg));
                if(i_pWebRTC->GenerateLocalMsg(&tVideoInfo,acOfferMsg,sizeof(acOfferMsg))>=0)
                {
                    memset(acGetMsg,0,sizeof(acGetMsg));
                    pSignalingInf->SendMsg(iPeerId,acOfferMsg,strlen(acOfferMsg),acGetMsg,&iRecvLen,sizeof(acGetMsg));
                    eWebRtcStatus=WEBRTC_OFFER_HANDLE_ANSWER;
                }
                else
                {
                    printf("pSignalingInf->GenerateLocalMsg err\r\n");
                }
                break;
            }
            
            case WEBRTC_OFFER_HANDLE_ANSWER:
            {
                memset(&tVideoInfo,0,sizeof(tVideoInfo));
                snprintf(tVideoInfo.strFormatName,sizeof(tVideoInfo.strFormatName),"%s",WEBRTC_VIDEO_ENCODE_FORMAT_NAME);
                tVideoInfo.dwTimestampFrequency=WEBRTC_H264_TIMESTAMP_FREQUENCY;  
                tVideoInfo.bRtpPayloadType=WEBRTC_RTP_PAYLOAD_H264;
                tVideoInfo.wPortNumForSDP=9;
                tVideoInfo.iMediaID=0;
                memset(acOfferMsg,0,sizeof(acOfferMsg));
                if(i_pWebRTC->GenerateLocalCandidateMsg(&tVideoInfo,acOfferMsg,sizeof(acOfferMsg))>=0)
                {
                    memset(acGetMsg,0,sizeof(acGetMsg));
                    if(0==pSignalingInf->SendMsg(iPeerId,acOfferMsg,strlen(acOfferMsg),acGetMsg,&iRecvLen,sizeof(acGetMsg)))
                    {
                        if(0==i_pWebRTC->HandleMsg(acGetMsg))
                        {
                            eWebRtcStatus=WEBRTC_OFFER_HANDLE_CANDIDATE;
                        }
                        else
                        {
                            printf("pSignalingInf->HandleOfferMsg err\r\n");
                        }
                    }
                    else
                    {
                        printf("pSignalingInf->Send acOfferMsg err\r\n");
                    }
                }
                else
                {
                    printf("pSignalingInf->GenerateLocalMsg err\r\n");
                }
                break;
            }
            
            case WEBRTC_OFFER_HANDLE_CANDIDATE:
            {
                memset(acGetMsg,0,sizeof(acGetMsg));
                if(pSignalingInf->GetCandidateMsg(acGetMsg,&iRecvLen,sizeof(acGetMsg)) >= 0)
                {
                    if(0==i_pWebRTC->HandleCandidateMsg(acGetMsg))
                    {
                        eWebRtcStatus=WEBRTC_OFFER_SEND_READY;
                    }
                    else
                    {
                        printf("pSignalingInf->HandleCandidateMsg err\r\n");
                    }
                }
                else
                {
                    printf("pSignalingInf->GetCandidateMsg err\r\n");
                }
                break;
            }
            case WEBRTC_OFFER_SEND_READY:
            {
                if(i_pWebRTC->GetSendReadyFlag() == 0)
                {
                    eWebRtcStatus=WEBRTC_OFFER_SEND_RTP;
                    printf("WEBRTC_SEND_RTP......\r\n");
                }
                else
                {
                    //printf("WEBRTC_SEND_READY......\r\n");
                }
                break;
            }
            
            case WEBRTC_OFFER_SEND_RTP:
            {
                if(iPacketNum > 0)
                {
                    for(i=0;i<iPacketNum;i++)
                    {
                        i_pWebRTC->SendProtectedRtp((char *)ppbPacketBuf[i], aiEveryPacketLen[i]);
                    }
                    iPacketNum = -1;
                }
                else
                {
                    printf("pRtpInterface->GetRtpData err\r\n");
                    eWebRtcStatus=WEBRTC_OFFER_SEND_RTP;
                }
                iPacketNum = pRtpInterface->GetRtpData(ppbPacketBuf,aiEveryPacketLen,WEBRTC_RTP_MAX_PACKET_SIZE,WEBRTC_RTP_MAX_PACKET_NUM);
                break;
            }
            
            case WEBRTC_OFFER_EXIT:
            {
                printf("#####################WEBRTC_EXIT:%d\r\n",eWebRtcStatus);
                break;
            }
            
            default:
            {
                printf("unkown eWebRtcStatus:%d\r\n",eWebRtcStatus);
                break;
            }
            usleep(40*1000);//25帧率
        }
        if(WEBRTC_OFFER_EXIT == eWebRtcStatus)
        {
            break;
        }
    }
    for(i=0;i<WEBRTC_RTP_MAX_PACKET_NUM;i++)
        free(ppbPacketBuf[i]);

    return iRet;
}
/*****************************************************************************
-Fuction        : AnswerProc
-Description    : SendAnswerProc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static int AnswerProc(WebRTC * i_pWebRTC,char * i_strServerIp, int i_iServerPort, char * i_strSelfName,char *i_strVideoPath)
{
    char acGetMsg[6*1024];
    T_VideoInfo tVideoInfo;
    char acAnswerMsg[6*1024];
    RtpInterface *pRtpInterface=NULL;
    SignalingInterface *pSignalingInf=NULL;
    int iRecvLen=0;
    int iPeerId = -1;
    int iRet=0;

    pSignalingInf = new SignalingInterface(1);
    pRtpInterface = new RtpInterface(i_strVideoPath);
    int iPacketNum=0;
    unsigned char *ppbPacketBuf[WEBRTC_RTP_MAX_PACKET_NUM]={0};
    int aiEveryPacketLen[WEBRTC_RTP_MAX_PACKET_NUM]={0};
    int i=0;
    for(i=0;i<WEBRTC_RTP_MAX_PACKET_NUM;i++)
    {
        ppbPacketBuf[i]=(unsigned char *)malloc(WEBRTC_RTP_MAX_PACKET_SIZE);//
        memset(ppbPacketBuf[i],0,WEBRTC_RTP_MAX_PACKET_SIZE);
    }
    E_WebRtcAnswerStatus eWebRtcStatus=WEBRTC_ANSWER_INIT;
    while(1)
    {
        switch(eWebRtcStatus)
        {
            case WEBRTC_ANSWER_INIT:
            {
                iPacketNum = pRtpInterface->GetRtpData(ppbPacketBuf,aiEveryPacketLen,WEBRTC_RTP_MAX_PACKET_SIZE,WEBRTC_RTP_MAX_PACKET_NUM);//第一次会失败
                if(pSignalingInf->Login(i_strServerIp,i_iServerPort,i_strSelfName)==0)
                {
                    eWebRtcStatus=WEBRTC_ANSWER_HANDLE_OFFER;
                }
                else
                {
                    printf("pSignalingInf->Login err\r\n");
                }
                break;
            }
            
            case WEBRTC_ANSWER_HANDLE_OFFER:
            {
                memset(acGetMsg,0,sizeof(acGetMsg));
                iPeerId = pSignalingInf->GetMsg(acGetMsg,&iRecvLen,sizeof(acGetMsg));
                if(iPeerId>=0)
                {
                    if(0==i_pWebRTC->HandleMsg(acGetMsg))
                    {
                        eWebRtcStatus=WEBRTC_ANSWER_HANDLE_CANDIDATE;
                    }
                    else
                    {
                        printf("pSignalingInf->HandleOfferMsg err\r\n");
                    }
                }
                else
                {
                    printf("pSignalingInf->GetOfferMsg err\r\n");
                }
                break;
            }
            
            case WEBRTC_ANSWER_HANDLE_CANDIDATE:
            {
                memset(acGetMsg,0,sizeof(acGetMsg));
                iPeerId = pSignalingInf->GetCandidateMsg(acGetMsg,&iRecvLen,sizeof(acGetMsg));
                if(iPeerId>=0)
                {
                    if(0==i_pWebRTC->HandleCandidateMsg(acGetMsg))
                    {
                        memset(acAnswerMsg,0,sizeof(acAnswerMsg));
                        memset(&tVideoInfo,0,sizeof(tVideoInfo));
                        snprintf(tVideoInfo.strFormatName,sizeof(tVideoInfo.strFormatName),"%s",WEBRTC_VIDEO_ENCODE_FORMAT_NAME);
                        tVideoInfo.dwTimestampFrequency=WEBRTC_H264_TIMESTAMP_FREQUENCY;  
                        tVideoInfo.bRtpPayloadType=WEBRTC_RTP_PAYLOAD_H264;
                        tVideoInfo.wPortNumForSDP=9;
                        if(i_pWebRTC->GenerateLocalMsg(&tVideoInfo,acAnswerMsg,sizeof(acAnswerMsg))>=0)
                        {
                            if(0==pSignalingInf->SendMsg(iPeerId,acAnswerMsg,strlen(acAnswerMsg),acGetMsg,&iRecvLen,sizeof(acGetMsg)))
                            {
                                eWebRtcStatus=WEBRTC_ANSWER_SEND_READY;
                            }
                            else
                            {
                                printf("pSignalingInf->SendAnswerMsg err\r\n");
                            }
                        }
                        else
                        {
                            printf("pSignalingInf->GenerateLocalMsg err\r\n");
                        }
                    }
                    else
                    {
                        printf("pSignalingInf->HandleCandidateMsg err\r\n");
                    }
                }
                else
                {
                    printf("pSignalingInf->GetCandidateMsg err\r\n");
                }
                break;
            }
            case WEBRTC_ANSWER_SEND_READY:
            {
                if(i_pWebRTC->GetSendReadyFlag() == 0)
                {
                    eWebRtcStatus=WEBRTC_ANSWER_SEND_RTP;
                    printf("WEBRTC_SEND_RTP......\r\n");
                }
                else
                {
                    //printf("WEBRTC_SEND_READY......\r\n");
                }
                break;
            }
            
            case WEBRTC_ANSWER_SEND_RTP:
            {
                if(iPacketNum > 0)
                {
                    for(i=0;i<iPacketNum;i++)
                    {
                        i_pWebRTC->SendProtectedRtp((char *)ppbPacketBuf[i], aiEveryPacketLen[i]);
                    }
                    iPacketNum = -1;
                }
                else
                {
                    printf("pRtpInterface->GetRtpData err\r\n");
                    eWebRtcStatus=WEBRTC_ANSWER_SEND_RTP;
                }
                iPacketNum = pRtpInterface->GetRtpData(ppbPacketBuf,aiEveryPacketLen,WEBRTC_RTP_MAX_PACKET_SIZE,WEBRTC_RTP_MAX_PACKET_NUM);
                break;
            }
            
            case WEBRTC_ANSWER_EXIT:
            {
                printf("#####################WEBRTC_EXIT:%d\r\n",eWebRtcStatus);
                break;
            }
            
            default:
            {
                printf("unkown eWebRtcStatus:%d\r\n",eWebRtcStatus);
                break;
            }
            usleep(40*1000);//25帧率
        }
        if(WEBRTC_ANSWER_EXIT == eWebRtcStatus)
        {
            break;
        }
    }
    for(i=0;i<WEBRTC_RTP_MAX_PACKET_NUM;i++)
        free(ppbPacketBuf[i]);

    return iRet;
}
#endif
