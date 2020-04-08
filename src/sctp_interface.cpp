/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       sctp_interface.cpp
* Description           : 		
参考webrtc官方源码sctp_transport.cc
使用usrsctp 开源库
sctp传输自定义应用数据相关协议，
即webrtc中的DataChannel(数据通道)

* Created               :       2020.02.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "sctp_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>

// DataMessageType is used for the SCTP "Payload Protocol Identifier", as
// defined in http://tools.ietf.org/html/rfc4960#section-14.4
//
// For the list of IANA approved values see:
// http://www.iana.org/assignments/sctp-parameters/sctp-parameters.xml
// The value is not used by SCTP itself. It indicates the protocol running
// on top of SCTP.
enum PayloadProtocolIdentifier {
  PPID_NONE = 0,  // No protocol is specified.
  // Matches the PPIDs in mozilla source and
  // https://datatracker.ietf.org/doc/draft-ietf-rtcweb-data-protocol Sec. 9
  // They're not yet assigned by IANA.
  PPID_CONTROL = 50,
  PPID_BINARY_PARTIAL = 52,
  PPID_BINARY_LAST = 53,
  PPID_TEXT_PARTIAL = 54,
  PPID_TEXT_LAST = 51
};


/*****************************************************************************
-Fuction        : Sctp
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
Sctp::Sctp(T_SctpCb *i_ptSctpCb)
{
    m_ptSocket=NULL;
    memset(&m_tSctpCb,0,sizeof(m_tSctpCb));

    memcpy(&m_tSctpCb,i_ptSctpCb,sizeof(m_tSctpCb));
    usrsctp_init(0, &Sctp::SendToOutCb, &Sctp::Debug);

    // To turn on/off detailed SCTP debugging. You will also need to have the
    // SCTP_DEBUG cpp defines flag, which can be turned on in media/BUILD.gn.
    // usrsctp_sysctl_set_sctp_debug_on(SCTP_DEBUG_ALL);
    usrsctp_sysctl_set_sctp_ecn_enable(0);
}

/*****************************************************************************
-Fuction        : ~Sctp
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
Sctp::~Sctp()
{
    int i;


	usrsctp_shutdown(m_ptSocket, SHUT_WR);
    for (i = 0; i < 300; ++i) 
    {  
        if (usrsctp_finish() == 0) 
        {
            break;
        }
        sleep(1);
    }

}



/*****************************************************************************
-Fuction        : DtlsInit
-Description    : 参考openssl的demo简化一下
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Sctp::Init()
{
    int ret = -1;
    m_ptSocket = usrsctp_socket(AF_CONN, SOCK_STREAM, IPPROTO_SCTP, &Sctp::RecvFromOutCb,NULL,(unsigned int)0,this);
    if(NULL == m_ptSocket)
    {
        printf("Sctp usrsctp_socket err \r\n");
        return ret;
    }

    if(0 != ConfigureSocket(m_ptSocket))
    {
        usrsctp_close(m_ptSocket);
        m_ptSocket = NULL;
        printf("Sctp ConfigureSocket err \r\n");
        return ret;
    }

    // Register this class as an address for usrsctp. This is used by SCTP to
    // direct the packets received (by the created socket) to this class.
    usrsctp_register_address(this);


    sockaddr_conn local_sconn;
    memset(&local_sconn,0, sizeof(struct sockaddr_conn));
    local_sconn.sconn_family = AF_CONN;
#ifdef HAVE_SCONN_LEN
    local_sconn.sconn_len = sizeof(sockaddr_conn);
#endif
    // Note: conversion from int to uint16_t happens here.
    local_sconn.sconn_port = htons(5000);//0
    local_sconn.sconn_addr = this;
    if (usrsctp_bind(m_ptSocket, (struct sockaddr *)&local_sconn, sizeof(struct sockaddr_conn)) < 0) 
    {
        usrsctp_close(m_ptSocket);
        m_ptSocket = NULL;
        printf("Sctp usrsctp_bind err \r\n");
        return ret;
    }

    // Note: conversion from int to uint16_t happens on assignment.
    sockaddr_conn remote_sconn;
    memset(&remote_sconn,0, sizeof(struct sockaddr_conn));
    remote_sconn.sconn_family = AF_CONN;
#ifdef HAVE_SCONN_LEN
    remote_sconn.sconn_len = sizeof(sockaddr_conn);
#endif
    // Note: conversion from int to uint16_t happens here.
    remote_sconn.sconn_port = htons(5000);//? ，先随便填
    remote_sconn.sconn_addr = this;
    int connect_result = usrsctp_connect(m_ptSocket,(struct sockaddr *)&remote_sconn, sizeof(struct sockaddr_conn));
    if (connect_result < 0) 
    {
        usrsctp_close(m_ptSocket);
        m_ptSocket = NULL;
        printf("Sctp usrsctp_bind err \r\n");
        return ret;
    }

    
    ret = 0;
	return ret;
}
/*****************************************************************************
-Fuction        : SendToOut
-Description    : 调用SendToOut接口后会触发该回调函数,然后发走
即外部数据经过SendToOut接口处理后再触发该函数发回外部
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Sctp::SendToOut(char * i_acSendBuf,int i_iSendLen)
{
    int iRet = -1;
    //void *info 参数有些类似透传,会在RecvFromOutCb中sctp_rcvinfo参数再次出现
    //参考webrtc官方源码
    struct sctp_sendv_spa spa;

    memset(&spa,0,sizeof(struct sctp_sendv_spa));
    spa.sendv_flags |= SCTP_SEND_SNDINFO_VALID;
    spa.sendv_sndinfo.snd_sid = 0;
    spa.sendv_sndinfo.snd_ppid = htonl(PPID_TEXT_LAST);//
    // Explicitly marking the EOR flag turns the usrsctp_sendv call below into a
    // non atomic operation. This means that the sctp lib might only accept the
    // message partially. This is done in order to improve throughput, so that we
    // don't have to wait for an empty buffer to send the max message length, for
    // example.
    spa.sendv_sndinfo.snd_flags |= SCTP_EOR;
    iRet = usrsctp_sendv(m_ptSocket,i_acSendBuf,i_iSendLen,NULL,0,&spa,(socklen_t)sizeof(spa),SCTP_SENDV_SPA,0);
    return iRet;
}
/*****************************************************************************
-Fuction        : SendToOutCb
-Description    : 调用SendToOut接口后会触发该回调函数,然后发走
即外部数据经过SendToOut接口处理后再触发该函数发回外部
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Sctp::SendToOutCb(void *addr, void *buffer,size_t length, uint8_t tos, uint8_t set_df)
{
    int iRet = -1;
    Sctp *pSctp=NULL;
    
    pSctp= (Sctp*)(addr);
    if(NULL == pSctp)
    {
        printf("SendToOutCb err\r\n");
        return iRet;
    }

    if(NULL == pSctp->m_tSctpCb.SendToOut)
    {
        printf("pSctp->m_tSctpCb.SendToOut err\r\n");
        return iRet;
    }
    else
    {
        iRet=pSctp->m_tSctpCb.SendToOut((char *)buffer,length);
    }
    return iRet;
}
/*****************************************************************************
-Fuction        : RecvFromOut
-Description    : 把外面的数据传进来,解析后最终会传回去
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Sctp::RecvFromOut(char * i_acRecvBuf,int i_iRecvLen)
{
    usrsctp_conninput(this, i_acRecvBuf, i_iRecvLen, 0);

    return 0;
}
/*****************************************************************************
-Fuction        : RecvFromOutCb
-Description    : 
调用RecvFromOut接口后会触发该回调函数,然后回吐出去
即外部数据经过RecvFromOut接口处理后再触发该函数发回外部
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Sctp::RecvFromOutCb(struct socket *sock, union sctp_sockstore addr, void *data,size_t datalen, struct sctp_rcvinfo, int flags, void *ulp_info) 
{
    int iRet = -1;
    Sctp *pSctp=NULL;
    
    pSctp= (Sctp*)(ulp_info);
    if(NULL == pSctp)
    {
        printf("RecvFromOutCb err\r\n");
        return iRet;
    }
    if(NULL == pSctp->m_tSctpCb.RecvFromOut)
    {
        printf("pSctp->m_tSctpCb.RecvFromOut err\r\n");
    }
    else
    {
        iRet=pSctp->m_tSctpCb.RecvFromOut((char *)data,datalen);
    }
    return iRet;
}


/*****************************************************************************
-Fuction        : Debug
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
void Sctp::Debug(const char *fmt, ...)
{
    char temp[1024];
    va_list ap;
    va_start (ap, fmt);
    vsnprintf(temp,sizeof(temp),fmt,ap);
    va_end (ap);
    printf("%s\r\n",temp);
}



/*****************************************************************************
-Fuction        : ConfigureSocket
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Sctp::ConfigureSocket(struct socket * i_ptSocket)
{
    int ret = -1;
    if(NULL == i_ptSocket)
    {
        printf("ConfigureSocket err\r\n");
        return ret;
    }

    // Make the socket non-blocking. Connect, close, shutdown etc will not block
    // the thread waiting for the socket operation to complete.
    if (usrsctp_set_non_blocking(i_ptSocket, 1) < 0)
    {
        printf("usrsctp_set_non_blocking err\r\n");
        return ret;
    }

    // This ensures that the usrsctp close call deletes the association. This
    // prevents usrsctp from calling OnSctpOutboundPacket with references to
    // this class as the address.
    linger linger_opt;
    linger_opt.l_onoff = 1;
    linger_opt.l_linger = 0;
    if (usrsctp_setsockopt(i_ptSocket, SOL_SOCKET, SO_LINGER, &linger_opt,sizeof(linger_opt))) 
    {
        printf("usrsctp_setsockopt SO_LINGER err\r\n");
        return ret;
    }

    // Enable stream ID resets.
    struct sctp_assoc_value stream_rst;
    stream_rst.assoc_id = SCTP_ALL_ASSOC;
    stream_rst.assoc_value = 1;
    if (usrsctp_setsockopt(i_ptSocket, IPPROTO_SCTP, SCTP_ENABLE_STREAM_RESET,&stream_rst, sizeof(stream_rst))) 
    {
        printf("usrsctp_setsockopt SCTP_ENABLE_STREAM_RESET err\r\n");
        return ret;
    }

    // Nagle.
    unsigned int nodelay = 1;
    if (usrsctp_setsockopt(i_ptSocket, IPPROTO_SCTP, SCTP_NODELAY, &nodelay,sizeof(nodelay))) 
    {
        printf("usrsctp_setsockopt SCTP_NODELAY err\r\n");
        return ret;
    }

    // Explicit EOR.
    unsigned int eor = 1;
    if (usrsctp_setsockopt(i_ptSocket, IPPROTO_SCTP, SCTP_EXPLICIT_EOR, &eor,sizeof(eor))) 
    {
        printf("usrsctp_setsockopt SCTP_EXPLICIT_EOR err\r\n");
        return ret;
    }

    // Subscribe to SCTP event notifications.
    int event_types[] = {SCTP_ASSOC_CHANGE, SCTP_PEER_ADDR_CHANGE,
                         SCTP_SEND_FAILED_EVENT, SCTP_SENDER_DRY_EVENT,
                         SCTP_STREAM_RESET_EVENT};
    struct sctp_event event = {0};
    event.se_assoc_id = SCTP_ALL_ASSOC;
    event.se_on = 1;
    int i;
    for (i = 0; i < sizeof(event_types)/sizeof(int); i++) 
    {
        event.se_type = event_types[i];
        if (usrsctp_setsockopt(i_ptSocket, IPPROTO_SCTP, SCTP_EVENT, &event,sizeof(event)) < 0) 
        {
            printf("usrsctp_setsockopt SCTP_EVENT err\r\n");
            return ret;
        }
    }
    ret = 0;
    return ret;
}



