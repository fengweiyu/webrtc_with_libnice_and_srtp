/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       dtls_only_handshake.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef DTLS_ONLY_HANDSHAKE_H
#define DTLS_ONLY_HANDSHAKE_H




/*****************************************************************************
-Class          : DtlsOnlyHandshake
-Description    : DtlsOnlyHandshake
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class DtlsOnlyHandshake
{
public:
	DtlsOnlyHandshake();
	~DtlsOnlyHandshake();
	int Init();


private:
     SSL_CTX * m_ptSslCtx = NULL;
     X509 * m_ptSslCert = NULL;
     EVP_PKEY * m_ptSslKey = NULL;
     char m_acLocalFingerprint[160];
     BIO_METHOD * m_ptDtlsBioFilterMethods = NULL;


};



#endif
