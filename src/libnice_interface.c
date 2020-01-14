/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       libnice_interface.c
* Description           : 	
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
#include <ctype.h>
#include <agent.h>

#include <gio/gnetworking.h>

static void LibniceCandidateGatheringDone(NiceAgent *i_ptAgent, guint i_dwStreamID,gpointer pData);
static void LibniceNewSelectPair(NiceAgent *agent, guint _stream_id,guint component_id, gchar *lfoundation,gchar *rfoundation, gpointer data);
static void LibniceComponentStateChanged(NiceAgent *agent, guint _stream_id,guint component_id, guint state,gpointer data);
static void LibniceRecv(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data);

static T_LocalCandidate g_tLocalCandidate;

/*****************************************************************************
-Fuction        : LibniceInit
-Description    : 会阻塞,线程函数
-Input          : i_iControlling 感觉不必要
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceInit(char * i_strStunAddr,unsigned int i_dwStunPort,int i_iControlling)
{
	int iRet = -1;
    static GMainLoop *ptLoop=NULL;//
    static GIOChannel* ptStdinIO=NULL;//
	NiceAgent *ptAgent=NULL;
    unsigned int dwStreamID=0;//

    memset(&g_tLocalCandidate,0,sizeof(T_LocalCandidate));
	g_networking_init();//
	ptLoop = g_main_loop_new(NULL, FALSE);//
	ptStdinIO = g_io_channel_unix_new(fileno(stdin));//

	// Create the nice agent
	ptAgent = nice_agent_new(g_main_loop_get_context (ptLoop),NICE_COMPATIBILITY_RFC5245);
	if (ptAgent == NULL)
		g_error("Failed to create agent");

	// Set the STUN settings and controlling mode
	if (i_strStunAddr) 
	{
		g_object_set(ptAgent, "stun-server", i_strStunAddr, NULL);
		g_object_set(ptAgent, "stun-server-port", i_dwStunPort, NULL);
	}
	g_object_set(ptAgent, "controlling-mode", i_iControlling, NULL);

	// Connect to the signals
	g_signal_connect(ptAgent, "candidate-gathering-done",G_CALLBACK(LibniceCandidateGatheringDone), NULL);
	g_signal_connect(ptAgent, "new-selected-pair",G_CALLBACK(LibniceNewSelectPair), NULL);//
	g_signal_connect(ptAgent, "component-state-changed",G_CALLBACK(LibniceComponentStateChanged), NULL);//

	// Create a new stream with one component
	dwStreamID = nice_agent_add_stream(ptAgent, 1);//
	if (dwStreamID == 0)
		g_error("Failed to add stream");

	// Attach to the component to receive the data
	// Without this call, candidates cannot be gathered
	nice_agent_attach_recv(ptAgent, dwStreamID, 1,g_main_loop_get_context (ptLoop), LibniceRecv, NULL);//

	// Start gathering local candidates
	if (!nice_agent_gather_candidates(ptAgent, dwStreamID))
		g_error("Failed to start candidate gathering");

	g_debug("waiting for candidate-gathering-done signal...");

	// Run the mainloop. Everything else will happen asynchronously
	// when the candidates are done gathering.
	g_main_loop_run (ptLoop);//

	g_main_loop_unref(ptLoop);//
	g_object_unref(ptAgent);//
	g_io_channel_unref (ptStdinIO);//
}
/*****************************************************************************
-Fuction        : PrintUsage
-Description    : PrintUsage
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceGetLocalCandidate(T_LocalCandidate * i_ptLocalCandidate)
{
	int iRet=-1;
	while(g_tLocalCandidate.iGatheringDoneFlag == 0)
	{

	}
	memcpy(i_ptLocalCandidate,&g_tLocalCandidate,sizeof(T_LocalCandidate));
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
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceSetRemoteCandidate(char * i_strStunAddr,unsigned int i_dwStunPort)
{



}


static void LibniceCandidateGatheringDone(NiceAgent *i_ptAgent, guint i_dwStreamID,gpointer pData)
{
  g_debug("SIGNAL candidate gathering done\n");

  // Candidate gathering is done. Send our local candidates on stdout
  int iRet = -1;
  gchar *strLocalUfrag = NULL;
  gchar *strLocalPassword = NULL;
  gchar ipaddr[INET6_ADDRSTRLEN];
  GSList *cands = NULL, *item = NULL;
  static const gchar *candidate_type_name[] = {"host", "srflx", "prflx", "relay"};
  
  if (!nice_agent_get_local_credentials(i_ptAgent, i_dwStreamID,&strLocalUfrag, &strLocalPassword))
      goto end;
  cands = nice_agent_get_local_candidates(i_ptAgent, i_dwStreamID, 1);
  if (cands == NULL)
      goto end;
  
  printf("%s %s", strLocalUfrag, strLocalPassword);
  
  for (item = cands; item; item = item->next) 
  {
      NiceCandidate *c = (NiceCandidate *)item->data;
  
      nice_address_to_string(&c->addr, ipaddr);
  
      // (foundation),(prio),(addr),(port),(type)
      //"candidate:3442447574 1 udp 2122260223 192.168.0.170 54653 typ host generation 0 ufrag gX6M network-id 1"
      snprintf(g_tLocalCandidate.strCandidateData,sizeof(g_tLocalCandidate.strCandidateData),
      " %s,%u,%s,%u,%s",
      c->foundation,
      c->priority,
      ipaddr,
      nice_address_get_port(&c->addr),
      candidate_type_name[c->type]);
  }
  g_tLocalCandidate.iGatheringDoneFlag = 1;
  iRet = 0;
  
  end:
      if (strLocalUfrag)
          g_free(strLocalUfrag);
      if (strLocalPassword)
          g_free(strLocalPassword);
      if (cands)
          g_slist_free_full(cands, (GDestroyNotify)&nice_candidate_free);
  return iRet;
}

 static void LibniceNewSelectPair(NiceAgent *agent, guint _stream_id,guint component_id, gchar *lfoundation,gchar *rfoundation, gpointer data)
{
	g_debug("SIGNAL: selected pair %s %s", lfoundation, rfoundation);
}

static void LibniceComponentStateChanged(NiceAgent *agent, guint _stream_id,guint component_id, guint state,gpointer data)
{
	static const gchar *state_name[] = {"disconnected", "gathering", "connecting","connected", "ready", "failed"};
    
	g_debug("SIGNAL: state changed %d %d %s[%d]\n",_stream_id, component_id, state_name[state], state);

	if (state == NICE_COMPONENT_STATE_CONNECTED) 
	{
	
	}
}

static void LibniceRecv(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data)
{
	if (len == 1 && buf[0] == '\0')
		g_main_loop_quit (gloop);
	printf("%.*s", len, buf);
	fflush(stdout);
}







