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
	memset(&m_tSrtp,0,sizeof(srtp_t));
    (void)srtp_init();
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
	char acKey[SRTP_MAX_KEY_LEN];//����Ҫ��̬

	if(i_acKey == NULL ||i_iKeyLen>SRTP_MAX_KEY_LEN)
	{
		printf("SrtpCreate null\r\n");
		return iRet;
	}
	memset(acKey,0,sizeof(acKey));
	memset(&tPolicy,0,sizeof(srtp_policy_t));
	memcpy(acKey,i_acKey,i_iKeyLen);
    srtp_crypto_policy_set_rtp_default(&tPolicy.rtp);//��ȷ��
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
    return srtp_shutdown();
}


