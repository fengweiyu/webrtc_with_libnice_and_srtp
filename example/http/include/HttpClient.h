/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       HttpClient.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "HttpCommon.h"

#if 0
/*****************************************************************************
-Class			: HttpClient
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class HttpClient
{
public:
	HttpClient();
	~HttpClient();
	int Init(char * i_strIP,unsigned short i_wPort);
	int Send(const char * i_srtMethod,char * i_srtURL,char * i_acSendBuf,int i_iSendLen,const char * i_srtContentType=NULL);
	int RecvBody(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen);
	int Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen);
private:
	TcpClient *m_pTcpClient;
	string m_strServerIp;
	int m_iServerPort;
};




#endif








#endif
