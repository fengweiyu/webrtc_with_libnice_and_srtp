/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcSession.c
* Description           : 	    webrtc媒体通道建立完成时间优化到600-700ms
亚马逊alexa支持pcma但是sdp中没有a=rtpmap:8 PCMA/8000 (只有个opus)，
可以返回的sdp中直接携带rtpmap:8 PCMA/8000，让对方支持自己的能力
对于sdp协商过程中耗时的媒体处理可以先缓存，后续再发送，实现不丢帧
//if(NULL != ptListFrame)//修改时间戳缓存，再发送，实现不丢帧的快放(会跳秒)，从而保证实时性
    //ptFrame->nTimeStamp=ptListFrame->nTimeStamp;//不用修改时间戳，浏览器webrtc会平滑处理(不会跳秒的放，也有实时性)
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "webrtc_session.h"
#include "cJSON.h"
#include "Base64.h"
#include "rtp_adapter.h"

#define WEBRTC_RECV_FILE_SAVE_PATH "./"//"/work/share/webrtc/"
#define WEBRTC_RTP_PAYLOAD_G711A     8//a=rtpmap:8 PCMA/8000 webrtc RTP_PAYLOAD_G711A(rfc规范中定义的值)
#define WEBRTC_H264_ENCODE_FORMAT_NAME "H264"
#define WEBRTC_H264_TIMESTAMP_FREQUENCY 90000
#define WEBRTC_AUDIO_ENCODE_FORMAT_NAME "PCMA"
#define WEBRTC_G711A_TIMESTAMP_FREQUENCY 8000
#define WEBRTC_H265_ENCODE_FORMAT_NAME "H265"
#define WEBRTC_H265_TIMESTAMP_FREQUENCY 90000
#define WEBRTC_FRAME_BUF_MAX_LEN	(2*1024*1024) 

/*****************************************************************************
-Fuction		: WebRtcSession
-Description	: WebRtcSession
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
WebRtcSession :: WebRtcSession(char * i_strStunAddr,unsigned int i_dwStunPort,T_WebRtcSessionCb i_tWebRtcSessionCb,int i_iID)
{
    T_WebRtcCfg tWebRtcCfg;
    T_WebRtcCb tWebRtcCb;

    memcpy(&m_tWebRtcSessionCb,&i_tWebRtcSessionCb,sizeof(T_WebRtcSessionCb));
    memset(&m_tWebRtcSdpMediaInfo,0,sizeof(T_WebRtcSdpMediaInfo));

    memset(&tWebRtcCb,0,sizeof(T_WebRtcCb));
    tWebRtcCb.RecvRtpData = WebRtcSession::RecvRtpData;
    tWebRtcCb.RecvRtcpData = WebRtcSession::RecvRtcpData;
    tWebRtcCb.RecvStopMsg = WebRtcSession::RecvClientStopMsg;
    tWebRtcCb.IsRtp = WebRtcSession::IsRtpCb;
    tWebRtcCb.IsRtcp = NULL;
    //stun:stun.oss.aliyuncs.com:3478
    memset(&tWebRtcCfg,0,sizeof(T_WebRtcCfg));
    snprintf(tWebRtcCfg.strStunAddr,sizeof(tWebRtcCfg.strStunAddr),"%s",i_strStunAddr);//"77.72.169.210" 不能使用域名，内部库不能解析
    tWebRtcCfg.dwStunPort = i_dwStunPort;//3478;//stun.voipbuster.com:3478
    tWebRtcCfg.eWebRtcRole= WEBRTC_ANSWER_ROLE;
    tWebRtcCfg.eControlling= ICE_CONTROLLED_ROLE;

    m_pWebRTC = new WebRtcInterface(tWebRtcCfg,tWebRtcCb,this);
    if(NULL == m_pWebRTC)
    {
        WEBRTC_LOGE("WebRtcMediaSession NULL \r\n");
    }
    m_pWebRtcProc = new thread(&WebRtcInterface::Proc, m_pWebRTC);
    m_pWebRtcProc->detach();//注意线程回收
    m_pRtpParseInterface = new RtpInterface(NULL);

    int i=0,len=0;
    m_pbPacketsBuf = new unsigned char [SRTP_PACKET_MAX_NUM*SRTP_PACKET_MAX_SIZE];
    memset(m_pbPacketsBuf,0,SRTP_PACKET_MAX_NUM*SRTP_PACKET_MAX_SIZE);
    m_ppbPacketBuf = new unsigned char *[SRTP_PACKET_MAX_NUM];
    memset(m_ppbPacketBuf,0,SRTP_PACKET_MAX_NUM*sizeof(unsigned char *));
    for(i=0;i<SRTP_PACKET_MAX_NUM;i++)
    {
        m_ppbPacketBuf[i]=m_pbPacketsBuf+len;
        len+=SRTP_PACKET_MAX_SIZE;
    }
    m_piEveryPacketLen = new int[SRTP_PACKET_MAX_NUM];
    memset(m_piEveryPacketLen,0,SRTP_PACKET_MAX_NUM*sizeof(int));
    m_pRtpInterface = NULL;
    m_iSendSdpSuccess = -1;
    m_iPackNum = -1;
    
    memset(&m_tPushFrameInfo,0,sizeof(T_MediaFrameInfo));
    m_tPushFrameInfo.pbFrameBuf = new unsigned char [WEBRTC_FRAME_BUF_MAX_LEN];
    m_tPushFrameInfo.iFrameBufMaxLen = WEBRTC_FRAME_BUF_MAX_LEN;
    m_tPushFrameInfo.iFrameBufLen = 0;
    m_pFileProc = NULL;
    m_iFileProcFlag = 0;
    m_iFileExitProcFlag = 0;
    m_pFileName = NULL;
    m_iTalkTestFlag = 0;
    
    m_iLogID = i_iID;
    
    m_iPullVideoFrameRate=0;
    m_dwVideoPullTimeStamp=0;
    m_dwAudioPullTimeStamp=0;
    m_iFindedKeyFrame=0;
    dwLastAudioTimeStamp=0;
    dwLastVideoTimeStamp=0;
    m_pMediaFile=NULL;
    m_pMediaFileMP4=NULL;
    m_pbFileBuf= new unsigned char [WEBRTC_FRAME_BUF_MAX_LEN];
}

/*****************************************************************************
-Fuction		: ~WebRtcServer
-Description	: ~WebRtcServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
WebRtcSession :: ~WebRtcSession()
{
    if(NULL!= m_pRtpParseInterface)
    {
        delete m_pRtpParseInterface;
        m_pRtpParseInterface = NULL;//
    }
    if(NULL!= m_pRtpInterface)
    {
        delete m_pRtpInterface;
        m_pRtpInterface = NULL;//
    }
    if(NULL != m_pWebRtcProc)
    {
        m_pWebRTC->StopProc();
        while(0 == m_pWebRTC->GetStopedFlag()){usleep(10);};//使用这个等待退出delete m_pWebRtcProc会报子线程未退出错误
        //m_pWebRtcProc->join();//用join()等待退出,如果和前面一起使用这里会一直阻塞(可能因为线程已退出)
        //delete m_pWebRtcProc;//暂时用m_pWebRtcProc->detach();后续优化
    }
    if(NULL!= m_pFileProc)
    {
        m_iFileProcFlag = 0;//m_pWebRTC->StopProc();
        while(0 == m_iFileExitProcFlag){usleep(10);};
        //m_pFileProc->join();//用join()等待退出这里会一直阻塞
        delete m_pFileProc;
        m_pFileProc = NULL;//
    }
    if(NULL!= m_pbPacketsBuf)
    {
        delete[] m_pbPacketsBuf;
        m_pbPacketsBuf = NULL;//
    }
    if(NULL!= m_ppbPacketBuf)
    {
        delete[] m_ppbPacketBuf;
        m_ppbPacketBuf = NULL;//
    }
    if(NULL!= m_piEveryPacketLen)
    {
        delete[] m_piEveryPacketLen;
        m_piEveryPacketLen = NULL;//
    }
    if(NULL!= m_pWebRTC)
    {
        delete m_pWebRTC;
        m_pWebRTC = NULL;//
    }
    if(NULL!= m_pWebRtcProc)
    {
        delete m_pWebRtcProc;
        m_pWebRtcProc = NULL;//
    }
    if(NULL!= m_tPushFrameInfo.pbFrameBuf)
    {
        delete[] m_tPushFrameInfo.pbFrameBuf;
        m_tPushFrameInfo.pbFrameBuf = NULL;
    }
    if(NULL!= m_tFileFrameInfo.pbFrameBuf)
    {
        delete[] m_tFileFrameInfo.pbFrameBuf;
        m_tFileFrameInfo.pbFrameBuf = NULL;//
    }
    if(NULL!= m_pFileName)
    {
        delete m_pFileName;
        m_pFileName = NULL;//
    }
    if(NULL!= m_pbFileBuf)
    {
        delete[] m_pbFileBuf;
        m_pbFileBuf = NULL;//
    }
    if(NULL!= m_pMediaFile)
    {
        fclose(m_pMediaFile);
        m_pMediaFile = NULL;//
    }
    if(NULL!= m_pMediaFileMP4)
    {
        fclose(m_pMediaFileMP4);
        m_pMediaFileMP4 = NULL;//
    }
    WEBRTC_LOGW("~WebRtcSession end  %d\r\n",m_iFileProcFlag);
}

/*****************************************************************************
-Fuction        : SetReqData
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::SetReqData(const char *i_strReqURL,const char *i_strReqBody)
{
    int iRet =-1;
    if(NULL == i_strReqBody || NULL == i_strReqURL)
    {
        WEBRTC_LOGE("SetReqData NULL\r\n");
        return -1;
    }
    WEBRTC_LOGW2(m_iLogID,"SetReqData %s,%s\r\n",i_strReqURL,i_strReqBody);
    m_strReqBody.assign(i_strReqBody);
    m_strReqURL.assign(i_strReqURL);
    //只执行一次
    m_pWebRTC->DtlsInit();//dtls初始化耗时300ms
    
    //iRet = m_pWebRTC->GetGatheringDoneFlag();//正好收集地址(耗时300ms)并行处理
    //WEBRTC_LOGW2(m_iLogID,"m_pWebRTC->GetGatheringDoneFlag() %d,%s \r\n",iRet,i_strReqURL);
    if(0 == TestURL(i_strReqURL))
    {
        iRet = this->HandleRequest(i_strReqURL);
        if(iRet < 0)
        {
            WEBRTC_LOGD2(m_iLogID,"HandleRequest err [%d]%s\r\n", iRet,i_strReqURL);
            this->StopSession(400);
        }
        return iRet;   
    }
    return iRet;   
}
/*****************************************************************************
-Fuction        : PlayTestURL
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::TestURL(const char * url)
{
    string strURL(url);
    
    auto dwTestTalkPos = strURL.find("testTalk/");
    if(string::npos != dwTestTalkPos)
    {
        WEBRTC_LOGW2(m_iLogID,"testTalk %s\r\n",url);
        m_iTalkTestFlag = 1;
        if(NULL != m_pFileName)
            delete m_pFileName;
        auto dwWebRtcTalkPos = strURL.find(".webrtc");
        m_pFileName = new string(strURL.substr(dwTestTalkPos+strlen("testTalk/"),dwWebRtcTalkPos-(dwTestTalkPos+strlen("testTalk/"))).c_str());
        WEBRTC_LOGW2(m_iLogID,"testTalk m_pFileName %s\r\n",m_pFileName->c_str());
        return 0;
    }
    auto dwTestAvTalkPos = strURL.find("testVideoCall/");
    if(string::npos != dwTestAvTalkPos)
    {
        WEBRTC_LOGW2(m_iLogID,"testVideoCall %s\r\n",url);
        m_iTalkTestFlag = 1;
        if(NULL != m_pFileName)
            delete m_pFileName;
        auto dwWebRtcTalkAvPos = strURL.find(".webrtc");
        m_pFileName = new string(strURL.substr(dwTestAvTalkPos+strlen("testVideoCall/"),dwWebRtcTalkAvPos-(dwTestAvTalkPos+strlen("testVideoCall/"))).c_str());
        string strFileFd(WEBRTC_RECV_FILE_SAVE_PATH);
        strFileFd.append(m_pFileName->c_str());
        string strFilePathName(strFileFd.c_str());
        strFilePathName.append("_recv.flv");
        m_pMediaFile=fopen(strFilePathName.c_str(),"wb");
        string strFilePathNameMP4(strFileFd.c_str());
        strFilePathNameMP4.append(".mp4");
        m_pMediaFileMP4=fopen(strFilePathNameMP4.c_str(),"wb");
        WEBRTC_LOGW2(m_iLogID,"testVideoCall m_pFileName %s,%s\r\n",strFilePathName.c_str(),strFilePathNameMP4.c_str());
        return 0;
    }
    
    auto dwTestPos = strURL.find("test/");
    if(string::npos != dwTestPos)//"D:\\test\\2023AAC.flv"
    {
        if(NULL != m_pFileName)
            delete m_pFileName;
        auto dwWebRtcPos = strURL.find(".webrtc");
        m_pFileName = new string(strURL.substr(dwTestPos+strlen("test/"),dwWebRtcPos-(dwTestPos+strlen("test/"))).c_str());
        WEBRTC_LOGW2(m_iLogID,"test m_pFileName %s\r\n",m_pFileName->c_str());
        
        return 0;
    }

    return -1;
}

/*****************************************************************************
-Fuction        : HandleRequest
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::HandleRequest(const char * strURL)
{
    int iRet = -1;
    //iRet = this->HandleRemoteSDP();//必须先设置到内部，内部解析了才能生成sdp，
    //if(iRet < 0)
    {
        //WEBRTC_LOGD2(m_iLogID,"HandleRemoteSDP err [%d]%s\r\n", iRet,strURL);
        //return iRet;
    }
    if(NULL != m_pFileName)
    {
        m_pRtpInterface = new RtpInterface((char *)m_pFileName->c_str());//m_pFileName->c_str());
        m_pFileProc = new thread(&WebRtcSession::TestProc, this);
        m_pFileProc->detach();//注意线程回收
        WEBRTC_LOGD2(m_iLogID,"HandleRequest m_pFileName [%d]\r\n", m_iTalkTestFlag);
        return 0;
    }
    m_pRtpInterface = new RtpInterface(NULL);
    
    return 0;
}

/*****************************************************************************
-Fuction        : TestProc
-Description    : 如果是实时预览(直播)，如果到达的帧的时间戳不够均匀不够线性(这种情况使用帧时间戳大概率会跳秒),
则可使用到达时间作为时间戳来保证实时性流畅性
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
void WebRtcSession::TestProc()
{
    int iRet = -1;
    int iFirstPacketSendedFlag=0;
    int iGatheringDoneFlag=0;
	unsigned int dwFileLastTimeStamp=0;
	unsigned int dwSleepTime=0;
	unsigned int dwFileCurTick=0;
	unsigned int dwFileLastTick=0;
	
    memset(&m_tFileFrameInfo,0,sizeof(T_MediaFrameInfo));
    m_tFileFrameInfo.pbFrameBuf = new unsigned char [WEBRTC_FRAME_BUF_MAX_LEN];
    m_tFileFrameInfo.iFrameBufMaxLen = WEBRTC_FRAME_BUF_MAX_LEN;
    m_iFileProcFlag = 1;
    m_iFileExitProcFlag = 0;
    while(m_iFileProcFlag)
    {
        if(0 == iGatheringDoneFlag)
        {
            iRet = m_pWebRTC->GetGatheringDoneFlag();//正好收集地址(耗时300ms)并行处理
            WEBRTC_LOGW2(m_iLogID,"m_pWebRTC->GetGatheringDoneFlag() %d\r\n",iRet);
            if(0 != iRet)
            {
                usleep(30*1000);
                continue;
            }
            iRet = this->HandleRemoteSDP();//必须先设置到内部，内部解析了才能生成sdp，
            if(iRet < 0)
            {
                WEBRTC_LOGW2(m_iLogID,"HandleRemoteSDP err [%d]%s\r\n", iRet,m_strReqBody.c_str());
                this->StopSession(400);
                break;
            }
            iGatheringDoneFlag = 1;
        }


        iRet = this->HandleMediaFrame(&m_tFileFrameInfo);
        if(iRet<0)
        {
            unsigned int dwProfileLevelId = (m_tFileFrameInfo.tVideoEncodeParam.abSPS[1]<<16) | (m_tFileFrameInfo.tVideoEncodeParam.abSPS[2]<<8) | m_tFileFrameInfo.tVideoEncodeParam.abSPS[3];
            WEBRTC_LOGE2(m_iLogID,"TestProc exit eEncType%d eFrameType%d,%s,dwProfileLevelId %#x\r\n",m_tFileFrameInfo.eEncType,m_tFileFrameInfo.eFrameType,m_pFileName->c_str(),dwProfileLevelId);
            this->StopSession(-2!=iRet?400:(int)dwProfileLevelId);
            break;
        }
        if(m_tFileFrameInfo.dwTimeStamp<dwFileLastTimeStamp)
        {
            WEBRTC_LOGE2(m_iLogID,"dwTimeStamp err exit %d,%d\r\n",m_tFileFrameInfo.dwTimeStamp,dwFileLastTimeStamp);
            this->StopSession(402);
            break;
        }
        if(0 == dwFileLastTimeStamp && 0 == dwFileLastTick)
        {
            dwFileLastTimeStamp=m_tFileFrameInfo.dwTimeStamp;
            dwFileLastTick = GetTickCount();
            WEBRTC_LOGE2(m_iLogID,"TestProc Init [%s]\r\n",m_pFileName->c_str());
            continue;
        }
        dwSleepTime=m_tFileFrameInfo.dwTimeStamp-dwFileLastTimeStamp;
        if(0 == iFirstPacketSendedFlag)//dwSleepTime && MEDIA_FRAME_TYPE_AUDIO_FRAME != m_tFileFrameInfo.eFrameType)
        {
            usleep(50*1000);//后续优化
            iRet = this->SendDatas(&m_tFileFrameInfo);
            if(iRet>=0)//&&0 == m_iStartedPushStreamFlag)
            {
                iFirstPacketSendedFlag= 1;
            }
            dwFileLastTick = GetTickCount();
            continue;
        }
        dwFileLastTimeStamp=m_tFileFrameInfo.dwTimeStamp;
        dwFileCurTick=GetTickCount();
        //WEBRTC_LOGD2(m_iLogID,"%d dwFileCurTick-dwFileLastTick %d dwSleepTime%d\r\n",m_iLastRecvDataTime,dwFileCurTick-dwFileLastTick,dwSleepTime);
        if(dwFileCurTick-dwFileLastTick <= dwSleepTime)
        {
            usleep((dwSleepTime-(dwFileCurTick-dwFileLastTick))*1000);
        }
        //usleep(20*1000);//卡顿
        dwFileLastTick = GetTickCount();
        
        iRet = this->SendDatas(&m_tFileFrameInfo);
        //WEBRTC_LOGD2(m_iLogID,"%d SendDatas iRet %d dwSleepTime%d\r\n",m_iLastRecvDataTime,iRet,dwSleepTime);
        //m_iLastRecvDataTime = GetTickCount;//心跳机制，20s无流则主动断开
    }
    m_iFileExitProcFlag = 1;
}

/*****************************************************************************
-Fuction        : HandleRemoteSDP
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::HandleRemoteSDP()
{
    int iRet = -1;
    if(m_strReqBody.length() > 0)
    {
        cJSON * ptJson = NULL;
        cJSON * ptNode = NULL;
        ptJson = cJSON_Parse(m_strReqBody.c_str());
        if(NULL != ptJson)
        {
            ptNode = cJSON_GetObjectItem(ptJson,"sdp");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                iRet = m_pWebRTC->HandleMsg((char *)ptNode->valuestring,1,&m_tWebRtcSdpMediaInfo);//重试,后续优化
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptJson,"FrameRate");
            if(NULL != ptNode)
            {
                m_iPullVideoFrameRate=ptNode->valueint;
                ptNode = NULL;
            }
            cJSON_Delete(ptJson);
        }
    }
    WEBRTC_LOGD3(m_iLogID,"HandleRemoteSDP %d,m_iPullVideoFrameRate %d,%s",iRet,m_iPullVideoFrameRate,m_strReqBody.c_str());
    if(iRet < 0)
    {
        WEBRTC_LOGE2(m_iLogID,"HandleRemoteSDP m_strReqBody err %s",m_strReqBody.c_str());
        //this->StopSession(400);
    }

    return iRet;
}

/*****************************************************************************
-Fuction        : SendDatas
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::HandleMediaFrame(T_MediaFrameInfo * i_ptFrameInfo) 
{
    int iRet = -1;
    int i=0;
    
    if(NULL == i_ptFrameInfo)
    {
        WEBRTC_LOGE2(m_iLogID,"HandleMediaFrame NULL[%d]\r\n", iRet);
        return -1;
    }
    //WEBRTC_LOGD2(_nClientPort," _nClientPortHandleMediaFrame m_iPackNum %d\r\n", m_iPackNum);

    if(NULL!= m_pRtpInterface && m_iPackNum < 0)
    {
        iRet = m_pRtpInterface->GetFrame((void *)i_ptFrameInfo);
        if(iRet < 0)
        {
            WEBRTC_LOGE2(m_iLogID," m_pRtpInterface->GetFrame err [%d]\r\n", iRet);
            return -1;
        }
        m_iPackNum = 0;//拿到一帧数据
    }
    //WEBRTC_LOGD2(_nClientPort," HandleMediaFrame m_iPackNum %d\r\n", m_iPackNum);


    if(0 != m_iSendSdpSuccess && i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS > 0 && i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS > 0)
    {
        iRet = this->SendLocalSDP(i_ptFrameInfo);//
        if(iRet < 0)
        {
            WEBRTC_LOGD2(m_iLogID,"HandleMediaFrame SendLocalSDP err [%d]\r\n", iRet);
            return -2;
        }
    }
    else if(0 != m_iSendSdpSuccess && 0 != m_iTalkTestFlag)
    {
        iRet = this->SendLocalSDP(NULL);//对讲不需要i帧
        if(iRet < 0)
        {
            WEBRTC_LOGD2(m_iLogID,"HandleMediaFrame SendLocalSDP TalkTestFlag err [%d]\r\n", iRet);
            return -2;
        }
    }
    if(0 != m_iSendSdpSuccess)
    {
        WEBRTC_LOGD2(m_iLogID,"no m_iSendSdpSuccess eFrameType [%d]\r\n", i_ptFrameInfo->eFrameType);
        m_iPackNum = -1;//等待i帧
        return 0;
    }

    if(NULL!= m_pRtpInterface && m_iPackNum == 0)
    {
        m_iPackNum = m_pRtpInterface->GetRtpPackets((void *)i_ptFrameInfo,m_ppbPacketBuf,m_piEveryPacketLen,SRTP_PACKET_MAX_NUM);
    }
    if(m_iPackNum <= 0)
    {
        WEBRTC_LOGE2(m_iLogID,"m_iPackNum err %d,%d\r\n", m_iPackNum,i_ptFrameInfo->eEncType);
        return -1;
    }
    //WEBRTC_LOGD2(_nClientPort," GetRtpPackets m_iPackNum %d\r\n", m_iPackNum);
    return 0;
}

/*****************************************************************************
-Fuction        : SendDatas
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::SendDatas(T_MediaFrameInfo * i_ptFrameInfo) 
{
    int iRet = -1;
    int i=0;
    
    if(NULL == i_ptFrameInfo||NULL == m_pWebRTC)
    {
        WEBRTC_LOGE2(m_iLogID,"SendDatas NULL %p,%p[%d]\r\n",i_ptFrameInfo,m_pWebRTC,iRet);
        return -1;
    }

    if(m_pWebRTC->GetSendReadyFlag() != 0)
    {
        //WEBRTC_LOGW2(_nClientPort,"m_pWebRTC GetSendReadyFlag no");
        return -1;
    }
    //if(0 == m_iStartedPushStreamFlag)
    {
        //return 0;//第一次先不发，强制i帧增加实时性
    }
    if(m_iPackNum > 0)
    {
        for(i=0;i<m_iPackNum;i++)
        {
            iRet = m_pWebRTC->SendProtectedRtp((char *)m_ppbPacketBuf[i], m_piEveryPacketLen[i],SRTP_PACKET_MAX_SIZE);
            if(iRet < 0)
            {
                WEBRTC_LOGE2(m_iLogID,"m_pWebRTC->SendProtectedRtp err %d %d %d\r\n", iRet,i,i_ptFrameInfo->eFrameType);
                //return 0;//-1; 发送相同的包，srtp会报重放攻击错误码
            }
        }
        m_iPackNum = -1;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : SendLocalSDP
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::SendLocalSDP(T_MediaFrameInfo * i_ptFrameInfo)
{
    T_WebRtcMediaInfo tMediaInfo;
    char strSPS[128];
    char strPPS[64];
    char *strSDP=NULL;
    int iRet = -1;
    T_RtpMediaInfo tRtpMediaInfo;
    const char * strVideoEncFormatName=NULL;
    unsigned int dwVideoTimestampFrequency=WEBRTC_H264_TIMESTAMP_FREQUENCY;//
    T_VideoEncodeParam *ptVideoEncodeParam=NULL;
    int iVideoEnc=MEDIA_ENCODE_TYPE_H264;//普通语音对讲默认取值这个
    
    
    if(NULL == m_tWebRtcSessionCb.SendDataOut || NULL == m_tWebRtcSessionCb.pObj)
    {
        WEBRTC_LOGE2(m_iLogID,"SendLocalSDP SendDataOut NULL");
        return -1;
    }
    if(NULL != i_ptFrameInfo)
    {
        iVideoEnc=i_ptFrameInfo->eEncType;
        ptVideoEncodeParam=&i_ptFrameInfo->tVideoEncodeParam;
        switch(i_ptFrameInfo->eEncType)
        {
            case MEDIA_ENCODE_TYPE_H264:
            {
                strVideoEncFormatName=WEBRTC_H264_ENCODE_FORMAT_NAME;
                dwVideoTimestampFrequency=WEBRTC_H264_TIMESTAMP_FREQUENCY;
                break;
            }
            case MEDIA_ENCODE_TYPE_H265:
            {
                strVideoEncFormatName=WEBRTC_H265_ENCODE_FORMAT_NAME;
                dwVideoTimestampFrequency=WEBRTC_H265_TIMESTAMP_FREQUENCY;
                break;
            }
            default :
            {
                WEBRTC_LOGE2(m_iLogID,"SendLocalSDP i_ptFrameInfo->eEncType %d err",i_ptFrameInfo->eEncType);
                break;
            }

        }
    }

    memset(&tMediaInfo,0,sizeof(T_WebRtcMediaInfo));
    if(NULL == i_ptFrameInfo)
    {
        iRet = GetSupportedVideoInfoFromSDP(WEBRTC_H264_ENCODE_FORMAT_NAME,WEBRTC_H264_TIMESTAMP_FREQUENCY,1,0,&tMediaInfo.tVideoInfo);
    }
    else
    {
        unsigned int dwProfileLevelId = (ptVideoEncodeParam->abSPS[1]<<16) | (ptVideoEncodeParam->abSPS[2]<<8) | ptVideoEncodeParam->abSPS[3];
        iRet = GetSupportedVideoInfoFromSDP(strVideoEncFormatName,dwVideoTimestampFrequency,1,dwProfileLevelId,&tMediaInfo.tVideoInfo);
        char * strSPS_Base64 = base64Encode((const char*)ptVideoEncodeParam->abSPS,ptVideoEncodeParam->iSizeOfSPS);
        char * strPPS_Base64 = base64Encode((const char*)ptVideoEncodeParam->abPPS,ptVideoEncodeParam->iSizeOfPPS);
        snprintf(strSPS,sizeof(strSPS),"%s",strSPS_Base64);
        snprintf(strPPS,sizeof(strPPS),"%s",strPPS_Base64);
        delete[] strSPS_Base64;
        delete[] strPPS_Base64;
        tMediaInfo.tVideoInfo.dwProfileLevelId = dwProfileLevelId;
        tMediaInfo.tVideoInfo.strSPS_Base64 = strSPS;
        tMediaInfo.tVideoInfo.strPPS_Base64= strPPS;
    }
    iRet |= GetSupportedAudioInfoFromSDP(WEBRTC_AUDIO_ENCODE_FORMAT_NAME,WEBRTC_G711A_TIMESTAMP_FREQUENCY,&tMediaInfo.tAudioInfo);
    if(0!= iRet)
    {
        WEBRTC_LOGE2(m_iLogID,"GetSupportedMediaInfoFromSDP err %s ,%s\r\n",strVideoEncFormatName,WEBRTC_AUDIO_ENCODE_FORMAT_NAME);
        return -1;
    }
    iRet = -1;
    tRtpMediaInfo.iAudioEnc = MEDIA_ENCODE_TYPE_G711A;
    tRtpMediaInfo.iAudioPayload = tMediaInfo.tAudioInfo.bRtpPayloadType;
    tRtpMediaInfo.iVideoEnc= iVideoEnc;
    tRtpMediaInfo.iVideoPayload= tMediaInfo.tVideoInfo.bRtpPayloadType;
    if(NULL!= m_pRtpInterface)
    {
        iRet = m_pRtpInterface->SetRtpTypeInfo((void *)&tRtpMediaInfo);
        iRet |= m_pRtpParseInterface->SetRtpTypeInfo((void *)&tRtpMediaInfo);
    }
    if(iRet != 0)
    {
        WEBRTC_LOGE2(m_iLogID,"m_pRtpInterface->SetRtpTypeInfo err %d %d,%d %d\r\n",tRtpMediaInfo.iAudioEnc,tRtpMediaInfo.iVideoEnc,tRtpMediaInfo.iAudioPayload,tRtpMediaInfo.iVideoPayload);
        return -1;
    }
    if(NULL!= m_pRtpInterface)//先设置rtp参数才能获取ssrc
    {
        iRet = m_pRtpInterface->GetSSRC(&tMediaInfo.tVideoInfo.dwSSRC,&tMediaInfo.tAudioInfo.dwSSRC);
        if(iRet != 0)
        {
            WEBRTC_LOGE2(m_iLogID,"m_pRtpInterface->GetSSRC err %d %d\r\n",tRtpMediaInfo.iAudioPayload,tRtpMediaInfo.iVideoPayload);
            return -1;
        }
        WEBRTC_LOGD2(m_iLogID,"SendLocalSDP dwSSRC %d %d\r\n",tMediaInfo.tVideoInfo.dwSSRC,tMediaInfo.tAudioInfo.dwSSRC);
    }
    else
    {
        tMediaInfo.tVideoInfo.dwSSRC = 12212321;//接收流可以随意填
        tMediaInfo.tAudioInfo.dwSSRC = 12212322;
    }
    if(NULL== m_pWebRTC)
    {
        WEBRTC_LOGE2(m_iLogID,"m_pWebRTC NULL err \r\n");
        return -1;
    }

    
    strSDP = new char[6*1024];
    if(NULL== strSDP)
    {
        WEBRTC_LOGE2(m_iLogID,"SendLocalSDP strSDP NULL");
        return -1;
    }
    memset(strSDP,0,6*1024);
    if(m_pWebRTC->GenerateLocalSDP(&tMediaInfo,strSDP,6*1024)<0)
    {
        WEBRTC_LOGE2(m_iLogID,"SendLocalSDP GenerateLocalSDP err");
        delete[] strSDP;
        return -1;
    }
    WEBRTC_LOGD2(m_iLogID,"SendLocalSDP strSDP %s\r\n",strSDP);
    
    cJSON * root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "action", cJSON_CreateString("answer"));//cJSON_AddStringToObject(root,"action","answer");
    cJSON_AddItemToObject(root, "sdp", cJSON_CreateString(strSDP));//cJSON_AddStringToObject(root,"sdp",strSDP);
    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet =m_tWebRtcSessionCb.SendDataOut(buf,m_tWebRtcSessionCb.pObj);
        free(buf);
    }
    cJSON_Delete(root);

    delete[] strSDP;
    m_iSendSdpSuccess = 0;

    //iRet = this->HandleRemoteSDP();//必须先设置到内部，内部解析了才能生成sdp，
    return iRet;
}
/*****************************************************************************
-Fuction        : GetSupportedVideoInfoFromSDP
-Description    : 
Webrtc 对于High profile则sps要一致(兼容ios，level要比要求的低，
第二字节是00则标志都是0则是无要求则不用比对，
谷歌的由于64001f 中间是00也就是无特殊要求)，
对于main和base，要第一字节一致，并且第三字节即level要比要求的低
(比如浏览器是1f,则服务端的流要<=1f的流，level等级高则码率高，浏览器接受不了，
(如果严格一点的话，则第二字节也就是编码规范标志高2位也要符合，
比如浏览器要求e0则服务流至少要c0,如果浏览器第二字节是00则标志都是0
则是无要求则不用比对))
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::GetSupportedVideoInfoFromSDP(const char * i_strVideoFormatName,unsigned int i_dwVideoTimestampFrequency,unsigned char i_bPacketizationMode,unsigned int i_dwProfileLevelId,T_VideoInfo *o_ptVideoInfo)
{
    int i=0;
    int iRet = -1;
    unsigned char bLocalProfile =(unsigned char)((i_dwProfileLevelId>>16)&0xff);//0x64 high ,0x4d main,0x42 base
    unsigned char bLocalConstraintFlag =(unsigned char)((i_dwProfileLevelId>>8)&0xff);//
    unsigned char bLocalLevel =(unsigned char)((i_dwProfileLevelId)&0xff);//
    unsigned char bRemoteProfile =0;//0x64 high ,0x4d main,0x42 base
    unsigned char bRemoteConstraintFlag =0;//
    unsigned char bRemoteLevel =0;//
    unsigned char bNoProfileLevelIdCnt =0;//

    if(NULL == o_ptVideoInfo||NULL == i_strVideoFormatName)
    {
        WEBRTC_LOGE2(m_iLogID,"GetSupportedVideoInfoFromSDP NULL");
        return -1;
    }
    for(i=0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
    {
        if(0 == m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId)
            bNoProfileLevelIdCnt++;
        if(0 != strcmp(WEBRTC_H265_ENCODE_FORMAT_NAME,i_strVideoFormatName) && 0 != i_dwProfileLevelId)//对讲则无需判断
        {//265不做判断，因为sdp中没有dwProfileLevelId
            bRemoteProfile =(unsigned char)((m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId>>16)&0xff);//0x64 high ,0x4d main,0x42 base
            bRemoteConstraintFlag =(unsigned char)((m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId>>8)&0xff);//0x64 high ,0x4d main,0x42 base
            bRemoteLevel =(unsigned char)((m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId)&0xff);//
            //WEBRTC_LOGD2(m_iLogID,"i_dwProfileLevelId %#x,%#x,%#x,%#x,%#x,%#x,\r\n",bLocalProfile,bLocalConstraintFlag,bLocalLevel,bRemoteProfile,bRemoteConstraintFlag,bRemoteLevel);
            if(bLocalProfile!=bRemoteProfile && string::npos == m_strReqURL.find("comms_sdk_version=14"))
            {//"comms_sdk_version=14" 谷歌音箱只有baseline，但可以播放mainprofile所以不过滤，
                continue;
            }
            if(bLocalLevel>bRemoteLevel)
            {
                WEBRTC_LOGW2(m_iLogID,"bLocalLevel>bRemoteLevel %#x,%#x,%#x,%#x,%#x,%#x,\r\n",bLocalProfile,bLocalConstraintFlag,bLocalLevel,bRemoteProfile,bRemoteConstraintFlag,bRemoteLevel);
                //continue;//增强兼容性，暂时注释，可能会影响ios的播放
            }
            if(0x64 == bLocalProfile)
            {//严格处理ConstraintFlag也要符合(兼容ios，ios highprofile ConstraintFlag有编码有特殊规范，所以也要符合)
                if(0 != bRemoteConstraintFlag && (bLocalConstraintFlag&bRemoteConstraintFlag)!=bRemoteConstraintFlag)
                {//浏览器置位代表有要求，则服务端对应的位置也要置位
                    continue;
                }
            }
        }

        if(0 == strcmp(i_strVideoFormatName,m_tWebRtcSdpMediaInfo.tVideoInfos[i].strFormatName) && i_dwVideoTimestampFrequency==m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwTimestampFrequency &&
        (0 == strcmp(WEBRTC_H265_ENCODE_FORMAT_NAME,i_strVideoFormatName)||i_bPacketizationMode==m_tWebRtcSdpMediaInfo.tVideoInfos[i].bPacketizationMode))//bLevelAsymmetryAllowed都是1
        {//265不做判断，因为sdp中没有bPacketizationMode
            iRet = 0;
            memcpy(o_ptVideoInfo,&m_tWebRtcSdpMediaInfo.tVideoInfos[i],sizeof(T_VideoInfo));//i_dwProfileLevelId不一样也可以
            if(i_dwProfileLevelId==m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId)
            {
                memcpy(o_ptVideoInfo,&m_tWebRtcSdpMediaInfo.tVideoInfos[i],sizeof(T_VideoInfo));
                break;
            }
            if(i_dwProfileLevelId==0 && 0x42 == (unsigned char)((m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId>>16)&0xff))
            {//对讲情况下，优先选择baseline
                break;
            }
        }
    }
    if(bNoProfileLevelIdCnt>=WEBRTC_SDP_MEDIA_INFO_MAX_NUM)
    {//兼容所有都不带fmtp的情况
        for(i=0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
        {
            if(0 == strcmp(i_strVideoFormatName,m_tWebRtcSdpMediaInfo.tVideoInfos[i].strFormatName) && i_dwVideoTimestampFrequency==m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwTimestampFrequency)//bLevelAsymmetryAllowed都是1
            {
                iRet = 0;
                WEBRTC_LOGW2(m_iLogID,"fmtp bProfileLevelIdCnt>=WEBRTC_SDP_MEDIA_INFO_MAX_NUM %d,%d,%s,%d\r\n",bNoProfileLevelIdCnt,
                WEBRTC_SDP_MEDIA_INFO_MAX_NUM,i_strVideoFormatName,i_dwVideoTimestampFrequency);
                memcpy(o_ptVideoInfo,&m_tWebRtcSdpMediaInfo.tVideoInfos[i],sizeof(T_VideoInfo));//i_dwProfileLevelId不一样也可以
                break;
            }
        }
    }
    if(i>=WEBRTC_SDP_MEDIA_INFO_MAX_NUM)
    {
        WEBRTC_LOGW2(m_iLogID,"GetSupportedVideoInfoFromSDP warn %x,%d,%s,%d,find mid %s,Type %d,%x,%d,%s,%d\r\n",i_dwProfileLevelId,i_bPacketizationMode,i_strVideoFormatName,i_dwVideoTimestampFrequency,
        o_ptVideoInfo->strMediaID,o_ptVideoInfo->bRtpPayloadType,o_ptVideoInfo->dwProfileLevelId,o_ptVideoInfo->bPacketizationMode,o_ptVideoInfo->strFormatName,o_ptVideoInfo->dwTimestampFrequency);
        for(i=0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
        {
            if(0 != strlen(m_tWebRtcSdpMediaInfo.tVideoInfos[i].strFormatName))
                WEBRTC_LOGW2(m_iLogID,"tVideoInfos %d,err %x,%d,%s,%d\r\n",i,m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId,m_tWebRtcSdpMediaInfo.tVideoInfos[i].bPacketizationMode,m_tWebRtcSdpMediaInfo.tVideoInfos[i].strFormatName,m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwTimestampFrequency);
        }
    }
    return iRet;
}
/*****************************************************************************
-Fuction        : PushDataSave
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::GetSupportedAudioInfoFromSDP(const char * i_strAudioFormatName,unsigned int i_dwAudioTimestampFrequency,T_AudioInfo *o_ptAudioInfo)
{
    int i=0;
    int iRet = -1;
    
    if(NULL == o_ptAudioInfo||NULL == i_strAudioFormatName)
    {
        WEBRTC_LOGE2(m_iLogID,"GetSupportedAudioInfoFromSDP NULL");
        return -1;
    }
    for(i=0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
    {
        if(0 == strcmp(i_strAudioFormatName,m_tWebRtcSdpMediaInfo.tAudioInfos[i].strFormatName) && i_dwAudioTimestampFrequency==m_tWebRtcSdpMediaInfo.tAudioInfos[i].dwTimestampFrequency)
        {
            memcpy(o_ptAudioInfo,&m_tWebRtcSdpMediaInfo.tAudioInfos[i],sizeof(T_AudioInfo));
            iRet = 0;
            break;
        }
    }
    if(i>=WEBRTC_SDP_MEDIA_INFO_MAX_NUM)
    {
        WEBRTC_LOGE2(m_iLogID,"GetSupportedAudioInfoFromSDP err %s,%d\r\n",i_strAudioFormatName,i_dwAudioTimestampFrequency);
        for(i=0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)//如果找不到符合的音频格式，则使用默认的
        {//亚马逊alexa 支持pcma但是sdp中没写，这样可以兼容
            if(0 != strlen(m_tWebRtcSdpMediaInfo.tAudioInfos[i].strFormatName))//找一个不为空的格式，然后使用其有用的值
            {//其他值使用默认的
                if(0 == strcmp(i_strAudioFormatName,WEBRTC_AUDIO_ENCODE_FORMAT_NAME))
                {//使用默认值，也要和传入的格式匹配
                    memcpy(o_ptAudioInfo,&m_tWebRtcSdpMediaInfo.tAudioInfos[i],sizeof(T_AudioInfo));
                    o_ptAudioInfo->dwTimestampFrequency=i_dwAudioTimestampFrequency;
                    snprintf(o_ptAudioInfo->strFormatName,sizeof(o_ptAudioInfo->strFormatName),"%s",i_strAudioFormatName);
                    o_ptAudioInfo->bRtpPayloadType=WEBRTC_RTP_PAYLOAD_G711A;
                    iRet = 0;
                }
                //打印所有格式
                WEBRTC_LOGE2(m_iLogID,"tAudioInfos %d,err %s,%d\r\n",i,m_tWebRtcSdpMediaInfo.tAudioInfos[i].strFormatName,m_tWebRtcSdpMediaInfo.tAudioInfos[i].dwTimestampFrequency);
            }
        }
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : ParseRtpData
-Description    : 
-Input          : 
-Output         : 
-Return         : <0 err,0 need more data,>0 success
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::ParseRtpData(char * i_acDataBuf,int i_iDataLen)
{
    int iRet = -1;
	int iHeaderLen=0,iWriteLen=0;

    
    if(NULL == i_acDataBuf)
    {
        WEBRTC_LOGE2(m_iLogID,"ParseRtpData NULL \r\n");
        return iRet;
    }
    if(NULL == m_pRtpParseInterface)
    {
        WEBRTC_LOGE2(m_iLogID,"ParseRtpData m_pRtpInterface NULL \r\n");
        return iRet;
    }
    //m_tPushFrameInfo.iFrameBufLen=0;//注释掉则ParseRtpPacket会缓存重组出符合的帧长(pcma 80->160) //不注释则没有这个逻辑即不会重组
    iRet =m_pRtpParseInterface->ParseRtpPacket((unsigned char *)i_acDataBuf,i_iDataLen,(void *)&m_tPushFrameInfo);
    if(iRet < 0)
    {
        WEBRTC_LOGE2(m_iLogID,"m_pRtpParseInterface ParseRtpPacket err %d \r\n",i_iDataLen);
        return iRet;
    }
    if(0 == iRet)
    {
        return iRet;//need more data
    }
    //WEBRTC_LOGW2(m_iLogID,"m_pRtpParseInterface.GetFrame iFrameBufLen%d eEncType%d \r\n",m_tPushFrameInfo.iFrameBufLen,m_tPushFrameInfo.eEncType);
    m_tPushFrameInfo.eStreamType = STREAM_TYPE_MUX_STREAM;//需要先GetFrame处理m_tPushFrameInfo iFrameBufLen
    iRet=m_pRtpParseInterface->GetFrame((void *)&m_tPushFrameInfo);//这样iFrameBufLen 才能得到处理，同时也才能让下次可以正确接收数据
    if(iRet < 0)//解析接收到数据,解析后缓存位置pbFrameStartPos iFrameLen
    {
        if(0 != m_tPushFrameInfo.iFrameBufLen)//亚马逊alexa 80分包重组时，返回正确 
            return 0;//need more data
        WEBRTC_LOGE2(m_iLogID,"m_pRtpParseInterface.GetFrame iFrameBufLen%d iFrameLen%d err \r\n",m_tPushFrameInfo.iFrameBufLen,m_tPushFrameInfo.iFrameLen);
        return iRet;
    }
    iRet =HandleRtpTimestamp();
    if(iRet < 0)
    {
        return iRet;
    }
    if(MEDIA_FRAME_TYPE_VIDEO_P_FRAME==m_tPushFrameInfo.eFrameType||MEDIA_FRAME_TYPE_VIDEO_B_FRAME==m_tPushFrameInfo.eFrameType)
    {
        if(m_dwVideoPullTimeStamp -dwLastSendTimeStamp<40)//30fps,33ms间隔，
        {//间隔丢帧，降低帧率，防止转码转不过来
            //return 0;//丢掉p帧，后面的p帧会花屏，无法解码，p帧向前依赖，依赖前一个p帧(或i帧)
        }
        dwLastSendTimeStamp=dwLastVideoTimeStamp;
    }
    
    iWriteLen=m_cMediaHandle.FrameToContainer(&m_tPushFrameInfo, STREAM_TYPE_ENHANCED_FLV_STREAM,m_pbFileBuf,WEBRTC_FRAME_BUF_MAX_LEN, &iHeaderLen);
    if(iWriteLen < 0)
    {
        WEBRTC_LOGE2(m_iLogID,"FrameToContainer err iWriteLen %d\r\n",iRet);
        return iRet;
    }
    if(iWriteLen > 0)
    {
        iRet = fwrite(m_pbFileBuf, 1,iWriteLen, m_pMediaFile);
        if(iRet != iWriteLen)
        {
            WEBRTC_LOGD2(m_iLogID,"fwrite err %d iWriteLen%d\r\n",iRet,iWriteLen);
            return iRet;
        }
    }
    iWriteLen=m_cMediaMP4Handle.FrameToContainer(&m_tPushFrameInfo, STREAM_TYPE_FMP4_STREAM,m_pbFileBuf,WEBRTC_FRAME_BUF_MAX_LEN, &iHeaderLen);
    if(iWriteLen < 0)
    {
        WEBRTC_LOGE2(m_iLogID,"FrameToContainerFMP4 err iWriteLen %d\r\n",iRet);
        return iRet;
    }
    if(iWriteLen > 0)
    {
        iRet = fwrite(m_pbFileBuf, 1,iWriteLen, m_pMediaFileMP4);
        if(iRet != iWriteLen)
        {
            WEBRTC_LOGD2(m_iLogID,"fwriteFMP4 err %d iWriteLen%d\r\n",iRet,iWriteLen);
            return iRet;
        }
    }

	WEBRTC_LOGD2(m_iLogID,"RecvData %p,iFrameLen %d \r\n",m_tPushFrameInfo.pbFrameStartPos,m_tPushFrameInfo.iFrameLen);//iFrameLen指向裸流数据长度，可保存为文件
    return m_tPushFrameInfo.iFrameLen;
}
/*****************************************************************************
-Fuction        : ParseRtcpData
-Description    : 
-Input          : 
-Output         : 
-Return         : <0 err,0 need more data,>0 success
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::ParseRtcpData(unsigned char * i_abDataBuf,int i_iDataLen)
{
    int iRet = -1;

    
    if(NULL == i_abDataBuf ||i_iDataLen<2)
    {
        WEBRTC_LOGE2(m_iLogID,"ParseRtcpData NULL \r\n");
        return iRet;
    }

    switch(i_abDataBuf[1])
    {
        case 200:
        {
            WEBRTC_LOGD2(m_iLogID,"recv Rtcp SR ,len %d\r\n",i_iDataLen);
            break;
        }
        case 201:
        {
            WEBRTC_LOGD2(m_iLogID,"recv Rtcp RR ,len %d\r\n",i_iDataLen);
            break;
        }
        case 202:
        {
            WEBRTC_LOGD2(m_iLogID,"recv Rtcp SDES ,len %d\r\n",i_iDataLen);
            break;
        }
        case 203:
        {
            WEBRTC_LOGD2(m_iLogID,"recv Rtcp BYE ,len %d\r\n",i_iDataLen);
            break;
        }
        case 204:
        {
            WEBRTC_LOGD2(m_iLogID,"recv Rtcp APP ,len %d\r\n",i_iDataLen);
            break;
        }
        case 205:
        {
            WEBRTC_LOGD2(m_iLogID,"recv Rtcp RTPFB fmt%d,len%d\r\n",i_abDataBuf[0]&0x1f,i_iDataLen);
            break;
        }
        case 206:
        {
            WEBRTC_LOGD2(m_iLogID,"recv Rtcp PSFB ,len %d\r\n",i_iDataLen);
            break;
        }
        default:
        {
            WEBRTC_LOGD2(m_iLogID,"ParseRtcpData type %d,Len %d \r\n",i_abDataBuf[1],i_iDataLen);
            break;
        }
    }

    //m_iLastRecvRtcpTime = OS::GetSecondCount();//可做心跳机制，20s没收到rtcp则认为超时断开
    return 0;
}

/*****************************************************************************
-Fuction        : HandleRtpTimestamp
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::HandleRtpTimestamp()
{
    int iRet = -1;
    unsigned int dwRtpTimeStamp=0;//ms

    
    if(0 != m_iTalkTestFlag && string::npos != m_pFileName->find("testTalk/") && 0 == m_iFindedKeyFrame)
    {//普通对讲不需要音视频同源
        if(0 != m_iPullVideoFrameRate)
        {
            dwLastVideoTimeStamp=0;///(WEBRTC_H264_TIMESTAMP_FREQUENCY/1000);
            dwLastSendTimeStamp=dwLastVideoTimeStamp;
            m_dwVideoPullTimeStamp=0;//音视频同源
            m_iFindedKeyFrame=1;
        }
        else
        {
            dwLastVideoTimeStamp=dwRtpTimeStamp/(WEBRTC_H264_TIMESTAMP_FREQUENCY/1000);//dwRtpTimeStamp;
            dwLastSendTimeStamp=dwLastVideoTimeStamp;
            m_dwVideoPullTimeStamp=0;//音视频同源
            m_iFindedKeyFrame=1;
        }
    }
    
    if(0 != m_iPullVideoFrameRate)
    {
        //按照固定帧率的方式计算时间戳
        if(MEDIA_FRAME_TYPE_VIDEO_I_FRAME!=m_tPushFrameInfo.eFrameType)
        {
            if(0 == m_iFindedKeyFrame)
            {
                WEBRTC_LOGD2(m_iLogID,"no MEDIA_FRAME_TYPE_VIDEO_I_FRAME FrameType%d\r\n",m_tPushFrameInfo.eFrameType);
                return iRet;
            }
        }
        else
        {
            if(0 == m_iFindedKeyFrame)
            {
                dwLastVideoTimeStamp=0;///(WEBRTC_H264_TIMESTAMP_FREQUENCY/1000);
                dwLastSendTimeStamp=dwLastVideoTimeStamp;
                m_dwVideoPullTimeStamp=0;//音视频同源
                m_iFindedKeyFrame=1;
            }
        }
        switch(m_tPushFrameInfo.eFrameType)
        {
            case MEDIA_FRAME_TYPE_AUDIO_FRAME:
            {
                if(0 == dwLastAudioTimeStamp)
                {
                    dwLastAudioTimeStamp=dwLastVideoTimeStamp;///(WEBRTC_G711A_TIMESTAMP_FREQUENCY/1000);
                    m_dwAudioPullTimeStamp=m_dwVideoPullTimeStamp;//音视频同源
                }
                m_dwAudioPullTimeStamp=dwLastAudioTimeStamp;//(dwRtpTimeStamp/(WEBRTC_G711A_TIMESTAMP_FREQUENCY/1000))-dwLastAudioTimeStamp;//20;//
                dwLastAudioTimeStamp+=20;///(WEBRTC_G711A_TIMESTAMP_FREQUENCY/1000);
                m_tPushFrameInfo.tAudioEncodeParam.dwChannels=1;//必须赋值，否则mp4无法播放
                m_tPushFrameInfo.dwSampleRate=WEBRTC_G711A_TIMESTAMP_FREQUENCY;//
                WEBRTC_LOGD2(m_iLogID,"AUDIO->dwTimeStamp%d,dwLastAudioTimeStamp%d FrameType%d\r\n",
                m_dwAudioPullTimeStamp,dwLastAudioTimeStamp,m_tPushFrameInfo.eFrameType,m_tPushFrameInfo);
                m_tPushFrameInfo.dwTimeStamp=m_dwAudioPullTimeStamp;
                break;
            }
            case MEDIA_FRAME_TYPE_VIDEO_I_FRAME: //sps
            case MEDIA_FRAME_TYPE_VIDEO_P_FRAME://pps
            {
                m_dwVideoPullTimeStamp=dwLastVideoTimeStamp;//(dwRtpTimeStamp/(WEBRTC_H264_TIMESTAMP_FREQUENCY/1000))-dwLastVideoTimeStamp;//33;//
                dwLastVideoTimeStamp+=(1000/m_iPullVideoFrameRate);///(WEBRTC_H264_TIMESTAMP_FREQUENCY/1000);
                m_tPushFrameInfo.dwSampleRate=WEBRTC_H264_TIMESTAMP_FREQUENCY;//
                WEBRTC_LOGD2(m_iLogID,"VIDEO->dwTimeStamp%d,dwLastVideoTimeStamp%d FrameType%d,dwWidth%d dwHeight%d\r\n",
                m_dwVideoPullTimeStamp,dwLastVideoTimeStamp,m_tPushFrameInfo.eFrameType,m_tPushFrameInfo.dwWidth,m_tPushFrameInfo.dwHeight);
                m_tPushFrameInfo.dwTimeStamp=m_dwVideoPullTimeStamp;
                break;
            }
            default:
            {
                WEBRTC_LOGE2(m_iLogID,"m_tPushFrameInfo.eFrameType %d\r\n",m_tPushFrameInfo.eFrameType);
                break;
            }
        }
        return 0;
    }

    //采用当前时间,由于包到达的时间不准，所以不合适
    dwRtpTimeStamp = m_tPushFrameInfo.dwTimeStamp;//GetTickCount();//
    if(MEDIA_FRAME_TYPE_VIDEO_I_FRAME!=m_tPushFrameInfo.eFrameType)
    {
        if(0 == m_iFindedKeyFrame)
        {
            return iRet;
        }
    }
    else
    {
        if(0 == m_iFindedKeyFrame)
        {
            dwLastVideoTimeStamp=dwRtpTimeStamp/(WEBRTC_H264_TIMESTAMP_FREQUENCY/1000);//dwRtpTimeStamp;
            dwLastSendTimeStamp=dwLastVideoTimeStamp;
            m_dwVideoPullTimeStamp=0;//音视频同源
            m_iFindedKeyFrame=1;
        }
    }
    switch(m_tPushFrameInfo.eFrameType)
    {
        case MEDIA_FRAME_TYPE_AUDIO_FRAME:
        {
            if(0 == dwLastAudioTimeStamp)
            {
                dwLastAudioTimeStamp=dwRtpTimeStamp/(WEBRTC_G711A_TIMESTAMP_FREQUENCY/1000);//dwRtpTimeStamp;
                m_dwAudioPullTimeStamp=m_dwVideoPullTimeStamp;//音视频同源
            }
            m_dwAudioPullTimeStamp+=(dwRtpTimeStamp/(WEBRTC_G711A_TIMESTAMP_FREQUENCY/1000))-dwLastAudioTimeStamp;//20;//dwRtpTimeStamp-dwLastAudioTimeStamp;//
            dwLastAudioTimeStamp=dwRtpTimeStamp/(WEBRTC_G711A_TIMESTAMP_FREQUENCY/1000);//dwRtpTimeStamp;
            m_tPushFrameInfo.tAudioEncodeParam.dwChannels=1;//必须赋值，否则mp4无法播放
            m_tPushFrameInfo.dwSampleRate=WEBRTC_G711A_TIMESTAMP_FREQUENCY;//
            WEBRTC_LOGD2(m_iLogID,"AUDIO->dwTimeStamp%d,dwLastAudioTimeStamp%d FrameType%d\r\n",
            m_dwAudioPullTimeStamp,dwLastAudioTimeStamp,m_tPushFrameInfo.eFrameType);
            m_tPushFrameInfo.dwTimeStamp=m_dwAudioPullTimeStamp;
            break;
        }
        case MEDIA_FRAME_TYPE_VIDEO_I_FRAME: //sps
        case MEDIA_FRAME_TYPE_VIDEO_P_FRAME://pps
        {
            m_dwVideoPullTimeStamp+=(dwRtpTimeStamp/(WEBRTC_H264_TIMESTAMP_FREQUENCY/1000))-dwLastVideoTimeStamp;//33;//dwRtpTimeStamp-dwLastVideoTimeStamp;//
            dwLastVideoTimeStamp=dwRtpTimeStamp/(WEBRTC_H264_TIMESTAMP_FREQUENCY/1000);//dwRtpTimeStamp;
            m_tPushFrameInfo.dwSampleRate=WEBRTC_H264_TIMESTAMP_FREQUENCY;//
            WEBRTC_LOGD2(m_iLogID,"VIDEO->dwTimeStamp%d,dwLastVideoTimeStamp%d FrameType%d,dwWidth%d dwHeight%d\r\n",
            m_dwVideoPullTimeStamp,dwLastVideoTimeStamp,m_tPushFrameInfo.eFrameType,m_tPushFrameInfo.dwWidth,m_tPushFrameInfo.dwHeight);
            m_tPushFrameInfo.dwTimeStamp=m_dwVideoPullTimeStamp;
            break;
        }
        default:
        {
            WEBRTC_LOGE2(m_iLogID,"m_tPushFrameInfo.eFrameType %d\r\n",m_tPushFrameInfo.eFrameType);
            break;
        }
    }
    return 0;
}

/*****************************************************************************
-Fuction        : StopSession
-Description    : 多线程访问
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::StopSession(int i_iError)
{
    int iRet = -1;
    
    WEBRTC_LOGW2(m_iLogID,"WebRtcSession StopSession %d,%s \r\n",i_iError,m_strReqURL.c_str());
    if(NULL!= m_pMediaFileMP4)
    {
        int iWriteLen,iHeaderLen;
        iWriteLen=m_cMediaMP4Handle.FrameToContainer(NULL, STREAM_TYPE_FMP4_STREAM,m_pbFileBuf,WEBRTC_FRAME_BUF_MAX_LEN, &iHeaderLen);
        if(iWriteLen > 0)
        {
            iRet = fwrite(m_pbFileBuf, 1,iWriteLen, m_pMediaFileMP4);
        }
    }
    
    if(NULL != m_pWebRtcProc)
    {
        WEBRTC_LOGW2(m_iLogID,"WebRtcSession m_pWebRtcProc %d\r\n",i_iError);
        m_pWebRTC->StopProc();
    }
    if(NULL!= m_pFileProc)
    {
        m_iFileProcFlag = 0;
    }
    iRet = SendErrorCodeAndExit(i_iError);
    if(iRet < 0)
    {////
        WEBRTC_LOGE2(m_iLogID,"SendErrorCodeAndExit NULL DeleteSelf\r\n");
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : SendErrorCode
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::SendErrorCodeAndExit(int i_iErrorCode) 
{
    const char *strErrCode = NULL;

    if(NULL == m_tWebRtcSessionCb.SendErrorCodeAndExit || NULL == m_tWebRtcSessionCb.pObj)
    {
        WEBRTC_LOGE2(m_iLogID,"SendErrorCodeAndExit NULL Delete[%d]", i_iErrorCode);
        return -1;
    }
    
	return m_tWebRtcSessionCb.SendErrorCodeAndExit(this,i_iErrorCode,m_tWebRtcSessionCb.pObj);
}
/*****************************************************************************
-Fuction        : SendErrorCode
-Description    : // 返回自系统开机以来的毫秒数
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
unsigned int WebRtcSession::GetTickCount()	// milliseconds
{
#ifdef _WIN32
	return ::GetTickCount64() & 0xFFFFFFFF;
#else  
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	uint64_t cnt = ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
	return cnt & 0xFFFFFFFF;
#endif
}
/*****************************************************************************
-Fuction        : IsRtp
-Description    : 
-Input          : 
-Output         : 
-Return         : 0 否，1是
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::IsRtp(char * i_acDataBuf,int i_iDataLen)
{
    if(NULL != m_pRtpInterface)
    {
        return m_pRtpInterface->IsRtp(i_acDataBuf,i_iDataLen);
    }
    return 0;
}
/*****************************************************************************
-Fuction        : RecvRtpData
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::RecvRtpData(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle)
{
    int iRet = -1;
    
    if(NULL == i_pIoHandle ||NULL == i_acDataBuf)
    {
        WEBRTC_LOGE("PushRtpData NULL \r\n");
    }
	//WEBRTC_LOGD("RecvRtpData %d \r\n",i_iDataLen);
    WebRtcSession *pWebRtcSession = (WebRtcSession *)i_pIoHandle;
    
    return pWebRtcSession->ParseRtpData(i_acDataBuf,i_iDataLen);
}
/*****************************************************************************
-Fuction        : RecvRtcpData
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::RecvRtcpData(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle)
{
    int iRet = -1;
    
    if(NULL == i_pIoHandle ||NULL == i_acDataBuf)
    {
        WEBRTC_LOGE("PushRtcpData NULL \r\n");
    }
	//WEBRTC_LOGD("RecvRtcpData %d \r\n",i_iDataLen);
    WebRtcSession *pWebRtcSession = (WebRtcSession *)i_pIoHandle;
    
    return pWebRtcSession->ParseRtcpData((unsigned char *)i_acDataBuf,i_iDataLen);
}

/*****************************************************************************
-Fuction        : SendClientStopMsg
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::RecvClientStopMsg(void *i_pIoHandle)
{
    int iRet = -1;
    
    if(NULL == i_pIoHandle)
    {
        WEBRTC_LOGE("PushRtpData NULL \r\n");
    }
	WEBRTC_LOGD("RecvClientStopMsg %d \r\n",iRet);
    WebRtcSession *pWebRtcSession = (WebRtcSession *)i_pIoHandle;
    return pWebRtcSession->StopSession(0);
}

/*****************************************************************************
-Fuction        : StartPlay
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::IsRtpCb(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle)
{
    int iRet = -1;
    WebRtcSession *pWebRtcSession = NULL;

    if(NULL != i_pIoHandle)
    {
        pWebRtcSession = (WebRtcSession *)i_pIoHandle;
        return pWebRtcSession->IsRtp(i_acDataBuf,i_iDataLen);
    }
    return 0;
}


