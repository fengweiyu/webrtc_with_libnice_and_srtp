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
WebRTC::WebRTC(char * i_strStunAddr,unsigned int i_dwStunPort,int i_iControlling):m_Libnice(i_strStunAddr,i_dwStunPort,i_iControlling),m_Srtp();
{
    m_pDtlsOnlyHandshake = NULL;
    memset(&m_tDtlsOnlyHandshakeCb,0,sizeof(T_DtlsOnlyHandshakeCb));
    m_tDtlsOnlyHandshakeCb.SendDataOut=m_Libnice.SendData;
    m_pDtlsOnlyHandshake = new DtlsOnlyHandshake(m_tDtlsOnlyHandshakeCb);
    
    memset(&m_tLibniceCb,0,sizeof(T_LibniceCb));
    m_tLibniceCb.Handshake= m_pDtlsOnlyHandshake->Handshake;
    m_tLibniceCb.HandleRecvData= m_pDtlsOnlyHandshake->HandleRecvData;
    m_Libnice.SetCallback(&m_tLibniceCb)
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
int WebRTC::HandleOfferMsg(char * i_strOfferMsg,int i_iOfferMsgLen)
{
    return m_Libnice.LibniceProc();
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
int WebRTC::SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen);
{

}
