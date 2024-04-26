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

#include <regex.h>

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
    int Regex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch);
private:

};













#endif
