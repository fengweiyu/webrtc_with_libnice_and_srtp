/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RtpSession.cpp
* Description		: 	RtpSession operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include <stdlib.h>//������Ҫ.h
#include <stdio.h>
#include <string.h>
#include <iostream>//����.h,c++�µ�ͷ�ļ�

#include "Definition.h"
#include "RtpSession.h"

using std::cout;//��Ҫ<iostream>
using std::endl;

/*****************************************************************************
-Fuction		: RtpSession
-Description	: RtpSession
-Input			: i_iVideoOrAudio 0 video,1audio
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpSession :: RtpSession(int i_wPayloadType,unsigned int i_dwSampleRate)
{
    m_pRtpClientOverUDP = NULL;
    m_pRtpClientOverTCP = NULL;
    m_pRtcpClientOverTCP = NULL;
    m_pRtcpClientOverUDP = NULL;
    memset(&m_tRtpPacketParam,0,sizeof(m_tRtpPacketParam));
    m_tRtpPacketParam.dwSSRC=GetSSRC();
    m_tRtpPacketParam.dwTimestampFreq=i_dwSampleRate;
    m_tRtpPacketParam.wPayloadType=i_wPayloadType;
}

/*****************************************************************************
-Fuction		: RtpSession
-Description	: ~RtpSession
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpSession :: ~RtpSession()
{
    if(NULL != m_pRtpClientOverUDP)
    {
        delete m_pRtpClientOverUDP;
        m_pRtpClientOverUDP = NULL;
    }
        
    if(NULL != m_pRtcpClientOverUDP)
    {
        delete m_pRtcpClientOverUDP;
        m_pRtcpClientOverUDP = NULL;
    }
        
    if(NULL != m_pRtpClientOverTCP)
    {
        delete m_pRtpClientOverTCP;
        m_pRtpClientOverTCP = NULL;
    }
        
    if(NULL != m_pRtcpClientOverTCP)
    {
        delete m_pRtcpClientOverTCP;
        m_pRtcpClientOverTCP = NULL;
    }
}

/*****************************************************************************
-Fuction		: Init
-Description	: Init
-Input			: i_blIsTcp false udp,true tcp
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpSession :: Init(bool i_blIsTcp,string i_strLocalIP,string i_strIP,unsigned short i_wRtpPort,unsigned short i_wRtcpPort)
{
    int iRet=FALSE;
    unsigned short wPort=0;
    if(false == i_blIsTcp)
    {
        if(NULL == m_pRtpClientOverUDP)
            m_pRtpClientOverUDP = new UdpClient();
        if(NULL == m_pRtcpClientOverUDP)
            m_pRtcpClientOverUDP = new UdpClient();
            
        for (int i= 65536/4*3/2*2;i< 65536;i += 2) 
        {
            wPort=i;
            if(FALSE==m_pRtpClientOverUDP->Init(&i_strIP,i_wRtpPort,&i_strLocalIP,wPort))
            {
            }
            else
            {
                wPort=i+1;
                if(FALSE==m_pRtcpClientOverUDP->Init(&i_strIP,i_wRtcpPort,&i_strLocalIP,wPort))
                {
                }
                else
                {
                    iRet=TRUE;
                    break;
                }
            }
        }

    }
    else
    {
        if(NULL == m_pRtpClientOverTCP)
            m_pRtpClientOverTCP = new TcpClient();
        if(NULL == m_pRtcpClientOverTCP)
            m_pRtcpClientOverTCP = new TcpClient();
        iRet=m_pRtpClientOverTCP->Init(i_strIP,i_wRtpPort);
        iRet|=m_pRtcpClientOverTCP->Init(i_strIP,i_wRtcpPort);
    }
    if(iRet!=TRUE)
    {
        iRet=FALSE;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : SendRtpData
-Description    : �����Ĳ�����ʽ
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RtpSession::SendRtpData(char * i_acSendBuf,int i_iSendLen)
{
    int iRet=FALSE;
    if(NULL == m_pRtpClientOverUDP)
    {
        iRet=m_pRtpClientOverTCP->Send(i_acSendBuf,i_iSendLen);
    }
    else
    {
        iRet=m_pRtpClientOverUDP->Send(i_acSendBuf,i_iSendLen);
    }

    return iRet;
}

/*****************************************************************************
-Fuction        : GetRtpSocket
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RtpSession::GetRtpSocket()
{
    int iRet=FALSE;
    if(NULL == m_pRtpClientOverUDP)
    {
        iRet=m_pRtpClientOverTCP->GetClientSocket();
    }
    else
    {
        iRet=m_pRtpClientOverUDP->GetClientSocket();
    }

    return iRet;
}

/*****************************************************************************
-Fuction        : GetRtcpSocket
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RtpSession::GetRtcpSocket()
{
    int iRet=FALSE;
    if(NULL == m_pRtcpClientOverUDP)
    {
        iRet=m_pRtcpClientOverTCP->GetClientSocket();
    }
    else
    {
        iRet=m_pRtcpClientOverUDP->GetClientSocket();
    }

    return iRet;
}
/*****************************************************************************
-Fuction        : GetRtpPacketParam
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RtpSession::GetRtpPacketParam(T_RtpPacketParam *o_RtpPacketParam)
{
    int iRet=FALSE;
    if(NULL == o_RtpPacketParam)
    {
        cout<<"GetRtpPacketParam err NULL"<<endl;
    }
    else
    {
        memcpy(o_RtpPacketParam,&m_tRtpPacketParam,sizeof(T_RtpPacketParam));
        iRet=TRUE;
    }
    return iRet;
}
/*****************************************************************************
-Fuction        : GetRtpPacketParam
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RtpSession::SetRtpPacketParam(T_RtpPacketParam *i_ptRtpPacketParam)
{
    int iRet=FALSE;
    if(NULL == i_ptRtpPacketParam)
    {
        cout<<"SetRtpPacketParam err NULL"<<endl;
    }
    else
    {
        memcpy(&m_tRtpPacketParam,i_ptRtpPacketParam,sizeof(T_RtpPacketParam));
        iRet=TRUE;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : GetSSRC
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
unsigned int RtpSession:: GetSSRC(void)
{
	static unsigned int s_wSSRC = 0x22345678;
	return s_wSSRC++;
}

/*****************************************************************************
-Fuction        : Close
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
void RtpSession::Close()
{
    if(NULL == m_pRtpClientOverUDP)
    {
        m_pRtpClientOverTCP->Close();
    }
    else
    {
        m_pRtpClientOverUDP->Close();
    }

    if(NULL == m_pRtcpClientOverUDP)
    {
        m_pRtcpClientOverTCP->Close();
    }
    else
    {
        m_pRtcpClientOverUDP->Close();
    }
}

