/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       HttpClient.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "HttpClient.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>


using std::cout;
using std::endl;

/*****************************************************************************
-Fuction		: HttpClient
-Description	: HttpClient
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
HttpServer :: HttpServer()
{

}

/*****************************************************************************
-Fuction		: ~HttpClient
-Description	: ~HttpClient
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
HttpServer :: ~HttpClient()
{

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
    
    if(NULL == i_pcReqData ||NULL == o_ptHttpReqPacket )
    {
        printf("ParseRequest NULL\r\n");
        return iRet;
    }
    
    if(NULL != m_pTcpClient)
    {
        delete m_pTcpClient;
        m_pTcpClient =NULL;
    }
    m_pTcpClient = new TcpClient();//http短连接从新建立连接
    if(NULL == m_pTcpClient)
    {
        printf("HttpClient :: Send NULL\r\n");
        return iRet;
    }
    m_pTcpClient->Init(m_strServerIp,m_iServerPort);

    pSendBuf = (char *)malloc(i_iSendLen+512);//512是头部大小
    if(NULL != pSendBuf)
    {
        memset(pSendBuf,0,i_iSendLen+512);
        iSendLen+=snprintf(pSendBuf+iSendLen,i_iSendLen+512-iSendLen,"%s %s %s\r\n",i_strMethod,i_strURL,HTTP_VERSION);
        if(NULL != i_strContentType)
        {
            iSendLen+=snprintf(pSendBuf+iSendLen,i_iSendLen+512-iSendLen,"Content-Length:%d\r\nContent-Type:%s\r\n",i_iSendLen,i_strContentType);
        }
        iSendLen+=snprintf(pSendBuf+iSendLen,i_iSendLen+512-iSendLen,"\r\n");
        if(NULL != i_acSendBuf)
        {
            memcpy(pSendBuf+iSendLen,i_acSendBuf,i_iSendLen);
            iSendLen+=i_iSendLen;
        }

        iRet=m_pTcpClient->Send(pSendBuf,iSendLen);

        
        free(pSendBuf);
    }
    
    return iRet;
} 


/*****************************************************************************
-Fuction		: RecvBody
-Description	: 200返回码后续作判断
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpServer :: CreateResponse(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen)
{
    int iRet = -1;
    char acRecvBuf[HTTP_PACKET_MAX_LEN];
    int iRecvLen=0;
    char *pBody = NULL;
    const char * strHttpBodyFlag = "\r\n\r\n";
    
    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen)
    {
        cout<<"RecvBody NULL"<<endl;
        return iRet;
    }
    if(NULL == m_pTcpClient)
    {
        printf("HttpClient :: RecvBody err no request\r\n");
        return iRet;
    }
    
    memset(acRecvBuf,0,sizeof(acRecvBuf));
    iRet=m_pTcpClient->Recv(acRecvBuf,&iRecvLen,sizeof(acRecvBuf));
    if(iRet == 0)
    {
        pBody = strstr(acRecvBuf,strHttpBodyFlag);
        if(NULL != pBody && i_iRecvBufMaxLen>=iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag)))
        {
            *o_piRecvLen = iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag));
            memcpy(o_acRecvBuf,pBody+strlen(strHttpBodyFlag),*o_piRecvLen);
        }
        else
        {
            iRet = -1;
            printf("HttpClient :: Recv err ,%p,%d,%d\r\n",pBody,i_iRecvBufMaxLen,(int)(iRecvLen-(pBody-acRecvBuf+strlen(strHttpBodyFlag))));
        }
    }
    //delete m_pTcpClient;//每次发的时候会释放
    //m_pTcpClient =NULL;//因为可能有接收两次的情况
    return iRet;
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
int HttpServer::Regex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch)
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
        WEBRTC_LOGE("Regex Error:\r\n");
    }
    else
    {
        iRet = regexec(&tReg, i_strBuf, MAX_MATCH_NUM, o_ptMatch, 0); //匹配他
        if (iRet == REG_NOMATCH)
        { //如果没匹配上
            WEBRTC_LOGE("Regex No Match!\r\n");
        }
        else if (iRet == REG_NOERROR)
        { //如果匹配上了
            /*WEBRTC_LOGD("Match\r\n");
            int i=0,j=0;
			for(j=0;j<MAX_MATCH_NUM;j++)
			{
				for (i= o_ptMatch[j].rm_so; i < o_ptMatch[j].rm_eo; i++)
				{ //遍历输出匹配范围的字符串
					printf("%c", i_strBuf[i]);
				}
				printf("\n");
			}*/
        }
        else
        {
            WEBRTC_LOGE("Regex Unknow err:\r\n");
        }
        regfree(&tReg);  //释放正则表达式
    }
    
    return iRet;
}


