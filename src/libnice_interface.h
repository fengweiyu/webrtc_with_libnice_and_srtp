/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef LIBNICE_INTERFACE_H
#define LIBNICE_INTERFACE_H


typedef struct LocalCandidate
{
	int iGatheringDoneFlag;//0 未收集,1收集成功
	char strCandidateData[256];
//"candidate:3442447574 1 udp 2122260223 192.168.0.170 54653 typ host generation 0 ufrag gX6M network-id 1"
}T_LocalCandidate;



int LibniceInit(char * i_strStunAddr,unsigned int i_dwStunPort,int i_iControlling);
int LibniceGetLocalCandidate(T_LocalCandidate * i_ptLocalCandidate);
int LibniceSetRemoteCandidate(char * i_strStunAddr,unsigned int i_dwStunPort);





#endif
