/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       sctp_interface.cpp
* Description           : 		
�ο�webrtc�ٷ�Դ��sctp_transport.cc
ʹ��usrsctp ��Դ��
sctp�����Զ���Ӧ���������Э�飬
��webrtc�е�DataChannel(����ͨ��)

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
#include "usrsctp.h"



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
Sctp::Sctp()
{
    m_ptSocket=NULL:
    memset(&m_tSctpCb,0,sizeof(m_tSctpCb));

    
    usrsctp_init(0, &Sctp::SendCb, &Sctp::Debug);

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


	usrsctp_shutdown(s, SHUT_WR);
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
-Description    : �ο�openssl��demo��һ��
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
    m_ptSocket = usrsctp_socket(AF_CONN, SOCK_STREAM, IPPROTO_SCTP, &Sctp::RecvFromOutCb,NULL,0,this);
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
    remote_sconn.sconn_port = htons(5000);//?
    remote_sconn.sconn_addr = this;
    int connect_result = usrsctp_connect(m_ptSocket,(struct sockaddr *)&remote_sconn, sizeof(struct sockaddr_conn));
    if (connect_result < 0 && errno != SCTP_EINPROGRESS) 
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
-Description    : ����SendToOut�ӿں�ᴥ���ûص�����,Ȼ����
���ⲿ���ݾ���SendToOut�ӿڴ�����ٴ����ú��������ⲿ
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
    ssize_t send_res = usrsctp_sendv(
        sock_, message->data(), message->size(), NULL, 0, &spa,
        rtc::checked_cast<socklen_t>(sizeof(spa)), SCTP_SENDV_SPA, 0);

    iRet=0;
    return iRet;
}
/*****************************************************************************
-Fuction        : SendToOutCb
-Description    : ����SendToOut�ӿں�ᴥ���ûص�����,Ȼ����
���ⲿ���ݾ���SendToOut�ӿڴ�����ٴ����ú��������ⲿ
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Sctp::SendToOutCb(void *addr, void *buffer, int length, unsigned char tos, unsigned char set_df)
{
    int iRet = -1;
    
    m_tSctpCb.SendToOut(buffer,length);
    iRet=0;
    return iRet;
}
/*****************************************************************************
-Fuction        : RecvFromOut
-Description    : ����������ݴ�����,���������ջᴫ��ȥ
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
}
/*****************************************************************************
-Fuction        : RecvFromOutCb
-Description    : 
����RecvFromOut�ӿں�ᴥ���ûص�����,Ȼ����³�ȥ
���ⲿ���ݾ���RecvFromOut�ӿڴ�����ٴ����ú��������ⲿ
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Sctp::RecvFromOutCb(struct socket *sock, union sctp_sockstore addr, void *data,int datalen, struct sctp_rcvinfo, int flags, void *ulp_info) 
{



    m_tSctpCb.RecvFromOut(data,datalen);
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



