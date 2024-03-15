/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc_interface.cpp
* Description           : 	    �ӿڲ㣬��ֹ��¶�ڲ��ļ�
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "webrtc.h"
#include "webrtc_interface.h"


/*****************************************************************************
-Fuction        : WebRtcInterface
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcInterface::WebRtcInterface(T_WebRtcCfg i_tWebRtcCfg,T_WebRtcCb i_tWebRtcCb)
{
    m_pHandle = NULL;
    memcpy(&m_tWebRtcCfg,&i_tWebRtcCfg,sizeof(T_WebRtcCfg));
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
        m_pHandle = new WebRtcOffer(m_tWebRtcCfg.strStunAddr,m_tWebRtcCfg.dwStunPort,m_tWebRtcCfg.eControlling);
    else
        m_pHandle = new WebRtcAnswer(m_tWebRtcCfg.strStunAddr,m_tWebRtcCfg.dwStunPort,m_tWebRtcCfg.eControlling);
}
/*****************************************************************************
-Fuction        : ~WebRtcInterface
-Description    : ~WebRtcInterface
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcInterface::~WebRtcInterface()
{
    if(NULL != m_pHandle)
    {
        if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
        {
            WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
            delete pWebRtcOffer;
        }
        else
        {
            WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
            delete pWebRtcAnswer;
        }
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
int WebRtcInterface::Proc()
{
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
    {
        WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
        return pWebRtcOffer->Proc();
    }
    WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
    return pWebRtcAnswer->Proc();
}


/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcInterface::GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen)
{
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
    {
        WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
        return pWebRtcOffer->GenerateLocalSDP(i_ptVideoInfo,o_strSDP,i_iSdpMaxLen);
    }
    WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
    return pWebRtcAnswer->GenerateLocalSDP(i_ptVideoInfo,o_strSDP,i_iSdpMaxLen);
}

/*****************************************************************************
-Fuction        : HandleMsg
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcInterface::HandleMsg(char * i_strMsg,int i_iNotJsonMsgFlag)
{
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
    {
        WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
        return pWebRtcOffer->HandleMsg(i_strMsg,i_iNotJsonMsgFlag);
    }
    WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
    return pWebRtcAnswer->HandleMsg(i_strMsg,i_iNotJsonMsgFlag);
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
int WebRtcInterface::HandleCandidateMsg(char * i_strCandidateMsg,int i_iNotJsonMsgFlag)
{
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
    {
        WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
        return pWebRtcOffer->HandleCandidateMsg(i_strCandidateMsg,i_iNotJsonMsgFlag);
    }
    WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
    return pWebRtcAnswer->HandleCandidateMsg(i_strCandidateMsg,i_iNotJsonMsgFlag);
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
int WebRtcInterface::GetSendReadyFlag()
{
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
    {
        WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
        return pWebRtcOffer->GetSendReadyFlag();
    }
    WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
    return pWebRtcAnswer->GetSendReadyFlag();
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
int WebRtcInterface::SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen)
{
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
    {
        WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
        return pWebRtcOffer->SendProtectedRtp(i_acRtpBuf,i_iRtpBufLen);
    }
    WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
    return pWebRtcAnswer->SendProtectedRtp(i_acRtpBuf,i_iRtpBufLen);
}



