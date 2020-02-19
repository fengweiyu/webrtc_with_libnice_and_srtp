/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       libnice_interface.cpp
* Description           : 	
后续可以把音视频流做成map或list(特别是流类型比较多的时候)，
这样通过streamid进行分发
本层就可以不关心音频还是视频了交给上层管理


* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "libnice_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char * Libnice::m_astrCandidateTypeName[] = {"host", "srflx", "prflx", "relay"};
int Libnice::m_iLibniceSendReadyFlag = 0;//0不可发送,1准备好通道可以发送
/*****************************************************************************
-Fuction        : LibniceInit
-Description    : LibniceInit
-Input          : i_iControlling 感觉不必要
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
Libnice::Libnice(char * i_strStunAddr,unsigned int i_dwStunPort,int i_iControlling)
{
    m_ptAgent=NULL;
    m_pRemoteCandidatesList = NULL;
//    m_dwStreamID=0;
    m_iLibniceSendReadyFlag =0;
	memset(&m_tLibniceDepData,0,sizeof(T_LibniceDepData));
	snprintf(m_tLibniceDepData.strStunAddr,sizeof(m_tLibniceDepData.strStunAddr),"%s",i_strStunAddr);
	m_tLibniceDepData.dwStunPort = i_dwStunPort;
	m_tLibniceDepData.iControlling = i_iControlling;

    memset(&m_tLibniceCb,0,sizeof(T_LibniceCb));
    memset(&m_tVideoStream,0,sizeof(T_StreamInfo));
    memset(&m_tAudioStream,0,sizeof(T_StreamInfo));
    m_strRemoteSDP.assign("");
}

/*****************************************************************************
-Fuction        : LibniceInit
-Description    : LibniceInit
-Input          : i_iControlling 感觉不必要
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
Libnice::~Libnice()
{


}
/*****************************************************************************
-Fuction        : LibniceInit
-Description    : LibniceInit
-Input          : i_iControlling 感觉不必要
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SetCallback(T_LibniceCb *i_ptLibniceCb)
{
	int iRet = -1;
	if (NULL == i_ptLibniceCb)
	{
	    printf("SetCallback NULL \r\n");
	}
	else
	{
        memset(&m_tLibniceCb,0,sizeof(T_LibniceCb));
        memcpy(&m_tLibniceCb,i_ptLibniceCb,sizeof(T_LibniceCb));
        iRet = 0;
	}
	return iRet;
}


/*****************************************************************************
-Fuction        : LibniceProc
-Description    : 会阻塞,线程函数
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::LibniceProc()
{
	int iRet = -1;
    static GMainLoop *ptLoop=NULL;//
    static GIOChannel* ptStdinIO=NULL;//
    int inx =0;

    m_iLibniceSendReadyFlag = 0;
    memset(&m_tLocalCandidate,0,sizeof(T_LocalCandidate));
	g_networking_init();//
	ptLoop = g_main_loop_new(NULL, FALSE);//
	ptStdinIO = g_io_channel_unix_new(fileno(stdin));//

	// Create the nice agent
	m_ptAgent = nice_agent_new(g_main_loop_get_context (ptLoop),NICE_COMPATIBILITY_RFC5245);
	if (m_ptAgent == NULL)
		g_error("Failed to create agent");

	// Set the STUN settings and controlling mode
	if (strlen(m_tLibniceDepData.strStunAddr)>0) 
	{
		g_object_set(m_ptAgent, "stun-server", m_tLibniceDepData.strStunAddr, NULL);
		g_object_set(m_ptAgent, "stun-server-port", m_tLibniceDepData.dwStunPort, NULL);
	}
	g_object_set(m_ptAgent, "controlling-mode", m_tLibniceDepData.iControlling, NULL);

	// Connect to the signals
	g_signal_connect(m_ptAgent, "candidate-gathering-done",G_CALLBACK(&Libnice::CandidateGatheringDone), this);
	g_signal_connect(m_ptAgent, "new-selected-pair",G_CALLBACK(&Libnice::NewSelectPair), this);//
	g_signal_connect(m_ptAgent, "component-state-changed",G_CALLBACK(&Libnice::ComponentStateChanged), this);//

	// Create a new stream with one component
    AddVideoStream(m_ptAgent,(char *)"video",1);
    AddAudioStream(m_ptAgent,(char *)"audio",1);//防止设置sdp时出错，暂时只是添加不使用

	// Attach to the component to receive the data
	// Without this call, candidates cannot be gathered
    for (inx = 1; inx <= m_tVideoStream.iNum; inx++)
    {
        nice_agent_attach_recv(m_ptAgent, m_tVideoStream.iID, inx, g_main_loop_get_context (ptLoop),&Libnice::RecvVideoData, this);
    }
    for (inx = 1; inx <= m_tAudioStream.iNum; inx++)
    {
        nice_agent_attach_recv(m_ptAgent, m_tAudioStream.iID, inx, g_main_loop_get_context (ptLoop),&Libnice::RecvAudioData, this);
    }

	// Start gathering local candidates
	if (!nice_agent_gather_candidates(m_ptAgent, m_tVideoStream.iID))
		g_error("Failed to start candidate gathering");

	g_debug("waiting for candidate-gathering-done signal...");

	// Run the mainloop. Everything else will happen asynchronously
	// when the candidates are done gathering.
	g_main_loop_run (ptLoop);////g_main_loop_quit (gloop);//
    

	g_main_loop_unref(ptLoop);//
	g_object_unref(m_ptAgent);//
	g_io_channel_unref (ptStdinIO);//
}
/*****************************************************************************
-Fuction        : LibniceGetLocalCandidate
-Description    : LibniceGetLocalCandidate
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::GetLocalCandidate(T_LocalCandidate * i_ptLocalCandidate)
{
	int iRet=-1;
    if (i_ptLocalCandidate == NULL) 
    {
		printf("GetLocalCandidate NULL\r\n");
		return iRet;
    }
	if(m_tLocalCandidate.iGatheringDoneFlag == 0)
	{
		printf("GetLocalCandidate err\r\n");
		return iRet;
	}
	memcpy(i_ptLocalCandidate,&m_tLocalCandidate,sizeof(T_LocalCandidate));
	iRet = 0;
	return iRet;
}
/*****************************************************************************
-Fuction        : LibniceGetLocalSDP
-Description    : local sdp缺少信息只好自己组包
m=- 39240 ICE/SDP\n
c=IN IP4 192.168.0.105\n
a=ice-ufrag:F77M\n
a=ice-pwd:xIem6dTc1rcgJMt3QUzDcE\n
a=candidate:1 1 UDP 2013266431 fe80::20c:29ff:fe7e:5629 36651 typ host\n
a=candidate:2 1 TCP 1015022847 fe80::20c:29ff:fe7e:5629 9 typ host tcptype active\n
a=candidate:3 1 TCP 1010828543 fe80::20c:29ff:fe7e:5629 53316 typ host tcptype passive\n
a=candidate:4 1 UDP 2013266430 192.168.0.105 54379 typ host\n
a=candidate:5 1 TCP 1015022079 192.168.0.105 9 typ host tcptype active\n
a=candidate:6 1 TCP 1010827775 192.168.0.105 39240 typ host tcptype passive\n

-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::GetLocalSDP(char * i_strSDP,int i_iSdpLen)
{
	int iRet=-1;
	char *strSDP = NULL;
    if (i_strSDP == NULL || m_ptAgent == NULL || i_iSdpLen <= 0) 
    {
		printf("LibniceGetLocalCandidate NULL\r\n");
		return iRet;
    }
	if(m_tLocalCandidate.iGatheringDoneFlag == 0)
	{
		printf("GetLocalSDP err\r\n");
		return iRet;
	}
    // Candidate gathering is done. Send our local candidates on stdout
    strSDP = nice_agent_generate_local_sdp (m_ptAgent);
    if(NULL!=strSDP)
    {
		snprintf(i_strSDP,i_iSdpLen,"%s",strSDP);
	    g_free (strSDP);
	    iRet = 0;
    }
	return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSetRemoteCredentials
-Description    : 后续区分音视频
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SetRemoteCredentials(char * i_strUfrag,char * i_strPasswd)
{
	int iRet = -1;
    if (i_strUfrag == NULL || i_strPasswd == NULL || m_ptAgent == NULL || m_tVideoStream.iID == 0) 
    {
		g_message("line must have at least ufrag, password, and one candidate");
		return iRet;
    }
    
    if (!nice_agent_set_remote_credentials(m_ptAgent, m_tVideoStream.iID, i_strUfrag, i_strPasswd)) 
    {
		g_message("failed to set remote credentials");
		return iRet;
    }
    iRet = 0;
    return iRet;
}
/*****************************************************************************
-Fuction        : LibniceSetRemoteCandidateToGlist
-Description    : LibniceSetRemoteCandidateToGlist
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SetRemoteCandidateToGlist(char * i_strCandidate)
{
	int iRet = -1;
    NiceCandidate *ptCand = NULL;
    NiceCandidateType ntype;
    char **tokens = NULL;
    unsigned int i=0;
	if(NULL == i_strCandidate || 0==m_tVideoStream.iID)
	{
        g_message("line must have at least ufrag, password, and one candidate");
        return iRet;
	}
	// Remaining args are serialized canidates (at least one is required)
    tokens = g_strsplit (i_strCandidate, " ", 5);
    for (i = 0; tokens[i]; i++);
	if (i != 5)
	{
        g_strfreev(tokens);
        return iRet;
	}
	for (i = 0; i < G_N_ELEMENTS (m_astrCandidateTypeName); i++) 
	{
		if (strcmp(tokens[4], m_astrCandidateTypeName[i]) == 0)
		{
			ntype = (NiceCandidateType)i;
			break;
		}
	}
	if (i == G_N_ELEMENTS (m_astrCandidateTypeName))
    {
        g_strfreev(tokens);
        return iRet;
    }    
	ptCand = nice_candidate_new(ntype);
	ptCand->component_id = 1;
	ptCand->stream_id = m_tVideoStream.iID;
	ptCand->transport = NICE_CANDIDATE_TRANSPORT_UDP;
	strncpy(ptCand->foundation, tokens[0], NICE_CANDIDATE_MAX_FOUNDATION);
	ptCand->foundation[NICE_CANDIDATE_MAX_FOUNDATION - 1] = 0;
	ptCand->priority = atoi (tokens[1]);
	if (!nice_address_set_from_string(&ptCand->addr, tokens[2])) 
	{
		g_message("failed to parse addr: %s", tokens[2]);
		nice_candidate_free(ptCand);
		ptCand = NULL;
        g_strfreev(tokens);
        return iRet;
	}
    nice_address_set_port(&ptCand->addr, atoi (tokens[3]));
    
	if (ptCand == NULL) 
	{
		g_message("failed to parse candidate: %s", i_strCandidate);
        g_strfreev(tokens);
	    return iRet;
	}
	m_pRemoteCandidatesList = g_slist_prepend(m_pRemoteCandidatesList, ptCand);
    g_strfreev(tokens); 
    iRet=0;
    return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSetRemoteCandidates
-Description    : 后续区分音视频
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SetRemoteCandidates()
{
	int iRet = -1;
    if (m_pRemoteCandidatesList == NULL) 
    {
		printf("m_pRemoteCandidatesList null \r\n");
	    return iRet;
    }
    if (m_ptAgent == NULL || m_tVideoStream.iID == 0) 
    {
		printf("m_ptAgent null \r\n");
		return iRet;
    }
	// Note: this will trigger the start of negotiation.
	for(int i=1;i<=m_tVideoStream.iNum;i++)
	{
        if (nice_agent_set_remote_candidates(m_ptAgent, m_tVideoStream.iID, i,m_pRemoteCandidatesList) < 1) 
        {
            g_message("failed to set remote candidates");
        }
        else
        {
            iRet=0;
        }
	}
	if (m_pRemoteCandidatesList != NULL)
	{
		g_slist_free_full(m_pRemoteCandidatesList, (GDestroyNotify)&nice_candidate_free);
		m_pRemoteCandidatesList =NULL;
	}
	return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSetRemoteSDP
-Description    : LibniceSetRemoteSDP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SaveRemoteSDP(char * i_strSDP)
{
	int iRet = -1;
    if (m_ptAgent == NULL || i_strSDP == NULL) 
    {
		printf("SaveRemoteSDP m_ptAgent null \r\n");
		return iRet;
    }
    m_strRemoteSDP.assign(i_strSDP);
    //printf("##########SaveRemoteSDP:%s ##########\r\n",m_strRemoteSDP.c_str());
    iRet = 0;
	return iRet;
}

/*****************************************************************************
-Fuction        : SetRemoteCandidateAndSDP
-Description    : LibniceSetRemoteSDP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SetRemoteCandidateAndSDP(char * i_strCandidate)
{
	int iRet = -1;
	int i=0;
    if (m_ptAgent == NULL || i_strCandidate == NULL ||m_strRemoteSDP.length()<=0) 
    {
		printf("Libnice SetRemoteCandidateAndSDP m_ptAgent null \r\n");
		return iRet;
    }
    m_strRemoteSDP.append("a=");//webrtc对方sdp中没有包含candidate,所以只好组合起来
    m_strRemoteSDP.append(i_strCandidate);
    m_strRemoteSDP.append("\r\n");
    char* ufrag = NULL;
    char* pwd = NULL;
    GSList * plist = nice_agent_parse_remote_stream_sdp (m_ptAgent, m_tVideoStream.iID, m_strRemoteSDP.c_str(), &ufrag, &pwd);
    if (ufrag && pwd && g_slist_length(plist) > 0)
    {//audio streamid暂时不处理,
        ufrag[strlen(ufrag)-1] = 0;
        pwd[strlen(pwd)-1] = 0;

        NiceCandidate* c = (NiceCandidate*)g_slist_nth(plist, 0)->data; 

        if (!nice_agent_set_remote_credentials(m_ptAgent, m_tVideoStream.iID, ufrag, pwd)) 
        {
            printf("failed to set remote credentials\r\n");
        }
        else
        {
            // Note: this will trigger the start of negotiation.
            for (i = 1; i <= m_tVideoStream.iNum; i++)
            {
                if (nice_agent_set_remote_candidates(m_ptAgent, m_tVideoStream.iID, i, plist) < 1) 
                {
                    printf("###failed to set remote candidates:%d\r\n",i);
                }
                else
                {
                    iRet=0;
                }
            }
        }
        g_free(ufrag);
        g_free(pwd);
        //g_slist_free(plist);
        g_slist_free_full(plist, (GDestroyNotify)&nice_candidate_free);
    }
	return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSetRemoteSDP
-Description    : 由于webrtc对方发来的sdp中不含candidate所以该
接口暂时不用
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SetRemoteSDP(char * i_strSDP)
{
	int iRet = -1;
    if (m_ptAgent == NULL || i_strSDP == NULL) 
    {
		printf("LibniceSetRemoteSDP m_ptAgent null \r\n");
		return iRet;
    }

	// Parse remote candidate list and set it on the agent
	if (nice_agent_parse_remote_sdp (m_ptAgent, i_strSDP) > 0) 
	{
		iRet = 0;
	} 
	else 
	{
		printf("LibniceSetRemoteSDP nice_agent_parse_remote_sdp fail \r\n");
	}
	return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSendData
-Description    : LibniceSendData
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SendVideoData(char * i_acBuf,int i_iBufLen)
{
	int iRet = -1;
	int i=0;
    if( m_ptAgent == NULL || m_tVideoStream.iID== 0 || i_acBuf == NULL) 
    {
		printf("LibniceSendData m_ptAgent null \r\n");
		return iRet;
    }
    /*if(m_iLibniceSendReadyFlag == 0) 
    {//发送标记由上层控制，因为发送协商报文也是这个接口但是还不是ready
		printf("LibniceSendReady err \r\n");
		return iRet;
    }*/
    for (i = 1; i <= m_tVideoStream.iNum; i++)
    {
        iRet = nice_agent_send(m_ptAgent, m_tVideoStream.iID, i, i_iBufLen, i_acBuf);
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSendData
-Description    : LibniceSendData
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SendAudioData(char * i_acBuf,int i_iBufLen)
{
	int iRet = -1;
	int i=0;
    if( m_ptAgent == NULL || m_tAudioStream.iID== 0 || i_acBuf == NULL) 
    {
		printf("LibniceSendData m_ptAgent null \r\n");
		return iRet;
    }
    if(m_iLibniceSendReadyFlag == 0) 
    {
		printf("LibniceSendReady err \r\n");
		return iRet;
    }
    for (i = 1; i <= m_tAudioStream.iNum; i++)
    {
        iRet = nice_agent_send(m_ptAgent, m_tAudioStream.iID, i, i_iBufLen, i_acBuf);
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : LibniceGetSendReadyFlag
-Description    : LibniceGetSendReadyFlag
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::GetSendReadyFlag()
{
    return m_iLibniceSendReadyFlag;
}

void Libnice::CandidateGatheringDone(NiceAgent *i_ptAgent, guint i_dwStreamID,gpointer pData)
{
	g_debug("SIGNAL candidate gathering done\n");
    Libnice *pLibnice=NULL;

	// Candidate gathering is done. Send our local candidates on stdout
	int iRet = -1;
	gchar *strLocalUfrag = NULL;
	gchar *strLocalPassword = NULL;
	gchar ipaddr[INET6_ADDRSTRLEN];
	GSList *cands = NULL, *item = NULL;
	static const gchar *transport_name[] = {"udp", "tcp_active", "tcp_passive", "tcp_so"};
	
	if (!nice_agent_get_local_credentials(i_ptAgent, i_dwStreamID,&strLocalUfrag, &strLocalPassword))
		goto end;
	cands = nice_agent_get_local_candidates(i_ptAgent, i_dwStreamID, 1);
	if (cands == NULL)
		goto end;
    pLibnice =(Libnice *)pData;
	if (pLibnice == NULL)
		goto end;
    snprintf(pLibnice->m_tLocalCandidate.strUfrag,sizeof(pLibnice->m_tLocalCandidate.strUfrag),"%s",strLocalUfrag);
    snprintf(pLibnice->m_tLocalCandidate.strPassword,sizeof(pLibnice->m_tLocalCandidate.strPassword),"%s",strLocalPassword);
	for (item = cands; item; item = item->next) 
	{
		NiceCandidate *c = (NiceCandidate *)item->data;

		nice_address_to_string(&c->addr, ipaddr);

		// RFC 5245
		// a=candidate:<foundation> <component-id> <transport> <priority>
		// <connection-address> <port> typ <candidate-types>
		// [raddr <connection-address>] [rport <port>]
		// *(SP extension-att-name SP extension-att-value)
		//"candidate:3442447574 1 udp 2122260223 192.168.0.170 54653 typ host generation 0 ufrag gX6M network-id 1"
		snprintf(pLibnice->m_tLocalCandidate.strCandidateData,sizeof(pLibnice->m_tLocalCandidate.strCandidateData),
		"candidate:%s %u %s %u %s %u typ %s",
		c->foundation,c->component_id,transport_name[c->transport],c->priority,
		ipaddr,nice_address_get_port(&c->addr),m_astrCandidateTypeName[c->type]);
        snprintf(pLibnice->m_tLocalCandidate.strIP,sizeof(pLibnice->m_tLocalCandidate.strIP),"%s",ipaddr);
        if(NULL!= strstr(pLibnice->m_tLocalCandidate.strCandidateData,"udp")&&NULL!= strstr(pLibnice->m_tLocalCandidate.strCandidateData,"."))
        {//后续可以优化为全部取出来放到数组中,sdp中的ip填0.0.0.0
            break;//或者试试(NiceCandidate *)g_slist_nth(cands, 0)->data;可能是最优路径
        }
	}
	printf("CandidateGatheringDone:%s ,%s %s\r\n", pLibnice->m_tLocalCandidate.strCandidateData,strLocalUfrag, strLocalPassword);
	pLibnice->m_tLocalCandidate.iGatheringDoneFlag = 1;
	iRet = 0;

	end:
		if (strLocalUfrag)
			g_free(strLocalUfrag);
		if (strLocalPassword)
			g_free(strLocalPassword);
		if (cands)
			g_slist_free_full(cands, (GDestroyNotify)&nice_candidate_free);
	return;
}

void Libnice::NewSelectPair(NiceAgent *agent, guint _stream_id,guint component_id, gchar *lfoundation,gchar *rfoundation, gpointer data)
{//此处开始dtls握手
	printf("SIGNAL: selected pair %s %s\r\n", lfoundation, rfoundation);
    Libnice *pLibnice=NULL;
    pLibnice = (Libnice *)data;
	if (NULL != pLibnice)
	{
        if (NULL != pLibnice->m_tLibniceCb.Handshake)
        {//这里接收浏览器发出的报文(包括dtls协商报文)
             pLibnice->m_tLibniceCb.Handshake(pLibnice->m_tLibniceCb.pObjCb);
        }
	}
	
}

void Libnice::ComponentStateChanged(NiceAgent *agent, guint _stream_id,guint component_id, guint state,gpointer data)
{
	static const gchar *state_name[] = {"disconnected", "gathering", "connecting","connected", "ready", "failed"};
    
	printf("SIGNAL: state changed %d %d %s[%d]\n",_stream_id, component_id, state_name[state], state);
    Libnice *pLibnice=NULL;
    pLibnice = (Libnice *)data;
	if (state == NICE_COMPONENT_STATE_READY) 
	{//协商成功
        if (NULL != pLibnice)
        {
            pLibnice->m_iLibniceSendReadyFlag = 1;
        }
	}
}

void Libnice::RecvVideoData(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data)
{
    Libnice *pLibnice=NULL;
    pLibnice = (Libnice *)data;
	if (NULL != pLibnice)
	{
        if (NULL != pLibnice->m_tLibniceCb.HandleRecvData)
        {//这里接收浏览器发出的报文(包括dtls协商报文)
             pLibnice->m_tLibniceCb.HandleRecvData(buf,len,pLibnice->m_tLibniceCb.pObjCb);
        }
	}
	printf("%.*s", len, buf);
	//fflush(stdout);
}

void Libnice::RecvAudioData(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data)
{
	printf("RecvAudioData:%d\r\n", len);
	//fflush(stdout);
}

int Libnice::AddVideoStream(NiceAgent *i_ptNiceAgent,char *i_strName, int i_iNum)
{
    int iRet = -1;
	if (NULL == i_ptNiceAgent||NULL == i_strName ||i_iNum <1)
    {
        printf("Libnice: AddVideoStream NULL:%d\r\n",i_iNum);
        return iRet;
    }
    memset(&m_tVideoStream,0,sizeof(T_StreamInfo));
    snprintf(m_tVideoStream.strName,sizeof(m_tVideoStream.strName),"%s",i_strName);
    m_tVideoStream.iNum=i_iNum;
    m_tVideoStream.iID = nice_agent_add_stream(i_ptNiceAgent, m_tVideoStream.iNum);
    if (m_tVideoStream.iID == 0)
    {
        printf("Libnice: AddVideoStream err:%d\r\n",i_iNum);
    }
    else
    {
        nice_agent_set_stream_name(i_ptNiceAgent, m_tVideoStream.iID, m_tVideoStream.strName);
        iRet=0;
    }
    return iRet;
}

int Libnice::AddAudioStream(NiceAgent *i_ptNiceAgent,char *i_strName, int i_iNum)
{
    int iRet = -1;
	if (NULL == i_ptNiceAgent||NULL == i_strName ||i_iNum <1)
    {
        printf("Libnice: AddAudioStream NULL:%d\r\n",i_iNum);
        return iRet;
    }
    memset(&m_tAudioStream,0,sizeof(T_StreamInfo));
    snprintf(m_tAudioStream.strName,sizeof(m_tAudioStream.strName),"%s",i_strName);
    m_tAudioStream.iNum=i_iNum;
    m_tAudioStream.iID = nice_agent_add_stream(i_ptNiceAgent, m_tAudioStream.iNum);
    if (m_tAudioStream.iID == 0)
    {
        printf("Libnice: AddAudioStream err:%d\r\n",i_iNum);
    }
    else
    {
        nice_agent_set_stream_name(i_ptNiceAgent, m_tAudioStream.iID, m_tAudioStream.strName);
        iRet=0;
    }
    return iRet;
}




