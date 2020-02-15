/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc.cpp
* Description           : 	
浏览器发起请求是offer,提供视频流播放是answer(服务端)
使用cJson组装和解析协议

* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "webrtc.h"
#include "cJSON.h"


/*****************************************************************************
-Fuction        : WebRTC
-Description    : WebRTC
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRTC::WebRTC(char * i_strStunAddr,unsigned int i_dwStunPort,int i_iControlling):m_Libnice(i_strStunAddr,i_dwStunPort,i_iControlling),m_Srtp()
{
    m_pDtlsOnlyHandshake = NULL;
    memset(&m_tDtlsOnlyHandshakeCb,0,sizeof(T_DtlsOnlyHandshakeCb));
    m_tDtlsOnlyHandshakeCb.SendDataOut=&WebRTC::SendDataOutCb;
    m_tDtlsOnlyHandshakeCb.pObj=&m_Libnice;
    m_pDtlsOnlyHandshake = new DtlsOnlyHandshake(m_tDtlsOnlyHandshakeCb);
    
    memset(&m_tLibniceCb,0,sizeof(T_LibniceCb));//改回调为传入对象的方法,回调必须带对象指针参数(回调函数要访问对象里的内容)
    m_tLibniceCb.Handshake= &WebRTC::HandshakeCb;//回调函数必须static,
    m_tLibniceCb.HandleRecvData= &WebRTC::HandleRecvDataCb;//如果函数内调用对象里的则必须改c函数传对象指针
    m_tLibniceCb.pObjCb=m_pDtlsOnlyHandshake;//不再libnice类里面做指针转换,这是防止底层模块相互依赖
    m_Libnice.SetCallback(&m_tLibniceCb);

    m_iSrtpCreatedFlag = 0;


    m_pDtlsOnlyHandshake->Init();
    m_pDtlsOnlyHandshake->Create();
}
/*****************************************************************************
-Fuction        : ~WebRTC
-Description    : ~WebRTC
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRTC::~WebRTC()
{
    m_Srtp.Shutdown();
    m_iSrtpCreatedFlag = 0;
    
    delete m_pDtlsOnlyHandshake;
}
/*****************************************************************************
-Fuction        : Proc
-Description    : Proc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::Proc()
{
    return m_Libnice.LibniceProc();
}
/*****************************************************************************
-Fuction        : HandleOfferMsg
-Description    : HandleOfferMsg
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::HandleOfferMsg(char * i_strOfferMsg,char * o_strAnswerMsg,int i_iAnswerMaxLen)
{
    int iRet = -1;
    cJSON * ptOfferJson = NULL;
    cJSON * ptNode = NULL;
    char acRemoteSDP[5*1024];
    char acLocalSDP[5*1024];
    
    if(NULL == i_strOfferMsg||NULL == o_strAnswerMsg)
    {
        printf("HandleOfferMsg NULL \r\n");
        return iRet;
    }
    ptOfferJson = cJSON_Parse(i_strOfferMsg);
    if(NULL != ptOfferJson)
    {
        ptNode = cJSON_GetObjectItem(ptOfferJson,"type");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(0 == strcmp(ptNode->valuestring,"offer"))
            {
                iRet = 0;
            }
            ptNode = NULL;
        }
        ptNode = cJSON_GetObjectItem(ptOfferJson,"sdp");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(sizeof(acRemoteSDP)<strlen(ptNode->valuestring))
            {
                printf("cJSON_GetObjectItem sdp err \r\n");
            }
            memset(acRemoteSDP,0,sizeof(acRemoteSDP));
            strncpy(acRemoteSDP,ptNode->valuestring,sizeof(acRemoteSDP));
            ptNode = NULL;
        }
        cJSON_Delete(ptOfferJson);
    }
    if(0 != iRet)
    {
    }
    else
    {
        memset(acLocalSDP,0,sizeof(acLocalSDP));
        m_Libnice.GetLocalSDP(acLocalSDP,sizeof(acLocalSDP));
        cJSON * root = cJSON_CreateObject();
        cJSON_AddStringToObject(root,"sdp",acLocalSDP);
        cJSON_AddStringToObject(root,"type","answer");
        char * buf = cJSON_PrintUnformatted(root);
        if(buf)
        {
            snprintf(o_strAnswerMsg,i_iAnswerMaxLen,"%s",buf);
            free(buf);
        }
        cJSON_Delete(root);
        iRet=m_Libnice.SetRemoteSDP(acRemoteSDP);
    }
    return iRet;
}
/*****************************************************************************
-Fuction        : SendProtectedRtp
-Description    : SendProtectedRtp
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen)
{
    int iRet = -1;
    T_PolicyInfo tPolicyInfo;
    int iProtectRtpLen;
    if(NULL == i_acRtpBuf)
    {
        printf("SendProtectedRtp NULL \r\n");
        return iRet;
    }
    
    if(0 == m_iSrtpCreatedFlag)
    {
        memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
        m_pDtlsOnlyHandshake->GetPolicyInfo(&tPolicyInfo);
        m_Srtp.Create(tPolicyInfo.key, sizeof(tPolicyInfo.key), SRTP_SSRC_PROTECT);
        m_iSrtpCreatedFlag = 1;
    }
    iProtectRtpLen = i_iRtpBufLen;
    m_Srtp.ProtectRtp(i_acRtpBuf,&iProtectRtpLen,i_iRtpBufLen);
    iRet=m_Libnice.SendData(i_acRtpBuf, iProtectRtpLen);

    return iRet;
}



/*****************************************************************************
-Fuction        : HandshakeCb
-Description    : HandshakeCb
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
void WebRTC::HandshakeCb(void * pArg)
{
    DtlsOnlyHandshake *pDtlsOnlyHandshake=NULL;
    if(NULL!=pArg)//防止该静态函数对本对象的依赖,
    {//所以不直接用m_pDtlsOnlyHandshake->Handshake();
        pDtlsOnlyHandshake = (DtlsOnlyHandshake *)pArg;
        pDtlsOnlyHandshake->Handshake();
        
    }
}

/*****************************************************************************
-Fuction        : HandshakeCb
-Description    : HandshakeCb
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
void WebRTC::HandleRecvDataCb(char * i_acData,int i_iLen,void * pArg)
{
    DtlsOnlyHandshake *pDtlsOnlyHandshake=NULL;
    if(NULL!=pArg)//防止该静态函数对本对象的依赖,
    {//所以不直接用m_pDtlsOnlyHandshake->Handshake();
        pDtlsOnlyHandshake = (DtlsOnlyHandshake *)pArg;
        pDtlsOnlyHandshake->HandleRecvData(i_acData,i_iLen);
        
    }
}

/*****************************************************************************
-Fuction        : SendDataOutCb
-Description    : SendDataOutCb
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::SendDataOutCb(char * i_acData,int i_iLen,void * pArg)
{
    Libnice *pLibnice=NULL;
    if(NULL!=pArg)//防止该静态函数对本对象的依赖,
    {//所以不直接用m_pDtlsOnlyHandshake->Handshake();
        pLibnice = (Libnice *)pArg;
        pLibnice->SendData(i_acData,i_iLen);
        
    }
}






