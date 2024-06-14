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
//#include "cJSON.h"

#define MAX_MATCH_NUM       8

const int WebRTC::s_iAvMultiplex = 0;//���ڴ��뵽m_Libnice
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
WebRTC::WebRTC(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling,int i_iStreamType):m_Libnice(i_strStunAddr,i_dwStunPort,i_eControlling,s_iAvMultiplex)
{
    T_DtlsOnlyHandshakeCb tDtlsOnlyHandshakeCb;

    m_iLibniceProcStartedFlag = 0;
    m_iLibniceProcStopFlag = 0;
    memset(&m_tWebRtcCb,0,sizeof(m_tWebRtcCb));
    m_pWebRtcCbObj = NULL;

    m_pVideoDtlsOnlyHandshake = NULL;
    m_pVideoSrtp = NULL;
    m_pVideoDecSrtp = NULL;
    memset(&tDtlsOnlyHandshakeCb,0,sizeof(T_DtlsOnlyHandshakeCb));
    tDtlsOnlyHandshakeCb.SendDataOut=&WebRTC::SendVideoDataOutCb;
    tDtlsOnlyHandshakeCb.RecvStopPacket=&WebRTC::RecvVideoStopPacket;
    tDtlsOnlyHandshakeCb.pObj=this;
    m_pVideoDtlsOnlyHandshake = new DtlsOnlyHandshake(tDtlsOnlyHandshakeCb);
    m_pVideoSrtp = new Srtp();
    m_pAudioDtlsOnlyHandshake = NULL;
    m_pAudioSrtp = NULL;
    m_pAudioDecSrtp = NULL;
    if(s_iAvMultiplex > 0)
    {
        memset(&tDtlsOnlyHandshakeCb,0,sizeof(T_DtlsOnlyHandshakeCb));
        tDtlsOnlyHandshakeCb.SendDataOut=&WebRTC::SendAudioDataOutCb;
        tDtlsOnlyHandshakeCb.RecvStopPacket=&WebRTC::RecvAudioStopPacket;
        tDtlsOnlyHandshakeCb.pObj=this;
        m_pAudioDtlsOnlyHandshake = new DtlsOnlyHandshake(tDtlsOnlyHandshakeCb);
        m_pAudioSrtp = new Srtp();
    }

    memset(&m_tLibniceCb,0,sizeof(T_LibniceCb));//�Ļص�Ϊ�������ķ���,�ص����������ָ�����(�ص�����Ҫ���ʶ����������)
    m_tLibniceCb.Handshake= &WebRTC::HandshakeCb;//�ص���������static,
    m_tLibniceCb.HandleRecvData= &WebRTC::HandleRecvDataCb;//��������ڵ��ö������������c����������ָ��
    m_tLibniceCb.pVideoDtlsObjCb=m_pVideoDtlsOnlyHandshake;//����libnice��������ָ��ת��,���Ƿ�ֹ�ײ�ģ���໥����
    m_tLibniceCb.pAudioDtlsObjCb=m_pAudioDtlsOnlyHandshake;
    m_tLibniceCb.pWebRtcObjCb=this;
    m_Libnice.SetCallback(&m_tLibniceCb);

    m_iVideoSrtpCreatedFlag = 0;
    m_iAudioSrtpCreatedFlag = 0;
    m_iStreamType = i_iStreamType;
    m_pSctp = NULL;
    memset(&m_tSctpCb,0,sizeof(m_tSctpCb));
    m_tSctpCb.SendToOut = NULL;//û��Ӧ������ͨ��sctp����ʱ����
    m_tSctpCb.RecvFromOut = NULL;
    m_pSctp = new Sctp(&m_tSctpCb);

    m_eControlling = i_eControlling;//DtlsInit
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
    m_iVideoSrtpCreatedFlag = 0;
    m_iAudioSrtpCreatedFlag = 0;
    if(NULL!= m_pAudioDtlsOnlyHandshake)
    {
        delete m_pAudioDtlsOnlyHandshake;
    }
    if(NULL!= m_pVideoDtlsOnlyHandshake)
    {
        delete m_pVideoDtlsOnlyHandshake;
    }
    if(NULL!= m_pVideoSrtp)
    {
        delete m_pVideoSrtp;
    }
    if(NULL!= m_pVideoDecSrtp)
    {
        delete m_pVideoDecSrtp;
    }
    if(NULL!= m_pAudioSrtp)
    {
        delete m_pAudioSrtp;
    }
    if(NULL!= m_pAudioDecSrtp)
    {
        delete m_pAudioDecSrtp;
    }
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
int WebRTC::SetCallback(T_WebRtcCb *i_ptWebRtcCb,void *pObj)
{
    int iRet = -1;
    if(NULL == i_ptWebRtcCb || NULL == pObj)
    {
        WEBRTC_LOGW("SetCallback NULL \r\n");
        return iRet;
    }
    memset(&m_tWebRtcCb,0,sizeof(T_WebRtcCb));
    memcpy(&m_tWebRtcCb,i_ptWebRtcCb,sizeof(T_WebRtcCb));
    m_pWebRtcCbObj = pObj;
    return 0;
}
/*****************************************************************************
-Fuction        : DtlsInit
-Description    : DtlsInit
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::DtlsInit()
{
    int iRet = 0;
    
    iRet = m_pVideoDtlsOnlyHandshake->Init();//�����ʱ300ms�����Է�Proc()��
    if(iRet < 0)
    {
        WEBRTC_LOGE("m_pDtlsOnlyHandshake->Init err%d",iRet);
        return iRet;//
    }
    if(ICE_CONTROLLED_ROLE == m_eControlling)//�������Գ����һ����ɫ��webrtc.h
        iRet = m_pVideoDtlsOnlyHandshake->Create(DTLS_ROLE_SERVER);//������DTLS_ROLE_SERVER��DTLS_ROLE_CLIENTʵ��ʧ��
    else
        iRet = m_pVideoDtlsOnlyHandshake->Create(DTLS_ROLE_SERVER);//offer�ˣ�ʹ��sever
    if(iRet < 0)//���Է�Proc()��
    {
        WEBRTC_LOGE("m_pVideoDtlsOnlyHandshake->Create err i_eControlling %d iRet %d",m_eControlling,iRet);
        return iRet;//
    }


    if(NULL != m_pAudioDtlsOnlyHandshake)
    {
        iRet = m_pAudioDtlsOnlyHandshake->Init();//���Է�Proc()��
        if(iRet < 0)
        {
            WEBRTC_LOGE("m_pAudioDtlsOnlyHandshake->Init err%d",iRet);
            return iRet;//
        }
        if(ICE_CONTROLLED_ROLE == m_eControlling)//�������Գ����һ����ɫ��webrtc.h
            iRet = m_pAudioDtlsOnlyHandshake->Create(DTLS_ROLE_SERVER);//������DTLS_ROLE_SERVER��DTLS_ROLE_CLIENTʵ��ʧ��
        else
            iRet = m_pAudioDtlsOnlyHandshake->Create(DTLS_ROLE_SERVER);//offer�ˣ�ʹ��sever
        if(iRet < 0)//���Է�Proc()��
        {
            WEBRTC_LOGE("m_pAudioDtlsOnlyHandshake->Create err i_eControlling %d iRet %d",m_eControlling,iRet);
            return iRet;//
        }
    }
    return iRet;//
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
    m_iLibniceProcStartedFlag = 1;
    int iRet = m_Libnice.LibniceProc();
    m_iLibniceProcStartedFlag = 0;
    WEBRTC_LOGW("WebRTC::Proc exit %d\r\n",iRet);
    return iRet;
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
int WebRTC::StopProc()
{
    int iRet = -1;
    if(0 == m_iLibniceProcStopFlag)
    {
        iRet = m_Libnice.StopProc();
        m_iLibniceProcStopFlag = 1;
    }
    WEBRTC_LOGW("WebRTC::Proc StopProc StartedFlag%d StopFlag%d,%d\r\n",m_iLibniceProcStartedFlag,m_iLibniceProcStopFlag,iRet);
    return iRet;
}
/*****************************************************************************
-Fuction        : GetStopedFlag
-Description    : GetStopedFlag
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GetStopedFlag()
{
    int iRet = 0;
    if(0 == m_iLibniceProcStartedFlag)
    {
        iRet = 1;
    }
    else
    {
        iRet = 0;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : HandleCandidateMsg
-Description    : Offer��Ϣ����������Candidate֮ǰ�ģ���������ʱ��Ҫ��
����webrtcץ�����ֵģ����Բ��������ʱ���򷵻ش���
-Input          : i_iNotJsonMsgFlag 1��ʾ��json
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::HandleCandidateMsg(char * i_strCandidateMsg,int i_iNotJsonMsgFlag)
{
    int iRet = -1;
    char acRemoteCandidate[1024];
    
    if(NULL == i_strCandidateMsg)
    {
        printf("HandleOfferMsg NULL \r\n");
        return iRet;
    }
    if(1 == i_iNotJsonMsgFlag)
    {
        iRet=m_Libnice.SetRemoteCandidateAndSDP(i_strCandidateMsg);//
        return iRet;
    }
    printf("HandleOfferMsg no sup \r\n");
    return iRet;
/*
    cJSON * ptCandidateJson = NULL;
    cJSON * ptNode = NULL;
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
    }
    return iRet;*/
}

/*****************************************************************************
-Fuction        : SendProtectedRtp
-Description    : i_iStreamType 0 δ����(Ĭ�ϣ�s_iAvMultiplex = 0��ʹ��)��1��Ƶ��2��Ƶ
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen,int i_iRtpBufMaxLen,int i_iStreamType)
{
    int iRet = -1;

    if(0 != m_iLibniceProcStopFlag)
    {
        WEBRTC_LOGE("SendProtectedRtp ProcStop err %d\r\n",m_iLibniceProcStopFlag);
        return iRet;
    }

    if(s_iAvMultiplex > 0)
    {
        if(1 == i_iStreamType)
        {
            iRet = SendProtectedVideoRtp(i_acRtpBuf,i_iRtpBufLen,i_iRtpBufMaxLen);
        }
        else if(2 == i_iStreamType)
        {
            iRet = SendProtectedVideoRtp(i_acRtpBuf,i_iRtpBufLen,i_iRtpBufMaxLen);//SendProtectedAudioRtp
        }
        return iRet;
    }
    return SendProtectedVideoRtp(i_acRtpBuf,i_iRtpBufLen,i_iRtpBufMaxLen);
}

/*****************************************************************************
-Fuction        : HandleRecvSrtp
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::HandleRecvSrtp(char * i_acSrtpBuf,int i_iSrtpBufLen,DtlsOnlyHandshake *pDtlsOnlyHandshake)
{
    int iRet = -1;
    Srtp * pSrtp=NULL;
    T_PolicyInfo tPolicyInfo;
    unsigned char bPayload=0;
    
    if(NULL != m_tWebRtcCb.IsRtp && NULL != m_pWebRtcCbObj)
    {
        iRet = m_tWebRtcCb.IsRtp(i_acSrtpBuf,i_iSrtpBufLen,m_pWebRtcCbObj);
    }
    if(1 != iRet)
    {
        bPayload = (unsigned char)((unsigned char)i_acSrtpBuf[1]&0x7f);
        if(!IsSrtcp(i_acSrtpBuf))
        {
            WEBRTC_LOGD("pSrtp->IsRtcp   %#x,%d\r\n",bPayload,i_iSrtpBufLen);   
            return iRet;
        }
    }
    
    if(m_pVideoDtlsOnlyHandshake==pDtlsOnlyHandshake)//
    {
        if(NULL == m_pVideoDecSrtp)
        {
            m_pVideoDecSrtp = new Srtp();
            memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
            iRet = m_pVideoDtlsOnlyHandshake->GetRemotePolicyInfo(&tPolicyInfo);
            if(0 != iRet)
            {
                WEBRTC_LOGE("m_pVideoDecSrtp.GetRemotePolicyInfo GetRemotePolicyInfo err \r\n",iRet);
                return iRet;
            }
            m_pVideoDecSrtp->Create(tPolicyInfo.key, sizeof(tPolicyInfo.key), SRTP_SSRC_UNPROTECT);
        }
        pSrtp = m_pVideoDecSrtp;
    }
    else if(m_pAudioDtlsOnlyHandshake==pDtlsOnlyHandshake)//
    {
        if(NULL == m_pAudioDecSrtp)
        {
            m_pAudioDecSrtp = new Srtp();
            memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
            iRet =m_pAudioDtlsOnlyHandshake->GetRemotePolicyInfo(&tPolicyInfo);
            if(0 != iRet)
            {
                WEBRTC_LOGE("m_pAudioDecSrtp.GetRemotePolicyInfo GetRemotePolicyInfo err \r\n",iRet);
                return iRet;
            }
            m_pAudioDecSrtp->Create(tPolicyInfo.key, sizeof(tPolicyInfo.key), SRTP_SSRC_UNPROTECT);
        }
        pSrtp = m_pAudioDecSrtp;
    }
    if(NULL== pSrtp)
    {
        WEBRTC_LOGE("pSrtp NULL err %d\r\n",iRet);
        return -1;
    }
    if(IsSrtcp(i_acSrtpBuf))
    {//srtcp
        //WEBRTC_LOGD("UnProtectRtcp->UnProtectRtcp %d,bPayload %d,Len %d,pt %d\r\n",iRet,bPayload,i_iSrtpBufLen,(unsigned char)i_acSrtpBuf[1]);
        //iRet = pSrtp->UnProtectRtcp(i_acSrtpBuf,&i_iSrtpBufLen);//�ݲ�����rtcp���������ױ���
        //WEBRTC_LOGD("pSrtp->UnProtectRtcp %d,bPayload %d,Len %d,pt %d\r\n",iRet,bPayload,i_iSrtpBufLen,(unsigned char)i_acSrtpBuf[1]);
        return iRet;
    }
    iRet = pSrtp->UnProtectRtp(i_acSrtpBuf,&i_iSrtpBufLen);
    if(0 != iRet)
    {
        WEBRTC_LOGE("pSrtp->UnProtectRtp err %d\r\n",iRet);
        return iRet;
    }
    
    if(NULL != m_tWebRtcCb.RecvRtpData && NULL != m_pWebRtcCbObj)
    {
        return m_tWebRtcCb.RecvRtpData(i_acSrtpBuf,i_iSrtpBufLen,m_pWebRtcCbObj);//
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
int WebRTC::SendProtectedVideoRtp(char * i_acRtpBuf,int i_iRtpBufLen,int i_iRtpBufMaxLen)
{
    int iRet = -1;
    T_PolicyInfo tPolicyInfo;
    int iProtectRtpLen;
    if(NULL == i_acRtpBuf)
    {
        WEBRTC_LOGE("SendProtectedRtp NULL \r\n");
        return iRet;
    }
    if(i_iRtpBufLen+SRTP_MAX_TRAILER_LEN > i_iRtpBufMaxLen)
    {
        WEBRTC_LOGE("i_iRtpBufLen%d+SRTP_MAX_TRAILER_LEN%d > i_iRtpBufMaxLen%d err\r\n",i_iRtpBufLen,SRTP_MAX_TRAILER_LEN,i_iRtpBufMaxLen);
        return iRet;
    }
    if(0 == m_iVideoSrtpCreatedFlag)
    {
        memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
        iRet = m_pVideoDtlsOnlyHandshake->GetLocalPolicyInfo(&tPolicyInfo);
        if(0 != iRet)
        {
            WEBRTC_LOGE("m_Srtp.ProtectRtp GetLocalPolicyInfo err \r\n",iRet);
            return iRet;
        }
        m_pVideoSrtp->Create(tPolicyInfo.key, sizeof(tPolicyInfo.key), SRTP_SSRC_PROTECT);
        m_iVideoSrtpCreatedFlag = 1;
    }
    iProtectRtpLen = i_iRtpBufLen;
    iRet=m_pVideoSrtp->ProtectRtp(i_acRtpBuf,&iProtectRtpLen,i_iRtpBufLen);
    if (iRet) 
    {
        WEBRTC_LOGE("m_Srtp.ProtectRtp err %d i_iRtpBufLen %d\r\n",iRet,i_iRtpBufLen);
        return -1;//������webrtc����ͼ
    }
    iRet=m_Libnice.SendVideoData(i_acRtpBuf, iProtectRtpLen);

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
int WebRTC::SendProtectedAudioRtp(char * i_acRtpBuf,int i_iRtpBufLen,int i_iRtpBufMaxLen)
{
    int iRet = -1;
    T_PolicyInfo tPolicyInfo;
    int iProtectRtpLen;
    if(NULL == i_acRtpBuf)
    {
        WEBRTC_LOGE("SendProtectedRtp NULL \r\n");
        return iRet;
    }
    if(i_iRtpBufLen+SRTP_MAX_TRAILER_LEN > i_iRtpBufMaxLen)
    {
        WEBRTC_LOGE("Audio i_iRtpBufLen%d+SRTP_MAX_TRAILER_LEN%d > i_iRtpBufMaxLen%d err\r\n",i_iRtpBufLen,SRTP_MAX_TRAILER_LEN,i_iRtpBufMaxLen);
        return iRet;
    }
    if(0 == m_iAudioSrtpCreatedFlag)
    {
        memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
        m_pAudioDtlsOnlyHandshake->GetLocalPolicyInfo(&tPolicyInfo);
        m_pAudioSrtp->Create(tPolicyInfo.key, sizeof(tPolicyInfo.key), SRTP_SSRC_PROTECT);
        m_iAudioSrtpCreatedFlag = 1;
    }
    iProtectRtpLen = i_iRtpBufLen;
    iRet=m_pAudioSrtp->ProtectRtp(i_acRtpBuf,&iProtectRtpLen,i_iRtpBufLen);
    if (iRet) 
    {
        WEBRTC_LOGE("m_Srtp.ProtectRtp err %d \r\n",iRet);
        return iRet;//������webrtc����ͼ
    }
    iRet=m_Libnice.SendAudioData(i_acRtpBuf, iProtectRtpLen);

    return iRet;
}

/*****************************************************************************
-Fuction        : GetGatheringDoneFlag
-Description    : �ռ���ַ��ʱ300ms
-Input          : 
-Output         : 
-Return         : -1���ɷ���,0׼����ͨ�����Է���
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GetGatheringDoneFlag()
{
    int iRet = -1;
    T_LocalCandidate tLocalVideoCandidate;
    T_LocalCandidate tLocalAudioCandidate;
    
    memset(&tLocalVideoCandidate,0,sizeof(T_LocalCandidate));
    memset(&tLocalAudioCandidate,0,sizeof(T_LocalCandidate));
    m_Libnice.GetLocalCandidate(&tLocalVideoCandidate,&tLocalAudioCandidate);
    if(0 == s_iAvMultiplex && tLocalVideoCandidate.iGatheringDoneFlag != 0)
    {
		iRet = 0;
    }
    if(1 == s_iAvMultiplex && tLocalVideoCandidate.iGatheringDoneFlag != 0&& tLocalAudioCandidate.iGatheringDoneFlag != 0)
	{
		iRet = 0;
	}
    return iRet;
}

/*****************************************************************************
-Fuction        : GetSendReadyFlag
-Description    : 
�ӿ�ʼ���е�ͨ��׼���ã��ܺ�ʱԼ600-700ms(1s)
1.dtls���ɱ�����Կ��ʱ300ms��
2.�ռ���ַ��ʱ300ms,(��1�͵�2�Ѿ�������DtlsInit�ӿڣ����Բ��д���)
3.sdp���͸��Է���(��ʼ�������ӵ�)connected״̬����ʱ200-270ms��(sdp���͵��Է����úò���Ӧ��ʱ70ms)
4.�ȴ��������Ӻ�ĵ�һ�����ݰ���ʱ100ms(����ͨ������ǰһ�������Ż������鷳)
-Input          : 
-Output         : 
-Return         : -1���ɷ���,0׼����ͨ�����Է���
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GetSendReadyFlag()
{
    int iRet = -1;
    int iVideoReadyFlag=0,iAudioReadyFlag=0;
    T_PolicyInfo tPolicyInfo;

    m_Libnice.GetSendReadyFlag(&iVideoReadyFlag,&iAudioReadyFlag);
    if((0 == s_iAvMultiplex) && 0 != iVideoReadyFlag)
    {
        memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
        iRet=m_pVideoDtlsOnlyHandshake->GetLocalPolicyInfo(&tPolicyInfo);//��ȡ�ɹ���ʾͨ��Э�̳ɹ���
    }
	else
	{
	    //WEBRTC_LOGW("GetSendReadyFlag 0\r\n");
	}
    if((s_iAvMultiplex > 0) &&0 != iAudioReadyFlag)
    {
        memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
        iRet|=m_pAudioDtlsOnlyHandshake->GetLocalPolicyInfo(&tPolicyInfo);//��ȡ�ɹ���ʾͨ��Э�̳ɹ���
    }
	else
	{
	    if(s_iAvMultiplex > 0)
            iRet = -1;
	    //WEBRTC_LOGW("GetSendReadyFlag 0\r\n");
	}
    
    //if(iRet == 0 && m_iSendReadyFlag == 0)
    {
        //m_pSctp->Init();
        //m_iSendReadyFlag = 1;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : 
// 'setup' attribute can take in an offer/answer exchange:
//       Offer      Answer
//      ________________
//      active     passive / holdconn
//      passive    active / holdconn
//      actpass    active / passive / holdconn
//      holdconn   holdconn



-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GenerateLocalSDP(T_WebRtcMediaInfo *i_ptMediaInfo,char *o_strSDP,int i_iSdpMaxLen)
{
	int iRet=-1;
    struct timeval tCreateTime;
    T_LocalCandidate tLocalCandidate;
    char strLocalFingerprint[160];
    char strCandidate[128];
    const char *strStreamType="video";
    string strSdpFmt("");
    int i=0;
    T_VideoInfo *i_ptVideoInfo=NULL;
    
    if (o_strSDP == NULL || NULL==i_ptMediaInfo || i_iSdpMaxLen <= 0) 
    {
		printf("GenerateLocalSDP NULL\r\n");
		return iRet;
    }
    i_ptVideoInfo=&i_ptMediaInfo->tVideoInfo;
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
    m_pVideoDtlsOnlyHandshake->GetLocalFingerprint(strLocalFingerprint,sizeof(strLocalFingerprint));
    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
        "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
        "t=0 0\r\na=group:BUNDLE %s\r\n"//t=<start time><stop time> ;Time ��sdpMLineIndex sdpMid���һ��
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u UDP/TLS/RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%s\r\n"//��sdpMLineIndex sdpMid���һ��
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
        "a=ssrc:%ld label:janusa0\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {//ĿǰΪ1�������Ҳ��ʧ��
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp")&&NULL!= strstr(tLocalCandidate.strCandidateData[i],"."))
        {//���������Ż�Ϊȫ��ȡ�����ŵ�������,sdp�е�ip��0.0.0.0
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
            break;
        }
    }

    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,tLocalCandidate.strIP[i-1],
        i_ptVideoInfo->strMediaID,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->bRtpPayloadType,
        tLocalCandidate.strIP[i-1],//"0.0.0.0"����ʧ�ܣ����Ҳ��ʧ��
        i_ptVideoInfo->strMediaID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->bRtpPayloadType,i_ptVideoInfo->strFormatName,i_ptVideoInfo->dwTimestampFrequency,
        i_ptVideoInfo->dwSSRC, strStreamType,i_ptVideoInfo->dwSSRC, i_ptVideoInfo->dwSSRC, i_ptVideoInfo->dwSSRC);

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
int WebRTC::GenerateLocalCandidateMsg(T_VideoInfo *i_ptVideoInfo,char * o_strCandidateMsg,int i_iCandidateMaxLen)
{
    int iRet = -1;
#if 0    
    cJSON * ptNode = NULL;//��δʹ�ã���ֹ���ⲿ��cjson���ظ�����
    T_LocalCandidate tLocalCandidate;
    char strCandidate[128];
    int i;
    char strID[4];
    
    if(i_iCandidateMaxLen <= 0||NULL == o_strCandidateMsg ||NULL==i_ptVideoInfo)
    {
        printf("WebRtcOffer GenerateLocalMsg NULL \r\n");
        return iRet;
    }
    memset(&tLocalCandidate,0,sizeof(T_LocalCandidate));
    m_Libnice.GetLocalCandidate(&tLocalCandidate);
	if(tLocalCandidate.iGatheringDoneFlag == 0)
	{
		printf("GenerateLocalSDP err\r\n");
		return iRet;
	}
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {//ѡ�����ŵ�һ��
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp")&&NULL!= strstr(tLocalCandidate.strCandidateData[i],"."))
        {//���������Ż�Ϊȫ��ȡ�����ŵ�������,sdp�е�ip��0.0.0.0
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"%s",
            tLocalCandidate.strCandidateData[i]);
            break;
        }
    }
    snprintf(strID,sizeof(strID),"%d",i_ptVideoInfo->iID);
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"candidate",strCandidate);
    cJSON_AddNumberToObject(root,"sdpMLineIndex",i_ptVideoInfo->iID);
    cJSON_AddStringToObject(root,"sdpMid",strID);
    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet=snprintf(o_strCandidateMsg,i_iCandidateMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);
#endif    
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
void WebRTC::HandleRecvDataCb(char * i_acData,int i_iLen,void * pDtlsArg,void * pWebRtcArg)
{
    int iRet = -1;
    DtlsOnlyHandshake *pDtlsOnlyHandshake=NULL;
    WebRTC * pWebRTC=NULL;

    
    if(NULL!=pDtlsArg)//��ֹ�þ�̬�����Ա����������,
    {
        pDtlsOnlyHandshake = (DtlsOnlyHandshake *)pDtlsArg;
    }
    
    if (IsDtls(i_acData))
    {
        if(NULL!=pDtlsOnlyHandshake)//��ֹ�þ�̬�����Ա����������,
        {//���Բ�ֱ����m_pDtlsOnlyHandshake->Handshake();
            pDtlsOnlyHandshake->HandleRecvData(i_acData,i_iLen);
        }
    }
    else
    {
        if(NULL==pWebRtcArg)//��ֹ�þ�̬�����Ա����������,
        {
            return;
        }
        pWebRTC = (WebRTC *)pWebRtcArg;
        pWebRTC->HandleRecvSrtp(i_acData,i_iLen,pDtlsOnlyHandshake);
    }
}

/*****************************************************************************
-Fuction        : IsDtls
-Description    : IsDtls
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::HandleVideoDtlsStopMsg() 
{
    int iRet = -1;
    
    if(NULL != m_tWebRtcCb.RecvStopMsg && NULL != m_pWebRtcCbObj)
    {
        iRet = m_tWebRtcCb.RecvStopMsg(m_pWebRtcCbObj);
    }

    return iRet;
}

/*****************************************************************************
-Fuction        : IsDtls
-Description    : IsDtls
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
Libnice * WebRTC::GetLibniceObj() 
{
    return &m_Libnice;//buf < 64
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
int WebRTC::SendVideoDataOutCb(char * i_acData,int i_iLen,void * pArg)
{
    int iRet=-1;
    WebRTC *pWebRTC=NULL;
    if(NULL!=pArg)//��ֹ�þ�̬�����Ա����������,
    {//���Բ�ֱ����m_pDtlsOnlyHandshake->Handshake();
        pWebRTC = (WebRTC *)pArg;
        iRet=pWebRTC->GetLibniceObj()->SendVideoData(i_acData,i_iLen);
    }
    printf("WebRTC::SendDataOutCb:%d\r\n",iRet);
    return iRet;
}

/*****************************************************************************
-Fuction        : SendAudioDataOutCb
-Description    : SendAudioDataOutCb
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::SendAudioDataOutCb(char * i_acData,int i_iLen,void * pArg)
{
    int iRet=-1;
    WebRTC *pWebRTC=NULL;
    if(NULL!=pArg)//��ֹ�þ�̬�����Ա����������,
    {//���Բ�ֱ����m_pDtlsOnlyHandshake->Handshake();
        pWebRTC = (WebRTC *)pArg;
        iRet=pWebRTC->GetLibniceObj()->SendAudioData(i_acData,i_iLen);
    }
    printf("WebRTC::SendAudioDataOutCb:%d\r\n",iRet);
    return iRet;
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
int WebRTC::RecvVideoStopPacket(void * pArg)
{
    int iRet=-1;
    WebRTC *pWebRTC=NULL;
    
    printf("WebRTC::RecvVideoStopPacket \r\n");
    if(NULL!=pArg)//��ֹ�þ�̬�����Ա����������,
    {
        pWebRTC = (WebRTC *)pArg;
        iRet = pWebRTC->HandleVideoDtlsStopMsg();
    }
    return iRet;
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
int WebRTC::RecvAudioStopPacket(void * pArg)
{
    int iRet=-1;
    printf("WebRTC::RecvAuidoStopPacket:%d\r\n",iRet);
    return iRet;
}

/*****************************************************************************
-Fuction        : IsDtls
-Description    : IsDtls
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
bool WebRTC::IsDtls(char *buf) 
{
    return ((*buf >= 20) && (*buf <= 64));//buf < 64
}

/*****************************************************************************
-Fuction        : IsDtls
-Description    : IsDtls
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
bool WebRTC::IsSrtcp(char *buf) 
{
    unsigned char bPayload=0;
    bPayload = (unsigned char)((unsigned char)buf[1]&0x7f);
    return (bPayload >= 64 && bPayload < 96);
}


/*****************************************************************************
-Fuction        : WebRtcOffer
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcOffer::WebRtcOffer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling,int i_iStreamType):WebRTC(i_strStunAddr,i_dwStunPort,i_eControlling,i_iStreamType)
{
}

/*****************************************************************************
-Fuction        : ~WebRtcOffer
-Description    : ~WebRtcOffer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcOffer::~WebRtcOffer()
{
}

/*****************************************************************************
-Fuction        : HandleMsg
-Description    : 
-Input          : i_iNotJsonMsgFlag 1��ʾ��json
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcOffer::HandleMsg(char * i_strAnswerMsg,int i_iNotJsonMsgFlag,T_WebRtcSdpMediaInfo *o_ptSdpMediaInfo)
{
    int iRet = -1;
    char acRemoteSDP[5*1024];
    
    if(NULL == i_strAnswerMsg)
    {
        printf("WebRtcOffer HandleMsg NULL \r\n");
        return iRet;
    }
    if(1 == i_iNotJsonMsgFlag)
    {
        iRet=m_Libnice.SaveRemoteSDP(i_strAnswerMsg);
        if(NULL != strstr(i_strAnswerMsg,"candidate:"))
        {
            iRet=m_Libnice.SetRemoteCandidateAndSDP(NULL);//
        }
        return iRet;
    }

    printf("HandleMsg cJSON no sup \r\n");
    return iRet;
    /*cJSON * ptAnswerJson = NULL;
    cJSON * ptNode = NULL;
    ptAnswerJson = cJSON_Parse(i_strAnswerMsg);
    if(NULL != ptAnswerJson)
    {
        ptNode = cJSON_GetObjectItem(ptAnswerJson,"type");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(0 == strcmp(ptNode->valuestring,"answer"))
            {
                iRet = 0;
            }
            ptNode = NULL;
        }
        ptNode = cJSON_GetObjectItem(ptAnswerJson,"sdp");
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
        cJSON_Delete(ptAnswerJson);
    }
    if(0 != iRet)
    {
    }
    else
    {
        iRet=m_Libnice.SaveRemoteSDP(acRemoteSDP);
        if(NULL != strstr(acRemoteSDP,"candidate:"))
        {
            iRet=m_Libnice.SetRemoteCandidateAndSDP(NULL);//
        }
    }
    return iRet;*/
}

/*****************************************************************************
-Fuction        : GenerateLocalMsg
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcOffer::GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strOfferMsg,int i_iOfferMaxLen)
{
    int iRet = -1;
    /*cJSON * ptNode = NULL;
    char acLocalSDP[5*1024];
    
    if(i_iOfferMaxLen <= 0||NULL == o_strOfferMsg ||NULL==i_ptVideoInfo)
    {
        printf("WebRtcOffer GenerateLocalMsg NULL \r\n");
        return iRet;
    }
    memset(acLocalSDP,0,sizeof(acLocalSDP));
    //m_Libnice.GetLocalSDP(acLocalSDP,sizeof(acLocalSDP));//local sdpȱ����Ϣֻ���Լ����
    iRet=GenerateLocalSDP(i_ptVideoInfo,acLocalSDP,sizeof(acLocalSDP));
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"sdp",acLocalSDP);
    cJSON_AddStringToObject(root,"type","offer");
    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        snprintf(o_strOfferMsg,i_iOfferMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);*/
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : 
// From RFC 4145, section-4.1, The following are the values that the
// 'setup' attribute can take in an offer/answer exchange:
//       Offer      Answer
//      ________________
//      active     passive / holdconn
//      passive    active / holdconn
//      actpass    active / passive / holdconn
//      holdconn   holdconn




-Input          : T_WebRtcMediaInfo *i_ptMediaInfo,
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcOffer::GenerateLocalSDP(T_WebRtcMediaInfo *i_ptMediaInfo,char *o_strSDP,int i_iSdpMaxLen)
{
	int iRet=-1;
    struct timeval tCreateTime;
    T_LocalCandidate tLocalCandidate;
    char strLocalFingerprint[160];
    char strCandidate[128];
    const char *strStreamType="video";
    string strSdpFmt("");
    int i=0;
    T_VideoInfo *i_ptVideoInfo=NULL;
    
    if (o_strSDP == NULL || NULL==i_ptMediaInfo || i_iSdpMaxLen <= 0) 
    {
		printf("GenerateLocalSDP NULL\r\n");
		return iRet;
    }
    i_ptVideoInfo=&i_ptMediaInfo->tVideoInfo;
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
    m_pVideoDtlsOnlyHandshake->GetLocalFingerprint(strLocalFingerprint,sizeof(strLocalFingerprint));
#if 0    
    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
        "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
        "t=0 0\r\na=group:BUNDLE %d\r\n"//t=<start time><stop time> ;Time ��sdpMLineIndex sdpMid���һ��
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u UDP/TLS/RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//��sdpMLineIndex sdpMid���һ��
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
        "a=ssrc:%ld label:janusa0\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {//ĿǰΪ1�������Ҳ��ʧ��
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp")&&NULL!= strstr(tLocalCandidate.strCandidateData[i],"."))
        {//���������Ż�Ϊȫ��ȡ�����ŵ�������,sdp�е�ip��0.0.0.0
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
            break;
        }
    }
    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,tLocalCandidate.strIP[i-1],
        i_ptVideoInfo->iID,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->ucRtpPayloadType,
        tLocalCandidate.strIP[i-1],//"0.0.0.0"����ʧ�ܣ����Ҳ��ʧ��
        i_ptVideoInfo->iID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->ucRtpPayloadType,i_ptVideoInfo->pstrFormatName,i_ptVideoInfo->dwTimestampFrequency,
        tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec);
#endif

    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
        "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
        "t=0 0\r\na=group:BUNDLE %s\r\n"//t=<start time><stop time> ;BUNDLE ��sdpMLineIndex sdpMid���һ��
        "m=%s %u RTP/SAVPF %d\r\n"//����dtls����Ҫsrtp���ܵ�
        "c=IN IP4 %s\r\n"
        "a=mid:%s\r\n"//��sdpMLineIndex sdpMid���һ��
        "a=sendonly\r\n"
        "a=rtcp-mux\r\n"//a=group:BUNDLEҪ���������һ��,https://blog.csdn.net/Rin_Cherish/article/details/86677126
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=ice-options:trickle\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=rtpmap:%d %s/%d\r\n"
        /*2.packetization-mode:packetization-mode��ʾͼ�����ݰ��ֲ��͵ķ�ʽ��
        0: Single NAL (Network Abstraction Layer)��ÿ֡ͼ������ȫ������һ��NAL��Ԫ���ͣ�
        1: Not Interleaved��ÿ֡ͼ�����ݱ���ŵ����NAL��Ԫ���ͣ���ЩNAL��Ԫ���͵�˳���ǰ��ս����˳���ͣ�
        2: Interleaved��ÿ֡ͼ�����ݱ���ŵ����NAL��Ԫ���ͣ�������ЩNAL��Ԫ���͵�˳����Բ����ս����˳����
        ʵ���ϣ�ֻ��I֡���Ա���ַ��ͣ�P֡��B֡�����ܱ���ַ��͡�
        �������packetization-mode=1������ζ��I֡�ᱻ��ַ��͡�
        level-asymmetry-allowed��ʾ�Ƿ��������˱����Level��һ�¡�ע��������˵�SDP�и�ֵ��Ϊ1����Ч��*/
        //"a=fmtp:%d level-asymmetry-allowed=1;packetization-mode=1\r\n"//��Ҫsps,pps���������޷�����
        "a=fmtp:%d level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=%06X;sprop-parameter-sets=%s,%s\r\n"
        "a=ssrc:%ld msid:janus janusv0\r\n"//��rtp�е�SSRC һ��
        "a=setup:actpass\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp"))
        {//webrtc�ٷ����������ֻ֧��:udp tcp ssltcp tls ��������ͬ��ֻ��udp
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
        }
    }//ȥ��sctpһ������ͨ����ͨ
    /*strSdpFmt.append("m=%s %u DTLS/SCTP %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//��sdpMLineIndex sdpMid��ļ�1
        "a=sendrecv\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:actpass\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp"))
        {//webrtc�ٷ����������ֻ֧��:udp tcp ssltcp tls ��������ͬ��ֻ��udp
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
        }
    }*/
    
    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,"0.0.0.0",
        i_ptVideoInfo->strMediaID,//i_ptVideoInfo->iID+1,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->bRtpPayloadType,
        "0.0.0.0",//"0.0.0.0"����ʧ�ܣ����Ҳ��ʧ��
        i_ptVideoInfo->strMediaID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->bRtpPayloadType,i_ptVideoInfo->strFormatName,i_ptVideoInfo->dwTimestampFrequency,
        i_ptVideoInfo->bRtpPayloadType,i_ptVideoInfo->dwProfileLevelId,i_ptVideoInfo->strSPS_Base64,i_ptVideoInfo->strPPS_Base64,i_ptVideoInfo->dwSSRC);
        /*"application",i_ptVideoInfo->wPortNumForSDP,102,//"m=application 9 DTLS/SCTP". Reason: Expects at least 4 fields
        "0.0.0.0",
        i_ptVideoInfo->iID+1,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint);*/

	return iRet;
}
/*****************************************************************************
-Fuction        : WebRtcAnswer
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcAnswer::WebRtcAnswer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling,int i_iStreamType):WebRTC(i_strStunAddr,i_dwStunPort,i_eControlling,i_iStreamType)
{
    m_pVideoID = NULL;
    m_pAudioID = NULL;
    m_iAvDiff = 0;
}

/*****************************************************************************
-Fuction        : ~~WebRtcAnswer
-Description    : ~~WebRtcAnswer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcAnswer::~WebRtcAnswer()
{
    if(NULL!= m_pVideoID)
    {
        delete m_pVideoID;
    }
    if(NULL!= m_pAudioID)
    {
        delete m_pAudioID;
    }
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
int WebRtcAnswer::HandleMsg(char * i_strOfferMsg,int i_iNotJsonMsgFlag,T_WebRtcSdpMediaInfo *o_ptSdpMediaInfo)
{
    int iRet = -1;
    //char acRemoteSDP[5*1024];
    
    if(NULL == i_strOfferMsg)
    {
        WEBRTC_LOGE("HandleOfferMsg NULL \r\n");
        return iRet;
    }
    string strMsg(i_strOfferMsg);
    size_t iVideoPos = strMsg.find("m=video");//�����ж��(�����Ƶ������ж��m=video),��������
    if(string::npos != iVideoPos)
    {
        size_t iVideoMidPos = strMsg.find("a=mid:",iVideoPos);
        if(string::npos != iVideoMidPos)
        {
            size_t iVideoMidIdPos = strMsg.find("\r\n",iVideoMidPos);
            if(string::npos != iVideoMidIdPos)
            {
                m_pVideoID = new string(strMsg.substr(iVideoMidPos+strlen("a=mid:"),iVideoMidIdPos-(iVideoMidPos+strlen("a=mid:"))).c_str());
            }
        }
    }
    size_t iAudioPos = strMsg.find("m=audio");//�����ж��(�����Ƶ������ж��m=audio),��������
    if(string::npos != iAudioPos)
    {
        size_t iAudioMidPos = strMsg.find("a=mid:",iAudioPos);
        if(string::npos != iAudioMidPos)
        {
            size_t iAudioMidIdPos = strMsg.find("\r\n",iAudioMidPos);
            if(string::npos != iAudioMidIdPos)
            {
                m_pAudioID = new string(strMsg.substr(iAudioMidPos+strlen("a=mid:"),iAudioMidIdPos-(iAudioMidPos+strlen("a=mid:"))).c_str());
            }
        }
    }
    if(string::npos == iVideoPos && string::npos == iAudioPos)
    {
        WEBRTC_LOGE("string::npos == iVideoPos && string::npos == iAudioPos err \r\n");
        return iRet;
    }
    m_iAvDiff=(int)(iAudioPos-iVideoPos);
    printf("HandleMsg m_iAvDiff %d \r\n",m_iAvDiff);
    if(NULL != o_ptSdpMediaInfo)
    {
        o_ptSdpMediaInfo->iAvMediaDiff = m_iAvDiff;
        if(m_iAvDiff>0)
        {
            GetSdpVideoInfo(strMsg.substr(iVideoPos,iAudioPos-iVideoPos).c_str(),o_ptSdpMediaInfo);
            GetSdpAudioInfo(strMsg.substr(iAudioPos,strMsg.size()-iAudioPos).c_str(),o_ptSdpMediaInfo);
        }
        else
        {
            GetSdpAudioInfo(strMsg.substr(iAudioPos,iVideoPos-iAudioPos).c_str(),o_ptSdpMediaInfo);
            GetSdpVideoInfo(strMsg.substr(iVideoPos,strMsg.size()-iVideoPos).c_str(),o_ptSdpMediaInfo);
        }
    }
    if(1 == i_iNotJsonMsgFlag)
    {
        iRet=m_Libnice.SaveRemoteSDP(i_strOfferMsg);
        if(NULL != strstr(i_strOfferMsg,"candidate:"))
        {
            iRet=0;//m_Libnice.SetRemoteCandidateAndSDP(NULL);//����sdp�������ã���ֹ����sdp��ʱ�ã��������ú�ȴ���
        }
        return iRet;
    }
    
    printf("HandleMsg cJSON no sup \r\n");
    return iRet;
    /*cJSON * ptOfferJson = NULL;
    cJSON * ptNode = NULL;
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
                WEBRTC_LOGE("cJSON_GetObjectItem sdp err \r\n");
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
        if(NULL != strstr(acRemoteSDP,"candidate:"))
        {
            iRet=m_Libnice.SetRemoteCandidateAndSDP(NULL);//
        }
    }
    return iRet;*/
}

/*****************************************************************************
-Fuction        : GenerateLocalMsg
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strAnswerMsg,int i_iAnswerMaxLen)
{
    int iRet = -1;
    /*cJSON * ptNode = NULL;
    char acLocalSDP[5*1024];
    
    if(i_iAnswerMaxLen <= 0||NULL == o_strAnswerMsg ||NULL==i_ptVideoInfo)
    {
        printf("GetAnswerMsg NULL \r\n");
        return iRet;
    }
    memset(acLocalSDP,0,sizeof(acLocalSDP));
    //m_Libnice.GetLocalSDP(acLocalSDP,sizeof(acLocalSDP));//local sdpȱ����Ϣֻ���Լ����
    iRet=GenerateLocalSDP(i_ptVideoInfo,acLocalSDP,sizeof(acLocalSDP));
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"sdp",acLocalSDP);
    cJSON_AddStringToObject(root,"type","answer");
    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        snprintf(o_strAnswerMsg,i_iAnswerMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);*/
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : 
// From RFC 4145, section-4.1, The following are the values that the
// 'setup' attribute can take in an offer/answer exchange:
//       Offer      Answer
//      ________________
//      active     passive / holdconn
//      passive    active / holdconn
//      actpass    active / passive / holdconn
//      holdconn   holdconn




-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::GenerateLocalSDP(T_WebRtcMediaInfo *i_ptMediaInfo,char *o_strSDP,int i_iSdpMaxLen)
{
	int iRet=-1;
    struct timeval tCreateTime;
    T_LocalCandidate tVideoLocalCandidate;
    T_LocalCandidate tAudioLocalCandidate;
    T_LocalCandidate *ptVideoLocalCandidate=NULL;
    T_LocalCandidate *ptAudioLocalCandidate=NULL;
    char strVideoLocalFingerprint[160];
    char strAudioLocalFingerprint[160];
    char *pVideoLocalFingerprint=NULL;
    char *pAudioLocalFingerprint=NULL;
    char strCandidate[128];
    string strSdpFmt("");
    int i=0;
    T_VideoInfo *ptVideoInfo=NULL;
    T_AudioInfo *ptAudioInfo=NULL;
    
    if (o_strSDP == NULL || NULL==i_ptMediaInfo || i_iSdpMaxLen <= 0) 
    {
		WEBRTC_LOGE("GenerateLocalSDP NULL\r\n");
		return iRet;
    }
    ptVideoInfo=&i_ptMediaInfo->tVideoInfo;
    ptAudioInfo=&i_ptMediaInfo->tAudioInfo;
    if((strlen(ptAudioInfo->strFormatName) != 0 && 0 == m_iStreamType)||(strlen(ptVideoInfo->strFormatName) != 0 && 1 == m_iStreamType))
    {
		//WEBRTC_LOGE("GenerateLocalSDP err %p,%p,%d\r\n",ptAudioInfo->pstrFormatName,ptVideoInfo->pstrFormatName,m_iStreamType);
		//return iRet;
    }
    if((strlen(ptVideoInfo->strFormatName) == 0 && strlen(ptAudioInfo->strFormatName) == 0)/*||
    (strlen(ptVideoInfo->strFormatName) != 0 && NULL == m_pVideoID)||(strlen(ptAudioInfo->strFormatName) != 0 && NULL == m_pAudioID)*/)
    {
		WEBRTC_LOGE("GenerateLocalSDP err %d,%d,%p,%p\r\n",strlen(ptAudioInfo->strFormatName),strlen(ptVideoInfo->strFormatName),m_pVideoID,m_pAudioID);
		return iRet;
    }
	if(GetGatheringDoneFlag() < 0)
	{
		WEBRTC_LOGE("GenerateLocalSDP err GetGatheringDoneFlag() < 0\r\n");
		return iRet;
	}
    memset(&tVideoLocalCandidate,0,sizeof(T_LocalCandidate));
    memset(&tAudioLocalCandidate,0,sizeof(T_LocalCandidate));
    m_Libnice.GetLocalCandidate(&tVideoLocalCandidate,&tAudioLocalCandidate);
    
    memset(&tCreateTime,0,sizeof(struct timeval));
    gettimeofday(&tCreateTime, NULL);
    memset(strVideoLocalFingerprint,0,sizeof(strVideoLocalFingerprint));
    m_pVideoDtlsOnlyHandshake->GetLocalFingerprint(strVideoLocalFingerprint,sizeof(strVideoLocalFingerprint));
#if 0    
    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
        "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
        "t=0 0\r\na=group:BUNDLE %d\r\n"//t=<start time><stop time> ;Time ��sdpMLineIndex sdpMid���һ��
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u UDP/TLS/RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=setup:active\r\n"
        "a=mid:%d\r\n"//��sdpMLineIndex sdpMid���һ��
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
        "a=ssrc:%ld label:janusa0\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {//ĿǰΪ1�������Ҳ��ʧ��
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp")&&NULL!= strstr(tLocalCandidate.strCandidateData[i],"."))
        {//���������Ż�Ϊȫ��ȡ�����ŵ�������,sdp�е�ip��0.0.0.0
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
            break;
        }
    }
    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,tLocalCandidate.strIP[i-1],
        i_ptVideoInfo->iID,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->ucRtpPayloadType,
        tLocalCandidate.strIP[i-1],//"0.0.0.0"����ʧ�ܣ����Ҳ��ʧ��
        i_ptVideoInfo->iID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->ucRtpPayloadType,i_ptVideoInfo->pstrFormatName,i_ptVideoInfo->dwTimestampFrequency,
        tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec);
#endif
    if(strlen(ptAudioInfo->strFormatName) == 0 && strlen(ptVideoInfo->strFormatName) != 0)
    {
        strSdpFmt.assign("v=0\r\n"
            "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
            "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
            "t=0 0\r\na=group:BUNDLE %s\r\n"//"t=0 0\r\na=group:BUNDLE %d %d\r\n" sctp�ݲ���Ҫȥ��һ��%d//t=<start time><stop time> ;BUNDLE ��sdpMLineIndex sdpMid���һ��
            //"a=ice-lite\r\n"
            "a=msid-semantic: WMS ywf-mslabel\r\n"//"a=msid-semantic: WMS ywf\r\n"
            );//a=setup:actpass ������ᱨ��
        iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
            tCreateTime.tv_sec,tCreateTime.tv_usec,1,"0.0.0.0",
            ptVideoInfo->strMediaID);//m_pVideoID->c_str()
    }
    else if(strlen(ptVideoInfo->strFormatName) == 0 && strlen(ptAudioInfo->strFormatName) != 0)
    {
        strSdpFmt.assign("v=0\r\n"
            "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
            "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
            "t=0 0\r\na=group:BUNDLE %s\r\n"//"t=0 0\r\na=group:BUNDLE %d %d\r\n" sctp�ݲ���Ҫȥ��һ��%d//t=<start time><stop time> ;BUNDLE ��sdpMLineIndex sdpMid���һ��
            //"a=ice-lite\r\n"
            "a=msid-semantic: WMS ywf-mslabel\r\n"//"a=msid-semantic: WMS ywf\r\n"
            );//a=setup:actpass ������ᱨ��
        iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
            tCreateTime.tv_sec,tCreateTime.tv_usec,1,"0.0.0.0",
            ptAudioInfo->strMediaID);//m_pAudioID->c_str()
    }
    else
    {
        if(s_iAvMultiplex > 0)
        {
            strSdpFmt.assign("v=0\r\n"
                "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
                "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
                "t=0 0\r\n"//"t=0 0\r\na=group:BUNDLE %d %d\r\n" sctp�ݲ���Ҫȥ��һ��%d//t=<start time><stop time> ;BUNDLE ��sdpMLineIndex sdpMid���һ��
                //"a=group:BUNDLE %d %d\r\n"//a=group:BUNDLE��ʾ����Ƶ������һ��ͨ��
                //"a=ice-lite\r\n"
                "a=msid-semantic: WMS ywf-mslabel\r\n"//"a=msid-semantic: WMS ywf\r\n"
                );//a=setup:actpass ������ᱨ��
            iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
                tCreateTime.tv_sec,tCreateTime.tv_usec,1,"0.0.0.0");
                //ptVideoInfo->iID,ptAudioInfo->iID);
        }
        else
        {
            strSdpFmt.assign("v=0\r\n"
                "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,�����˻Ự�ķ�������Ϣ
                "s=ywf webrtc\r\n"//s=<sessionname> ;������Session Name
                "t=0 0\r\n"//"t=0 0\r\na=group:BUNDLE %d %d\r\n" sctp�ݲ���Ҫȥ��һ��%d//t=<start time><stop time> ;BUNDLE ��sdpMLineIndex sdpMid���һ��
                "a=group:BUNDLE %s %s\r\n"//a=group:BUNDLE��ʾ����Ƶ������һ��ͨ��
                //"a=ice-lite\r\n"
                "a=msid-semantic: WMS ywf-mslabel\r\n"//"a=msid-semantic: WMS ywf\r\n"
                );//a=setup:actpass ������ᱨ��
            iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
                tCreateTime.tv_sec,tCreateTime.tv_usec,1,"0.0.0.0",
                ptVideoInfo->strMediaID,ptAudioInfo->strMediaID);//m_pVideoID->c_str(),m_pAudioID->c_str()
        }
    }

    ptVideoLocalCandidate = &tVideoLocalCandidate;
    pVideoLocalFingerprint = strVideoLocalFingerprint;
    if(s_iAvMultiplex > 0)
    {
        ptAudioLocalCandidate = &tAudioLocalCandidate;
        memset(strAudioLocalFingerprint,0,sizeof(strAudioLocalFingerprint));
        m_pAudioDtlsOnlyHandshake->GetLocalFingerprint(strAudioLocalFingerprint,sizeof(strAudioLocalFingerprint));
        pAudioLocalFingerprint = strAudioLocalFingerprint;
    }
    else
    {
        ptAudioLocalCandidate = &tVideoLocalCandidate;
        pAudioLocalFingerprint = strVideoLocalFingerprint;
    }

    if(m_iAvDiff < 0)
    {
        if(strlen(ptAudioInfo->strFormatName) != 0)
        {
            iRet+=GenerateAudioSDP(ptAudioLocalCandidate,pAudioLocalFingerprint,ptAudioInfo,o_strSDP+iRet,i_iSdpMaxLen-iRet);
        }
        if(strlen(ptVideoInfo->strFormatName) != 0)
        {
            iRet+=GenerateVideoSDP(ptVideoLocalCandidate,pVideoLocalFingerprint,ptVideoInfo,o_strSDP+iRet,i_iSdpMaxLen-iRet);
        }
    }
    else
    {
        if(strlen(ptVideoInfo->strFormatName) != 0)
        {
            iRet+=GenerateVideoSDP(ptVideoLocalCandidate,pVideoLocalFingerprint,ptVideoInfo,o_strSDP+iRet,i_iSdpMaxLen-iRet);
        }
        if(strlen(ptAudioInfo->strFormatName) != 0)
        {
            iRet+=GenerateAudioSDP(ptAudioLocalCandidate,pAudioLocalFingerprint,ptAudioInfo,o_strSDP+iRet,i_iSdpMaxLen-iRet);
        }
    }
    //HandleMsg��ע��,�����Ⱥ�˳���ͼʱ�䶼һ��
    if(m_Libnice.SetRemoteCandidateAndSDP(NULL)<0)//����sdp�������ã���ֹ����sdp��ʱ�ã��������ú�ȴ���
        return -1;
	return iRet;
}

/*****************************************************************************
-Fuction        : GenerateVideoSDP
-Description    : GenerateVideoSDP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::GenerateVideoSDP(T_LocalCandidate *ptLocalCandidate,char *strLocalFingerprint,T_VideoInfo *ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen)
{
    string strSdpFmt("");
    int i=0;
    char strCandidate[128];
	int iRet=0;
    const char *strVideoStreamType="video";

    if(NULL == ptVideoInfo->strSPS_Base64 || NULL == ptVideoInfo->strPPS_Base64)
    {//����ֻ�ղ���ý���������
        strSdpFmt.assign("m=%s %u RTP/SAVPF %d\r\n"
            "c=IN IP4 %s\r\n"
            "a=mid:%d\r\n"//��sdpMLineIndex sdpMid���һ��
            "a=sendrecv\r\n"
            "a=rtcp-mux\r\n"
            "a=ice-ufrag:%s\r\n"
            "a=ice-pwd:%s\r\n"
            //"a=ice-lite\r\n"
            //"a=ice-options:trickle\r\n"//��ʾice��candidate�ֿ����䣬����һ��������ע�͵�
            "a=fingerprint:sha-256 %s\r\n"
            //"a=setup:passive\r\n"
            //"a=connection:new\r\n"
            "a=rtpmap:%d %s/%d\r\n"
            "a=rtcp-fb:%d nack\r\n"
            "a=rtcp-fb:%d nack pli\r\n"
            //"a=rtcp-fb:%d ccm fir\r\n"
            "a=msid:ywf-mslabel ywf-label-%d\r\n"
            "a=ssrc:%d msid:ywf-mslabel ywf-label-%d\r\n"//��rtp�е�SSRC һ��
            "a=setup:passive\r\n");//a=setup:actpass ������ᱨ��,active��ʾ�ͻ���,passive��ʾ�����actpass���ǿͻ������Ƿ����,�ɶԷ�����
            /*"a=ssrc:%ld cname:ywf%s\r\n"
            "a=ssrc:%ld msid:janus janusa0\r\n"
            "a=ssrc:%ld mslabel:janus\r\n"
            "a=ssrc:%ld label:janusa0\r\n");*/
    }
    else
    {
        strSdpFmt.assign("m=%s %u RTP/SAVPF %d\r\n"
            "c=IN IP4 %s\r\n"
            "a=mid:%s\r\n"//��sdpMLineIndex sdpMid���һ��
            "a=sendrecv\r\n"
            "a=rtcp-mux\r\n"
            "a=ice-ufrag:%s\r\n"
            "a=ice-pwd:%s\r\n"
            //"a=ice-lite\r\n"
            //"a=ice-options:trickle\r\n"//��ʾice��candidate�ֿ����䣬����һ��������ע�͵�
            "a=fingerprint:sha-256 %s\r\n"
            //"a=setup:passive\r\n"
            //"a=connection:new\r\n"
            "a=rtpmap:%d %s/%d\r\n"
            "a=rtcp-fb:%d nack\r\n"
            "a=rtcp-fb:%d nack pli\r\n"
            //"a=rtcp-fb:%d ccm fir\r\n"
            "a=fmtp:%d level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=%06X;sprop-parameter-sets=%s,%s\r\n"
            "a=msid:ywf-mslabel ywf-label-%s\r\n"
            "a=ssrc:%d msid:ywf-mslabel ywf-label-%s\r\n"//��rtp�е�SSRC һ��
            "a=setup:passive\r\n");//a=setup:actpass ������ᱨ��,active��ʾ�ͻ���,passive��ʾ�����actpass���ǿͻ������Ƿ����,�ɶԷ�����
            /*"a=ssrc:%ld cname:ywf%s\r\n"
            "a=ssrc:%ld msid:janus janusa0\r\n"
            "a=ssrc:%ld mslabel:janus\r\n"
            "a=ssrc:%ld label:janusa0\r\n");*/
    }
    for(i=0;i<ptLocalCandidate->iCurCandidateNum;i++)
    {
        if(NULL!= strstr(ptLocalCandidate->strCandidateData[i],"udp"))
        {//webrtc�ٷ�ֻ֧��:udp tcp ssltcp tls ��������ͬ��ֻ��udp
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",ptLocalCandidate->strCandidateData[i]);
            strSdpFmt.append(strCandidate);
        }
    }
    //memset(strCandidate,0,sizeof(strCandidate));
    //snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n","candidate:21 1 udp 110 139.9.149.150 9018 typ host");
    //strSdpFmt.append(strCandidate);
    //ȥ��sctpһ������ͨ����ͨ
    /*strSdpFmt.append("m=%s %u DTLS/SCTP %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//��sdpMLineIndex sdpMid��ļ�1
        "a=sendrecv\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:passive\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp"))
        {//webrtc�ٷ�ֻ֧��:udp tcp ssltcp tls ��������ͬ��ֻ��udp
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
        }
    }*/
    if(NULL == ptVideoInfo->strSPS_Base64 || NULL == ptVideoInfo->strPPS_Base64)
    {
        iRet=snprintf(o_strSDP+iRet,i_iSdpMaxLen-iRet,strSdpFmt.c_str(),
            strVideoStreamType,ptVideoInfo->wPortNumForSDP,ptVideoInfo->bRtpPayloadType,
            "0.0.0.0",//"0.0.0.0"����ʧ�ܣ����Ҳ��ʧ��
            ptVideoInfo->strMediaID,
            ptLocalCandidate->strUfrag, 
            ptLocalCandidate->strPassword,
            strLocalFingerprint,
            ptVideoInfo->bRtpPayloadType,ptVideoInfo->strFormatName,ptVideoInfo->dwTimestampFrequency,
            ptVideoInfo->bRtpPayloadType,ptVideoInfo->bRtpPayloadType,//ptVideoInfo->bRtpPayloadType,
            ptVideoInfo->strMediaID,ptVideoInfo->dwSSRC,ptVideoInfo->strMediaID);
    }
    else
    {
        iRet=snprintf(o_strSDP+iRet,i_iSdpMaxLen-iRet,strSdpFmt.c_str(),
            strVideoStreamType,ptVideoInfo->wPortNumForSDP,ptVideoInfo->bRtpPayloadType,
            "0.0.0.0",//"0.0.0.0"����ʧ�ܣ����Ҳ��ʧ��
            ptVideoInfo->strMediaID,
            ptLocalCandidate->strUfrag, 
            ptLocalCandidate->strPassword,
            strLocalFingerprint,
            ptVideoInfo->bRtpPayloadType,ptVideoInfo->strFormatName,ptVideoInfo->dwTimestampFrequency,
            ptVideoInfo->bRtpPayloadType,ptVideoInfo->bRtpPayloadType,//ptVideoInfo->bRtpPayloadType,
            ptVideoInfo->bRtpPayloadType,ptVideoInfo->dwProfileLevelId,ptVideoInfo->strSPS_Base64,ptVideoInfo->strPPS_Base64,
            ptVideoInfo->strMediaID,ptVideoInfo->dwSSRC,ptVideoInfo->strMediaID);
            /*tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec,
            "application",i_ptVideoInfo->wPortNumForSDP,102,//"m=application 9 DTLS/SCTP". Reason: Expects at least 4 fields
            "0.0.0.0",
            i_ptVideoInfo->iID+1,
            tLocalCandidate.strUfrag, 
            tLocalCandidate.strPassword,
            strLocalFingerprint);*/
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateAudioSDP
-Description    : GenerateAudioSDP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::GenerateAudioSDP(T_LocalCandidate *ptLocalCandidate,char *strLocalFingerprint,T_AudioInfo *ptAudioInfo,char *o_strSDP,int i_iSdpMaxLen)
{
    string strSdpFmt("");
    int i=0;
    char strCandidate[128];
	int iRet=0;
    const char *strAudioStreamType="audio";

    strSdpFmt.assign(
        "m=%s %u RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%s\r\n"//��sdpMLineIndex sdpMid���һ��
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        //"a=ice-lite\r\n"
        //"a=ice-options:trickle\r\n"//��ʾice��candidate�ֿ����䣬����һ��������ע�͵�
        "a=fingerprint:sha-256 %s\r\n"
        //"a=setup:passive\r\n"
        //"a=connection:new\r\n"
        "a=rtpmap:%d %s/%d\r\n"
        "a=msid:ywf-mslabel ywf-label-%s\r\n"
        "a=ssrc:%d msid:ywf-mslabel ywf-label-%s\r\n"//��rtp�е�SSRC һ��
        "a=setup:passive\r\n");//a=setup:actpass ������ᱨ��
        /*"a=ssrc:%ld cname:ywf%s\r\n"
        "a=ssrc:%ld msid:janus janusa0\r\n"
        "a=ssrc:%ld mslabel:janus\r\n"
        "a=ssrc:%ld label:janusa0\r\n");*/
    for(i=0;i<ptLocalCandidate->iCurCandidateNum;i++)
    {
        if(NULL!= strstr(ptLocalCandidate->strCandidateData[i],"udp"))
        {//webrtc�ٷ�ֻ֧��:udp tcp ssltcp tls ��������ͬ��ֻ��udp
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",ptLocalCandidate->strCandidateData[i]);
            strSdpFmt.append(strCandidate);
        }
    }
    iRet=snprintf(o_strSDP+iRet,i_iSdpMaxLen-iRet,strSdpFmt.c_str(),
        strAudioStreamType,ptAudioInfo->wPortNumForSDP,ptAudioInfo->bRtpPayloadType,
        "0.0.0.0",//
        ptAudioInfo->strMediaID,
        ptLocalCandidate->strUfrag, 
        ptLocalCandidate->strPassword,
        strLocalFingerprint,
        ptAudioInfo->bRtpPayloadType,ptAudioInfo->strFormatName,ptAudioInfo->dwTimestampFrequency,
        ptAudioInfo->strMediaID,ptAudioInfo->dwSSRC,ptAudioInfo->strMediaID);
    return iRet;
}

/*****************************************************************************
-Fuction        : GetSdpMediaInfo
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::GetSdpVideoInfo(const char * i_strSDP,T_WebRtcSdpMediaInfo *o_ptSdpMediaInfo)
{
    int iRet = -1;
	regmatch_t atMatch[MAX_MATCH_NUM];
	const char *strVideoPortPatten="m=video ([0-9]+)";
	const char *strMediaIdPatten="a=mid:([A-Za-z0-9]+)";
	const char *strRtpMapPatten=NULL;
	const char *strFmtpPatten=NULL;
	string strFindRes;
	string strSubSDP;
    string strSDP(i_strSDP);
    size_t iRtpMapPos = 0;
    size_t iFmtpPos = 0;
    int i = 0;
    unsigned short wPortNumForSDP=9;
    char *endptr;
	char strMediaID[8];//0/mid

    if (i_strSDP == NULL || NULL==o_ptSdpMediaInfo) 
    {
		WEBRTC_LOGE("GetSdpVideoInfo NULL\r\n");
		return iRet;
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == this->Regex(strVideoPortPatten,(char *)strSDP.c_str(),atMatch))
    {
        strFindRes.assign(strSDP,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);//0������
        wPortNumForSDP = atoi(strFindRes.c_str());
		//WEBRTC_LOGD("wPortNumForSDP %s,%d\r\n",strFindRes.c_str(),wPortNumForSDP);
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == this->Regex(strMediaIdPatten,(char *)strSDP.c_str(),atMatch))
    {
        strFindRes.assign(strSDP,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(strMediaID,sizeof(strMediaID),"%s",strFindRes.c_str());
		WEBRTC_LOGD("iMediaID %s,%s\r\n",strFindRes.c_str(),strMediaID);
    }
    
    strRtpMapPatten=".*a=rtpmap:([0-9]+) ([A-Za-z0-9]+)/([0-9]+).*";//a=rtpmap:96 VP8/90000
    strFmtpPatten="a=fmtp:([0-9]+) level-asymmetry-allowed=([0-9]+);packetization-mode=([0-9]+);profile-level-id=([A-Za-z0-9]+).*";//a=fmtp:127 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=4d001f
    for(i =0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
    {
        memset(atMatch,0,sizeof(atMatch));
        strSubSDP.assign(strSDP.substr(iFmtpPos,strSDP.size() - iFmtpPos).c_str());
        if(REG_NOERROR != this->Regex(strFmtpPatten,(char *)strSubSDP.c_str(),atMatch))
        {
            break;
        }
        iRet=0;
        strFindRes.assign(strSubSDP,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        o_ptSdpMediaInfo->tVideoInfos[i].bRtpPayloadType= atoi(strFindRes.c_str());
        strFindRes.assign(strSubSDP,atMatch[2].rm_so,atMatch[2].rm_eo-atMatch[2].rm_so);
        o_ptSdpMediaInfo->tVideoInfos[i].bLevelAsymmetryAllowed= atoi(strFindRes.c_str());
        strFindRes.assign(strSubSDP,atMatch[3].rm_so,atMatch[3].rm_eo-atMatch[3].rm_so);
        o_ptSdpMediaInfo->tVideoInfos[i].bPacketizationMode= atoi(strFindRes.c_str());
        strFindRes.assign(strSubSDP,atMatch[4].rm_so,atMatch[4].rm_eo-atMatch[4].rm_so);
        o_ptSdpMediaInfo->tVideoInfos[i].dwProfileLevelId= (unsigned int)strtol(strFindRes.c_str(),&endptr,16);//4d001f
		//WEBRTC_LOGD("dwProfileLevelId %s,%#x\r\n",strFindRes.c_str(),o_ptSdpMediaInfo->tVideoInfos[i].dwProfileLevelId);
        iFmtpPos+=atMatch[0].rm_so;
        memset(atMatch,0,sizeof(atMatch));
		//WEBRTC_LOGD("strSDP.substr(0,iFmtpPos).c_str() %s,%d\r\n",strSDP.substr(0,iFmtpPos).c_str(),iFmtpPos);
        strSubSDP.assign(strSDP.substr(0,iFmtpPos).c_str());
        if(REG_NOERROR != this->Regex(strRtpMapPatten,(char *)strSubSDP.c_str(),atMatch))
        {
            break;
        }
        strFindRes.assign(strSubSDP,atMatch[2].rm_so,atMatch[2].rm_eo-atMatch[2].rm_so);
        snprintf(o_ptSdpMediaInfo->tVideoInfos[i].strFormatName,sizeof(o_ptSdpMediaInfo->tVideoInfos[i].strFormatName),"%s",strFindRes.c_str());
        strFindRes.assign(strSubSDP,atMatch[3].rm_so,atMatch[3].rm_eo-atMatch[3].rm_so);
        o_ptSdpMediaInfo->tVideoInfos[i].dwTimestampFrequency = atoi(strFindRes.c_str());
        
        o_ptSdpMediaInfo->tVideoInfos[i].wPortNumForSDP= wPortNumForSDP;
        snprintf(o_ptSdpMediaInfo->tVideoInfos[i].strMediaID,sizeof(o_ptSdpMediaInfo->tVideoInfos[i].strMediaID),"%s",strMediaID);
        iFmtpPos+=strlen("a=fmtp:");
    }
    if(0 != iRet)
    {
        for(i =0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
        {
            iRtpMapPos = strSDP.find("a=rtpmap",iRtpMapPos);
            if(string::npos == iRtpMapPos)
            {
                break;
            }
            iRet=0;
            memset(atMatch,0,sizeof(atMatch));
            strSubSDP.assign(strSDP.substr(iRtpMapPos,strSDP.size() - iRtpMapPos).c_str());
            if(REG_NOERROR == this->Regex(strRtpMapPatten,(char *)strSubSDP.c_str(),atMatch))
            {
                strFindRes.assign(strSubSDP,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
                o_ptSdpMediaInfo->tVideoInfos[i].bRtpPayloadType= atoi(strFindRes.c_str());
                strFindRes.assign(strSubSDP,atMatch[2].rm_so,atMatch[2].rm_eo-atMatch[2].rm_so);
                snprintf(o_ptSdpMediaInfo->tVideoInfos[i].strFormatName,sizeof(o_ptSdpMediaInfo->tAudioInfos[i].strFormatName),"%s",strFindRes.c_str());
                strFindRes.assign(strSubSDP,atMatch[3].rm_so,atMatch[3].rm_eo-atMatch[3].rm_so);
                o_ptSdpMediaInfo->tVideoInfos[i].dwTimestampFrequency = atoi(strFindRes.c_str());
        
                o_ptSdpMediaInfo->tVideoInfos[i].wPortNumForSDP= wPortNumForSDP;
                snprintf(o_ptSdpMediaInfo->tVideoInfos[i].strMediaID,sizeof(o_ptSdpMediaInfo->tVideoInfos[i].strMediaID),"%s",strMediaID);
            }
            iRtpMapPos+=strlen("a=rtpmap");
        }
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : GetSdpAudioInfo
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::GetSdpAudioInfo(const char * i_strSDP,T_WebRtcSdpMediaInfo *o_ptSdpMediaInfo)
{
    int iRet = -1;
	regmatch_t atMatch[MAX_MATCH_NUM];
	const char *strAudioPortPatten=".*m=audio ([0-9]+) .*";
	const char *strMediaIdPatten=".*a=mid:([0-9]+).*";
	const char *strRtpMapPatten=NULL;
	const char *strFmtpPatten=NULL;
	string strFindRes;
	string strSubSDP;
    string strSDP(i_strSDP);
    size_t iRtpMapPos = 0;
    size_t iFmtpPos = 0;
    int i = 0;
    unsigned short wPortNumForSDP=9;
	int iMediaID=1;//1/mid
	char strMediaID[8];//1/mid

	
    if (i_strSDP == NULL || NULL==o_ptSdpMediaInfo) 
    {
		WEBRTC_LOGE("GetSdpAudioInfo NULL\r\n");
		return iRet;
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == this->Regex(strAudioPortPatten,(char *)strSDP.c_str(),atMatch))
    {
        strFindRes.assign(strSDP,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);//0������
        wPortNumForSDP = atoi(strFindRes.c_str());
		//WEBRTC_LOGD("audio wPortNumForSDP %s,%d\r\n",strFindRes.c_str(),wPortNumForSDP);
    }
    memset(atMatch,0,sizeof(atMatch));
    if(REG_NOERROR == this->Regex(strMediaIdPatten,(char *)strSDP.c_str(),atMatch))
    {
        strFindRes.assign(strSDP,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
        snprintf(strMediaID,sizeof(strMediaID),"%s",strFindRes.c_str());
		WEBRTC_LOGD("audio iMediaID %s,%d\r\n",strFindRes.c_str(),strMediaID);
    }
    strRtpMapPatten="a=rtpmap:([0-9]+) ([A-Za-z0-9]+)/([0-9]+).*";//����.*��ƥ�䵽���һ��rtpmap
    for(i =0;i<WEBRTC_SDP_MEDIA_INFO_MAX_NUM;i++)
    {
        iRtpMapPos = strSDP.find("a=rtpmap",iRtpMapPos);
        if(string::npos == iRtpMapPos)
        {
            break;
        }
        memset(atMatch,0,sizeof(atMatch));
        strSubSDP.assign(strSDP.substr(iRtpMapPos,strSDP.size() - iRtpMapPos).c_str());
        if(REG_NOERROR == this->Regex(strRtpMapPatten,(char *)strSubSDP.c_str(),atMatch))
        {
            strFindRes.assign(strSubSDP,atMatch[1].rm_so,atMatch[1].rm_eo-atMatch[1].rm_so);
            o_ptSdpMediaInfo->tAudioInfos[i].bRtpPayloadType= atoi(strFindRes.c_str());
            strFindRes.assign(strSubSDP,atMatch[2].rm_so,atMatch[2].rm_eo-atMatch[2].rm_so);
            snprintf(o_ptSdpMediaInfo->tAudioInfos[i].strFormatName,sizeof(o_ptSdpMediaInfo->tAudioInfos[i].strFormatName),"%s",strFindRes.c_str());
            strFindRes.assign(strSubSDP,atMatch[3].rm_so,atMatch[3].rm_eo-atMatch[3].rm_so);
            o_ptSdpMediaInfo->tAudioInfos[i].dwTimestampFrequency = atoi(strFindRes.c_str());

            o_ptSdpMediaInfo->tAudioInfos[i].wPortNumForSDP= wPortNumForSDP;
            snprintf(o_ptSdpMediaInfo->tAudioInfos[i].strMediaID,sizeof(o_ptSdpMediaInfo->tAudioInfos[i].strMediaID),"%s",strMediaID);
        }
		//WEBRTC_LOGD("strFormatName %s,%d\r\n",o_ptSdpMediaInfo->tAudioInfos[i].strFormatName,o_ptSdpMediaInfo->tAudioInfos[i].dwTimestampFrequency);

        iRtpMapPos+=strlen("a=rtpmap");
    }
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
int WebRtcAnswer::Regex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch)
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

