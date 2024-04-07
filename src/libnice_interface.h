/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Libnice_interface.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef LIBNICE_INTERFACE_H
#define LIBNICE_INTERFACE_H


#include "webrtc_common.h"

#include <string>
#include "ctype.h"
#include "agent.h"

#include "gio/gnetworking.h"

using std::string;

#define MAX_CANDIDATE_NUM 20



typedef struct StreamInfo
{
	char strName[16];
	int iID;//通道id(指示哪一路通道，如视频通道，音频通道，数据通道)
	int iNum;//(视音频)流的路数，如表示视频流有几路，一般只有1路视频流
}T_StreamInfo;

typedef struct LocalCandidate
{
	int iGatheringDoneFlag;//0 未收集,1收集成功
    char strUfrag[16];
    char strPassword[64];
    char strIP[MAX_CANDIDATE_NUM][32];
	char strCandidateData[MAX_CANDIDATE_NUM][256];
	int iCurCandidateNum;
//"candidate:3442447574 1 udp 2122260223 192.168.0.170 54653 typ host generation 0 ufrag gX6M network-id 1"
}T_LocalCandidate;
typedef struct LibniceDepData
{
    char strStunAddr[24];
    unsigned int dwStunPort;
    E_IceControlRole eControlling;
    int iAvMultiplex;//0 表示音视频复用一个通道(使用视频通道,只建立一个通道),1表示音视频各用一个通道
}T_LibniceDepData;

typedef struct LibniceCb
{
    void ( *Handshake)(void * pArg);
	void ( *HandleRecvData)(char * i_acData,int i_iLen,void * pDtlsArg,void * pWebRtcArg);
    void *pVideoDtlsObjCb;
    void *pAudioDtlsObjCb;
    void *pWebRtcObjCb;
}T_LibniceCb;

/*****************************************************************************
-Class          : Libnice
-Description    : Libnice
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class Libnice
{
public:
	Libnice(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling,int i_iAvMultiplex=0);//0 表示音视频复用一个通道(使用视频通道,只建立一个通道),1表示音视频各用一个通道
	~Libnice();//i_iAvMultiplex=0需要SDP设置a=group:BUNDLE  
    int SetCallback(T_LibniceCb *i_ptLibniceCb);
    int LibniceProc();
    int StopProc();
    int GetLocalCandidate(T_LocalCandidate * o_ptLocalVideoCandidate,T_LocalCandidate * o_ptLocalAudioCandidate=NULL);
    int GetLocalSDP(char * i_strSDP,int i_iSdpLen);
    int SaveRemoteSDP(char * i_strSDP);
    int SetRemoteCandidateAndSDP(char * i_strCandidate);
    
    int GetSendReadyFlag(int * o_piVideoReadyFlag,int * o_piAudioReadyFlag);
    int SetSendReadyFlag(int i_iStreamID,int i_iSendReadyFlag);
    int SendVideoData(char * i_acBuf,int i_iBufLen);
    int SendAudioData(char * i_acBuf,int i_iBufLen);
    static void RecvVideoData(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data);
    static void RecvAudioData(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data);

    
    int SetRemoteCredentials(char * i_strUfrag,char * i_strPasswd,int i_iStreamID);
    int SetRemoteCandidateToGlist(char * i_strCandidate,int i_iStreamID);
    int SetRemoteCandidates(int i_iStreamID,int i_iStreamNum);
    int SetRemoteSDP(char * i_strSDP);
    
	static const char *m_astrCandidateTypeName[];
    T_LibniceCb m_tLibniceCb;//回调需使用
    T_LocalCandidate m_tLocalVideoCandidate;//m_iLibniceSendReadyFlag被ComponentStateChanged调用
    T_LocalCandidate m_tLocalAudioCandidate;//后续这些音视频相关的信息都优化放到T_StreamInfo中
    T_StreamInfo m_tVideoStream;
    T_StreamInfo m_tAudioStream;
private:
    int SetRemoteSdpToStream(char * i_strCandidate,int i_iStreamID,int i_iStreamNum,const char *i_strSDP);

    static void CandidateGatheringDone(NiceAgent *i_ptAgent, guint i_dwStreamID,gpointer pData);
    static void NewSelectPair(NiceAgent *agent, guint _stream_id,guint component_id, gchar *lfoundation,gchar *rfoundation, gpointer data);
    static void ComponentStateChanged(NiceAgent *agent, guint _stream_id,guint component_id, guint state,gpointer data);
    int AddVideoStream(NiceAgent *i_ptNiceAgent,char *i_strName, int i_iNum);
    int AddAudioStream(NiceAgent *i_ptNiceAgent,char *i_strName, int i_iNum);

    NiceAgent * m_ptAgent;
    GSList * m_pRemoteCandidatesList;
//    unsigned int m_dwStreamID;目前只有一个streamid使用,即videostream里面的
    int m_iLibniceVideoSendReadyFlag;//0不可发送,1准备好通道可以发送
    int m_iLibniceAudioSendReadyFlag;//后续这些音视频相关的信息都优化放到T_StreamInfo中
    
    T_LibniceDepData m_tLibniceDepData;
    string m_strRemoteSDP;
    GMainLoop *m_ptLoop;//
    GIOChannel* m_ptStdinIO;//
};





#endif
