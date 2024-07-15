/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Http.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef HTTP_H
#define HTTP_H

#include "HttpCommon.h"
#include <regex>
#include <string>

using std::string;
using std::smatch;
using std::regex;



#define HTTP_MAX_MATCH_NUM       8

/*****************************************************************************
-Class			: Http
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class Http
{
public:
	Http();
	~Http();
    int ParseReqHeader(string *i_strHttpHeader,T_HttpReqPacket *o_ptHttpReqPacket);
    int ParseResHeader(string *i_strHttpHeader,T_HttpResPacket *o_ptHttpResPacket);
    int Regex(const char *i_strPattern,string *i_strBuf,smatch &o_Match);
private:

};













#endif
