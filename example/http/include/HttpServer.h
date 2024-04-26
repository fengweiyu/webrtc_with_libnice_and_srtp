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
	int CreateResponse(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen);
private:
    int Regex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch);//·ÅHTTP
};













#endif
