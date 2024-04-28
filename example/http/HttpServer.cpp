/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       HttpServer.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "HttpServer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "Http.h"

using std::cout;
using std::endl;


/*****************************************************************************
-Fuction		: HttpServer
-Description	: HttpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
HttpServer :: HttpServer()
{
    m_pHttp = new Http();
}

/*****************************************************************************
-Fuction		: ~HttpServer
-Description	: ~HttpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
HttpServer :: ~HttpServer()
{
    if(NULL != m_pHttp)
    {
        delete (Http *)m_pHttp;
    }
}


/*****************************************************************************
-Fuction		: ParseRequest
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpServer :: ParseRequest(char *i_pcReqData,int i_iDataLen,T_HttpReqPacket *o_ptHttpReqPacket)
{
    int iRet = -1;
    char *pBody = NULL;
    const char * strHttpBodyFlag = HTTP_CONTENT_FLAG;
	string strHttpHeader;
	string strFindRes;
	regmatch_t atMatch[HTTP_MAX_MATCH_NUM];
	const char *strFirstLinePatten="([A-Z]+) ([A-Za-z0-9/.]+) ([A-Z0-9/.]+)\r\n";
	const char *strConnectionPatten="Connection: ([A-Za-z0-9-]+)\r\n";
	const char *strContentLenPatten="Content-Length: ([0-9]+)\r\n";
	const char *strContentTypePatten="Content-type: ([A-Za-z0-9-/;.]+)\r\n";
	Http *pHttp = ((Http *)m_pHttp);
	
    if(NULL == i_pcReqData ||NULL == o_ptHttpReqPacket )
    {
        HTTP_LOGE("ParseRequest NULL\r\n");
        return iRet;
    }
    pBody = strstr(i_pcReqData,strHttpBodyFlag);
    if(NULL != pBody && HTTP_PACKET_BODY_MAX_LEN>=i_iDataLen-(pBody-i_pcReqData+strlen(strHttpBodyFlag)))
    {
        o_ptHttpReqPacket->iBodyLength = i_iDataLen-(pBody-i_pcReqData+strlen(strHttpBodyFlag));
        memcpy(o_ptHttpReqPacket->acBody,pBody+strlen(strHttpBodyFlag),o_ptHttpReqPacket->iBodyLength);
    }
    strHttpHeader.assign(i_pcReqData,0,pBody-i_pcReqData);

    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == pHttp->Regex(strFirstLinePatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);//0ÊÇÕûÐÐ
        snprintf(o_ptHttpReqPacket->strMethod,sizeof(o_ptHttpReqPacket->strMethod),"%s",strFindRes.c_str());
        strFindRes.assign(strHttpHeader,atMatch[2].rm_so,atMatch[2].rm_eo-atMatch[2].rm_so);
        snprintf(o_ptHttpReqPacket->strURL,sizeof(o_ptHttpReqPacket->strURL),"%s",strFindRes.c_str());
        strFindRes.assign(strHttpHeader,atMatch[3].rm_so,atMatch[3].rm_eo-atMatch[3].rm_so);
        snprintf(o_ptHttpReqPacket->strVersion,sizeof(o_ptHttpReqPacket->strVersion),"%s",strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == pHttp->Regex(strConnectionPatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(o_ptHttpReqPacket->strConnection,sizeof(o_ptHttpReqPacket->strConnection),"%s",strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == pHttp->Regex(strContentLenPatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        o_ptHttpReqPacket->iContentLength=atoi(strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == pHttp->Regex(strContentTypePatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(o_ptHttpReqPacket->strContentType,sizeof(o_ptHttpReqPacket->strContentType),"%s",strFindRes.c_str());
    }

    if(0 != strlen(o_ptHttpReqPacket->strMethod) && 0 != strlen(o_ptHttpReqPacket->strURL))
    {
        iRet = 0;
    }
    return iRet;
} 


/*****************************************************************************
-Fuction		: CreateResponse
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpServer :: CreateResponse(int i_iCode, const char * i_strMsg,const char * i_strVersion)
{
    int iRet = -1;
    char strRes[32];

    
    if(NULL == i_strMsg || NULL == i_strVersion)
    {
        HTTP_LOGE("CreateResponse NULL\r\n");
        return iRet;
    }
    iRet=snprintf(strRes,sizeof(strRes),"%s %d %s\r\n",i_strVersion,i_iCode,i_strMsg);
    strResHeader.assign(strRes);
    
    return iRet;
}


/*****************************************************************************
-Fuction		: SetResHeaderValue
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpServer :: SetResHeaderValue(const char *i_strKey,const char *i_strValue)
{
    int iRet = -1;
    char strRes[128];
    
    if(NULL == i_strKey || NULL == i_strValue)
    {
        HTTP_LOGE("SetResHeaderValue NULL\r\n");
        return iRet;
    }
    if(0 == strResHeader.length())
    {
        HTTP_LOGE("SetResHeaderValue strResHeader NULL\r\n");
        return iRet;
    }
    iRet=snprintf(strRes,sizeof(strRes),"%s: %s\r\n",i_strKey,i_strValue);
    strResHeader.append(strRes);
    
    return iRet;
}


/*****************************************************************************
-Fuction		: FormatResToStream
-Description	: 
-Input			: 
-Output 		: 
-Return 		: int *o_piBufLen,
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpServer :: FormatResToStream(char *i_pcContentData,int i_iDataLen,char *o_acBuf,int i_iBufMaxLen)
{
    int iRet = -1;
    char strRes[128];
    
    if(NULL == o_acBuf)
    {
        HTTP_LOGE("FormatResToStream NULL\r\n");
        return iRet;
    }
    if(0 == strResHeader.length())
    {
        HTTP_LOGE("FormatResToStream strResHeader NULL\r\n");
        return iRet;
    }
    if(strResHeader.length()+i_iDataLen+strlen("\r\n") > i_iBufMaxLen)
    {
        HTTP_LOGE("strResHeader->length()%d+i_iDataLen%d > i_iBufMaxLen%d err\r\n",strResHeader.length(),i_iDataLen,i_iBufMaxLen);
        return iRet;
    }
    iRet=strResHeader.length();
    memcpy(o_acBuf,(char *)strResHeader.c_str(),iRet);
    iRet+=snprintf(o_acBuf+iRet,i_iBufMaxLen-iRet,"%s","\r\n");
    if(NULL != i_pcContentData)
    {
        memcpy(o_acBuf+iRet,i_pcContentData,i_iDataLen);
        iRet+=i_iDataLen;
    }
    
    return iRet;
}


