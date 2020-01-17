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
#include <openssl/bio.h>

/* Duration for the self-generated certs: 1 year */
#define DTLS_AUTOCERT_DURATION	60*60*24*365
/* DTLS stuff */
#define DTLS_CIPHERS	"ALL:NULL:eNULL:aNULL"

static int DtlsVerifyCallback(int i_iPreverifyOk, X509_STORE_CTX *ctx); 
static int DtlsGenerateKeys(X509 ** i_pptCertificate, EVP_PKEY ** i_pptPrivateKey);
static int DtlsBioFilterInit(void);

static SSL_CTX * g_ptSslCtx = NULL;
static X509 * g_ptSslCert = NULL;
static EVP_PKEY * g_ptSslKey = NULL;
static char g_acLocalFingerprint[160];
static BIO_METHOD * g_ptDtlsBioFilterMethods = NULL;


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
int DtlsInit()
{
    g_ptSslCtx = SSL_CTX_new(DTLS_method());
    if(!g_ptSslCtx) 
    {
        printf("Ops, error creating DTLS context?\n");
        return -1;
    }
    SSL_CTX_set_verify(g_ptSslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, DtlsVerifyCallback);
    SSL_CTX_set_tlsext_use_srtp(g_ptSslCtx, "SRTP_AES128_CM_SHA1_80");	/* FIXME Should we support something else as well? */

    if (DtlsGenerateKeys(&g_ptSslCert, &g_ptSslKey) != 0) 
    {
        printf("Error generating DTLS key/certificate\n");
        return -2;
    }
    if(!SSL_CTX_use_certificate(g_ptSslCtx, g_ptSslCert)) 
    {
        printf("Certificate error (%s)\n", ERR_reason_error_string(ERR_get_error()));
        return -4;
    }
    if(!SSL_CTX_use_PrivateKey(g_ptSslCtx, g_ptSslKey)) 
    {
        printf("Certificate key error (%s)\n", ERR_reason_error_string(ERR_get_error()));
        return -5;
    }
    if(!SSL_CTX_check_private_key(g_ptSslCtx)) 
    {
        printf("Certificate check error (%s)\n", ERR_reason_error_string(ERR_get_error()));
        return -6;
    }
    SSL_CTX_set_read_ahead(g_ptSslCtx,1);
    
    unsigned int size;
    unsigned char fingerprint[EVP_MAX_MD_SIZE];
    if(X509_digest(g_ptSslCert, EVP_sha256(), (unsigned char *)fingerprint, &size) == 0) 
    {
        printf("Error converting X509 structure (%s)\n", ERR_reason_error_string(ERR_get_error()));
        return -7;
    }
    char *lfp = (char *)&g_acLocalFingerprint;
    unsigned int i = 0;
    for(i = 0; i < size; i++) 
    {
        snprintf(lfp, 4, "%.2X:", fingerprint[i]);
        lfp += 3;
    }
    *(lfp-1) = 0;

    printf("Fingerprint of our certificate: %s\n", g_acLocalFingerprint);
    SSL_CTX_set_cipher_list(g_ptSslCtx, DTLS_CIPHERS);
    if(dtls_bio_filter_init() < 0) 
    {
        printf("Error initializing BIO filter\n");
        return -8;
    }
	return 0;
}

static int DtlsVerifyCallback(int i_iPreverifyOk, X509_STORE_CTX *ctx) 
{
    /* We just use the verify_callback to request a certificate from the client */
    return 1;
}

static int DtlsGenerateKeys(X509 ** i_pptCertificate, EVP_PKEY ** i_pptPrivateKey) 
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

static int DtlsBioFilterInit(void) 
{
    g_ptDtlsBioFilterMethods = BIO_meth_new(BIO_TYPE_FILTER | BIO_get_new_index(), "janus filter");
    if(!g_ptDtlsBioFilterMethods)
        return -1;
    BIO_meth_set_write(g_ptDtlsBioFilterMethods, dtls_bio_filter_write);
    BIO_meth_set_ctrl(g_ptDtlsBioFilterMethods, dtls_bio_filter_ctrl);
    BIO_meth_set_create(g_ptDtlsBioFilterMethods, dtls_bio_filter_new);
    BIO_meth_set_destroy(g_ptDtlsBioFilterMethods, dtls_bio_filter_free);

    return 0;
}

int dtls_bio_filter_new(BIO *bio) 
{
    /* Create a filter state struct */
    dtls_bio_filter *filter = new dtls_bio_filter();//(dtls_bio_filter *)g_malloc0(sizeof(dtls_bio_filter));
    //filter->packets = NULL;
    //mutex_init(&filter->mutex);

    BIO_set_init(bio, 1);
    BIO_set_data(bio, filter);

    return 1;
}

int dtls_bio_filter_free(BIO *bio) 
{
    if(bio == NULL)
        return 0;
    
    dtls_bio_filter *filter = (dtls_bio_filter *)BIO_get_data(bio);

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

int dtls_bio_filter_write(BIO *bio, const char *in, int inl) {
    printf("dtls_bio_filter_write: %p, %d\n", in, inl);

    long ret = BIO_write(BIO_next(bio), in, inl);

    printf("  -- %ld\n", ret);

    /* Keep track of the packet, as we'll advertize them one by one after a pending check */
    dtls_bio_filter *filter = (dtls_bio_filter *)BIO_get_data(bio);

    if(filter != NULL)
    {
        boost::mutex::scoped_lock lock(filter->mutex);
        //filter->packets = g_list_append(filter->packets, GINT_TO_POINTER(ret));
        filter->packets.push_back(ret);
        printf("New list length: %d\n", filter->packets.size());
    }
    return ret;
}

long dtls_bio_filter_ctrl(BIO *bio, int cmd, long num, void *ptr) {
    switch(cmd) 
    {
    case BIO_CTRL_FLUSH:
        /* The OpenSSL library needs this */
        return 1;
    case BIO_CTRL_DGRAM_QUERY_MTU:
        /* Let's force the MTU that was configured */
        printf("Advertizing MTU: %d\n", mtu);
        return mtu;
    case BIO_CTRL_WPENDING:
        return 0L;
    case BIO_CTRL_PENDING: 
        {
            /* We only advertize one packet at a time, as they may be fragmented */
            dtls_bio_filter *filter = (dtls_bio_filter *)BIO_get_data(bio);

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

