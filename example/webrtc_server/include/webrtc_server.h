/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       WebRtcServer.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_SERVER_H
#define WEBRTC_SERVER_H

#include <mutex>
#include <string>
#include <list>
#include <map>
#include "webrtc_http_session.h"

using std::map;
using std::string;
using std::list;
using std::mutex;

/*****************************************************************************
-Class			: WebRtcServer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class WebRtcServer : public TcpServer
{
public:
	WebRtcServer(int i_iServerPort);
	virtual ~WebRtcServer();
    int Proc(char * i_strStunAddr,unsigned int i_dwStunPort);
    int DelHttpSession(void *i_pSrcIoHandle,int i_iClientSocketFd);
    
private:
    static int HttpSessionExit(void *i_pSrcIoHandle,int i_iClientSocketFd,void *i_pIoHandle);
    int CheckHttpSession();
    int DelMapHttpSession(int i_iClientSocketFd,WebRtcHttpSession * i_pWebRtcHttpSession);
    int AddMapHttpSession(WebRtcHttpSession * i_pWebRtcHttpSession,int i_iClientSocketFd);
    
    map<int, WebRtcHttpSession *>  m_DelHttpSessionMap;
    mutex m_DelMapMtx;
    map<int, WebRtcHttpSession *>  m_HttpSessionMap;
    mutex m_MapMtx;
};

#endif
