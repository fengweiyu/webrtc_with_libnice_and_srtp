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
#include "Http.h"

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
HttpClient :: HttpClient()
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
HttpClient :: ~HttpClient()
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
int HttpClient :: ParseResponse(char *i_pcResData,int i_iDataLen,T_HttpResPacket *o_ptHttpResPacket)
{
    int iRet = -1;
    char *pBody = NULL;
    const char * strHttpBodyFlag = HTTP_CONTENT_FLAG;
	string strHttpHeader;
	string strFindRes;
	smatch Match;
	const char *strFirstLinePatten="([A-Za-z0-9/._]+) ([0-9]+) ([A-Za-z0-9/.]+)\r\n";
	const char *strConnectionPatten="Connection: ([A-Za-z0-9-]+)\r\n";
	const char *strContentLenPatten="Content-Length: ([0-9]+)\r\n";
	const char *strContentTypePatten="Content-type: ([A-Za-z0-9-/;.]+)\r\n";
	Http *pHttp = ((Http *)m_pHttp);
	
    if(NULL == i_pcResData ||NULL == o_ptHttpResPacket )
    {
        HTTP_LOGE("ParseResponse NULL\r\n");
        return iRet;
    }
    pBody = strstr(i_pcResData,strHttpBodyFlag);
    if(NULL != pBody)
    {
        if(NULL != o_ptHttpResPacket->pcBody && o_ptHttpResPacket->iBodyMaxLen>=i_iDataLen-(pBody-i_pcResData+strlen(strHttpBodyFlag)))
        {
            o_ptHttpResPacket->iBodyCurLen= i_iDataLen-(pBody-i_pcResData+strlen(strHttpBodyFlag));
            memcpy(o_ptHttpResPacket->pcBody,pBody+strlen(strHttpBodyFlag),o_ptHttpResPacket->iBodyCurLen);
        }
        else
        {
            o_ptHttpResPacket->pcBody=pBody+strlen(strHttpBodyFlag);
            o_ptHttpResPacket->iBodyCurLen= i_iDataLen-(pBody-i_pcResData+strlen(strHttpBodyFlag));
        }
    }
    
    strHttpHeader.assign(i_pcResData,0,pBody-i_pcResData);
    return pHttp->ParseResHeader(&strHttpHeader,o_ptHttpResPacket);//由于C++正则需要gcc 4.9及以上的版本，所以使用该函数，若是4.9或以上可注释掉该该行

    if(0 == pHttp->Regex(strFirstLinePatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpResPacket->strVersion,sizeof(o_ptHttpResPacket->strVersion),"%s",strFindRes.c_str());
        strFindRes.assign(Match[2].str());
        o_ptHttpResPacket->iStatusCode=atoi(strFindRes.c_str());
        strFindRes.assign(Match[3].str());
        snprintf(o_ptHttpResPacket->strStatusMsg,sizeof(o_ptHttpResPacket->strStatusMsg),"%s",strFindRes.c_str());
    }
    //memset(atMatch,0,sizeof(atMatch));//某次匹配失败，std::smatch对象将不会保存上一次的匹配结果
    if(0 == pHttp->Regex(strConnectionPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        snprintf(o_ptHttpResPacket->strConnection,sizeof(o_ptHttpResPacket->strConnection),"%s",strFindRes.c_str());
    }
    if(0 == pHttp->Regex(strContentLenPatten,&strHttpHeader,Match))
    {
        strFindRes.assign(Match[1].str());//0是整行
        o_ptHttpResPacket->iContentLength=atoi(strFindRes.c_str());
    }
    if(0 == pHttp->Regex(strContentTypePatten,&strHttpHeader,Match))
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
int HttpClient :: CreateRequest(const char * i_strMethod, const char * i_strURL,const char * i_strVersion)
{
    int iRet = -1;
    char strReq[512];

    
    if(NULL == i_strMethod || NULL == i_strURL || NULL == i_strVersion)
    {
        HTTP_LOGE("CreateRequest NULL\r\n");
        return iRet;
    }
    iRet=snprintf(strReq,sizeof(strReq),"%s %s %s\r\n",i_strMethod,i_strURL,i_strVersion);
    m_strReqHeader.assign(strReq);
    
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
int HttpClient :: SetReqHeaderValue(const char *i_strKey,const char *i_strValue)
{
    int iRet = -1;
    char strReq[128];
    
    if(NULL == i_strKey || NULL == i_strValue)
    {
        HTTP_LOGE("SetReqHeaderValue NULL\r\n");
        return iRet;
    }
    if(0 == m_strReqHeader.length())
    {
        HTTP_LOGE("SetReqHeaderValue m_strReqHeader NULL\r\n");
        return iRet;
    }
    iRet=snprintf(strReq,sizeof(strReq),"%s: %s\r\n",i_strKey,i_strValue);
    m_strReqHeader.append(strReq);
    
    return iRet;
}


/*****************************************************************************
-Fuction		: SetReqHeaderValue
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: SetReqHeaderValue(const char *i_strKey,int i_iValue)
{
    int iRet = -1;
    char strReq[128];
    
    if(NULL == i_strKey)
    {
        HTTP_LOGE("SetReqHeaderValue i_iValue NULL\r\n");
        return iRet;
    }
    if(0 == m_strReqHeader.length())
    {
        HTTP_LOGE("SetReqHeaderValue m_strReqHeader i_iValue NULL\r\n");
        return iRet;
    }
    iRet=snprintf(strReq,sizeof(strReq),"%s: %d\r\n",i_strKey,i_iValue);
    m_strReqHeader.append(strReq);
    
    return iRet;
}


/*****************************************************************************
-Fuction		: FormatReqToStream
-Description	: 
-Input			: 
-Output 		: 
-Return 		: int *o_piBufLen,
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: FormatReqToStream(char *i_pcContentData,int i_iDataLen,char *o_acBuf,int i_iBufMaxLen)
{
    int iRet = -1;
    char strRes[128];
    
    if(NULL == o_acBuf)
    {
        HTTP_LOGE("FormatReqToStream NULL\r\n");
        return iRet;
    }
    if(0 == m_strReqHeader.length())
    {
        HTTP_LOGE("FormatReqToStream strResHeader NULL\r\n");
        return iRet;
    }
    if(m_strReqHeader.length()+i_iDataLen+strlen("\r\n") > i_iBufMaxLen)
    {
        HTTP_LOGE("m_strReqHeader->length()%d+i_iDataLen%d > i_iBufMaxLen%d err\r\n",m_strReqHeader.length(),i_iDataLen,i_iBufMaxLen);
        return iRet;
    }
    iRet=m_strReqHeader.length();
    memcpy(o_acBuf,(char *)m_strReqHeader.c_str(),iRet);
    iRet+=snprintf(o_acBuf+iRet,i_iBufMaxLen-iRet,"%s","\r\n");
    if(NULL != i_pcContentData)
    {
        memcpy(o_acBuf+iRet,i_pcContentData,i_iDataLen);
        iRet+=i_iDataLen;
    }
    
    return iRet;
}









#if 0
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
HttpClient :: HttpClient()
{
    m_pTcpClient =NULL;
    m_iServerPort = -1;
    m_strServerIp.assign("");

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
HttpClient :: ~HttpClient()
{
    if(NULL != m_pTcpClient)
        delete m_pTcpClient;
}

/*****************************************************************************
-Fuction		: ~Init
-Description	: ~Init
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: Init(char * i_strIP,unsigned short i_wPort)
{
    int iRet=-1;
    if(NULL==i_strIP || i_wPort<0)
    {
        printf("HttpClient :: Init NULL\r\n");
        return iRet;
    }
    m_strServerIp.assign(i_strIP);
    m_iServerPort=i_wPort;
    iRet=0;
    return iRet;
}


/*****************************************************************************
-Fuction		: Send
-Description	: Send
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: Send(const char * i_strMethod,char * i_strURL,char * i_acSendBuf,int i_iSendLen,const char * i_strContentType)
{
    int iRet = -1;
    char * pSendBuf =NULL;
    int iSendLen = 0;
    
    if(NULL == i_strMethod ||NULL == i_strURL )
    {
        cout<<"Send NULL"<<endl;
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
int HttpClient :: RecvBody(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen)
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
-Fuction		: Recv
-Description	: 返回原始数据
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int HttpClient :: Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen)
{
    int iRet = -1;
    
    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen)
    {
        cout<<"Recv NULL"<<endl;
        return iRet;
    }
    if(NULL == m_pTcpClient)
    {
        printf("HttpClient :: RecvBody err no request\r\n");
        return iRet;
    }
    
    iRet=m_pTcpClient->Recv(o_acRecvBuf,o_piRecvLen,i_iRecvBufMaxLen);
    delete m_pTcpClient;
    m_pTcpClient =NULL;
    return iRet;
}

#endif
