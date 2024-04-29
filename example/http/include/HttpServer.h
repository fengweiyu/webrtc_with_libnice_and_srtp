/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       HttpServer.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "HttpCommon.h"
#include <string>


using std::string;

/*****************************************************************************
-Class			: HttpServer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class HttpServer
{
public:
	HttpServer();
	~HttpServer();
    int ParseRequest(char *i_pcReqData,int i_iDataLen,T_HttpReqPacket *o_ptHttpReqPacket);
	int CreateResponse(int i_iCode = 200, const char * i_strMsg = "OK",const char * i_strVersion = HTTP_VERSION);
	int SetResHeaderValue(const char *i_strKey,const char *i_strValue);
    int SetResHeaderValue(const char *i_strKey,int i_iValue);
	int FormatResToStream(char *i_pcContentData,int i_iDataLen,char *o_acBuf,int i_iBufMaxLen);
private:
    string strResHeader;
    string strResponse;
    void *m_pHttp;
};













#endif
