/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcSession.c
* Description           : 	    webrtcý��ͨ���������ʱ���Ż���600-700ms
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

#define WEBRTC_VIDEO_ENCODE_FORMAT_NAME "H264"
#define WEBRTC_H264_TIMESTAMP_FREQUENCY 90000
#define WEBRTC_AUDIO_ENCODE_FORMAT_NAME "PCMA"
#define WEBRTC_G711A_TIMESTAMP_FREQUENCY 8000
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
    tWebRtcCb.RecvStopMsg = WebRtcSession::RecvClientStopMsg;
    tWebRtcCb.IsRtp = WebRtcSession::IsRtpCb;
    tWebRtcCb.IsRtcp = NULL;
    //stun:stun.oss.aliyuncs.com:3478
    memset(&tWebRtcCfg,0,sizeof(T_WebRtcCfg));
    snprintf(tWebRtcCfg.strStunAddr,sizeof(tWebRtcCfg.strStunAddr),"%s",i_strStunAddr);//"77.72.169.210" ����ʹ���������ڲ��ⲻ�ܽ���
    tWebRtcCfg.dwStunPort = i_dwStunPort;//3478;//stun.voipbuster.com:3478
    tWebRtcCfg.eWebRtcRole= WEBRTC_ANSWER_ROLE;
    tWebRtcCfg.eControlling= ICE_CONTROLLED_ROLE;

    m_pWebRTC = new WebRtcInterface(tWebRtcCfg,tWebRtcCb,this);
    if(NULL == m_pWebRTC)
    {
        WEBRTC_LOGE("WebRtcMediaSession NULL \r\n");
    }
    m_pWebRtcProc = new thread(&WebRtcInterface::Proc, m_pWebRTC);
    m_pWebRtcProc->detach();//ע���̻߳���


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
    m_pFileProc = NULL;
    m_iFileProcFlag = 0;
    m_iFileExitProcFlag = 0;
    m_pFileName = NULL;
    m_iTalkTestFlag = 0;
    
    m_iLogID = i_iID;
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
    if(NULL!= m_pRtpInterface)
    {
        delete m_pRtpInterface;
        m_pRtpInterface = NULL;//
    }
    if(NULL != m_pWebRtcProc)
    {
        m_pWebRTC->StopProc();
        while(0 == m_pWebRTC->GetStopedFlag()){usleep(10);};//ʹ������ȴ��˳�delete m_pWebRtcProc�ᱨ���߳�δ�˳�����
        //m_pWebRtcProc->join();//��join()�ȴ��˳�,�����ǰ��һ��ʹ�������һֱ����(������Ϊ�߳����˳�)
        //delete m_pWebRtcProc;//��ʱ��m_pWebRtcProc->detach();�����Ż�
    }
    if(NULL!= m_pFileProc)
    {
        m_iFileProcFlag = 0;//m_pWebRTC->StopProc();
        while(0 == m_iFileExitProcFlag){usleep(10);};
        //m_pFileProc->join();//��join()�ȴ��˳������һֱ����
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
    //ִֻ��һ��
    m_pWebRTC->DtlsInit();//dtls��ʼ����ʱ300ms
    
    //iRet = m_pWebRTC->GetGatheringDoneFlag();//�����ռ���ַ(��ʱ300ms)���д���
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
    //auto dwTestAvTalkPos = strURL.find("videoCall/");
    if(string::npos != dwTestTalkPos)// ||string::npos != dwTestAvTalkPos)
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
    //iRet = this->HandleRemoteSDP();//���������õ��ڲ����ڲ������˲�������sdp��
    //if(iRet < 0)
    {
        //WEBRTC_LOGD2(m_iLogID,"HandleRemoteSDP err [%d]%s\r\n", iRet,strURL);
        //return iRet;
    }
    if(NULL != m_pFileName)
    {
        m_pRtpInterface = new RtpInterface((char *)m_pFileName->c_str());//m_pFileName->c_str());
        m_pFileProc = new thread(&WebRtcSession::TestProc, this);
        m_pFileProc->detach();//ע���̻߳���
        WEBRTC_LOGD2(m_iLogID,"HandleRequest m_pFileName [%d]\r\n", m_iTalkTestFlag);
        return 0;
    }
    m_pRtpInterface = new RtpInterface(NULL);
    
    return 0;
}

/*****************************************************************************
-Fuction        : TestProc
-Description    : 
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
            iRet = m_pWebRTC->GetGatheringDoneFlag();//�����ռ���ַ(��ʱ300ms)���д���
            WEBRTC_LOGW2(m_iLogID,"m_pWebRTC->GetGatheringDoneFlag() %d\r\n",iRet);
            if(0 != iRet)
            {
                usleep(30*1000);
                continue;
            }
            iRet = this->HandleRemoteSDP();//���������õ��ڲ����ڲ������˲�������sdp��
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
            WEBRTC_LOGE2(m_iLogID,"TestProc exit %d,%s,dwProfileLevelId %#x\r\n",iRet,m_pFileName->c_str(),dwProfileLevelId);
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
            usleep(50*1000);//�����Ż�
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
        //usleep(20*1000);//����
        dwFileLastTick = GetTickCount();
        
        iRet = this->SendDatas(&m_tFileFrameInfo);
        //WEBRTC_LOGD2(m_iLogID,"%d SendDatas iRet %d dwSleepTime%d\r\n",m_iLastRecvDataTime,iRet,dwSleepTime);
        //m_iLastRecvDataTime = GetTickCount;//�������ƣ�20s�����������Ͽ�
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
                iRet = m_pWebRTC->HandleMsg((char *)ptNode->valuestring,1,&m_tWebRtcSdpMediaInfo);//����,�����Ż�
                ptNode = NULL;
            }
            cJSON_Delete(ptJson);
        }
    }
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
        m_iPackNum = 0;//�õ�һ֡����
    }
    //WEBRTC_LOGD2(_nClientPort," HandleMediaFrame m_iPackNum %d\r\n", m_iPackNum);


    if(0 != m_iSendSdpSuccess && i_ptFrameInfo->tVideoEncodeParam.iSizeOfSPS > 0 && i_ptFrameInfo->tVideoEncodeParam.iSizeOfPPS > 0)
    {
        iRet = this->SendLocalSDP(&i_ptFrameInfo->tVideoEncodeParam);//
        if(iRet < 0)
        {
            WEBRTC_LOGD2(m_iLogID,"HandleMediaFrame SendLocalSDP err [%d]\r\n", iRet);
            return -2;
        }
    }
    else if(0 != m_iSendSdpSuccess && 0 != m_iTalkTestFlag)
    {
        iRet = this->SendLocalSDP(NULL);//�Խ�����Ҫi֡
        if(iRet < 0)
        {
            WEBRTC_LOGD2(m_iLogID,"HandleMediaFrame SendLocalSDP TalkTestFlag err [%d]\r\n", iRet);
            return -2;
        }
    }
    if(0 != m_iSendSdpSuccess)
    {
        WEBRTC_LOGD2(m_iLogID,"no m_iSendSdpSuccess eFrameType [%d]\r\n", i_ptFrameInfo->eFrameType);
        m_iPackNum = -1;//�ȴ�i֡
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
        //return 0;//��һ���Ȳ�����ǿ��i֡����ʵʱ��
    }
    if(m_iPackNum > 0)
    {
        for(i=0;i<m_iPackNum;i++)
        {
            iRet = m_pWebRTC->SendProtectedRtp((char *)m_ppbPacketBuf[i], m_piEveryPacketLen[i]);
            if(iRet < 0)
            {
                WEBRTC_LOGE2(m_iLogID,"m_pWebRTC->SendProtectedRtp err %d %d %d\r\n", iRet,i,i_ptFrameInfo->eFrameType);
                //return 0;//-1; ������ͬ�İ���srtp�ᱨ�طŹ���������
            }
        }
        m_iPackNum = -1;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : WebRtcSession
-Description    : 
-Input          : 
-Output         : 
-Return         : len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::SendLocalSDP(T_VideoEncodeParam * i_ptVideoEncodeParam)
{
    T_WebRtcMediaInfo tMediaInfo;
    char strSPS[128];
    char strPPS[64];
    char *strSDP=NULL;
    int iRet = -1;
    T_RtpMediaInfo tRtpMediaInfo;

    
    if(NULL == m_tWebRtcSessionCb.SendDataOut || NULL == m_tWebRtcSessionCb.pObj)
    {
        WEBRTC_LOGE2(m_iLogID,"SendLocalSDP SendDataOut NULL");
        return -1;
    }

    memset(&tMediaInfo,0,sizeof(T_WebRtcMediaInfo));
    if(NULL == i_ptVideoEncodeParam)
    {
        iRet = GetSupportedVideoInfoFromSDP(WEBRTC_VIDEO_ENCODE_FORMAT_NAME,WEBRTC_H264_TIMESTAMP_FREQUENCY,1,0,&tMediaInfo.tVideoInfo);
    }
    else
    {
        unsigned int dwProfileLevelId = (i_ptVideoEncodeParam->abSPS[1]<<16) | (i_ptVideoEncodeParam->abSPS[2]<<8) | i_ptVideoEncodeParam->abSPS[3];
        iRet = GetSupportedVideoInfoFromSDP(WEBRTC_VIDEO_ENCODE_FORMAT_NAME,WEBRTC_H264_TIMESTAMP_FREQUENCY,1,dwProfileLevelId,&tMediaInfo.tVideoInfo);
        char * strSPS_Base64 = base64Encode((const char*)i_ptVideoEncodeParam->abSPS,i_ptVideoEncodeParam->iSizeOfSPS);
        char * strPPS_Base64 = base64Encode((const char*)i_ptVideoEncodeParam->abPPS,i_ptVideoEncodeParam->iSizeOfPPS);
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
        WEBRTC_LOGE2(m_iLogID,"GetSupportedMediaInfoFromSDP err %s ,%s\r\n",WEBRTC_VIDEO_ENCODE_FORMAT_NAME,WEBRTC_AUDIO_ENCODE_FORMAT_NAME);
        return -1;
    }
    iRet = -1;
    tRtpMediaInfo.iAudioEnc = MEDIA_ENCODE_TYPE_G711A;
    tRtpMediaInfo.iAudioPayload = tMediaInfo.tAudioInfo.bRtpPayloadType;
    tRtpMediaInfo.iVideoEnc= MEDIA_ENCODE_TYPE_H264;
    tRtpMediaInfo.iVideoPayload= tMediaInfo.tVideoInfo.bRtpPayloadType;
    if(NULL!= m_pRtpInterface)
    {
        iRet = m_pRtpInterface->SetRtpTypeInfo((void *)&tRtpMediaInfo);
    }
    if(iRet != 0)
    {
        WEBRTC_LOGE2(m_iLogID,"m_pRtpInterface->SetRtpTypeInfo err %d %d,%d %d\r\n",tRtpMediaInfo.iAudioEnc,tRtpMediaInfo.iVideoEnc,tRtpMediaInfo.iAudioPayload,tRtpMediaInfo.iVideoPayload);
        return -1;
    }
    if(NULL!= m_pRtpInterface)//������rtp�������ܻ�ȡssrc
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
        tMediaInfo.tVideoInfo.dwSSRC = 12212321;//����������������
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

    //iRet = this->HandleRemoteSDP();//���������õ��ڲ����ڲ������˲�������sdp��
    return iRet;
}
/*****************************************************************************
-Fuction        : GetSupportedVideoInfoFromSDP
-Description    : 
Webrtc ����High profile��spsҪһ��(����ios��levelҪ��Ҫ��ĵͣ�
�ڶ��ֽ���00���־����0������Ҫ�����ñȶԣ�
�ȸ������64001f �м���00Ҳ����������Ҫ��)��
����main��base��Ҫ��һ�ֽ�һ�£����ҵ����ֽڼ�levelҪ��Ҫ��ĵ�
(�����������1f,�����˵���Ҫ<=1f������level�ȼ��������ʸߣ���������ܲ��ˣ�
(����ϸ�һ��Ļ�����ڶ��ֽ�Ҳ���Ǳ���淶��־��2λҲҪ���ϣ�
���������Ҫ��e0�����������Ҫc0,���������ڶ��ֽ���00���־����0
������Ҫ�����ñȶ�))
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

    if(NULL == o_ptVideoInfo||NULL == i_strVideoFormatName)
    {
        WEBRTC_LOGE2(m_iLogID,"GetSupportedVideoInfoFromSDP NULL");
        return -1;
    }
    for(i=0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
    {
        if(0 != i_dwProfileLevelId)//�Խ��������ж�
        {
            bRemoteProfile =(unsigned char)((m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId>>16)&0xff);//0x64 high ,0x4d main,0x42 base
            bRemoteConstraintFlag =(unsigned char)((m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId>>8)&0xff);//0x64 high ,0x4d main,0x42 base
            bRemoteLevel =(unsigned char)((m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId)&0xff);//
            //WEBRTC_LOGD2(m_iLogID,"i_dwProfileLevelId %#x,%#x,%#x,%#x,%#x,%#x,\r\n",bLocalProfile,bLocalConstraintFlag,bLocalLevel,bRemoteProfile,bRemoteConstraintFlag,bRemoteLevel);
            if(bLocalProfile!=bRemoteProfile ||bLocalLevel>bRemoteLevel)
            {
                continue;
            }
            if(0x64 == bLocalProfile)
            {//�ϸ���ConstraintFlagҲҪ����(����ios��ios highprofile ConstraintFlag�б���������淶������ҲҪ����)
                if(0 != bRemoteConstraintFlag && (bLocalConstraintFlag&bRemoteConstraintFlag)!=bRemoteConstraintFlag)
                {//�������λ������Ҫ�������˶�Ӧ��λ��ҲҪ��λ
                    continue;
                }
            }
        }

        if(0 == strcmp(i_strVideoFormatName,m_tWebRtcSdpMediaInfo.tVideoInfos[i].strFormatName) && i_dwVideoTimestampFrequency==m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwTimestampFrequency &&
        i_bPacketizationMode==m_tWebRtcSdpMediaInfo.tVideoInfos[i].bPacketizationMode)//bLevelAsymmetryAllowed����1
        {
            iRet = 0;
            memcpy(o_ptVideoInfo,&m_tWebRtcSdpMediaInfo.tVideoInfos[i],sizeof(T_VideoInfo));//i_dwProfileLevelId��һ��Ҳ����
            if(i_dwProfileLevelId==m_tWebRtcSdpMediaInfo.tVideoInfos[i].dwProfileLevelId)
            {
                memcpy(o_ptVideoInfo,&m_tWebRtcSdpMediaInfo.tVideoInfos[i],sizeof(T_VideoInfo));
                break;
            }
        }
    }
    if(i>=WEBRTC_SDP_MEDIA_INFO_MAX_NUM)
    {
        WEBRTC_LOGW2(m_iLogID,"GetSupportedVideoInfoFromSDP err %x,%d,%s,%d\r\n",i_dwProfileLevelId,i_bPacketizationMode,i_strVideoFormatName,i_dwVideoTimestampFrequency);
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
        for(i=0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
        {
            if(0 != strlen(m_tWebRtcSdpMediaInfo.tAudioInfos[i].strFormatName))
                WEBRTC_LOGE2(m_iLogID,"tAudioInfos %d,err %s,%d\r\n",i,m_tWebRtcSdpMediaInfo.tAudioInfos[i].strFormatName,m_tWebRtcSdpMediaInfo.tAudioInfos[i].dwTimestampFrequency);
        }
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : PushRtpData
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int WebRtcSession::ParseRtpData(char * i_acDataBuf,int i_iDataLen)
{
    int iRet = -1;
    
    if(NULL == i_acDataBuf)
    {
        WEBRTC_LOGE("ParseRtpData NULL \r\n");
        return iRet;
    }
    if(NULL == m_pRtpInterface)
    {
        WEBRTC_LOGE("ParseRtpData m_pRtpInterface NULL \r\n");
        return iRet;
    }
    iRet =m_pRtpInterface->ParseRtpPacket((unsigned char *)i_acDataBuf,i_iDataLen,(void *)&m_tPushFrameInfo);
    if(iRet < 0)
    {
        WEBRTC_LOGD("RecvData ParseRtpPacket err %d \r\n",m_tPushFrameInfo.iFrameLen);
        return iRet;
    }
	WEBRTC_LOGD("RecvData %p,iFrameLen %d \r\n",m_tPushFrameInfo.pbFrameStartPos,m_tPushFrameInfo.iFrameLen);//iFrameLenָ���������ݳ��ȣ��ɱ���Ϊ�ļ�
    return iRet;
}

/*****************************************************************************
-Fuction        : StopSession
-Description    : ���̷߳���
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
-Description    : // ������ϵͳ���������ĺ�����
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
-Return         : 0 ��1��
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
-Fuction        : PushRtpData
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


