/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       sctp_interface.h
* Description           : 	
* Created               :       2020.02.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SCTP_INTERFACE_H
#define SCTP_INTERFACE_H




typedef struct SctpCb
{
    int (*SendToOut)(char * i_acSendBuf,int i_iSendLen);
    int (*RecvFromOut)(char * i_acRecvBuf,int i_iRecvLen);

}T_SctpCb;




/*****************************************************************************
-Class          : DtlsOnlyHandshake
-Description    : DtlsOnlyHandshake
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class Sctp
{
public:
	Sctp();
	~Sctp();
	int Init();
    int SendToOut(char * i_acSendBuf,int i_iSendLen);
    int RecvFromOut(char * i_acRecvBuf,int i_iRecvLen);

    
private:
    static int SendToOutCb(void *addr, void *buffer, int length, unsigned char tos, unsigned char set_df);
    static int RecvFromOutCb(struct socket *sock, union sctp_sockstore addr, void *data,int datalen, struct sctp_rcvinfo, int flags, void *ulp_info); 
    void Debug(const char *fmt, ...);
    int ConfigureSocket(struct socket * i_ptSocket);


    struct socket *m_ptSocket;
    T_SctpCb m_tSctpCb;

};



#endif
