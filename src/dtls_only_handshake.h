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

#include "pthread.h"

#include <list>
#include <openssl/bio.h>





/* SRTP stuff (http://tools.ietf.org/html/rfc3711) */
#define DTLS_MASTER_KEY_LENGTH	16
#define DTLS_MASTER_SALT_LENGTH	14
#define DTLS_MASTER_LENGTH (DTLS_MASTER_KEY_LENGTH + DTLS_MASTER_SALT_LENGTH)

typedef enum DtlsRole 
{
    DTLS_ROLE_ACTPASS = -1,
    DTLS_ROLE_SERVER,
    DTLS_ROLE_CLIENT,
} E_DtlsRole;


typedef struct DtlsOnlyHandshakeCb
{
	int (* SendDataOut)(char * i_acData,int i_iLen,void *pArg);
	int (* RecvStopPacket)(void * pArg);//Alert
	void *pObj;

}T_DtlsOnlyHandshakeCb;
typedef struct PolicyInfo
{
	char key[DTLS_MASTER_LENGTH];

	
}T_PolicyInfo;

/*****************************************************************************
-Class          : BioFilter
-Description    : BioFilter
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class BioFilter
{
public:
	BioFilter();
	~BioFilter();
	std::list < int > packets;    
    pthread_mutex_t mutex;

private:

};


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
	DtlsOnlyHandshake(T_DtlsOnlyHandshakeCb i_tDtlsOnlyHandshakeCb);
	~DtlsOnlyHandshake();
	int Init();
    int Create(E_DtlsRole i_eDtlsRole);
    void Handshake();
    void HandleRecvData(char *buf,int len);
    int GetLocalPolicyInfo(T_PolicyInfo *o_ptPolicyInfo);
    int GetRemotePolicyInfo(T_PolicyInfo *o_ptPolicyInfo);
    int GetLocalFingerprint(char * i_strLocalFingerprint,int i_iFingerprintMaxLen);
	static int VerifyCallback(int i_iPreverifyOk, X509_STORE_CTX *ctx);//被其他c函数访问就不是只有本类访问了
	
private:
    int BioFilterInit(void);
    int GenerateKeys(X509 ** i_pptCertificate, EVP_PKEY ** i_pptPrivateKey);
    void SendDataOut();
    unsigned int GetRandom();
    
    static void Callback(const SSL *ssl, int where, int ret);
    static int BioFilterNew(BIO *bio);
    static int BioFilterFree(BIO *bio);
    static int BioFilterWrite(BIO *bio, const char *in, int inl);
    static long BioFilterCrtl(BIO *bio, int cmd, long num, void *ptr);

	SSL_CTX * m_ptSslCtx;
	X509 * m_ptSslCert;
	EVP_PKEY * m_ptSslKey;
	char m_acLocalFingerprint[160];
	BIO_METHOD * m_ptDtlsBioFilterMethods;

    SSL *m_ptSsl;
    BIO *m_ptReadBio;//ssl读取数据io,调用ssl_read前需往这个io写数据
    BIO *m_ptWriteBio;//ssl发送数据出口io,需实现将这个io的数据读取来然后通过其他接口发走
    BIO *m_ptFilterBio;//(fix MTU fragmentation on outgoing DTLS data, if required)

    T_DtlsOnlyHandshakeCb m_tDtlsOnlyHandshakeCb;
    
    /*! \brief libsrtp policy for incoming SRTP packets */
    T_PolicyInfo m_tInPolicyInfo;//用于接收数据，即解密数据
    /*! \brief libsrtp policy for outgoing SRTP packets */
    T_PolicyInfo m_tOutPolicyInfo;//用于发数据，即加密数据
    
	int m_iShakeEndFlag;//协商结束标记,0未结束,1结束
    int m_iShakeStartedFlag;


	E_DtlsRole m_eDtlsRole;
};



#endif
