/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc_interface.cpp
* Description           : 	    接口层，防止曝露内部文件
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
WebRtcInterface::WebRtcInterface(T_WebRtcCfg i_tWebRtcCfg,T_WebRtcCb i_tWebRtcCb,void *pCbObj)
{
    m_pHandle = NULL;
    memcpy(&m_tWebRtcCfg,&i_tWebRtcCfg,sizeof(T_WebRtcCfg));
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
        m_pHandle = new WebRtcOffer(m_tWebRtcCfg.strStunAddr,m_tWebRtcCfg.dwStunPort,m_tWebRtcCfg.eControlling);
    else
        m_pHandle = new WebRtcAnswer(m_tWebRtcCfg.strStunAddr,m_tWebRtcCfg.dwStunPort,m_tWebRtcCfg.eControlling);
    WebRTC *pWebRTC = (WebRTC *)m_pHandle;
    pWebRTC->SetCallback(&i_tWebRtcCb,pCbObj);
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
    WebRTC *pWebRTC = (WebRTC *)m_pHandle;
    return pWebRTC->Proc();
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
int WebRtcInterface::StopProc()
{
    WebRTC *pWebRTC = (WebRTC *)m_pHandle;
    return pWebRTC->StopProc();
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
int WebRtcInterface::DtlsInit()
{
    WebRTC *pWebRTC = (WebRTC *)m_pHandle;
    return pWebRTC->DtlsInit();
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
int WebRtcInterface::GetStopedFlag()
{
    WebRTC *pWebRTC = (WebRTC *)m_pHandle;
    return pWebRTC->GetStopedFlag();
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
int WebRtcInterface::GenerateLocalSDP(T_WebRtcMediaInfo *i_ptMediaInfo,char *o_strSDP,int i_iSdpMaxLen)
{
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
    {
        WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
        return pWebRtcOffer->GenerateLocalSDP(i_ptMediaInfo,o_strSDP,i_iSdpMaxLen);
    }
    WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
    return pWebRtcAnswer->GenerateLocalSDP(i_ptMediaInfo,o_strSDP,i_iSdpMaxLen);
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
int WebRtcInterface::HandleMsg(char * i_strMsg,int i_iNotJsonMsgFlag,T_WebRtcSdpMediaInfo *o_ptSdpMediaInfo)
{
    if(WEBRTC_OFFER_ROLE == m_tWebRtcCfg.eWebRtcRole)
    {
        WebRtcOffer *pWebRtcOffer = (WebRtcOffer *)m_pHandle;
        return pWebRtcOffer->HandleMsg(i_strMsg,i_iNotJsonMsgFlag,o_ptSdpMediaInfo);
    }
    WebRtcAnswer *pWebRtcAnswer = (WebRtcAnswer *)m_pHandle;
    return pWebRtcAnswer->HandleMsg(i_strMsg,i_iNotJsonMsgFlag,o_ptSdpMediaInfo);
}

/*****************************************************************************
-Fuction        : HandleCandidateMsg
-Description    : Offer消息必须是是在Candidate之前的，有这样的时序要求
这是webrtc抓包发现的，所以不符合这个时序则返回错误
-Input          : i_iNotJsonMsgFlag 1表示非json
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
-Description    : -1不可发送,0准备好通道可以发送
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcInterface::GetGatheringDoneFlag()
{
    WebRTC *pWebRTC = (WebRTC *)m_pHandle;
    return pWebRTC->GetGatheringDoneFlag();
}
/*****************************************************************************
-Fuction        : GetSendReadyFlag
-Description    : -1不可发送,0准备好通道可以发送
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcInterface::GetSendReadyFlag()
{
    WebRTC *pWebRTC = (WebRTC *)m_pHandle;
    return pWebRTC->GetSendReadyFlag();
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
int WebRtcInterface::SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen,int i_iStreamType)
{
    WebRTC *pWebRTC = (WebRTC *)m_pHandle;
    return pWebRTC->SendProtectedRtp(i_acRtpBuf,i_iRtpBufLen,i_iStreamType);
}




