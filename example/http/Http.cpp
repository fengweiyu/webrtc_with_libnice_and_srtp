/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Http.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "Http.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using std::cout;
using std::endl;

#define HTTP_UN_SUPPORT_FUN (-2)

#ifndef _WIN32
#include <regex.h> //C++ regex要求gcc 4.9以上版本，所以linux还是用c的
/*****************************************************************************
-Fuction		: Regex
-Description	: 正则表达式
.点				匹配除“\r\n”之外的任何单个字符
*				匹配前面的子表达式任意次。例如，zo*能匹配“z”，也能匹配“zo”以及“zoo”。*等价于o{0,}
				其中.*的匹配结果不会存储到结果数组里
(pattern)		匹配模式串pattern并获取这一匹配。所获取的匹配可以从产生的Matches集合得到
[xyz]			字符集合。匹配所包含的任意一个字符。例如，“[abc]”可以匹配“plain”中的“a”。
+				匹配前面的子表达式一次或多次(大于等于1次）。例如，“zo+”能匹配“zo”以及“zoo”，但不能匹配“z”。+等价于{1,}。
				//如下例子中不用+，默认是一次，即只能匹配到一个数字6
				
[A-Za-z0-9] 	26个大写字母、26个小写字母和0至9数字
[A-Za-z0-9+/=]	26个大写字母、26个小写字母0至9数字以及+/= 三个字符


-Input			: i_strPattern 模式串,i_strBuf待匹配字符串,
-Output 		: o_ptMatch 存储匹配串位置的数组,用于存储匹配结果在待匹配串中的下标范围
//数组0单元存放主正则表达式匹配结果的位置,即所有正则组合起来的匹配结果，后边的单元依次存放子正则表达式匹配结果的位置
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/01	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpRegex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch)
{
    char acErrBuf[256];
    int iRet=-1;
    regex_t tReg;    //定义一个正则实例
    //const size_t dwMatch = 6;    //定义匹配结果最大允许数       //表示允许几个匹配


    //REG_ICASE 匹配字母时忽略大小写。
    iRet =regcomp(&tReg, i_strPattern, REG_EXTENDED);    //编译正则模式串
    if(iRet != 0) 
    {
        regerror(iRet, &tReg, acErrBuf, sizeof(acErrBuf));
        HTTP_LOGE("Regex Error:\r\n");
    }
    else
    {
        iRet = regexec(&tReg, i_strBuf, HTTP_MAX_MATCH_NUM, o_ptMatch, 0); //匹配他
        if (iRet == REG_NOMATCH)
        { //如果没匹配上
            HTTP_LOGE("Regex No Match!\r\n");
        }
        else if (iRet == REG_NOERROR)
        { //如果匹配上了
            HTTP_LOGD("Match\r\n");
            int i=0,j=0;
			for(j=0;j<HTTP_MAX_MATCH_NUM;j++)
			{
				for (i= o_ptMatch[j].rm_so; i < o_ptMatch[j].rm_eo; i++)
				{ //遍历输出匹配范围的字符串
					//printf("%c", i_strBuf[i]);
				}
				//printf("\n");
			}
        }
        else
        {
            HTTP_LOGE("Regex Unknow err:\r\n");
        }
        regfree(&tReg);  //释放正则表达式
    }
    
    return iRet;
}
/*****************************************************************************
-Fuction		: HttpParseReqHeader
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpParseReqHeader(string *i_strHttpHeader,T_HttpReqPacket *o_ptHttpReqPacket)
{
    int iRet = -1;
	string strHttpHeader;
	string strFindRes;
	regmatch_t atMatch[HTTP_MAX_MATCH_NUM];
	const char *strFirstLinePatten="([A-Z]+) ([A-Za-z0-9/._]+) ([A-Z0-9/.]+)\r\n";
	const char *strConnectionPatten="Connection: ([A-Za-z0-9-]+)\r\n";
	const char *strContentLenPatten="Content-Length: ([0-9]+)\r\n";
	const char *strContentTypePatten="Content-type: ([A-Za-z0-9-/;.]+)\r\n";
	const char *strUserAgentPatten="User-Agent: ([A-Za-z0-9-/;. ]+)\r\n";

    if(NULL == i_strHttpHeader)
    {
        HTTP_LOGE("HttpParseReqHeader NULL\r\n");
        return iRet;
    }
    strHttpHeader.assign(i_strHttpHeader->c_str());
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strFirstLinePatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);//0是整行
        snprintf(o_ptHttpReqPacket->strMethod,sizeof(o_ptHttpReqPacket->strMethod),"%s",strFindRes.c_str());
        strFindRes.assign(strHttpHeader,atMatch[2].rm_so,atMatch[2].rm_eo-atMatch[2].rm_so);
        snprintf(o_ptHttpReqPacket->strURL,sizeof(o_ptHttpReqPacket->strURL),"%s",strFindRes.c_str());
        strFindRes.assign(strHttpHeader,atMatch[3].rm_so,atMatch[3].rm_eo-atMatch[3].rm_so);
        snprintf(o_ptHttpReqPacket->strVersion,sizeof(o_ptHttpReqPacket->strVersion),"%s",strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strConnectionPatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(o_ptHttpReqPacket->strConnection,sizeof(o_ptHttpReqPacket->strConnection),"%s",strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strContentLenPatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        o_ptHttpReqPacket->iContentLength=atoi(strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strContentTypePatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(o_ptHttpReqPacket->strContentType,sizeof(o_ptHttpReqPacket->strContentType),"%s",strFindRes.c_str());
    }
    if(REG_NOERROR == HttpRegex(strUserAgentPatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(o_ptHttpReqPacket->strUserAgent,sizeof(o_ptHttpReqPacket->strUserAgent),"%s",strFindRes.c_str());
    }

    if(0 != strlen(o_ptHttpReqPacket->strMethod) && 0 != strlen(o_ptHttpReqPacket->strURL))
    {
        iRet = 0;
    }
    return iRet;
} 
/*****************************************************************************
-Fuction		: HttpParseReqHeader
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpParseResHeader(string *i_strHttpHeader,T_HttpResPacket *o_ptHttpResPacket)
{
    int iRet = -1;
	string strHttpHeader;
	string strFindRes;
	regmatch_t atMatch[HTTP_MAX_MATCH_NUM];
	const char *strFirstLinePatten="([A-Za-z0-9/._]+) ([0-9]+) ([A-Za-z0-9/.]+)\r\n";
	const char *strConnectionPatten="Connection: ([A-Za-z0-9-]+)\r\n";
	const char *strContentLenPatten="Content-Length: ([0-9]+)\r\n";
	const char *strContentTypePatten="Content-type: ([A-Za-z0-9-/;.]+)\r\n";

    if(NULL == i_strHttpHeader)
    {
        HTTP_LOGE("HttpParseResHeader NULL\r\n");
        return iRet;
    }
    strHttpHeader.assign(i_strHttpHeader->c_str());
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strFirstLinePatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);//0是整行
        snprintf(o_ptHttpResPacket->strVersion,sizeof(o_ptHttpResPacket->strVersion),"%s",strFindRes.c_str());
        strFindRes.assign(strHttpHeader,atMatch[2].rm_so,atMatch[2].rm_eo-atMatch[2].rm_so);
        o_ptHttpResPacket->iStatusCode=atoi(strFindRes.c_str());
        strFindRes.assign(strHttpHeader,atMatch[3].rm_so,atMatch[3].rm_eo-atMatch[3].rm_so);
        snprintf(o_ptHttpResPacket->strStatusMsg,sizeof(o_ptHttpResPacket->strStatusMsg),"%s",strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strConnectionPatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(o_ptHttpResPacket->strConnection,sizeof(o_ptHttpResPacket->strConnection),"%s",strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strContentLenPatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        o_ptHttpResPacket->iContentLength=atoi(strFindRes.c_str());
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strContentTypePatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(o_ptHttpResPacket->strContentType,sizeof(o_ptHttpResPacket->strContentType),"%s",strFindRes.c_str());
    }

    if(0 != strlen(o_ptHttpResPacket->strVersion) && 0 != strlen(o_ptHttpResPacket->strStatusMsg))
    {
        iRet = 0;
    }
    return iRet;
} 

#else
/*****************************************************************************
-Fuction		: HttpParseReqHeader
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpParseReqHeader(string *i_strHttpHeader,T_HttpReqPacket *o_ptHttpReqPacket)
{
    return HTTP_UN_SUPPORT_FUN;
} 
/*****************************************************************************
-Fuction		: HttpParseReqHeader
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpParseResHeader(string *i_strHttpHeader,T_HttpResPacket *o_ptHttpResPacket)
{
    return HTTP_UN_SUPPORT_FUN;
} 

#endif
/*****************************************************************************
-Fuction		: Http
-Description	: Http
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Http :: Http()
{

}

/*****************************************************************************
-Fuction		: ~Http
-Description	: ~Http
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Http :: ~Http()
{
}

/*****************************************************************************
-Fuction		: Regex
-Description	: 正则表达式
.点				匹配除“\r\n”之外的任何单个字符
*				匹配前面的子表达式任意次。例如，zo*能匹配“z”，也能匹配“zo”以及“zoo”。*等价于o{0,}
				其中.*的匹配结果不会存储到结果数组里
(pattern)		匹配模式串pattern并获取这一匹配。所获取的匹配可以从产生的Matches集合得到
[xyz]			字符集合。匹配所包含的任意一个字符。例如，“[abc]”可以匹配“plain”中的“a”。
+				匹配前面的子表达式一次或多次(大于等于1次）。例如，“zo+”能匹配“zo”以及“zoo”，但不能匹配“z”。+等价于{1,}。
				//如下例子中不用+，默认是一次，即只能匹配到一个数字6
				
[A-Za-z0-9] 	26个大写字母、26个小写字母和0至9数字
[A-Za-z0-9+/=]	26个大写字母、26个小写字母0至9数字以及+/= 三个字符


-Input			: i_strPattern 模式串,i_strBuf待匹配字符串,
-Output 		: o_ptMatch 存储匹配串位置的数组,用于存储匹配结果在待匹配串中的下标范围
//数组0单元存放主正则表达式匹配结果的位置,即所有正则组合起来的匹配结果，后边的单元依次存放子正则表达式匹配结果的位置
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/01	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Http::Regex(const char *i_strPattern,string *i_strBuf,smatch &o_Match)
{//依赖gcc 4.9以上的版本
    char acErrBuf[256];
    int iRet=-1;

    // 定义正则表达式模式
    regex Pattern(i_strPattern);//, std::regex_constants::extended 和std::regex_constants::basic都无效
    // 使用 std::regex_search 查找匹配的内容
    if (std::regex_search(*i_strBuf, o_Match, Pattern)) //某次匹配失败，std::smatch对象将不会保存上一次的匹配结果
    {//o_Match指向的子串内存在i_strBuf中，所以需要保证i_strBuf的作用域，即i_strBuf不能先被释放掉
        // 输出第一个匹配到的子串
        std::cout << "Match found: " <<o_Match.size()<< o_Match.str() << std::endl;

        // 输出所有匹配到的子串
        for (size_t i = 0; i < o_Match.size(); ++i) 
        {
            std::string matchStr = o_Match[i].str(); // 将 std::sub_match 转换为字符串
            std::cout << "Match " << i << ": " << matchStr << std::endl;
        }
        iRet = 0;
    } 
    else 
    {
        std::cout << "No match found." << std::endl;
    }
    
    return iRet;
}


/*****************************************************************************
-Fuction		: ParseReqHeader
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Http :: ParseReqHeader(string *i_strHttpHeader,T_HttpReqPacket *o_ptHttpReqPacket)
{
    int iRet = -1;
	string strHttpHeader;
	string strFindRes;
	smatch Match;
	const char *strFirstLinePatten="([A-Z]+) ([A-Za-z0-9/._]+) ([A-Z0-9/.]+)\r\n";
	const char *strConnectionPatten="Connection: ([A-Za-z0-9-]+)\r\n";
	const char *strContentLenPatten="Content-Length: ([0-9]+)\r\n";
	const char *strContentTypePatten="Content-type: ([A-Za-z0-9-/;.]+)\r\n";
	const char *strUserAgentPatten="User-Agent: ([A-Za-z0-9-/;. ]+)\r\n";

    if(NULL == i_strHttpHeader ||NULL == o_ptHttpReqPacket )
    {
        HTTP_LOGE("ParseReqHeader NULL\r\n");
        return iRet;
    }
    
    iRet = HttpParseReqHeader(i_strHttpHeader,o_ptHttpReqPacket);//C++ regex要求gcc 4.9以上版本，所以linux还是用c的
    if(HTTP_UN_SUPPORT_FUN !=   iRet)
    {
        return iRet;
    }

    
    strHttpHeader.assign(i_strHttpHeader->c_str());
    if(0 == this->Regex(strFirstLinePatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpReqPacket->strMethod,sizeof(o_ptHttpReqPacket->strMethod),"%s",strFindRes.c_str());
        strFindRes.assign(Match[2].str());
        snprintf(o_ptHttpReqPacket->strURL,sizeof(o_ptHttpReqPacket->strURL),"%s",strFindRes.c_str());
        strFindRes.assign(Match[3].str());
        snprintf(o_ptHttpReqPacket->strVersion,sizeof(o_ptHttpReqPacket->strVersion),"%s",strFindRes.c_str());
    }
    //memset(atMatch,0,sizeof(atMatch));//某次匹配失败，std::smatch对象将不会保存上一次的匹配结果
    if(0 == this->Regex(strConnectionPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpReqPacket->strConnection,sizeof(o_ptHttpReqPacket->strConnection),"%s",strFindRes.c_str());
    }
    if(0 == this->Regex(strContentLenPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        o_ptHttpReqPacket->iContentLength=atoi(strFindRes.c_str());
    }
    if(0 == this->Regex(strContentTypePatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpReqPacket->strContentType,sizeof(o_ptHttpReqPacket->strContentType),"%s",strFindRes.c_str());
    }
    if(0 == this->Regex(strUserAgentPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpReqPacket->strUserAgent,sizeof(o_ptHttpReqPacket->strUserAgent),"%s",strFindRes.c_str());
    }

    if(0 != strlen(o_ptHttpReqPacket->strMethod) && 0 != strlen(o_ptHttpReqPacket->strURL))
    {
        iRet = 0;
    }
    return iRet;
} 

/*****************************************************************************
-Fuction		: ParseReqHeader
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Http :: ParseResHeader(string *i_strHttpHeader,T_HttpResPacket *o_ptHttpResPacket)
{
    int iRet = -1;
	string strHttpHeader;
	string strFindRes;
	smatch Match;
	const char *strFirstLinePatten="([A-Za-z0-9/._]+) ([0-9]+) ([A-Za-z0-9/.]+)\r\n";
	const char *strConnectionPatten="Connection: ([A-Za-z0-9-]+)\r\n";
	const char *strContentLenPatten="Content-Length: ([0-9]+)\r\n";
	const char *strContentTypePatten="Content-type: ([A-Za-z0-9-/;.]+)\r\n";

    if(NULL == i_strHttpHeader ||NULL == o_ptHttpResPacket )
    {
        HTTP_LOGE("ParseResHeader NULL\r\n");
        return iRet;
    }
    
    iRet = HttpParseResHeader(i_strHttpHeader,o_ptHttpResPacket);//C++ regex要求gcc 4.9以上版本，所以linux还是用c的
    if(HTTP_UN_SUPPORT_FUN !=   iRet)
    {
        return iRet;
    }

    
    strHttpHeader.assign(i_strHttpHeader->c_str());
    if(0 == this->Regex(strFirstLinePatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpResPacket->strVersion,sizeof(o_ptHttpResPacket->strVersion),"%s",strFindRes.c_str());
        strFindRes.assign(Match[2].str());
        o_ptHttpResPacket->iStatusCode=atoi(strFindRes.c_str());
        strFindRes.assign(Match[3].str());
        snprintf(o_ptHttpResPacket->strStatusMsg,sizeof(o_ptHttpResPacket->strStatusMsg),"%s",strFindRes.c_str());
    }
    //memset(atMatch,0,sizeof(atMatch));//某次匹配失败，std::smatch对象将不会保存上一次的匹配结果
    if(0 == this->Regex(strConnectionPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpResPacket->strConnection,sizeof(o_ptHttpResPacket->strConnection),"%s",strFindRes.c_str());
    }
    if(0 == this->Regex(strContentLenPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        o_ptHttpResPacket->iContentLength=atoi(strFindRes.c_str());
    }
    if(0 == this->Regex(strContentTypePatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpResPacket->strContentType,sizeof(o_ptHttpResPacket->strContentType),"%s",strFindRes.c_str());
    }

    if(0 != strlen(o_ptHttpResPacket->strVersion) && 0 != strlen(o_ptHttpResPacket->strStatusMsg))
    {
        iRet = 0;
    }
    return iRet;
} 

