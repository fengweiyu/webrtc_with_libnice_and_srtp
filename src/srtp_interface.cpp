/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       srtp_interface.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "srtp_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int Srtp::m_iSrtpLibInited = 0;//0未初始化，1已初始化
/*****************************************************************************
-Fuction        : SrtpInit
-Description    : v
-Input          :  
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
Srtp::Srtp()
{	
	m_tSrtp=NULL;
	if(0 == m_iSrtpLibInited)
	{
        (void)srtp_init();
        m_iSrtpLibInited = 1;
	}
}


/*****************************************************************************
-Fuction        : SrtpInit
-Description    : v
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
Srtp::~Srtp()
{	
	if(0 != m_iSrtpLibInited)
	{
	    if(NULL!= m_tSrtp)
    	    srtp_dealloc(m_tSrtp);
        (void)srtp_shutdown();
        m_iSrtpLibInited = 0;
	}
}


/*****************************************************************************
-Fuction        : SrtpCreate
-Description    : SrtpCreate
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Srtp::Create(char *i_acKey,int i_iKeyLen,E_SrtpSsrcType i_eSrtpSsrcType)
{
	int iRet=-1;
	srtp_policy_t tPolicy;
	char acKey[SRTP_MAX_KEY_LEN];//可能要静态

	if(i_acKey == NULL ||i_iKeyLen>SRTP_MAX_KEY_LEN)
	{
		printf("SrtpCreate null\r\n");
		return iRet;
	}
	memset(acKey,0,sizeof(acKey));
	memset(&tPolicy,0,sizeof(srtp_policy_t));
	memcpy(acKey,i_acKey,i_iKeyLen);
    srtp_crypto_policy_set_rtp_default(&tPolicy.rtp);//不确定
    srtp_crypto_policy_set_rtcp_default(&tPolicy.rtcp);//
    tPolicy.ssrc.type =(srtp_ssrc_type_t) i_eSrtpSsrcType;
    //tPolicy.ssrc.value = 0;
    tPolicy.key = (unsigned char *)acKey;
    tPolicy.next = NULL;
    
    //tPolicy.window_size = 128;//
    //tPolicy.allow_repeat_tx = 0;//
    
	memset(&m_tSrtp,0,sizeof(srtp_t));
	iRet = srtp_create(&m_tSrtp, &tPolicy);
	return iRet;
}


/*****************************************************************************
-Fuction        : SrtpProtectRtp
-Description    : SrtpProtectRtp
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Srtp::ProtectRtp(char * i_acRtpData,int * o_piProtectDataLen,int i_iRtpDataLen)
{
    int iRet = -1;
	if(i_acRtpData == NULL ||o_piProtectDataLen == NULL)
	{
		printf("ProtectRtp null\r\n");
		return iRet;
	}
	*o_piProtectDataLen=i_iRtpDataLen;
	iRet = srtp_protect(m_tSrtp, i_acRtpData, o_piProtectDataLen);
    return iRet;
}

/*****************************************************************************
-Fuction        : UnProtectRtp
-Description    : srtp_err_status_t srtp_unprotect(srtp_t ctx, void *srtp_hdr, int *len_ptr);
-Input          : 
-Output         : 
-Return         : srtp_err_status_ok
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Srtp::UnProtectRtp(char * m_acSrtpData,int * m_piDataLen)
{
    int iRet = -1;
	if(m_acSrtpData == NULL ||m_piDataLen == NULL)
	{
		printf("UnProtectRtp null\r\n");
		return iRet;
	}
	iRet = srtp_unprotect(m_tSrtp, m_acSrtpData, m_piDataLen);
    return iRet;
}

/*****************************************************************************
-Fuction        : UnProtectRtp
-Description    : srtp_err_status_t srtp_unprotect(srtp_t ctx, void *srtp_hdr, int *len_ptr);
-Input          : 
-Output         : 
-Return         : srtp_err_status_ok
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Srtp::UnProtectRtcp(char * m_acSrtpData,int * m_piDataLen)
{
    int iRet = -1;
	if(m_acSrtpData == NULL ||m_piDataLen == NULL)
	{
		printf("UnProtectRtp null\r\n");
		return iRet;
	}
	iRet = srtp_unprotect_rtcp(m_tSrtp, m_acSrtpData, m_piDataLen);
    return iRet;
}


/*****************************************************************************
-Fuction        : SrtpShutdown
-Description    : SrtpShutdown
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Srtp::Shutdown()
{
    //return srtp_shutdown();
}


