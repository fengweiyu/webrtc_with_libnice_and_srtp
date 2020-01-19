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
DtlsOnlyHandshake::DtlsOnlyHandshake()
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
    m_iShakeEndFlag = 0;

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
    SSL_CTX_set_verify(m_ptSslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, VerifyCallback);
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
int DtlsOnlyHandshake::Create()
{
    int iRet = -1;
    
    m_ptSsl = SSL_new(m_ptSslCtx);
    if(!m_ptSsl) 
    {
        printf("[%I64u]     Error creating DTLS session! (%s)\n", ERR_reason_error_string(ERR_get_error()));        
        return iRet;
    }
    SSL_set_ex_data(m_ptSsl, 0, this);
    SSL_set_info_callback(m_ptSsl, Callback);

    m_ptReadBio = BIO_new(BIO_s_mem());
    if(!m_ptReadBio) 
    {
        printf(" Error creating read BIO! (%s)\n",  ERR_reason_error_string(ERR_get_error()));      
        return iRet;
    }
    BIO_set_mem_eof_return(m_ptReadBio, -1);
	m_ptWriteBio = BIO_new(BIO_s_mem());
	if(!m_ptWriteBio) 
    {
		printf("Error creating write BIO! (%s)\n", ERR_reason_error_string(ERR_get_error()));		
        return iRet;
	}
	BIO_set_mem_eof_return(m_ptWriteBio, -1);
	/* The write BIO needs our custom filter, or fragmentation won't work */
	m_ptFilterBio = BIO_new(m_ptDtlsBioFilterMethods);
	if(!m_ptFilterBio) 
    {
		printf("Error creating filter BIO! (%s)\n", ERR_reason_error_string(ERR_get_error()));		
		return iRet;
	}
	/* Chain filter and write BIOs */
	BIO_push(m_ptFilterBio, m_ptWriteBio);
	/* Set the filter as the BIO to use for outgoing data */
	SSL_set_bio(m_ptSsl, m_ptReadBio, m_ptFilterBio);
	
    SSL_set_connect_state(m_ptSsl);
	/* https://code.google.com/p/chromium/issues/detail?id=406458
	 * Specify an ECDH group for ECDHE ciphers, otherwise they cannot be
	 * negotiated when acting as the server. Use NIST's P-256 which is
	 * commonly supported.
	 */
	EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if(ecdh == NULL) 
    {
		printf("Error creating ECDH group! (%s)\n", ERR_reason_error_string(ERR_get_error()));		
        return iRet;
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
	if(m_ptSsl == NULL) {return;}
    
    if(DTLS_STATE_CREATED == m_dtls_state)
    {
        m_dtls_state = DTLS_STATE_TRYING;
    }
	SSL_do_handshake(m_ptSsl);
	fd_bridge();
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
    ASN1_INTEGER_set(X509_get_serialNumber(*i_pptCertificate), (long)g_random_int());//升级openssl

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

int DtlsOnlyHandshake::BioFilterWrite(BIO *bio, const char *in, int inl) {
    printf("dtls_bio_filter_write: %p, %d\n", in, inl);

    long ret = BIO_write(BIO_next(bio), in, inl);

    printf("  -- %ld\n", ret);

    /* Keep track of the packet, as we'll advertize them one by one after a pending check */
    BioFilter *filter = (BioFilter *)BIO_get_data(bio);

    if(filter != NULL)
    {
        boost::mutex::scoped_lock lock(filter->mutex);
        //filter->packets = g_list_append(filter->packets, GINT_TO_POINTER(ret));
        filter->packets.push_back(ret);
        printf("New list length: %d\n", filter->packets.size());
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

            boost::mutex::scoped_lock lock(filter->mutex);

            //mutex_lock(&filter->mutex);
            if(0 == filter->packets.size()) {return 0;}            
            
            /*GList *first = g_list_first(filter->packets);
            filter->packets = g_list_remove_link(filter->packets, first);
            int pending = GPOINTER_TO_INT(first->data);*/
            int pending = filter->packets.front();
            filter->packets.pop_front();
            /*g_list_free(first);
            mutex_unlock(&filter->mutex);*/

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
    packets.clear();
}

