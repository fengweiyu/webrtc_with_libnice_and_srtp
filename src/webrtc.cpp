/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc.cpp
* Description           : 	
���������������offer,�ṩ��Ƶ��������answer(�����)
ʹ��cJson��װ�ͽ���Э��

* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "webrtc.h"
#include <sys/time.h>
#include "cJSON.h"


/*****************************************************************************
-Fuction        : WebRTC
-Description    : �ײ����߹�ϵ�Ƚ��ң���ʱ�ص��������������ģʽ
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
    
    memset(&m_tLibniceCb,0,sizeof(T_LibniceCb));//�Ļص�Ϊ�������ķ���,�ص����������ָ�����(�ص�����Ҫ���ʶ����������)
    m_tLibniceCb.Handshake= &WebRTC::HandshakeCb;//�ص���������static,
    m_tLibniceCb.HandleRecvData= &WebRTC::HandleRecvDataCb;//��������ڵ��ö������������c����������ָ��
    m_tLibniceCb.pObjCb=m_pDtlsOnlyHandshake;//����libnice��������ָ��ת��,���Ƿ�ֹ�ײ�ģ���໥����
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
-Description    : �յ�offer���Ƿ�wait��Ϣ��ֻ���յ�Candidate�ŷ�answer
��Ϣ
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::HandleOfferMsg(char * i_strOfferMsg)
{
    int iRet = -1;
    cJSON * ptOfferJson = NULL;
    cJSON * ptNode = NULL;
    char acRemoteSDP[5*1024];
    
    if(NULL == i_strOfferMsg)
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
        iRet=m_Libnice.SaveRemoteSDP(acRemoteSDP);
    }
    return iRet;
}
/*****************************************************************************
-Fuction        : HandleCandidateMsg
-Description    : Offer��Ϣ����������Candidate֮ǰ�ģ���������ʱ��Ҫ��
����webrtcץ�����ֵģ����Բ��������ʱ���򷵻ش���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::HandleCandidateMsg(char * i_strCandidateMsg,T_VideoInfo *i_ptVideoInfo,char * o_strAnswerMsg,int i_iAnswerMaxLen)
{
    int iRet = -1;
    cJSON * ptCandidateJson = NULL;
    cJSON * ptNode = NULL;
    char acRemoteCandidate[1024];
    char acLocalSDP[5*1024];
    
    if(NULL == i_strCandidateMsg||NULL == o_strAnswerMsg ||NULL==i_ptVideoInfo)
    {
        printf("HandleOfferMsg NULL \r\n");
        return iRet;
    }
    ptCandidateJson = cJSON_Parse(i_strCandidateMsg);
    if(NULL != ptCandidateJson)
    {
        ptNode = cJSON_GetObjectItem(ptCandidateJson,"candidate");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(sizeof(acRemoteCandidate)<strlen(ptNode->valuestring))
            {
                printf("cJSON_GetObjectItem candidate err \r\n");
            }
            else
            {
                memset(acRemoteCandidate,0,sizeof(acRemoteCandidate));
                strncpy(acRemoteCandidate,ptNode->valuestring,sizeof(acRemoteCandidate));
                iRet=0;
            }
            ptNode = NULL;
        }
        cJSON_Delete(ptCandidateJson);
    }
    if(0 != iRet)
    {
    }
    else
    {
        iRet=m_Libnice.SetRemoteCandidateAndSDP(acRemoteCandidate);//
        memset(acLocalSDP,0,sizeof(acLocalSDP));
        //m_Libnice.GetLocalSDP(acLocalSDP,sizeof(acLocalSDP));//local sdpȱ����Ϣֻ���Լ����
        GenerateLocalSDP(i_ptVideoInfo,acLocalSDP,sizeof(acLocalSDP));
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
    iRet=m_Libnice.SendVideoData(i_acRtpBuf, iProtectRtpLen);

    return iRet;
}
/*****************************************************************************
-Fuction        : GetSendReadyFlag
-Description    : -1���ɷ���,0׼����ͨ�����Է���
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GetSendReadyFlag()
{
    int iRet = -1;
    T_PolicyInfo tPolicyInfo;

    if(0!= m_Libnice.GetSendReadyFlag())
    {
        memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
        iRet=m_pDtlsOnlyHandshake->GetPolicyInfo(&tPolicyInfo);//��ȡ�ɹ���ʾͨ��Э�̳ɹ���
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : GenerateLocalSDP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen)
{
	int iRet=-1;
    struct timeval tCreateTime;
    T_LocalCandidate tLocalCandidate;
    char strLocalFingerprint[160];
    const char *strStreamType="video";
    
    if (o_strSDP == NULL || NULL==i_ptVideoInfo || i_iSdpMaxLen <= 0) 
    {
		printf("GenerateLocalSDP NULL\r\n");
		return iRet;
    }
    memset(&tLocalCandidate,0,sizeof(T_LocalCandidate));
    m_Libnice.GetLocalCandidate(&tLocalCandidate);
	if(tLocalCandidate.iGatheringDoneFlag == 0)
	{
		printf("GenerateLocalSDP err\r\n");
		return iRet;
	}
    memset(&tCreateTime,0,sizeof(struct timeval));
    gettimeofday(&tCreateTime, NULL);
    memset(strLocalFingerprint,0,sizeof(strLocalFingerprint));
    m_pDtlsOnlyHandshake->GetLocalFingerprint(strLocalFingerprint,sizeof(strLocalFingerprint));
    iRet=snprintf(o_strSDP,i_iSdpMaxLen,"v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
        "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
        "t=0 0\r\na=group:BUNDLE %s\r\n"//t=<start time><stop time> ;Time
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%s\r\n"
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=ice-options:trickle\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:actpass\r\n"
        "a=connection:new\r\n"
        "a=rtpmap:%d %s/%d\r\n"
        "a=ssrc:%ld cname:ywf%s\r\n"
        "a=ssrc:%ld msid:janus janusa0\r\n"
        "a=ssrc:%ld mslabel:janus\r\n"
        "a=ssrc:%ld label:janusa0\r\n"
        "a=%s\r\n",
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,tLocalCandidate.strIP,
        strStreamType,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->ucRtpPayloadType,
        tLocalCandidate.strIP,
        strStreamType,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->ucRtpPayloadType,i_ptVideoInfo->pstrFormatName,i_ptVideoInfo->dwTimestampFrequency,
        tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec,
        tLocalCandidate.strCandidateData);

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
    if(NULL!=pArg)//��ֹ�þ�̬�����Ա����������,
    {//���Բ�ֱ����m_pDtlsOnlyHandshake->Handshake();
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
    if(NULL!=pArg)//��ֹ�þ�̬�����Ա����������,
    {//���Բ�ֱ����m_pDtlsOnlyHandshake->Handshake();
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
    if(NULL!=pArg)//��ֹ�þ�̬�����Ա����������,
    {//���Բ�ֱ����m_pDtlsOnlyHandshake->Handshake();
        pLibnice = (Libnice *)pArg;
        pLibnice->SendVideoData(i_acData,i_iLen);
        
    }
}






