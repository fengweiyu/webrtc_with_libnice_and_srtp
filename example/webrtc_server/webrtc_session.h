/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcSession.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_SESSION_H
#define WEBRTC_SESSION_H

#include <thread>
#include "webrtc_interface.h"
#include "MediaHandle.h"
#include "rtp_interface.h"

using std::thread;

typedef struct WebRtcSessionCb
{
    int (*SendDataOut)(const char * i_strData,void *i_pIoHandle);
    int (*SendErrorCodeAndExit)(void *i_pSrcIoHandle,int i_iErrorCode,void *i_pIoHandle);
    void *pObj;//
}T_WebRtcSessionCb;

/*****************************************************************************
-Class			: WebRtcSession
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class WebRtcSession
{
public:
	WebRtcSession(char * i_strStunAddr,unsigned int i_dwStunPort,T_WebRtcSessionCb i_tWebRtcSessionCb,int i_iID);
	virtual ~WebRtcSession();
    int SetReqData(const char *i_strReqURL,const char *i_strReqBody);
    int ParseRtpData(char * i_acDataBuf,int i_iDataLen);
    int ParseRtcpData(unsigned char * i_abDataBuf,int i_iDataLen);
    int StopSession(int i_iError);
    
	string * m_pFileName;
private:
    static int RecvRtpData(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
    static int RecvRtcpData(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
    static int RecvClientStopMsg(void *i_pIoHandle);
    static int IsRtpCb(char * i_acDataBuf,int i_iDataLen,void *i_pIoHandle);
    
    int TestURL(const char * url);
    int HandleRequest(const char * strURL);
    void TestProc();
    int SendErrorCodeAndExit(int i_iErrorCode) ;
    int HandleRemoteSDP();
    int HandleMediaFrame(T_MediaFrameInfo * i_ptFrameInfo) ;
    int SendDatas(T_MediaFrameInfo * i_ptFrameInfo);
    int SendLocalSDP(T_MediaFrameInfo * i_ptFrameInfo);
    int GetSupportedVideoInfoFromSDP(const char * i_strVideoFormatName,unsigned int i_dwVideoTimestampFrequency,unsigned char i_bPacketizationMode,unsigned int i_dwProfileLevelId,T_VideoInfo *o_ptVideoInfo);
    int GetSupportedAudioInfoFromSDP(const char * i_strAudioFormatName,unsigned int i_dwAudioTimestampFrequency,T_AudioInfo *o_ptAudioInfo);
    unsigned int GetTickCount();  // milliseconds
    int IsRtp(char * i_acDataBuf,int i_iDataLen);
    int HandleRtpTimestamp();
    
        
    T_WebRtcSessionCb m_tWebRtcSessionCb;

    WebRtcInterface * m_pWebRTC;
    RtpInterface * m_pRtpInterface;
    RtpInterface * m_pRtpParseInterface;
    thread *m_pWebRtcProc;
    MediaHandle m_cMediaHandle;

    T_WebRtcSdpMediaInfo m_tWebRtcSdpMediaInfo;
    unsigned char *m_pbPacketsBuf;
    unsigned char **m_ppbPacketBuf;
    int * m_piEveryPacketLen;
    int m_iSendSdpSuccess;//0 success
    int m_iPackNum;//0 success
    string m_strReqBody;
    string m_strReqURL;
    T_MediaFrameInfo m_tPushFrameInfo;
    
    int m_iLogID;
    T_MediaFrameInfo m_tFileFrameInfo;
    thread *m_pFileProc;
    int m_iFileProcFlag;
    int m_iFileExitProcFlag;
    int m_iTalkTestFlag;//0 否，1是
    
    int m_iPullVideoFrameRate;
    unsigned int m_dwVideoPullTimeStamp;//ms
    unsigned int m_dwAudioPullTimeStamp;//ms
    int m_iFindedKeyFrame;//0 否，1是
    unsigned int dwLastAudioTimeStamp;//ms
    unsigned int dwLastVideoTimeStamp;//ms
    unsigned int dwLastSendTimeStamp;//ms
    unsigned char * m_pbFileBuf;
    FILE  *m_pMediaFile;
};

#endif
