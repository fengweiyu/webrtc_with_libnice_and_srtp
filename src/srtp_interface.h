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
#ifndef SRTP_INTERFACE_H
#define SRTP_INTERFACE_H

#include "srtp.h"


typedef enum SrtpSsrcType
{
    SRTP_SSRC_UNDEFINED = 0,   /**< Indicates an undefined SSRC type.    */
    SRTP_SSRC_NEED_VALUE = 1,    /**< Indicates a specific SSRC value      */
    SRTP_SSRC_UNPROTECT = 2, /**< Indicates any inbound SSRC value     */
	                          /**< (i.e. a value that is used in the    */
	                          /**< function srtp_unprotect())           */
    SRTP_SSRC_PROTECT = 3 /**< Indicates any outbound SSRC value    */
	                          /**< (i.e. a value that is used in the    */
	                          /**< function srtp_protect())             */
} E_SrtpSsrcType;



/*****************************************************************************
-Class          : Srtp
-Description    : Srtp
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class Srtp
{
public:
	Srtp();
	~Srtp();
    int Create(char *i_acKey,int i_iKeyLen,E_SrtpSsrcType i_eSrtpSsrcType);
    int ProtectRtp(char * i_acRtpData,int * o_piProtectDataLen,int i_iRtpDataLen);
    int UnProtectRtp(char * m_acSrtpData,int * m_piDataLen);
    int Shutdown();


    static int m_iSrtpLibInited;//0未初始化，1已初始化
    
private:
    srtp_t m_tSrtp;

};





#endif
