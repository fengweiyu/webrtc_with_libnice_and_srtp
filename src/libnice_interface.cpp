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
#include "debug.h"



#define LIBNICE_MIN_PORT_NUM 10000
#define LIBNICE_MAX_PORT_NUM 65536




/*Host (Host Candidate)，该地址是一个真实的主机，参数中的地址和端口对应一个真实的主机地址，
该地址来源于本地的物理网卡或逻辑网卡上的地址，
对于具有公网地址或者同一内网的端可以用；
Srvflx (Server Reflexive Candidate)，该地址是通过 Cone NAT (锥形 NAT) 反射的类型，参数中的地址和端口
是端发送 Binding 请求到 STUN/TURN server 经过 NAT 时，NAT 上分配的地址和端口
Relay (Relayed Candidate)，该地址是端发送 Allocate 请求到 TURN server，由 TURN server 用于中继的地址和
端口，该地址和端口是 TURN 服务用于在两个对等点之间转发数据的地址和端口，
是一个中继地址端口；( 可能是本机或 NAT 地址)；
Prflx(Peer Reflexive Candidate)，该地址是通过发送 STUN Binding 时，通过 Binding 获取到的地址；
在建立连接检查期间新发生，参数中的地址和端口是端发送 Binding 请求到 
STUN/TURN server 经过 NAT 时，NAT 上分配的地址和端口*/
const char * Libnice::m_astrCandidateTypeName[] = {"host", "srflx", "prflx", "relay"};
//int Libnice::m_iLibniceSendReadyFlag = 0;//0不可发送,1准备好通道可以发送
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
Libnice::Libnice(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling,int i_iAvMultiplex)
{
    m_ptAgent=NULL;
    m_pRemoteCandidatesList = NULL;
//    m_dwStreamID=0;
    m_iLibniceVideoSendReadyFlag =0;
    m_iLibniceAudioSendReadyFlag =0;
	memset(&m_tLibniceDepData,0,sizeof(T_LibniceDepData));
	snprintf(m_tLibniceDepData.strStunAddr,sizeof(m_tLibniceDepData.strStunAddr),"%s",i_strStunAddr);
	m_tLibniceDepData.dwStunPort = i_dwStunPort;
	m_tLibniceDepData.eControlling = i_eControlling;
	m_tLibniceDepData.iAvMultiplex = i_iAvMultiplex;
	
    memset(&m_tLibniceCb,0,sizeof(T_LibniceCb));
    memset(&m_tVideoStream,0,sizeof(T_StreamInfo));
    memset(&m_tAudioStream,0,sizeof(T_StreamInfo));
    m_strRemoteSDP.assign("");

    m_ptLoop=NULL;//
    m_ptStdinIO=NULL;//
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
	int iRet = 0;
    static GMainLoop *ptLoop=NULL;//
    static GIOChannel* ptStdinIO=NULL;//
    int inx =0;
    NiceAddress tBaseAddr;

    m_iLibniceVideoSendReadyFlag = 0;
    m_iLibniceAudioSendReadyFlag = 0;
    memset(&m_tLocalVideoCandidate,0,sizeof(T_LocalCandidate));
    memset(&m_tLocalAudioCandidate,0,sizeof(T_LocalCandidate));
	g_networking_init();//
	m_ptLoop = g_main_loop_new(NULL, FALSE);//ptLoop
	m_ptStdinIO = g_io_channel_unix_new(fileno(stdin));//ptStdinIO

    nice_debug_enable(1);//不起作用
	// Create the nice agent
	m_ptAgent = nice_agent_new(g_main_loop_get_context (m_ptLoop),NICE_COMPATIBILITY_RFC5245);
	if (m_ptAgent == NULL)
		WEBRTC_LOGE("Failed to create agent\r\n");//g_error

	// Set the STUN settings and controlling mode
	if (strlen(m_tLibniceDepData.strStunAddr)>0) 
	{
		g_object_set(m_ptAgent, "stun-server", m_tLibniceDepData.strStunAddr, NULL);
		g_object_set(m_ptAgent, "stun-server-port", m_tLibniceDepData.dwStunPort, NULL);
	}
	g_object_set(m_ptAgent, "controlling-mode", (int)m_tLibniceDepData.eControlling, NULL);

	// Connect to the signals
	g_signal_connect(m_ptAgent, "candidate-gathering-done",G_CALLBACK(&Libnice::CandidateGatheringDone), this);
	g_signal_connect(m_ptAgent, "new-selected-pair",G_CALLBACK(&Libnice::NewSelectPair), this);//
	g_signal_connect(m_ptAgent, "component-state-changed",G_CALLBACK(&Libnice::ComponentStateChanged), this);//

	// Create a new stream with one component
    AddVideoStream(m_ptAgent,(char *)"video",1);
    AddAudioStream(m_ptAgent,(char *)"audio",1);//

	// Attach to the component to receive the data
	// Without this call, candidates cannot be gathered
    for (inx = 1; inx <= m_tVideoStream.iNum; inx++)
    {
        nice_agent_attach_recv(m_ptAgent, m_tVideoStream.iID, inx, g_main_loop_get_context (m_ptLoop),&Libnice::RecvVideoData, this);
    }
    for (inx = 1; inx <= m_tAudioStream.iNum; inx++)
    {
        nice_agent_attach_recv(m_ptAgent, m_tAudioStream.iID, inx, g_main_loop_get_context (m_ptLoop),&Libnice::RecvAudioData, this);
    }
    
    // 设置本地网络地址和端口
    //nice_address_init (&tBaseAddr);//设置了地址则只有candidate:2 1 tcp_active 1015023359 139.9.149.150 0 typ host,没找到修改为udp的方法
    //nice_address_set_from_string(&tBaseAddr, "139.9.149.150");
    //nice_address_set_port(&tBaseAddr,9018);
    //g_object_set (G_OBJECT (m_ptAgent), "ice-tcp", FALSE,  NULL);//可以使用这个修改是否udp，tcp
    // 添加候选地址（可以是对端地址）
    //nice_agent_add_local_address(m_ptAgent,&tBaseAddr);//目前收集地址很快(1s内)，但是后面还有建立链接和密钥协商耗时(总体1s内)
    
    // 设置本地端口范围，一般不做限制
    nice_agent_set_port_range(m_ptAgent, m_tVideoStream.iID,NICE_COMPONENT_TYPE_RTP,LIBNICE_MIN_PORT_NUM,LIBNICE_MAX_PORT_NUM);//设置了10000以上防止防火墙阻止

	// Start gathering local candidates
	if (!nice_agent_gather_candidates(m_ptAgent, m_tVideoStream.iID))
	{
		WEBRTC_LOGE("Failed to start candidate gathering m_tVideoStream\r\n");//g_error
		iRet = -1;
	}
	if(m_tLibniceDepData.iAvMultiplex > 0)
	{
        if (!nice_agent_gather_candidates(m_ptAgent, m_tAudioStream.iID))//这样后面才可以触发connecting
            WEBRTC_LOGE("Failed to start candidate gathering m_tAudioStream\r\n");//g_error
	}

	WEBRTC_LOGI("waiting for candidate-gathering-done signal...\r\n");//g_debug

	// Run the mainloop. Everything else will happen asynchronously
	// when the candidates are done gathering.
	g_main_loop_run (m_ptLoop);////g_main_loop_quit (gloop);//
    

	g_main_loop_unref(m_ptLoop);//ptLoop
	g_object_unref(m_ptAgent);//
	g_io_channel_unref (m_ptStdinIO);//ptStdinIO

	return iRet;
}

/*****************************************************************************
-Fuction        : StopProc
-Description    : StopProc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::StopProc()
{
    g_main_loop_quit (m_ptLoop);
    return 0;
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
int Libnice::GetLocalCandidate(T_LocalCandidate * o_ptLocalVideoCandidate,T_LocalCandidate * o_ptLocalAudioCandidate)
{
	int iRet=-1;
    if (o_ptLocalVideoCandidate == NULL) 
    {
		printf("GetLocalCandidate NULL\r\n");
		return iRet;
    }
	if(m_tLocalVideoCandidate.iGatheringDoneFlag == 0)
	{
		printf("GetLocalCandidate err\r\n");
		return iRet;
	}
	memcpy(o_ptLocalVideoCandidate,&m_tLocalVideoCandidate,sizeof(T_LocalCandidate));
	
    if (o_ptLocalAudioCandidate != NULL) 
    {
        memcpy(o_ptLocalAudioCandidate,&m_tLocalAudioCandidate,sizeof(T_LocalCandidate));
    }
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
	if(m_tLocalVideoCandidate.iGatheringDoneFlag == 0)
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
int Libnice::SetRemoteCredentials(char * i_strUfrag,char * i_strPasswd,int i_iStreamID)
{
	int iRet = -1;
    if (i_strUfrag == NULL || i_strPasswd == NULL || m_ptAgent == NULL || m_tVideoStream.iID == 0) 
    {
		printf("line must have at least ufrag, password, and one candidate %p,%p,%p,%d\r\n",i_strUfrag,i_strPasswd,m_ptAgent,m_tVideoStream.iID);//g_message
		return iRet;
    }
    
    if (!nice_agent_set_remote_credentials(m_ptAgent,i_iStreamID, i_strUfrag, i_strPasswd)) 
    {
		printf("failed to set remote m_tVideoStream credentials %d\r\n",i_iStreamID);//g_message
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
int Libnice::SetRemoteCandidateToGlist(char * i_strCandidate,int i_iStreamID)
{
	int iRet = -1;
    NiceCandidate *ptCand = NULL;
    NiceCandidateType ntype;
    char **tokens = NULL;
    unsigned int i=0;
	if(NULL == i_strCandidate || 0==m_tVideoStream.iID)
	{
        printf("line must have at least ufrag, password, and one candidate\r\n");
        return iRet;
	}
	//candidate:31 1 udp 21 40e-6e07 50803 typ host generation 0 ufrag 5E1n network-cost
	// Remaining args are serialized canidates (at least one is required)
    tokens = g_strsplit (i_strCandidate+strlen("candidate:"), " ", 0);//分割成8个部分
    for (i = 0; tokens[i]; i++);
	if (i <8)
	{
        g_strfreev(tokens);
        printf("g_strsplit i_strCandidate err:%d\r\n",i);
        return iRet;
	}
	for (i = 0; i < G_N_ELEMENTS (m_astrCandidateTypeName); i++) 
	{
		if (strcmp(tokens[7], m_astrCandidateTypeName[i]) == 0)
		{
			ntype = (NiceCandidateType)i;
			break;
		}
	}
	if (i == G_N_ELEMENTS (m_astrCandidateTypeName))
    {
        g_strfreev(tokens);
        printf("G_N_ELEMENTS m_astrCandidateTypeName err:%d\r\n",i);
        return iRet;
    }    
	ptCand = nice_candidate_new(ntype);
	ptCand->component_id = 1;
	ptCand->stream_id = i_iStreamID;
	ptCand->transport = NICE_CANDIDATE_TRANSPORT_UDP;
	strncpy(ptCand->foundation, tokens[0], NICE_CANDIDATE_MAX_FOUNDATION);
	ptCand->foundation[NICE_CANDIDATE_MAX_FOUNDATION - 1] = 0;
	ptCand->priority = atoi (tokens[3]);
	if (!nice_address_set_from_string(&ptCand->addr, tokens[4])) 
	{
		printf("failed to parse addr: %s\r\n", tokens[4]);
		nice_candidate_free(ptCand);
		ptCand = NULL;
        g_strfreev(tokens);
        return iRet;
	}
    nice_address_set_port(&ptCand->addr, atoi (tokens[5]));
    
	if (ptCand == NULL) 
	{
		printf("failed to parse candidate: %s\r\n", i_strCandidate);
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
int Libnice::SetRemoteCandidates(int i_iStreamID,int i_iStreamNum)
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
	for(int i=1;i<=i_iStreamNum;i++)
	{
        if (nice_agent_set_remote_candidates(m_ptAgent, i_iStreamID, i,m_pRemoteCandidatesList) < 1) 
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
		printf("SaveRemoteSDP null m_ptAgent %p i_strSDP %p\r\n",m_ptAgent,i_strSDP);
		return iRet;
    }
    m_strRemoteSDP.assign(i_strSDP);
    //printf("##########SaveRemoteSDP:%s ##########\r\n",m_strRemoteSDP.c_str());
    iRet = 0;
	return iRet;
}

/*****************************************************************************
-Fuction        : SetRemoteCandidateAndSDP
-Description    : SetRemoteSdpWithCandidate
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
    if (m_ptAgent == NULL ||m_strRemoteSDP.length()<=0) 
    {
		printf("Libnice SetRemoteCandidateAndSDP m_ptAgent null \r\n");
		return iRet;
    }
    if(string::npos==m_strRemoteSDP.find("candidate:"))
    {
        if (i_strCandidate == NULL) 
        {
            printf("Libnice SetRemoteCandidateAndSDP i_strCandidate null \r\n");
            return iRet;
        }
        m_strRemoteSDP.append("a=");//webrtc对方sdp中没有包含candidate,所以只好组合起来
        m_strRemoteSDP.append(i_strCandidate);
        m_strRemoteSDP.append("\r\n");
    }
    size_t dwAudioPos = m_strRemoteSDP.find("m=audio");
    if(/*string::npos != dwAudioPos && */m_tLibniceDepData.iAvMultiplex > 0)
    {
        iRet = SetRemoteSdpToStream(i_strCandidate,m_tVideoStream.iID,m_tVideoStream.iNum,m_strRemoteSDP.substr(0,dwAudioPos).c_str());
        iRet |= SetRemoteSdpToStream(i_strCandidate,m_tAudioStream.iID,m_tAudioStream.iNum,m_strRemoteSDP.substr(dwAudioPos).c_str());
    }
    else
    {
        iRet = SetRemoteSdpToStream(i_strCandidate,m_tVideoStream.iID,m_tVideoStream.iNum,m_strRemoteSDP.c_str());
    }
	return iRet;
}

/*****************************************************************************
-Fuction        : SetRemoteSdpToStream
-Description    : SetRemoteSdpWithCandidate
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Libnice::SetRemoteSdpToStream(char * i_strCandidate,int i_iStreamID,int i_iStreamNum,const char *i_strSDP)
{
	int iRet = -1;
	int i=0;
    char* ufrag = NULL;
    char* pwd = NULL;
    GSList * plist = NULL;
    
    plist = nice_agent_parse_remote_stream_sdp (m_ptAgent, i_iStreamID,i_strSDP, &ufrag, &pwd);
    printf("SetRemoteSdpToStream %d,%d\r\n%s \r\n",i_iStreamID,g_slist_length(plist),i_strSDP);
    if (ufrag && pwd && g_slist_length(plist) > 0)
    {//
        ufrag[strlen(ufrag)-1] = 0;
        pwd[strlen(pwd)-1] = 0;

        NiceCandidate* c = (NiceCandidate*)g_slist_nth(plist, 0)->data; 

        if (!nice_agent_set_remote_credentials(m_ptAgent, i_iStreamID, ufrag, pwd)) 
        {
            printf("failed to set remote credentials\r\n");
        }
        else
        {
            // Note: this will trigger the start of negotiation.
            for (i = 1; i <= i_iStreamNum; i++)
            {
                if (nice_agent_set_remote_candidates(m_ptAgent, i_iStreamID, i, plist) < 1) 
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
    else if(NULL !=ufrag && NULL!=pwd)
    {//nice_agent_parse_remote_stream_sdp部分失败则用自己的逻辑
        printf("failed to set remote candidates:%p,%p,%p,%d....use local\r\n",ufrag,pwd,plist,g_slist_length(plist));
        SetRemoteCredentials(ufrag, pwd,i_iStreamID);
        SetRemoteCandidateToGlist(i_strCandidate,i_iStreamID);//i_strCandidate需要属于对应的streamID
        iRet = SetRemoteCandidates(i_iStreamID,i_iStreamNum);
    }
    else
    {
        printf("1111failed to set remote candidates:%p,%p,%p,%d\r\n",ufrag,pwd,plist,g_slist_length(plist));
        printf("##########RemoteSDP:%s##########\r\n",i_strSDP);
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
		WEBRTC_LOGE("SendVideoData m_ptAgent null \r\n");
		return iRet;
    }
    if(m_iLibniceVideoSendReadyFlag == 0) //根据接口说明，这里必须ready才能发送成功
    {//发送标记由上层控制，因为发送协商报文也是这个接口但是还不是ready
		WEBRTC_LOGE("SendVideoData no Ready \r\n");//上面注释机制修改为ready才协商
		return iRet;
    }
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
		printf("SendAudioData m_ptAgent null \r\n");
		return iRet;
    }
    if(m_iLibniceAudioSendReadyFlag == 0) 
    {
		printf("SendAudioData no Ready \r\n");
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
int Libnice::GetSendReadyFlag(int * o_piVideoReadyFlag,int * o_piAudioReadyFlag)
{
    int iRet = -1;
    if(NULL != o_piVideoReadyFlag)
    {
        *o_piVideoReadyFlag = m_iLibniceVideoSendReadyFlag;
        iRet = 0;
    }
    if(NULL != o_piAudioReadyFlag)
    {
        *o_piAudioReadyFlag = m_iLibniceAudioSendReadyFlag;
        iRet = 0;
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
int Libnice::SetSendReadyFlag(int i_iStreamID,int i_iSendReadyFlag)
{
    int iRet = -1;
    if(i_iStreamID == m_tVideoStream.iID)
    {
        m_iLibniceVideoSendReadyFlag = i_iSendReadyFlag;
        iRet = 0;
    }
    else if(i_iStreamID == m_tAudioStream.iID)
    {
        m_iLibniceAudioSendReadyFlag = i_iSendReadyFlag;
        iRet = 0;
    }
    else
    {
        printf("SetSendReadyFlag err StreamID%d,%d,%d\r\n",i_iStreamID,m_tVideoStream.iID,m_tAudioStream.iID);
    }
    return iRet;
}

void Libnice::CandidateGatheringDone(NiceAgent *i_ptAgent, guint i_dwStreamID,gpointer pData)
{
	g_debug("SIGNAL candidate gathering done\n");
    Libnice *pLibnice=NULL;
    int i=0;
	// Candidate gathering is done. Send our local candidates on stdout
	int iRet = -1;
	gchar *strLocalUfrag = NULL;
	gchar *strLocalPassword = NULL;
	gchar ipaddr[INET6_ADDRSTRLEN];
	GSList *cands = NULL, *item = NULL;
	static const gchar *transport_name[] = {"udp", "tcp_active", "tcp_passive", "tcp_so"};
    T_LocalCandidate *pLocalCandidate = NULL;

    
    pLibnice =(Libnice *)pData;
	if (pLibnice == NULL)
	{
        printf("pLibnice == NULL\n");
		goto end;
	}
    if(i_dwStreamID == pLibnice->m_tVideoStream.iID)
    {
        pLocalCandidate = &pLibnice->m_tLocalVideoCandidate;
    }
    else if(i_dwStreamID == pLibnice->m_tAudioStream.iID)
    {
        pLocalCandidate = &pLibnice->m_tLocalAudioCandidate;
    }
    if(NULL == pLocalCandidate)
    {
        printf("CandidateGatheringDone err StreamID%d,%d,%d\r\n",i_dwStreamID,pLibnice->m_tVideoStream.iID,pLibnice->m_tAudioStream.iID);
        return;
    }
    
	if (!nice_agent_get_local_credentials(i_ptAgent, i_dwStreamID,&strLocalUfrag, &strLocalPassword))
		goto end;
	cands = nice_agent_get_local_candidates(i_ptAgent, i_dwStreamID, 1);//NICE_COMPONENT_TYPE_RTP
	if (cands == NULL)
		goto end;
	memset(pLocalCandidate,0,sizeof(T_LocalCandidate));	
    snprintf(pLocalCandidate->strUfrag,sizeof(pLocalCandidate->strUfrag),"%s",strLocalUfrag);
    snprintf(pLocalCandidate->strPassword,sizeof(pLocalCandidate->strPassword),"%s",strLocalPassword);
	for (item = cands; item; item = item->next) 
	{
		NiceCandidate *c = (NiceCandidate *)item->data;//从第一个开始去candidate列表内容
        //NiceCandidate *c = (NiceCandidate *)g_slist_nth(cands, 0)->data;//取列表第一个不一定是最好的那个
		nice_address_to_string(&c->addr, ipaddr);

		// RFC 5245 
		// a=candidate:<foundation> <component-id> <transport> <priority>
		// <connection-address> <port> typ <candidate-types>
		// [raddr <connection-address>] [rport <port>]
		// *(SP extension-att-name SP extension-att-value)
		//"candidate:3442447574 1 udp 2122260223 192.168.0.170 54653 typ host generation 0 ufrag gX6M network-id 1"
		//foundation 候选者的基础标识，用于唯一标识候选者
		//component ID：组件ID，表示候选者属于ICE代理的哪个组件1 代表 RTP; 2 代表 RTCP 
		//WebRTC 采用 Rtcp-mux 方式，也就是 RTP 和 RTCP 在同一通道内传输，减少 ICE 的协商和通道的保活 
		snprintf(pLocalCandidate->strCandidateData[i],sizeof(pLocalCandidate->strCandidateData[i]),
		"candidate:%s %u %s %u %s %u typ %s",
		c->foundation,c->component_id,transport_name[c->transport],c->priority,
		ipaddr,nice_address_get_port(&c->addr),m_astrCandidateTypeName[c->type]);
        snprintf(pLocalCandidate->strIP[i],sizeof(pLocalCandidate->strIP[i]),"%s",ipaddr);
        printf("strCandidateData %d,%s,\r\n",i,pLocalCandidate->strCandidateData[i]);
        i++;
        if(i>=MAX_CANDIDATE_NUM)
        {
            printf("m_tLocalCandidate too more %d,%d,\r\n",i,MAX_CANDIDATE_NUM);
            break;
        }
        /*if(NULL!= strstr(pLocalCandidate->strCandidateData[i],"udp")&&NULL!= strstr(pLocalCandidate->strCandidateData[i],"."))
        {//后续可以优化为全部取出来放到数组中,sdp中的ip填0.0.0.0
            i=1;
            break;//或者试试(NiceCandidate *)g_slist_nth(cands, 0)->data;可能是最优路径
        }*/
	}
	pLocalCandidate->iCurCandidateNum=i;
	printf("CandidateGatheringDone:%d ,%s %s\r\n",pLocalCandidate->iCurCandidateNum,strLocalUfrag, strLocalPassword);
	pLocalCandidate->iGatheringDoneFlag = 1;
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
	    //放在后面也能发出握手包，结果也一样
        //pLibnice->m_iLibniceSendReadyFlag = 1;
        if (NULL != pLibnice->m_tLibniceCb.Handshake)
        {//这里接收浏览器发出的报文(包括dtls协商报文)
             //pLibnice->m_tLibniceCb.Handshake(pLibnice->m_tLibniceCb.pObjCb);//ready才能发送报文
        }//selected pair在ready前发生
	}
	
}

void Libnice::ComponentStateChanged(NiceAgent *agent, guint _stream_id,guint component_id, guint state,gpointer data)
{
	static const gchar *state_name[] = {"disconnected", "gathering", "connecting","connected", "ready", "failed"};
    
	printf("SIGNAL: state changed %d %d %s[%d]\n",_stream_id, component_id, state_name[state], state);
    Libnice *pLibnice=NULL;
    pLibnice = (Libnice *)data;//sdp发送给对方到(开始建立链接到)connected状态，耗时200ms，(sdp发送到对方设置好并回应耗时70ms)
	if (state == NICE_COMPONENT_STATE_CONNECTED) //这样更快开始握手//NICE_COMPONENT_STATE_READY
	{//协商成功
        if (NULL != pLibnice)
        {
            //必须放到这里并开始dtls握手，
            pLibnice->SetSendReadyFlag(_stream_id,1);//否则收到数据就开始dtls的报文处理，则dtls的报文也发送不出去，
            if (NULL != pLibnice->m_tLibniceCb.Handshake)//还是要等NICE_COMPONENT_STATE_CONNECTED状态
            {//这里接收浏览器发出的报文(包括dtls协商报文)
                if(_stream_id == pLibnice->m_tVideoStream.iID)
                {
                    pLibnice->m_tLibniceCb.Handshake(pLibnice->m_tLibniceCb.pVideoDtlsObjCb);
                }
                else if(_stream_id == pLibnice->m_tAudioStream.iID)
                {
                    pLibnice->m_tLibniceCb.Handshake(pLibnice->m_tLibniceCb.pAudioDtlsObjCb);
                }
                else
                {
                    printf("ComponentStateChanged err StreamID%d,%d,%d\r\n",_stream_id,pLibnice->m_tVideoStream.iID,pLibnice->m_tAudioStream.iID);
                }
            }
        }
	}
}

void Libnice::RecvVideoData(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data)
{
    printf("Libnice::RecvSrtpData _stream_id%d,%d,%#x,%#x\r\n",_stream_id,len,(unsigned char)buf[0],(unsigned char)buf[1]);
    Libnice *pLibnice=NULL;
    pLibnice = (Libnice *)data;
	if (NULL != pLibnice)
	{
        if (NULL != pLibnice->m_tLibniceCb.HandleRecvData)
        {//这里接收浏览器发出的报文(包括dtls协商报文)
             pLibnice->m_tLibniceCb.HandleRecvData(buf,len,pLibnice->m_tLibniceCb.pVideoDtlsObjCb,pLibnice->m_tLibniceCb.pWebRtcObjCb);
        }
	}
	//printf("%d,%.*s\r\n", len,len, buf);
	//fflush(stdout);
}

void Libnice::RecvAudioData(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data)
{
    printf("Libnice::RecvAudioData _stream_id%d,%d,%#x\n",_stream_id,len,buf[0]);
    Libnice *pLibnice=NULL;
    pLibnice = (Libnice *)data;
	if (NULL != pLibnice)
	{
        if (NULL != pLibnice->m_tLibniceCb.HandleRecvData)
        {//这里接收浏览器发出的报文(包括dtls协商报文)
             pLibnice->m_tLibniceCb.HandleRecvData(buf,len,pLibnice->m_tLibniceCb.pAudioDtlsObjCb,pLibnice->m_tLibniceCb.pWebRtcObjCb);
        }
	}
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
{//暂时用来传输应用数据,防止出错
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



