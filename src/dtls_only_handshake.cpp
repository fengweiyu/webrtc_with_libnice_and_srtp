/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       dtls_only_handshake.c
* Description           : 		
仅含有协商机制的dtls,收发函数由外部模块提供
参照创建cpp主干(bio依赖list,list属于c++)
openssl版本1.1.0

* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "dtls_only_handshake.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/asn1.h>


/* Duration for the self-generated certs: 1 year */
#define DTLS_AUTOCERT_DURATION	60*60*24*365
/* DTLS stuff */
#define DTLS_CIPHERS	"ALL:NULL:eNULL:aNULL"




/*****************************************************************************
-Fuction        : DtlsOnlyHandshake
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
DtlsOnlyHandshake::DtlsOnlyHandshake(T_DtlsOnlyHandshakeCb i_tDtlsOnlyHandshakeCb)
{
    m_ptSslCtx = NULL;
    m_ptSslCert = NULL;
    m_ptSslKey = NULL;
    m_acLocalFingerprint[160];
    m_ptDtlsBioFilterMethods = NULL;
    memset(&m_acLocalFingerprint,0,sizeof(m_acLocalFingerprint));

    m_ptSsl = NULL;
    m_ptReadBio = NULL;
    m_ptWriteBio = NULL;
    m_ptFilterBio = NULL;

    memset(&m_tDtlsOnlyHandshakeCb,0,sizeof(T_DtlsOnlyHandshakeCb));
    memcpy(&m_tDtlsOnlyHandshakeCb,&i_tDtlsOnlyHandshakeCb,sizeof(T_DtlsOnlyHandshakeCb));
    memset(&m_tOutPolicyInfo,0,sizeof(T_PolicyInfo));
    memset(&m_tInPolicyInfo,0,sizeof(T_PolicyInfo));
    m_iShakeEndFlag = 0;
    m_iShakeStartedFlag = 0;

    m_eDtlsRole = DTLS_ROLE_SERVER;//默认server对应offer端
}

/*****************************************************************************
-Fuction        : ~DtlsOnlyHandshake
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
DtlsOnlyHandshake::~DtlsOnlyHandshake()
{

    /* Destroy DTLS stack and free resources */
    if(m_ptSsl != NULL) 
    {
        SSL_free(m_ptSsl);
        m_ptSsl = NULL;
    }
    /* BIOs are destroyed by SSL_free */
    m_ptReadBio = NULL;
    m_ptWriteBio = NULL;
    m_ptFilterBio = NULL;
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
int DtlsOnlyHandshake::Init()
{
    m_ptSslCtx = SSL_CTX_new(DTLS_method());
    if(!m_ptSslCtx) 
    {
        printf("Ops, error creating DTLS context?\n");
        return -1;
    }
    SSL_CTX_set_verify(m_ptSslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, &DtlsOnlyHandshake::VerifyCallback);
    SSL_CTX_set_tlsext_use_srtp(m_ptSslCtx, "SRTP_AES128_CM_SHA1_80");	/* FIXME Should we support something else as well? */

    if (GenerateKeys(&m_ptSslCert, &m_ptSslKey) != 0) 
    {
        printf("Error generating DTLS key/certificate\n");
        return -2;
    }
    if(!SSL_CTX_use_certificate(m_ptSslCtx, m_ptSslCert)) 
    {
        printf("Certificate error (%s)\n", ERR_reason_error_string(ERR_get_error()));
        return -4;
    }
    if(!SSL_CTX_use_PrivateKey(m_ptSslCtx, m_ptSslKey)) 
    {
        printf("Certificate key error (%s)\n", ERR_reason_error_string(ERR_get_error()));
        return -5;
    }
    if(!SSL_CTX_check_private_key(m_ptSslCtx)) 
    {
        printf("Certificate check error (%s)\n", ERR_reason_error_string(ERR_get_error()));
        return -6;
    }
    SSL_CTX_set_read_ahead(m_ptSslCtx,1);
    
    unsigned int size;
    unsigned char fingerprint[EVP_MAX_MD_SIZE];
    if(X509_digest(m_ptSslCert, EVP_sha256(), (unsigned char *)fingerprint, &size) == 0) 
    {
        printf("Error converting X509 structure (%s)\n", ERR_reason_error_string(ERR_get_error()));
        return -7;
    }
    char *lfp = (char *)&m_acLocalFingerprint;
    unsigned int i = 0;
    for(i = 0; i < size; i++) 
    {
        snprintf(lfp, 4, "%.2X:", fingerprint[i]);
        lfp += 3;
    }
    *(lfp-1) = 0;

    printf("Fingerprint of our certificate: %s\n", m_acLocalFingerprint);
    SSL_CTX_set_cipher_list(m_ptSslCtx, DTLS_CIPHERS);
    if(BioFilterInit() < 0) 
    {
        printf("Error initializing BIO filter\n");
        return -8;
    }
	return 0;
}
/*****************************************************************************
-Fuction        : Create
-Description    : 创建ssl
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int DtlsOnlyHandshake::Create(E_DtlsRole i_eDtlsRole)
{
    int iRet = -1;
    
    m_ptSsl = SSL_new(m_ptSslCtx);
    if(!m_ptSsl) 
    {
        printf("Error creating DTLS session! (%s)\n", ERR_reason_error_string(ERR_get_error()));        
        return -1;
    }
    SSL_set_ex_data(m_ptSsl, 0, this);
    SSL_set_info_callback(m_ptSsl, Callback);

    m_ptReadBio = BIO_new(BIO_s_mem());
    if(!m_ptReadBio) 
    {
        printf(" Error creating read BIO! (%s)\n",  ERR_reason_error_string(ERR_get_error()));      
        return -2;
    }
    BIO_set_mem_eof_return(m_ptReadBio, -1);
	m_ptWriteBio = BIO_new(BIO_s_mem());
	if(!m_ptWriteBio) 
    {
		printf("Error creating write BIO! (%s)\n", ERR_reason_error_string(ERR_get_error()));		
        return -3;
	}
	BIO_set_mem_eof_return(m_ptWriteBio, -1);
	/* The write BIO needs our custom filter, or fragmentation won't work */
	m_ptFilterBio = BIO_new(m_ptDtlsBioFilterMethods);
	if(!m_ptFilterBio) 
    {
		printf("Error creating filter BIO! (%s)\n", ERR_reason_error_string(ERR_get_error()));		
		return -4;
	}
	/* Chain filter and write BIOs */
	BIO_push(m_ptFilterBio, m_ptWriteBio);
	/* Set the filter as the BIO to use for outgoing data */
	SSL_set_bio(m_ptSsl, m_ptReadBio, m_ptFilterBio);

	m_eDtlsRole = i_eDtlsRole;
	if(m_eDtlsRole == DTLS_ROLE_CLIENT) 
    {
		printf("Setting connect state (DTLS client)\n");
        SSL_set_connect_state(m_ptSsl);
	}
    else 
    {
		printf("Setting accept state (DTLS server)\n");
		SSL_set_accept_state(m_ptSsl);
	}
	
	/* https://code.google.com/p/chromium/issues/detail?id=406458
	 * Specify an ECDH group for ECDHE ciphers, otherwise they cannot be
	 * negotiated when acting as the server. Use NIST's P-256 which is
	 * commonly supported.
	 */
	EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if(ecdh == NULL) 
    {
		printf("Error creating ECDH group! (%s)\n", ERR_reason_error_string(ERR_get_error()));		
        return -5;
	}
	SSL_set_options(m_ptSsl, SSL_OP_SINGLE_ECDH_USE);
	SSL_set_tmp_ecdh(m_ptSsl, ecdh);
	EC_KEY_free(ecdh);

    m_iShakeEndFlag=0;
    iRet=0;
    return iRet;
}
/*****************************************************************************
-Fuction        : Handshake
-Description    : 创建ssl
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
void DtlsOnlyHandshake::Handshake() 
{
	if(m_ptSsl == NULL) 
	{
        return;
	}
	SSL_do_handshake(m_ptSsl);
	SendDataOut();
	m_iShakeStartedFlag = 1;
}

/*****************************************************************************
-Fuction        : Handshake
-Description    : 创建ssl
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
void DtlsOnlyHandshake::HandleRecvData(char *buf,int len)
{       
    printf("DtlsOnlyHandshake HandleRecvData:%d\n",len);
    if(!m_ptSsl || !m_ptReadBio) 
    {
        printf("No DtlsOnlyHandshake HandleRecvData\n");
        return;
    }
    if(0 ==m_iShakeStartedFlag) 
    {//防止ssl阻塞导致libnice线程阻塞从而导致不打洞
        printf("No m_iShakeStartedFlag\n");//上述应该是死锁导致已经解决
        return;//还没握手，就收到握手包，则丢弃
    }

    
    SendDataOut();
    int written = BIO_write(m_ptReadBio, buf, len);
    if(written != len) 
    {
        printf("Only written %d/%d of those bytes on the read BIO...\n", written, len);
    } 
    else 
    {
        printf("Written %d bytes on the read BIO...\n", written);
    }
    SendDataOut();
    /* Try to read data */
    char data[1500];	/* FIXME */
    memset(&data, 0, 1500);
    int read = SSL_read(m_ptSsl, &data, 1500);
    printf("... and read %d of them from SSL...\n", read);
    if(read < 0) 
    {
        unsigned long err = SSL_get_error(m_ptSsl, read);
        if(err == SSL_ERROR_SSL) 
        {
            /* Ops, something went wrong with the DTLS handshake */
            char error[200];
            ERR_error_string_n(ERR_get_error(), error, 200);
            printf("Handshake error: %s\n", error);
            return;
        }
    }
    SendDataOut();
    
    if(!SSL_is_init_finished(m_ptSsl)) 
    {
        /* Nothing else to do for now */
        printf("Initialization not finished yet...\n");
        return;
    }
    if(m_iShakeEndFlag) 
    {
        /* There's data to be read? */
        printf("Any data available?\n");

        if(read > 0) 
        {
            printf("Data available but Data Channels support disabled...\n");
        }
    }
    else
    {        
        /* Check the remote fingerprint */
        X509 *rcert = SSL_get_peer_certificate(m_ptSsl);
        if(!rcert) 
        {
            printf("No remote certificate?? (%s)\n", ERR_reason_error_string(ERR_get_error()));
        } 
        else 
        {
            unsigned int rsize;
            unsigned char rfingerprint[EVP_MAX_MD_SIZE];
            char remote_fingerprint[160];
            char *rfp = (char *)&remote_fingerprint;
            //if(stream->remote_hashing && !strcasecmp(stream->remote_hashing, "sha-1")) {
            if(0)
            {
                printf("Computing sha-1 fingerprint of remote certificate...\n");
                X509_digest(rcert, EVP_sha1(), (unsigned char *)rfingerprint, &rsize);
            }
            else 
            {
                printf("Computing sha-256 fingerprint of remote certificate...\n");
                X509_digest(rcert, EVP_sha256(), (unsigned char *)rfingerprint, &rsize);
            }
            X509_free(rcert);
            rcert = NULL;
            unsigned int i = 0;
            for(i = 0; i < rsize; i++) 
            {
                snprintf(rfp, 4, "%.2X:", rfingerprint[i]);
                rfp += 3;
            }
            *(rfp-1) = 0;

            printf("Remote fingerprint (%s) of the client is %s\n", "sha-256", remote_fingerprint);


            unsigned char material[DTLS_MASTER_LENGTH*2];
            unsigned char *local_key, *local_salt, *remote_key, *remote_salt;
            /* Export keying material for SRTP */
            if (!SSL_export_keying_material(m_ptSsl, material, DTLS_MASTER_LENGTH*2, "EXTRACTOR-dtls_srtp", 19, NULL, 0, 0)) 
            {
                /* Oops... */
                printf("Oops, couldn't extract SRTP keying material for (%s)\n",ERR_reason_error_string(ERR_get_error()));
                return;
            }
            /* Key derivation (http://tools.ietf.org/html/rfc5764#section-4.2) */
            /* Key derivation (http://tools.ietf.org/html/rfc5764#section-4.2) */
            if(m_eDtlsRole == DTLS_ROLE_CLIENT) 
            {
                local_key = material;
                remote_key = local_key + DTLS_MASTER_KEY_LENGTH;
                local_salt = remote_key + DTLS_MASTER_KEY_LENGTH;
                remote_salt = local_salt + DTLS_MASTER_SALT_LENGTH;
            }
            else 
            {
                remote_key = material;
                local_key = remote_key + DTLS_MASTER_KEY_LENGTH;
                remote_salt = local_key + DTLS_MASTER_KEY_LENGTH;
                local_salt = remote_salt + DTLS_MASTER_SALT_LENGTH;
            }

            memcpy(m_tInPolicyInfo.key, remote_key, DTLS_MASTER_KEY_LENGTH);
            memcpy(m_tInPolicyInfo.key + DTLS_MASTER_KEY_LENGTH, remote_salt, DTLS_MASTER_SALT_LENGTH);

            memcpy(m_tOutPolicyInfo.key, local_key, DTLS_MASTER_KEY_LENGTH);
            memcpy(m_tOutPolicyInfo.key + DTLS_MASTER_KEY_LENGTH, local_salt, DTLS_MASTER_SALT_LENGTH);
            
            m_iShakeEndFlag = 1;
        }
    }
}
/*****************************************************************************
-Fuction        : GetPolicyInfo
-Description    : 获取本地的创建srtp需要的信息，用于加密
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int DtlsOnlyHandshake::GetLocalPolicyInfo(T_PolicyInfo *i_ptPolicyInfo) 
{
    int iRet = -1;
	if(i_ptPolicyInfo == NULL) 
	{
	    printf("GetPolicyInfo null\r\n");
        return iRet;
	}
	if(0 == m_iShakeEndFlag) 
	{
	    printf("GetPolicyInfo err\r\n");
        return iRet;
	}
    memcpy(i_ptPolicyInfo,&m_tOutPolicyInfo,sizeof(T_PolicyInfo));
    iRet = 0;
    return iRet;
}

/*****************************************************************************
-Fuction        : IoBridge
-Description    : IO网桥,输出到回调中
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
void DtlsOnlyHandshake::SendDataOut() 
{
	if(!m_ptWriteBio || NULL == m_tDtlsOnlyHandshakeCb.SendDataOut) 
    {
		printf("No handle/agent/bio, no DTLS bridge...\n");
		return;
	}
	int pending = BIO_ctrl_pending(m_ptFilterBio);
	while(pending > 0) 
    {
		printf("Going to send DTLS data: %d bytes\n", pending);
		char* outgoing = new char[pending];
        int noutgoinglen = pending;
		int out = BIO_read(m_ptWriteBio, outgoing, noutgoinglen);
		printf("Read %d bytes from the write_BIO...\n", out);
		if(out > 1500) 
        {
			/* FIXME Just a warning for now, this will need to be solved with proper fragmentation */
			printf("The DTLS stack is trying to send a packet of %d bytes, this may be larger than the MTU and get dropped!\n", out);
		}
        int bytes = m_tDtlsOnlyHandshakeCb.SendDataOut(outgoing,out,m_tDtlsOnlyHandshakeCb.pObj);
		if(bytes < out) 
        {
			printf("Error sending DTLS message on c%d (%d)\n", out, bytes);
		} 
		else 
		{
			printf(">> >> ... and sent %d of those bytes on the socket\n", bytes);
		}
		/* Update stats (TODO Do the same for the last second window as well)
		 * FIXME: the Data stats includes the bytes used for the handshake */
		if(bytes > 0) 
        {
// 			component->out_stats.data_packets++;
// 			component->out_stats.data_bytes += bytes;
		}
		delete [] outgoing;
		/* Check if there's anything left to send (e.g., fragmented packets) */
		pending = BIO_ctrl_pending(m_ptFilterBio);
	}
}

/*****************************************************************************
-Fuction        : GetLocalFingerprint
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int DtlsOnlyHandshake::GetLocalFingerprint(char * i_strLocalFingerprint,int i_iFingerprintMaxLen)
{
	int iRet=-1;
    if (i_strLocalFingerprint == NULL || i_iFingerprintMaxLen <= 0) 
    {
		printf("GetLocalFingerprint NULL\r\n");
		return iRet;
    }
    snprintf(i_strLocalFingerprint,i_iFingerprintMaxLen,"%s",m_acLocalFingerprint);
    iRet = 0;
	return iRet;
}


int DtlsOnlyHandshake::VerifyCallback(int i_iPreverifyOk, X509_STORE_CTX *ctx) 
{
    /* We just use the verify_callback to request a certificate from the client */
    return 1;
}

int DtlsOnlyHandshake::GenerateKeys(X509 ** i_pptCertificate, EVP_PKEY ** i_pptPrivateKey) 
{
    static const int num_bits = 2048;
    BIGNUM* bne = NULL;
    RSA* rsa_key = NULL;
    X509_NAME* cert_name = NULL;

    printf("Generating DTLS key / cert\n");

    /* Create a big number object. */
    bne = BN_new();
    if (!bne) 
    {
        printf("BN_new() failed\n");
        goto error;
    }

    if (!BN_set_word(bne, RSA_F4)) {  /* RSA_F4 == 65537 */
        printf("BN_set_word() failed\n");
        goto error;
    }

    /* Generate a RSA key. */
    rsa_key = RSA_new();
    if (!rsa_key) {
        printf("RSA_new() failed\n");
        goto error;
    }

    /* This takes some time. */
    if (!RSA_generate_key_ex(rsa_key, num_bits, bne, NULL)) 
    {
        printf("RSA_generate_key_ex() failed\n");
        goto error;
    }

    /* Create a private key object (needed to hold the RSA key). */
    *i_pptPrivateKey = EVP_PKEY_new();
    if (!*i_pptPrivateKey) 
    {
        printf("EVP_PKEY_new() failed\n");
        goto error;
    }

    if (!EVP_PKEY_assign_RSA(*i_pptPrivateKey, rsa_key)) 
    {
        printf("EVP_PKEY_assign_RSA() failed\n");
        goto error;
    }
    /* The RSA key now belongs to the private key, so don't clean it up separately. */
    rsa_key = NULL;

    /* Create the X509 certificate. */
    *i_pptCertificate = X509_new();
    if (!*i_pptCertificate) 
    {
        printf("X509_new() failed\n");
        goto error;
    }

    /* Set version 3 (note that 0 means version 1). */
    X509_set_version(*i_pptCertificate, 2);

    /* Set serial number. */
    ASN1_INTEGER_set(X509_get_serialNumber(*i_pptCertificate), (long)GetRandom()/*g_random_int()*/);//升级openssl

    /* Set valid period. */
    X509_gmtime_adj(X509_get_notBefore(*i_pptCertificate), -1 * DTLS_AUTOCERT_DURATION);  /* -1 year */
    X509_gmtime_adj(X509_get_notAfter(*i_pptCertificate), DTLS_AUTOCERT_DURATION);  /* 1 year */

    /* Set the public key for the certificate using the key. */
    if (!X509_set_pubkey(*i_pptCertificate, *i_pptPrivateKey)) 
    {
        printf("X509_set_pubkey() failed\n");
        goto error;
    }

    /* Set certificate fields. */
    cert_name = X509_get_subject_name(*i_pptCertificate);
    if (!cert_name) 
    {
        printf("X509_get_subject_name() failed\n");
        goto error;
    }
    X509_NAME_add_entry_by_txt(cert_name, "O", MBSTRING_ASC, (const unsigned char*)"Janus", -1, -1, 0);
    X509_NAME_add_entry_by_txt(cert_name, "CN", MBSTRING_ASC, (const unsigned char*)"Janus", -1, -1, 0);

    /* It is self-signed so set the issuer name to be the same as the subject. */
    if (!X509_set_issuer_name(*i_pptCertificate, cert_name)) 
    {
        printf("X509_set_issuer_name() failed\n");
        goto error;
    }

    /* Sign the certificate with the private key. */
    if (!X509_sign(*i_pptCertificate, *i_pptPrivateKey, EVP_sha1())) 
    {
        printf("X509_sign() failed\n");
        goto error;
    }

    /* Free stuff and resurn. */
    BN_free(bne);
    return 0;

error:
    if (bne)
        BN_free(bne);
    if (rsa_key && !*i_pptPrivateKey)
        RSA_free(rsa_key);
    if (*i_pptPrivateKey)
        EVP_PKEY_free(*i_pptPrivateKey);  /* This also frees the RSA key. */
    if (*i_pptCertificate)
        X509_free(*i_pptCertificate);
    return -1;
}

int DtlsOnlyHandshake::BioFilterInit(void) 
{
    m_ptDtlsBioFilterMethods = BIO_meth_new(BIO_TYPE_FILTER | BIO_get_new_index(), "janus filter");
    if(!m_ptDtlsBioFilterMethods)
        return -1;
    BIO_meth_set_write(m_ptDtlsBioFilterMethods, BioFilterWrite);
    BIO_meth_set_ctrl(m_ptDtlsBioFilterMethods, BioFilterCrtl);
    BIO_meth_set_create(m_ptDtlsBioFilterMethods, BioFilterNew);
    BIO_meth_set_destroy(m_ptDtlsBioFilterMethods, BioFilterFree);

    return 0;
}

int DtlsOnlyHandshake::BioFilterNew(BIO *bio) 
{
    /* Create a filter state struct */
    BioFilter *filter = new BioFilter();//(dtls_bio_filter *)g_malloc0(sizeof(dtls_bio_filter));
    //filter->packets = NULL;
    //mutex_init(&filter->mutex);

    BIO_set_init(bio, 1);
    BIO_set_data(bio, filter);

    return 1;
}

int DtlsOnlyHandshake::BioFilterFree(BIO *bio) 
{
    if(bio == NULL)
        return 0;
    
    BioFilter *filter = (BioFilter *)BIO_get_data(bio);

    if(filter != NULL) 
    {
        delete filter;
    }
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    bio->ptr = NULL;
    bio->init = 0;
    bio->flags = 0;
#else
    BIO_set_data(bio, NULL);
    BIO_set_init(bio, 0);
#endif
    return 1;
}

int DtlsOnlyHandshake::BioFilterWrite(BIO *bio, const char *in, int inl) 
{
    printf("dtls_bio_filter_write: %p, %d\n", in, inl);

    long ret = BIO_write(BIO_next(bio), in, inl);

    printf("  -- %ld\n", ret);

    /* Keep track of the packet, as we'll advertize them one by one after a pending check */
    BioFilter *filter = (BioFilter *)BIO_get_data(bio);

    if(filter != NULL)
    {
        pthread_mutex_lock(&filter->mutex);
        //filter->packets = g_list_append(filter->packets, GINT_TO_POINTER(ret));
        filter->packets.push_back(ret);
        pthread_mutex_unlock(&filter->mutex);
        printf("New list length: %lu\n", filter->packets.size());
    }
    return ret;
}

long DtlsOnlyHandshake::BioFilterCrtl(BIO *bio, int cmd, long num, void *ptr) 
{
    switch(cmd) 
    {
    case BIO_CTRL_FLUSH:
        /* The OpenSSL library needs this */
        return 1;
    case BIO_CTRL_DGRAM_QUERY_MTU:
        /* Let's force the MTU that was configured */
        printf("Advertizing MTU: %d\n", 1472);
        return 1472;
    case BIO_CTRL_WPENDING:
        return 0L;
    case BIO_CTRL_PENDING: 
        {
            /* We only advertize one packet at a time, as they may be fragmented */
            BioFilter *filter = (BioFilter *)BIO_get_data(bio);

            if(filter == NULL) {return 0;}

            pthread_mutex_lock(&filter->mutex);

            //mutex_lock(&filter->mutex);
            if(0 == filter->packets.size()) 
            {
                pthread_mutex_unlock(&filter->mutex);
                return 0;
            }            
            
            /*GList *first = g_list_first(filter->packets);
            filter->packets = g_list_remove_link(filter->packets, first);
            int pending = GPOINTER_TO_INT(first->data);*/
            int pending = filter->packets.front();
            filter->packets.pop_front();
            /*g_list_free(first);
            mutex_unlock(&filter->mutex);*/
            
            pthread_mutex_unlock(&filter->mutex);
            /* We return its size so that only part of the buffer is read from the write BIO */
            return pending;
        }
    default:
        printf("dtls_bio_filter_ctrl: %d\n", cmd);
    }
    return 0;
}
void DtlsOnlyHandshake::Callback(const SSL *ssl, int where, int ret) 
{
    /* We only care about alerts */
    if (!(where & SSL_CB_ALERT)) 
    {
        return;
    }
    printf("......................................dtls error from callback.........................\r\n");
}
/*****************************************************************************
-Fuction		: GetRandom
-Description	: 
   Return a 32-bit random number.
   Because "our_random()" returns a 31-bit random number, we call it a second
   time, to generate the high bit.
   (Actually, to increase the likelhood of randomness, we take the middle 16 bits of two successive calls to "our_random()")

-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
unsigned int DtlsOnlyHandshake::GetRandom()
{
    long random_1 = random();
    unsigned int random16_1 = (unsigned int)(random_1&0x00FFFF00);
    
    long random_2 = random();
    unsigned int random16_2 = (unsigned int)(random_2&0x00FFFF00);
    
    return (random16_1<<8) | (random16_2>>8);
}
/*****************************************************************************
-Fuction        : BioFilter
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
BioFilter::BioFilter()
{
    pthread_mutex_init(&mutex,NULL);

}

/*****************************************************************************
-Fuction        : ~BioFilter
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
BioFilter::~BioFilter()
{
    pthread_mutex_destroy(&mutex);
    packets.clear();
}

