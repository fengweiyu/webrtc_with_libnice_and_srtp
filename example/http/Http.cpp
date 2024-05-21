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
#include <regex.h> //C++ regexҪ��gcc 4.9���ϰ汾������linux������c��
/*****************************************************************************
-Fuction		: Regex
-Description	: ������ʽ
.��				ƥ�����\r\n��֮����κε����ַ�
*				ƥ��ǰ����ӱ��ʽ����Ρ����磬zo*��ƥ�䡰z����Ҳ��ƥ�䡰zo���Լ���zoo����*�ȼ���o{0,}
				����.*��ƥ��������洢�����������
(pattern)		ƥ��ģʽ��pattern����ȡ��һƥ�䡣����ȡ��ƥ����ԴӲ�����Matches���ϵõ�
[xyz]			�ַ����ϡ�ƥ��������������һ���ַ������磬��[abc]������ƥ�䡰plain���еġ�a����
+				ƥ��ǰ����ӱ��ʽһ�λ���(���ڵ���1�Σ������磬��zo+����ƥ�䡰zo���Լ���zoo����������ƥ�䡰z����+�ȼ���{1,}��
				//���������в���+��Ĭ����һ�Σ���ֻ��ƥ�䵽һ������6
				
[A-Za-z0-9] 	26����д��ĸ��26��Сд��ĸ��0��9����
[A-Za-z0-9+/=]	26����д��ĸ��26��Сд��ĸ0��9�����Լ�+/= �����ַ�


-Input			: i_strPattern ģʽ��,i_strBuf��ƥ���ַ���,
-Output 		: o_ptMatch �洢ƥ�䴮λ�õ�����,���ڴ洢ƥ�����ڴ�ƥ�䴮�е��±귶Χ
//����0��Ԫ�����������ʽƥ������λ��,�������������������ƥ��������ߵĵ�Ԫ���δ����������ʽƥ������λ��
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/01	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpRegex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch)
{
    char acErrBuf[256];
    int iRet=-1;
    regex_t tReg;    //����һ������ʵ��
    //const size_t dwMatch = 6;    //����ƥ�������������       //��ʾ������ƥ��


    //REG_ICASE ƥ����ĸʱ���Դ�Сд��
    iRet =regcomp(&tReg, i_strPattern, REG_EXTENDED);    //��������ģʽ��
    if(iRet != 0) 
    {
        regerror(iRet, &tReg, acErrBuf, sizeof(acErrBuf));
        HTTP_LOGE("Regex Error:\r\n");
    }
    else
    {
        iRet = regexec(&tReg, i_strBuf, HTTP_MAX_MATCH_NUM, o_ptMatch, 0); //ƥ����
        if (iRet == REG_NOMATCH)
        { //���ûƥ����
            HTTP_LOGE("Regex No Match!\r\n");
        }
        else if (iRet == REG_NOERROR)
        { //���ƥ������
            HTTP_LOGD("Match\r\n");
            int i=0,j=0;
			for(j=0;j<HTTP_MAX_MATCH_NUM;j++)
			{
				for (i= o_ptMatch[j].rm_so; i < o_ptMatch[j].rm_eo; i++)
				{ //�������ƥ�䷶Χ���ַ���
					printf("%c", i_strBuf[i]);
				}
				printf("\n");
			}
        }
        else
        {
            HTTP_LOGE("Regex Unknow err:\r\n");
        }
        regfree(&tReg);  //�ͷ�������ʽ
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
	
    if(NULL == i_strHttpHeader)
    {
        HTTP_LOGE("HttpParseReqHeader NULL\r\n");
        return iRet;
    }
    strHttpHeader.assign(i_strHttpHeader->c_str());
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == HttpRegex(strFirstLinePatten,(char *)strHttpHeader.c_str(),atMatch))
    {
        strFindRes.assign(strHttpHeader,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);//0������
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

    if(0 != strlen(o_ptHttpReqPacket->strMethod) && 0 != strlen(o_ptHttpReqPacket->strURL))
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
-Description	: ������ʽ
.��				ƥ�����\r\n��֮����κε����ַ�
*				ƥ��ǰ����ӱ��ʽ����Ρ����磬zo*��ƥ�䡰z����Ҳ��ƥ�䡰zo���Լ���zoo����*�ȼ���o{0,}
				����.*��ƥ��������洢�����������
(pattern)		ƥ��ģʽ��pattern����ȡ��һƥ�䡣����ȡ��ƥ����ԴӲ�����Matches���ϵõ�
[xyz]			�ַ����ϡ�ƥ��������������һ���ַ������磬��[abc]������ƥ�䡰plain���еġ�a����
+				ƥ��ǰ����ӱ��ʽһ�λ���(���ڵ���1�Σ������磬��zo+����ƥ�䡰zo���Լ���zoo����������ƥ�䡰z����+�ȼ���{1,}��
				//���������в���+��Ĭ����һ�Σ���ֻ��ƥ�䵽һ������6
				
[A-Za-z0-9] 	26����д��ĸ��26��Сд��ĸ��0��9����
[A-Za-z0-9+/=]	26����д��ĸ��26��Сд��ĸ0��9�����Լ�+/= �����ַ�


-Input			: i_strPattern ģʽ��,i_strBuf��ƥ���ַ���,
-Output 		: o_ptMatch �洢ƥ�䴮λ�õ�����,���ڴ洢ƥ�����ڴ�ƥ�䴮�е��±귶Χ
//����0��Ԫ�����������ʽƥ������λ��,�������������������ƥ��������ߵĵ�Ԫ���δ����������ʽƥ������λ��
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/01	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Http::Regex(const char *i_strPattern,string *i_strBuf,smatch &o_Match)
{//����gcc 4.9���ϵİ汾
    char acErrBuf[256];
    int iRet=-1;

    // ����������ʽģʽ
    regex Pattern(i_strPattern);//, std::regex_constants::extended ��std::regex_constants::basic����Ч
    // ʹ�� std::regex_search ����ƥ�������
    if (std::regex_search(*i_strBuf, o_Match, Pattern)) //ĳ��ƥ��ʧ�ܣ�std::smatch���󽫲��ᱣ����һ�ε�ƥ����
    {//o_Matchָ����Ӵ��ڴ���i_strBuf�У�������Ҫ��֤i_strBuf�������򣬼�i_strBuf�����ȱ��ͷŵ�
        // �����һ��ƥ�䵽���Ӵ�
        std::cout << "Match found: " <<o_Match.size()<< o_Match.str() << std::endl;

        // �������ƥ�䵽���Ӵ�
        for (size_t i = 0; i < o_Match.size(); ++i) 
        {
            std::string matchStr = o_Match[i].str(); // �� std::sub_match ת��Ϊ�ַ���
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
	
    if(NULL == i_strHttpHeader ||NULL == o_ptHttpReqPacket )
    {
        HTTP_LOGE("ParseReqHeader NULL\r\n");
        return iRet;
    }
    
    iRet = HttpParseReqHeader(i_strHttpHeader,o_ptHttpReqPacket);//C++ regexҪ��gcc 4.9���ϰ汾������linux������c��
    if(HTTP_UN_SUPPORT_FUN !=   iRet)
    {
        return iRet;
    }

    
    strHttpHeader.assign(i_strHttpHeader->c_str());
    if(0 == this->Regex(strFirstLinePatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0������
        snprintf(o_ptHttpReqPacket->strMethod,sizeof(o_ptHttpReqPacket->strMethod),"%s",strFindRes.c_str());
        strFindRes.assign(Match[2].str());
        snprintf(o_ptHttpReqPacket->strURL,sizeof(o_ptHttpReqPacket->strURL),"%s",strFindRes.c_str());
        strFindRes.assign(Match[3].str());
        snprintf(o_ptHttpReqPacket->strVersion,sizeof(o_ptHttpReqPacket->strVersion),"%s",strFindRes.c_str());
    }
    //memset(atMatch,0,sizeof(atMatch));//ĳ��ƥ��ʧ�ܣ�std::smatch���󽫲��ᱣ����һ�ε�ƥ����
    if(0 == this->Regex(strConnectionPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0������
        snprintf(o_ptHttpReqPacket->strConnection,sizeof(o_ptHttpReqPacket->strConnection),"%s",strFindRes.c_str());
    }
    if(0 == this->Regex(strContentLenPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0������
        o_ptHttpReqPacket->iContentLength=atoi(strFindRes.c_str());
    }
    if(0 == this->Regex(strContentTypePatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0������
        snprintf(o_ptHttpReqPacket->strContentType,sizeof(o_ptHttpReqPacket->strContentType),"%s",strFindRes.c_str());
    }

    if(0 != strlen(o_ptHttpReqPacket->strMethod) && 0 != strlen(o_ptHttpReqPacket->strURL))
    {
        iRet = 0;
    }
    return iRet;
} 

