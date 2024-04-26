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
    m_pTcpClient = new TcpClient();//http�����Ӵ��½�������
    if(NULL == m_pTcpClient)
    {
        printf("HttpClient :: Send NULL\r\n");
        return iRet;
    }
    m_pTcpClient->Init(m_strServerIp,m_iServerPort);

    pSendBuf = (char *)malloc(i_iSendLen+512);//512��ͷ����С
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
-Description	: 200������������ж�
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
    //delete m_pTcpClient;//ÿ�η���ʱ����ͷ�
    //m_pTcpClient =NULL;//��Ϊ�����н������ε����
    return iRet;
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
int HttpServer::Regex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch)
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
        WEBRTC_LOGE("Regex Error:\r\n");
    }
    else
    {
        iRet = regexec(&tReg, i_strBuf, MAX_MATCH_NUM, o_ptMatch, 0); //ƥ����
        if (iRet == REG_NOMATCH)
        { //���ûƥ����
            WEBRTC_LOGE("Regex No Match!\r\n");
        }
        else if (iRet == REG_NOERROR)
        { //���ƥ������
            /*WEBRTC_LOGD("Match\r\n");
            int i=0,j=0;
			for(j=0;j<MAX_MATCH_NUM;j++)
			{
				for (i= o_ptMatch[j].rm_so; i < o_ptMatch[j].rm_eo; i++)
				{ //�������ƥ�䷶Χ���ַ���
					printf("%c", i_strBuf[i]);
				}
				printf("\n");
			}*/
        }
        else
        {
            WEBRTC_LOGE("Regex Unknow err:\r\n");
        }
        regfree(&tReg);  //�ͷ�������ʽ
    }
    
    return iRet;
}


