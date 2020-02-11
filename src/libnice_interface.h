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

#include <ctype.h>
#include <agent.h>

#include <gio/gnetworking.h>

typedef struct LocalCandidate
{
	int iGatheringDoneFlag;//0 未收集,1收集成功
	char strCandidateData[256];
//"candidate:3442447574 1 udp 2122260223 192.168.0.170 54653 typ host generation 0 ufrag gX6M network-id 1"
}T_LocalCandidate;
typedef struct LibniceDepData
{
    char strStunAddr[24];
    unsigned int dwStunPort;
    int iControlling;
}T_LibniceDepData;

typedef struct LibniceCb
{
    void ( *Handshake)();
	void ( *HandleRecvData)(char * i_acData,int i_iLen);
    
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
	Libnice(char * i_strStunAddr,unsigned int i_dwStunPort,int i_iControlling);
	~Libnice();
    int SetCallback(T_LibniceCb *i_ptLibniceCb);
    int LibniceProc();
    int GetLocalCandidate(T_LocalCandidate * i_ptLocalCandidate);
    int GetLocalSDP(char * i_strSDP,int i_iSdpLen);
    int SetRemoteCredentials(char * i_strUfrag,char * i_strPasswd);
    int SetRemoteCandidateToGlist(char * i_strCandidate);
    int SetRemoteCandidates();
    int SetRemoteSDP(char * i_strSDP);
    int GetSendReadyFlag();
    int SendData(char * i_acBuf,int i_iBufLen);
    
    static void Recv(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data);

	static const char *m_astrCandidateTypeName[];
    
private:
    void CandidateGatheringDone(NiceAgent *i_ptAgent, guint i_dwStreamID,gpointer pData);
    void NewSelectPair(NiceAgent *agent, guint _stream_id,guint component_id, gchar *lfoundation,gchar *rfoundation, gpointer data);
    void ComponentStateChanged(NiceAgent *agent, guint _stream_id,guint component_id, guint state,gpointer data);


    T_LocalCandidate m_tLocalCandidate;
    NiceAgent * m_ptAgent;
    GSList * m_pRemoteCandidatesList;
    unsigned int m_dwStreamID;//m_iLibniceSendReadyFlag被ComponentStateChanged调用
    static int m_iLibniceSendReadyFlag;//0不可发送,1准备好通道可以发送
    T_LibniceDepData m_tLibniceDepData;
    T_LibniceCb m_tLibniceCb;
};





#endif
