/*****************************************************************************
* Copyright (C) 2023-2028 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       :   FMP4.h
* Description       :   FMP4 operation center
                        FMP4封装处理(.mov文件) demux muxer
                        SetParams中不应修改m_dwBoxSize，m_dwBoxSize放到ToBits中，
                        这样才能多次调用SetParams，后续优化
* Created           :   2023.11.21.
* Author            :   Yu Weifeng
* Function List     :   
* Last Modified     :   
* History           :   
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "FMP4.h"

#define Read24BE(ptr,val)     *val = ((unsigned char)ptr[0] << 16) | ((unsigned char)ptr[1] << 8) | (unsigned char)ptr[2]


#define Write24BE(p,val) \
do{ \
    p[0] = (unsigned char)((val >> 16) & 0xFF); \
    p[1] = (unsigned char)((val >> 8) & 0xFF); \
    p[2] = (unsigned char)((val) & 0xFF);  \
}while(0)



#define FMP4_TO_U32(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define FMP4_BRAND_ISOM    "isom"
#define FMP4_BRAND_AVC1    "avc1"
#define FMP4_BRAND_HVC1    "hvc1"
#define FMP4_BRAND_ISO2    "iso2"
#define FMP4_BRAND_MP71    "mp71"
#define FMP4_BRAND_ISO3    "iso3"
#define FMP4_BRAND_ISO4    "iso4"
#define FMP4_BRAND_ISO5    "iso5"
#define FMP4_BRAND_ISO6    "iso6"
#define FMP4_BRAND_MP41    "mp41" // ISO/IEC 14496-1:2001 MP4 File Format v1
#define FMP4_BRAND_MP42    "mp42"// ISO/IEC 14496-14:2003 MP4 File Format v2
#define FMP4_BRAND_MOV     "qt  "//('q', 't', ' ', ' '), // Apple Quick-Time File Format
#define FMP4_BRAND_DASH    "dash"// MPEG-DASH
#define FMP4_BRAND_MSDH    "msdh"// MPEG-DASH
#define FMP4_BRAND_MSIX    "msix"// MPEG-DASH

#define FMP4_VIDEO	FMP4_TO_U32('v', 'i', 'd', 'e') // ISO/IEC 14496-12:2015(E) 12.1 Video media (p169)
#define FMP4_AUDIO	FMP4_TO_U32('s', 'o', 'u', 'n') // ISO/IEC 14496-12:2015(E) 12.2 Audio media (p173)
#define FMP4_META	FMP4_TO_U32('m', 'e', 't', 'a') // ISO/IEC 14496-12:2015(E) 12.3 Metadata media (p181)
#define FMP4_HINT	FMP4_TO_U32('h', 'i', 'n', 't') // ISO/IEC 14496-12:2015(E) 12.4 Hint media (p183)
#define FMP4_TEXT	FMP4_TO_U32('t', 'e', 'x', 't') // ISO/IEC 14496-12:2015(E) 12.5 Text media (p184)
#define FMP4_SUBT	FMP4_TO_U32('s', 'u', 'b', 't') // ISO/IEC 14496-12:2015(E) 12.6 Subtitle media (p185)
#define FMP4_FONT	FMP4_TO_U32('f', 'd', 's', 'm') // ISO/IEC 14496-12:2015(E) 12.7 Font media (p186)

enum
{
	FMP4_TKHD_FLAG_TRACK_ENABLE = 0x01,
	FMP4_TKHD_FLAG_TRACK_IN_MOVIE = 0x02,
	FMP4_TKHD_FLAG_TRACK_IN_PREVIEW = 0x04,
};

#define FMP4_TREX_FLAG_SAMPLE_IS_NO_SYNC_SAMPLE			0x00010000
// 8.6.4 Independent and Disposable Samples Box (p55)
#define FMP4_TREX_FLAG_SAMPLE_DEPENDS_ON_I_PICTURE       0x02000000
#define FMP4_TREX_FLAG_SAMPLE_DEPENDS_ON_NOT_I_PICTURE   0x01000000

// ISO/IEC 14496-1:2010(E) 7.2.6.6 DecoderConfigDescriptor
// Table 6 - streamType Values (p51)
typedef enum
{
    FMP4_STREAM_ODS     = 0x01, /* ObjectDescriptorStream */
    FMP4_STREAM_CRS     = 0x02, /* ClockReferenceStream */
    FMP4_STREAM_SDS     = 0x03, /* SceneDescriptionStream */
    FMP4_STREAM_VISUAL  = 0x04, /* VisualStream *///video
    FMP4_STREAM_AUDIO   = 0x05, /* AudioStream */
    FMP4_STREAM_MP7     = 0x06, /* MPEG7Stream */
    FMP4_STREAM_IPMP    = 0x07, /* IPMPStream */
    FMP4_STREAM_OCIS    = 0x08, /* ObjectContentInfoStream */
    FMP4_STREAM_MPEGJ   = 0x09, /* MPEGJStream */
    FMP4_STREAM_IS      = 0x0A, /* Interaction Stream */
    FMP4_STREAM_IPMPTOOL = 0x0B, /* IPMPToolStream */
}E_FMP4_STREAM_TYPE;

// ISO/IEC 14496-1:2010(E) 7.2.6.6 DecoderConfigDescriptor (p48)
// MPEG-4 systems ObjectTypeIndication
// http://www.mp4ra.org/object.html
typedef enum
{
    FMP4_OBJECT_TYPE_NONE       = 0x00 ,// unknown object id
    FMP4_OBJECT_TYPE_TEXT       = 0x08 ,// Text Stream
    FMP4_OBJECT_TYPE_MP4V       = 0x20 ,// Visual ISO/IEC 14496-2 (c)
    FMP4_OBJECT_TYPE_H264       = 0x21 ,// Visual ITU-T Recommendation H.264 | ISO/IEC 14496-10
    FMP4_OBJECT_TYPE_H265       = 0x23 ,// Visual ISO/IEC 23008-2 | ITU-T Recommendation H.265
    FMP4_OBJECT_TYPE_AAC        = 0x40 ,// Audio ISO/IEC 14496-3
    FMP4_OBJECT_TYPE_MP2V       = 0x60 ,// Visual ISO/IEC 13818-2 Simple Profile
    FMP4_OBJECT_TYPE_AAC_MAIN   = 0x66 ,// MPEG-2 AAC Main
    FMP4_OBJECT_TYPE_AAC_LOW    = 0x67 ,// MPEG-2 AAC Low
    FMP4_OBJECT_TYPE_AAC_SSR    = 0x68 ,// MPEG-2 AAC SSR
    FMP4_OBJECT_TYPE_MP3        = 0x69 ,// Audio ISO/IEC 13818-3
    FMP4_OBJECT_TYPE_MP1V       = 0x6A ,// Visual ISO/IEC 11172-2
    FMP4_OBJECT_TYPE_MP1A       = 0x6B ,// Audio ISO/IEC 11172-3
    FMP4_OBJECT_TYPE_JPEG       = 0x6C ,// Visual ISO/IEC 10918-1 (JPEG)
    FMP4_OBJECT_TYPE_PNG        = 0x6D ,// Portable Network Graphics (f)
    FMP4_OBJECT_TYPE_JPEG2000   = 0x6E ,// Visual ISO/IEC 15444-1 (JPEG 2000)
    FMP4_OBJECT_TYPE_VC1        = 0xA3 ,// SMPTE VC-1 Video
    FMP4_OBJECT_TYPE_DIRAC      = 0xA4 ,// Dirac Video Coder
    FMP4_OBJECT_TYPE_AC3        = 0xA5 ,// AC-3
    FMP4_OBJECT_TYPE_EAC3       = 0xA6 ,// Enhanced AC-3
    FMP4_OBJECT_TYPE_G719       = 0xA8 ,// ITU G.719 Audio
    FMP4_OBJECT_TYPE_DTS        = 0xA9 ,// Core Substream
    FMP4_OBJECT_TYPE_OPUS       = 0xAD ,// Opus audio https:,//opus-codec.org/docs/opus_in_isobmff.html
    FMP4_OBJECT_TYPE_VP9        = 0xB1 ,// VP9 Video
    FMP4_OBJECT_TYPE_FLAC       = 0xC1 ,// nonstandard from FFMPEG
    FMP4_OBJECT_TYPE_VP8        = 0xC2 ,// nonstandard
    FMP4_OBJECT_TYPE_H266       = 0xFC ,// ITU-T Recommendation H.266
    FMP4_OBJECT_TYPE_G711A      = 0xFD ,// ITU G.711 alaw
    FMP4_OBJECT_TYPE_G711U      = 0xFE ,// ITU G.711 ulaw
    FMP4_OBJECT_TYPE_AV1        = 0xFF ,// AV1: https:,//aomediacodec.github.io/av1-isobmff

}E_FMP4_OBJECT_TYPE;
#define FMP4_OBJECT_TYPE_AVC        FMP4_OBJECT_TYPE_H264
#define FMP4_OBJECT_TYPE_HEVC       FMP4_OBJECT_TYPE_H265
#define FMP4_OBJECT_TYPE_VVC        FMP4_OBJECT_TYPE_H266
#define FMP4_OBJECT_TYPE_ALAW       FMP4_OBJECT_TYPE_G711A
#define FMP4_OBJECT_TYPE_ULAW       FMP4_OBJECT_TYPE_G711U


typedef struct VideoSampleEncParam
{
    unsigned int dwWidth;//
    unsigned int dwHeight;//
}T_VideoSampleEncParam;
typedef struct AudioSampleEncParam
{
    unsigned int dwChannels;
    unsigned int dwBitsPerSample;
}T_AudioSampleEncParam;


typedef struct FMP4SampleInfo
{
    E_FMP4_FRAME_TYPE eFrameType;

	uint64_t ddwPTS; // track mdhd timescale
	uint64_t ddwDTS;

	unsigned char * pbData;
	unsigned int dwDataSize;
	uint64_t ddwDataOffset; // is a 32 or 64 bit integer that gives the offset of the start of a chunk into its containing media file.
    unsigned int dwSampleDuration;//持续时间


    E_FMP4_OBJECT_TYPE eEncType;
    unsigned int dwSampleRate;//dwSamplesPerSecond
    T_VideoSampleEncParam tVideoEncParam;
    T_AudioSampleEncParam tAudioEncParam;
	unsigned char abEncExtraData[512];
	int iEncExtraDataLen;
}T_FMP4SampleInfo;//FMP4FrameInfo

typedef struct FMP4TrackInfo
{
	unsigned int dwHandlerType;
    unsigned int dwTrackId;
    
    uint64_t dwStartDts;
    unsigned int dwSampleCount;
    T_FMP4SampleInfo * aptSampleInfo;

}T_FMP4TrackInfo;

/*****************************************************************************
-Class          : FMP4
-Description    : Level Base 基础父类，非实际box
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4BaseBox 
{
public:
    FMP4BaseBox() 
    {
        memset(m_acBoxType,0,sizeof(m_acBoxType));
    };
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = m_dwBoxSize;
        int iLen = 0;
        unsigned char *pbBuf = NULL;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            return iLen;
        }
        Write32BE(o_pbBuf,m_dwBoxSize);
        iLen+=4;
        memcpy(o_pbBuf+iLen,m_acBoxType,sizeof(m_acBoxType));//Write32BE((o_pbBuf+iLen),m_dwBoxType);
        iLen+=sizeof(m_acBoxType);
        return iLen;
    };
public:
    unsigned int m_dwBoxSize= 8;//sizeof(m_dwBoxSize)+sizeof(m_acBoxType)//子类父类构造后就初始化了，值就不变了
    char m_acBoxType[4];// = {0,};
};

/*****************************************************************************
-Class          : FMP4
-Description    : Level Base 基础父类，非实际box ,
                  m_dwBoxSize   ,m_acBoxType外部赋值
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4FullBaseBox  : public FMP4BaseBox 
{
public:
    FMP4FullBaseBox() 
    {
        memset(m_abFlags,0,sizeof(m_abFlags));
        FMP4BaseBox::m_dwBoxSize += sizeof(m_bVersion)+sizeof(m_abFlags);
    };

    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            return iLen;
        }
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        o_pbBuf[iLen] = m_bVersion;
        iLen+=1;
        memcpy(o_pbBuf+iLen,m_abFlags,sizeof(m_abFlags));
        iLen+=sizeof(m_abFlags);

        return iLen;
    };
public:
    unsigned char m_bVersion = 0;//
    unsigned char m_abFlags[3];// = { 0, 0, 0 };
};
/**********************************Level 8************************************/

/*****************************************************************************
-Class          : FMP4AvcCBox
-Description    : Level 8,第八级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4AvcCBox : public FMP4BaseBox {
public:
	FMP4AvcCBox() 
	{
	    m_iExtraDataLen = 0;
        memcpy(FMP4BaseBox::m_acBoxType,"avcC",sizeof(FMP4BaseBox::m_acBoxType));
	};
    int SetParams(unsigned char *i_pbExtraData,int i_iExtraDataLen) 
    {
        if(NULL == i_pbExtraData || (int)sizeof(m_abExtraData) < i_iExtraDataLen)
        {
            FMP4_LOGE("FMP4AvcCBox SetParams err %d,%d\r\n",i_iExtraDataLen,sizeof(m_abExtraData));
            return -1;
        }
        FMP4_LOGE("FMP4AvcCBox i_iExtraDataLen,%d\r\n",i_iExtraDataLen);
        memcpy(m_abExtraData,i_pbExtraData,i_iExtraDataLen);
        m_iExtraDataLen = i_iExtraDataLen;
        FMP4BaseBox::m_dwBoxSize += m_iExtraDataLen;//设置参数后才知道长度
        return 0;
    };
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
		unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4AvcCBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        if(m_iExtraDataLen <= 0)
        {
            FMP4_LOGE("FMP4AvcCBox m_iExtraDataLen err %d\r\n",m_iExtraDataLen);
            return iLen;
        }

        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        memcpy(o_pbBuf+iLen,m_abExtraData,m_iExtraDataLen);
        iLen+=m_iExtraDataLen;

        return iLen;
    };
    
private:
    unsigned char m_abExtraData[256]; // H.264 sps/pps
    int m_iExtraDataLen;
};

/*****************************************************************************
-Class          : FMP4HvcCBox
-Description    : Level 8,第八级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4HvcCBox : public FMP4BaseBox 
{
public:
	FMP4HvcCBox() 
	{
	    m_iExtraDataLen = 0;
        FMP4BaseBox::m_dwBoxSize += m_iExtraDataLen;//设置参数后才知道长度
        memcpy(FMP4BaseBox::m_acBoxType,"hvcC",sizeof(FMP4BaseBox::m_acBoxType));
	};
    int SetParams(unsigned char *i_pbExtraData,int i_iExtraDataLen) 
    {
        if(NULL == i_pbExtraData || i_iExtraDataLen > sizeof(m_abExtraData))
        {
            FMP4_LOGE("FMP4HvcCBox SetParams err %d,%d\r\n",i_iExtraDataLen,sizeof(m_abExtraData));
            return -1;
        }
        memcpy(m_abExtraData,i_pbExtraData,i_iExtraDataLen);
        m_iExtraDataLen = i_iExtraDataLen;
        FMP4BaseBox::m_dwBoxSize += m_iExtraDataLen;//设置参数后才知道长度
        return 0;
    };
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
		unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4HvcCBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        if(m_iExtraDataLen <= 0)
        {
            FMP4_LOGE("FMP4HvcCBox m_iExtraDataLen err %d\r\n",m_iExtraDataLen);
            return iLen;
        }

        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        memcpy(o_pbBuf+iLen,m_abExtraData,m_iExtraDataLen);
        iLen+=m_iExtraDataLen;

        return iLen;
    };
    
private:
    unsigned char m_abExtraData[512]; // H.265 sps/pps
    int m_iExtraDataLen;
};

/*****************************************************************************
-Class          : FMP4EsdsBox
-Description    : Level 8,第八级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4EsdsBox : public FMP4FullBaseBox 
{
public:
	FMP4EsdsBox() 
	{
	    memset(m_abExtraData,0,sizeof(m_abExtraData));
	    memset(m_abBufferSizeDb,0,sizeof(m_abBufferSizeDb));
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_bEsDescrTag)+sizeof(m_dwEsDescrTagSize)+sizeof(m_wTrackID)+sizeof(m_bFlags)+
        sizeof(m_bDecoderConfigDescrTag)+sizeof(m_dwDecoderConfigDescrTagSize)+sizeof(m_bObjectTypeIndication)+sizeof(m_bStreamType)+
        sizeof(m_abBufferSizeDb)+sizeof(m_dwMaxBitRate)+sizeof(m_dwAvgBitRate)+sizeof(m_bSlConfigDescrTag)+sizeof(m_dwSlConfigDescrTagSize)+sizeof(m_bSlConfig);
        memcpy(FMP4FullBaseBox::m_acBoxType,"esds",sizeof(FMP4FullBaseBox::m_acBoxType));
	};
    int SetParams(unsigned short i_wTrackID,E_FMP4_OBJECT_TYPE i_eEncType,unsigned char *i_pbExtraData,int i_iExtraDataLen) 
    {
        FMP4_LOGD("FMP4EsdsBox SetParams %d %d,i_iExtraDataLen %d\r\n",i_wTrackID,i_eEncType,i_iExtraDataLen);
        if(NULL == i_pbExtraData || i_iExtraDataLen > sizeof(m_abExtraData))
        {
            FMP4_LOGE("FMP4EsdsBox SetParams err %d,%d\r\n",i_iExtraDataLen,sizeof(m_abExtraData));
            return -1;
        }
        m_bEsDescrTag = FMP4EsdsBox::ISO_ES_DESCR_TAG;
        m_dwEsDescrTagSize+= 5 + 13 + (i_iExtraDataLen > 0 ? i_iExtraDataLen + 5 : 0); // mp4_write_decoder_config_descriptor
        m_dwEsDescrTagSize += 5 + 1; // mp4_write_sl_config_descriptor
        m_wTrackID = i_wTrackID;
        
        m_bDecoderConfigDescrTag = FMP4EsdsBox::ISO_DECODER_CONFIG_DESCR_TAG;
        m_dwDecoderConfigDescrTagSize=13 + (i_iExtraDataLen > 0 ? i_iExtraDataLen + 5 : 0);
        m_bObjectTypeIndication=(unsigned char)i_eEncType;

        if(i_iExtraDataLen > 0)
        {
            m_bDecSpecificInfoTag = FMP4EsdsBox::ISO_DEC_SPEC_INFO_TAG;
            memcpy(m_abExtraData,i_pbExtraData,i_iExtraDataLen);
            m_dwExtraDataLen = (unsigned int)i_iExtraDataLen;
            FMP4FullBaseBox::m_dwBoxSize += sizeof(m_bDecSpecificInfoTag)+sizeof(m_dwExtraDataLen)+m_dwExtraDataLen;//设置参数后才知道长度
        }
        
        m_bSlConfigDescrTag = FMP4EsdsBox::ISO_SL_CONFIG_DESCR_TAG;
        return 0;
    };
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
		unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4EsdsBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }

        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        o_pbBuf[iLen]=m_bEsDescrTag;
        iLen++;
        iLen+=WriteTagLen(m_dwEsDescrTagSize,o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        Write16BE((o_pbBuf+iLen),m_wTrackID);
        iLen+=2;
        o_pbBuf[iLen]=m_bFlags;
        iLen++;

        o_pbBuf[iLen]=m_bDecoderConfigDescrTag;
        iLen++;
        iLen+=WriteTagLen(m_dwDecoderConfigDescrTagSize,o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        o_pbBuf[iLen]=m_bObjectTypeIndication;
        iLen++;
        o_pbBuf[iLen]=0x01/*reserved*/ | (m_bStreamType << 2);
        iLen++;
        memcpy(o_pbBuf+iLen,m_abBufferSizeDb,sizeof(m_abBufferSizeDb));
        iLen+=sizeof(m_abBufferSizeDb);
        Write32BE((o_pbBuf+iLen),m_dwMaxBitRate);
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwAvgBitRate);
        iLen+=4;
        
        if(m_dwExtraDataLen > 0)
        {
            o_pbBuf[iLen]=m_bDecSpecificInfoTag;
            iLen++;
            iLen+=WriteTagLen(m_dwExtraDataLen,o_pbBuf+iLen,i_dwMaxBufLen-iLen);
            memcpy(o_pbBuf+iLen,m_abExtraData,m_dwExtraDataLen);
            iLen+=m_dwExtraDataLen;
        }
        
        o_pbBuf[iLen]=m_bSlConfigDescrTag;
        iLen++;
        iLen+=WriteTagLen(m_dwSlConfigDescrTagSize,o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        o_pbBuf[iLen]=m_bSlConfig;
        iLen++;

        return iLen;
    };
    // ISO/IEC 14496-1:2010(E)
    // 7.2.2 Common data structures
    // Table-1 List of Class Tags for Descriptors (p31)
    static const unsigned int ISO_OBJECT_DESCR_TAG                  = 0x01;
    static const unsigned int ISO_INITAL_OBJECT_DESCR_TAG	        = 0x02;
    static const unsigned int ISO_ES_DESCR_TAG                      = 0x03;
    static const unsigned int ISO_DECODER_CONFIG_DESCR_TAG          = 0x04;
    static const unsigned int ISO_DEC_SPEC_INFO_TAG                 = 0x05;
    static const unsigned int ISO_SL_CONFIG_DESCR_TAG               = 0x06;
    static const unsigned int ISO_CONTENT_IDENT_DESCR_TAG           = 0x07;
    static const unsigned int ISO_SUPPL_CONTENT_IDENT_DESCR_TAG     = 0x08;
    static const unsigned int ISO_IPI_DESCR_POINTER_TAG             = 0x09;
    static const unsigned int ISO_IPMP_DESCR_POINTER_TAG            = 0x0A;
    static const unsigned int ISO_IPMP_DESCR_TAG                    = 0x0B;
    static const unsigned int ISO_QOS_DESCR_TAG                     = 0x0C;
    static const unsigned int ISO_REGISTRATION_DESCR_TAG            = 0x0D;
    static const unsigned int ISO_ES_ID_INC_TAG                     = 0x0E;
    static const unsigned int ISO_ES_ID_REF_TAG                     = 0x0F;
    static const unsigned int ISO_MP4_IOD_TAG                       = 0x10;
    static const unsigned int ISO_MP4_OD_TAG                        = 0x11;
    
private:
    int WriteTagLen(unsigned int i_dwLen,unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        int iLen = 0;
        o_pbBuf[iLen]=(unsigned char)(0x80 | (i_dwLen >> 21));
        iLen++;
        o_pbBuf[iLen]=(unsigned char)(0x80 | (i_dwLen >> 14));
        iLen++;
        o_pbBuf[iLen]=(unsigned char)(0x80 | (i_dwLen >> 7));
        iLen++;
        o_pbBuf[iLen]=(unsigned char)(0x7F & i_dwLen);
        iLen++;
        return iLen;
    }

    unsigned char m_bEsDescrTag=0x03;//ISO_ES_DESCR_TAG
    unsigned int m_dwEsDescrTagSize=3; // mp4_write_decoder_config_descriptor
    unsigned short m_wTrackID=1;
    unsigned char m_bFlags=0x00; // flags (= no flags)
    
    unsigned char m_bDecoderConfigDescrTag=0x04;//ISO_DECODER_CONFIG_DESCR_TAG
    unsigned int m_dwDecoderConfigDescrTagSize=0; // 
    unsigned char m_bObjectTypeIndication=0; // 
    unsigned char m_bStreamType=(unsigned char)FMP4_STREAM_AUDIO;
    unsigned char m_abBufferSizeDb[3];
    unsigned int m_dwMaxBitRate=88360;
    unsigned int m_dwAvgBitRate=88360;

    unsigned char m_bDecSpecificInfoTag=0x05;//ISO_DEC_SPEC_INFO_TAG
    unsigned int m_dwExtraDataLen=0;
    unsigned char m_abExtraData[512]; // H.265 sps/pps

    unsigned char m_bSlConfigDescrTag=0x06;//ISO_SL_CONFIG_DESCR_TAG
    unsigned int m_dwSlConfigDescrTagSize=1; // 
    unsigned char m_bSlConfig=0x02;
};

/**********************************Level 7************************************/

/*****************************************************************************
-Class          : FMP4UrlBox
-Description    : Level 7,第七级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4UrlBox : public FMP4FullBaseBox
{
public:
	FMP4UrlBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += 0;
        memcpy(FMP4FullBaseBox::m_acBoxType,"url ",sizeof(FMP4FullBaseBox::m_acBoxType));

		FMP4FullBaseBox::m_abFlags[2] = 1;
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4UrlBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        return iLen;
    };
	//location
};

/*****************************************************************************
-Class          : FMP4Avc1Box
-Description    : Level 7,第七级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4VideoBox : public FMP4BaseBox 
{
public:
	FMP4VideoBox() 
	{
	    m_eEncType = FMP4_OBJECT_TYPE_NONE;
        FMP4BaseBox::m_dwBoxSize += sizeof(m_abR0)+ sizeof(m_wDataReferenceIndex)+ sizeof(m_wPreDefined0)+ sizeof(m_wR1)+ sizeof(m_dwPreDefined1)+sizeof(m_dwPreDefined2)+
        sizeof(m_dwPreDefined3)+sizeof(m_wWidth)+sizeof(m_wHeight)+sizeof(m_dwHorizResolution)+sizeof(m_dwVerResolution)+sizeof(m_dwR2)+sizeof(m_wFramesCount)+
        sizeof(m_abCompressrName)+sizeof(m_wBitDepth)+sizeof(m_wPreDefined4);
        memcpy(FMP4BaseBox::m_acBoxType,"avc1",sizeof(FMP4BaseBox::m_acBoxType));//"h264"
	};

    int SetParams(E_FMP4_OBJECT_TYPE i_eEncType,unsigned short i_wWidth,unsigned short i_wHeight) 
    {
        int iRet = 0;
        //FMP4_LOGW("FMP4VideoBox%d  SetParams %d   %d %d\r\n",FMP4BaseBox::m_dwBoxSize,i_eEncType,i_wWidth,i_wHeight);
        m_eEncType = i_eEncType;
        switch(i_eEncType)
        {
            case FMP4_OBJECT_TYPE_H264:
            {
                FMP4BaseBox::m_dwBoxSize += m_AvcCBox.m_dwBoxSize;
                memcpy(FMP4BaseBox::m_acBoxType,"avc1",sizeof(FMP4BaseBox::m_acBoxType));// AVCSampleEntry  (ISO/IEC 14496-15:2010)
                break;
            }
            case FMP4_OBJECT_TYPE_H265:
            {
                FMP4BaseBox::m_dwBoxSize += m_HvcCBox.m_dwBoxSize;
                memcpy(FMP4BaseBox::m_acBoxType,"hvc1",sizeof(FMP4BaseBox::m_acBoxType));//"h265"// HEVCSampleEntry (ISO/IEC 14496-15:2013)
                break;
            }
#if 0            
            case FMP4_ENC_H266:
            {
                memcpy(FMP4FullBaseBox::m_acBoxType,"vvc1",sizeof(FMP4FullBaseBox::m_acBoxType));//"h266"// VVCSampleEntry (ISO/IEC 14496-15:2021)
                break;
            }
            case FMP4_ENC_VP8:
            {
                memcpy(FMP4FullBaseBox::m_acBoxType,"vp08",sizeof(FMP4FullBaseBox::m_acBoxType));
                break;
            }
            case FMP4_ENC_VP9:
            {
                memcpy(FMP4FullBaseBox::m_acBoxType,"vp09",sizeof(FMP4FullBaseBox::m_acBoxType));
                break;
            }
            case FMP4_ENC_VP10:
            {
                memcpy(FMP4FullBaseBox::m_acBoxType,"vp10",sizeof(FMP4FullBaseBox::m_acBoxType));
                break;
            }
            case FMP4_ENC_AV1:
            {
                memcpy(FMP4FullBaseBox::m_acBoxType,"av01",sizeof(FMP4FullBaseBox::m_acBoxType));
                break;
            }
            case FMP4_ENC_VC1:
            {
                memcpy(FMP4FullBaseBox::m_acBoxType,"vc-1",sizeof(FMP4FullBaseBox::m_acBoxType));
                break;
            }
            case FMP4_ENC_JPEG:
            {
                memcpy(FMP4FullBaseBox::m_acBoxType,"mp4v",sizeof(FMP4FullBaseBox::m_acBoxType));// MPEG-4 Video
                break;
            }
            case FMP4_ENC_PNG:
            {
                memcpy(FMP4FullBaseBox::m_acBoxType,"mp4v",sizeof(FMP4FullBaseBox::m_acBoxType));// MPEG-4 Video
                break;
            }
#endif            
            default:
            {
                FMP4_LOGE("FMP4VideoBox SetParams err i_eEncType %d",i_eEncType);
                iRet = -1;
                break;
            }
        }
        m_wWidth = i_wWidth;
        m_wHeight = i_wHeight;
        FMP4_LOGW("FMP4VideoBox m_dwBoxSize%d    SetParams %d %d %d\r\n",FMP4BaseBox::m_dwBoxSize,i_eEncType,i_wWidth,i_wHeight);
        return iRet;
    };
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        unsigned char *pbBuf=NULL;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MdhdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);

        memcpy(o_pbBuf+iLen,m_abR0,sizeof(m_abR0));
        iLen+=sizeof(m_abR0);

        pbBuf = o_pbBuf+iLen;//防止编译报错
        Write16BE(pbBuf,m_wDataReferenceIndex);
        iLen+=2;
        pbBuf = o_pbBuf+iLen;
        Write16BE(pbBuf,m_wPreDefined0);
        iLen+=2;
        pbBuf = o_pbBuf+iLen;
        Write16BE(pbBuf,m_wR1);
        iLen+=2;
        pbBuf = o_pbBuf+iLen;
        Write32BE(pbBuf,m_dwPreDefined1);
        iLen+=4;
        pbBuf = o_pbBuf+iLen;
        Write32BE(pbBuf,m_dwPreDefined2);
        iLen+=4;
        pbBuf = o_pbBuf+iLen;
        Write32BE(pbBuf,m_dwPreDefined3);
        iLen+=4;
        
        pbBuf = o_pbBuf+iLen;
        Write16BE(pbBuf,m_wWidth);
        iLen+=2;
        pbBuf = o_pbBuf+iLen;
        Write16BE(pbBuf,m_wHeight);
        iLen+=2;
        pbBuf = o_pbBuf+iLen;
        Write32BE(pbBuf,m_dwHorizResolution);
        iLen+=4;
        pbBuf = o_pbBuf+iLen;
        Write32BE(pbBuf,m_dwVerResolution);
        iLen+=4;
        
        pbBuf = o_pbBuf+iLen;
        Write32BE(pbBuf,m_dwR2);
        iLen+=4;
        pbBuf = o_pbBuf+iLen;
        Write16BE(pbBuf,m_wFramesCount);
        iLen+=2;
     
        memcpy(o_pbBuf+iLen,m_abCompressrName,sizeof(m_abCompressrName));
        iLen+=sizeof(m_abCompressrName);
        // ISO/IEC 14496-15:2017 4.5 Template field used (19)
        // 0x18 - the video sequence is in color with no alpha
        // 0x28 - the video sequence is in grayscale with no alpha
        // 0x20 - the video sequence has alpha (gray or color)
        pbBuf = o_pbBuf+iLen;
        Write16BE(pbBuf,m_wBitDepth);
        iLen+=2;
        pbBuf = o_pbBuf+iLen;
        Write16BE(pbBuf,m_wPreDefined4);
        iLen+=2;
        
        switch(m_eEncType)
        {
            case FMP4_OBJECT_TYPE_H264:
            {
                iLen += m_AvcCBox.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
                break;
            }
            case FMP4_OBJECT_TYPE_H265:
            {
                iLen += m_HvcCBox.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
                break;
            }
            default:
            {
                FMP4_LOGE("FMP4VideoBox ToBits err i_eEncType %d",m_eEncType);
                break;
            }
        }
        return iLen;
    };
public:
	//根据m_eEncType ，下列选一个
	FMP4AvcCBox m_AvcCBox;
	FMP4HvcCBox m_HvcCBox;
private:
	unsigned char m_abR0[6] = { 0 };
	unsigned short m_wDataReferenceIndex = 1;// ref [dref] Data Reference Boxes
	unsigned short m_wPreDefined0 = 0;/* Reserved / Codec stream version */
	unsigned short m_wR1 = 0;/* Reserved / Codec stream revision (=0) */
	unsigned int m_dwPreDefined1 = 0;/* Reserved */
	unsigned int m_dwPreDefined2 = 0;/* Reserved */
	unsigned int m_dwPreDefined3 = 0;/* Reserved */

	unsigned short m_wWidth = 0;//set
	unsigned short m_wHeight = 0;//set
	unsigned int m_dwHorizResolution = 0X00480000;/* Horizontal resolution 72dpi */
	unsigned int m_dwVerResolution = 0X00480000;/* Vertical resolution 72dpi */
	unsigned int m_dwR2 = 0;/* reserved / Data size (= 0) */
	unsigned short m_wFramesCount = 1;/* Frame count (= 1) */
	unsigned char m_abCompressrName[32] = { 0, };// ISO 14496-15:2017 AVCC \012AVC Coding // ISO 14496-15:2017 HVCC \013HEVC Coding
	unsigned short m_wBitDepth = 0x18;
	unsigned short m_wPreDefined4 = 0xFFFF;

    E_FMP4_OBJECT_TYPE m_eEncType;
};

/*****************************************************************************
-Class          : FMP4Mp4aBox
-Description    : Level 7,第七级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4AudioBox : public FMP4BaseBox 
{
public:
	FMP4AudioBox() 
	{
	    m_eEncType = FMP4_OBJECT_TYPE_NONE;
        FMP4BaseBox::m_dwBoxSize += sizeof(m_abR0)+ sizeof(m_wDataReferenceIndex)+ sizeof(m_wPreDefined0)+ sizeof(m_wR1)+ sizeof(m_dwPreDefined1)+sizeof(m_wChannelCount)+
        sizeof(m_wSampleSize)+sizeof(m_wPreDefined2)+sizeof(m_wR2)+sizeof(m_dwSampleRate);
        memcpy(FMP4BaseBox::m_acBoxType,"mp4a",sizeof(FMP4BaseBox::m_acBoxType));//"aac"
	};

    int SetParams(E_FMP4_OBJECT_TYPE i_eEncType,unsigned short i_wChannel,unsigned short i_wSampleSize,unsigned int i_dwSampleRate) 
    {
        int iRet = 0;
        FMP4_LOGW("FMP4AudioBox SetParams %d,   %d ,%d, %d\r\n",i_eEncType,i_wChannel,i_wSampleSize,i_dwSampleRate);
        m_eEncType = i_eEncType;
        m_wChannelCount = i_wChannel;
        m_wSampleSize = i_wSampleSize;
        m_dwSampleRate = ((unsigned int)(i_dwSampleRate > 56635 ? 0 : i_dwSampleRate)) << 16;

        switch(m_eEncType)
        {
            case FMP4_OBJECT_TYPE_G711A:
            {
                memcpy(FMP4BaseBox::m_acBoxType,"alaw",sizeof(FMP4BaseBox::m_acBoxType));
                break;
            }
            case FMP4_OBJECT_TYPE_G711U:
            {
                memcpy(FMP4BaseBox::m_acBoxType,"ulaw",sizeof(FMP4BaseBox::m_acBoxType));
                break;
            }
            case FMP4_OBJECT_TYPE_AAC:
            case FMP4_OBJECT_TYPE_MP3:
            {
                FMP4BaseBox::m_dwBoxSize += m_EsdsBox.m_dwBoxSize;
                memcpy(FMP4BaseBox::m_acBoxType,"mp4a",sizeof(FMP4BaseBox::m_acBoxType));
                break;
            }
            default:
            {
                FMP4_LOGE("FMP4AudioBox SetParams err i_eEncType %d",m_eEncType);
                iRet = -1;
                break;
            }
        }
        return iRet;
    };
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        unsigned char *pbBuf=NULL;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4AudioBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        pbBuf=o_pbBuf+iLen;
        memcpy(pbBuf,m_abR0,sizeof(m_abR0));
        pbBuf+=sizeof(m_abR0);
        
        Write16BE(pbBuf,m_wDataReferenceIndex);
        pbBuf+=2;
        Write16BE(pbBuf,m_wPreDefined0);
        pbBuf+=2;
        Write16BE(pbBuf,m_wR1);
        pbBuf+=2;
        Write32BE(pbBuf,m_dwPreDefined1);
        pbBuf+=4;
        
        Write16BE(pbBuf,m_wChannelCount);
        pbBuf+=2;
        Write16BE(pbBuf,m_wSampleSize);
        pbBuf+=2;
        Write16BE(pbBuf,m_wPreDefined2);
        pbBuf+=2;
        Write16BE(pbBuf,m_wR2);
        pbBuf+=2;
        // https://www.opus-codec.org/docs/opus_in_isobmff.html
        // 4.3 Definitions of Opus sample
        // OpusSampleEntry: 
        // 1. The samplesize field shall be set to 16.
        // 2. The samplerate field shall be set to 48000<<16.
        Write32BE(pbBuf,m_dwSampleRate);
        pbBuf+=4;
        
        iLen=pbBuf-o_pbBuf;
        switch(m_eEncType)
        {
            case FMP4_OBJECT_TYPE_AAC:
            case FMP4_OBJECT_TYPE_MP3:
            {
                iLen += m_EsdsBox.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
                break;
            }
            case FMP4_ENC_OPUS:
            {
                //iLen += m_DopsBox.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
                //break;
            }
            default:
            {
                FMP4_LOGE("FMP4AudioBox ToBits err i_eEncType %d",m_eEncType);
                break;
            }
        }
        return iLen;
    };
    
public:
	//根据m_eEncType ，下列选一个
	FMP4EsdsBox m_EsdsBox;
	//FMP4DopsBox m_DopsBox;
private:
	unsigned char m_abR0[6] = { 0 };
	unsigned short m_wDataReferenceIndex = 1;// ref [dref] Data Reference Boxes
	/* SoundDescription */
	unsigned short m_wPreDefined0 = 0;/* Reserved / Codec stream version */
	unsigned short m_wR1 = 0;/* Reserved /  Revision level */
	unsigned int m_dwPreDefined1 = 0;/* Reserved */
	
	unsigned short m_wChannelCount = 2;/* default 2 */
	unsigned short m_wSampleSize = 16;/* default 16 */

	unsigned short m_wPreDefined2 = 0;/* pre_defined */
	unsigned short m_wR2 = 0;/* reserved /  packet size (= 0) */
	
	unsigned int m_dwSampleRate = 0;/* { default samplerate of media } << 16  48000<<16*/


    E_FMP4_OBJECT_TYPE m_eEncType;
};

/**********************************Level 6************************************/

/*****************************************************************************
-Class          : FMP4DrefBox(data reference box)
-Description    : Level 6,第六级，
“dref”下会包含若干个“url”或“urn”，这些box组成一个表，
用来定位track数据。简单的说，track可以被分成若干段，
每一段都可以根据“url”或“urn”指向的地址来获取数据，
sample描述中会用这些片段的序号将这些片段组成一个完整的track。
一般情况下，当数据被完全包含在文件中时，
“url”或“urn”中的定位字符串是空的。
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4DrefBox : public FMP4FullBaseBox
{
public:
	FMP4DrefBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwEntryCount)+m_URL.m_dwBoxSize;
        memcpy(FMP4FullBaseBox::m_acBoxType,"dref",sizeof(FMP4FullBaseBox::m_acBoxType));
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        unsigned char *pbBuf=NULL;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MdhdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        pbBuf=o_pbBuf+iLen;
        Write32BE(pbBuf,m_dwEntryCount); 
        iLen+=4;

        iLen += m_URL.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        
        return iLen;
    };
private:
	unsigned int m_dwEntryCount = 1;
	FMP4UrlBox m_URL;
};

/*****************************************************************************
-Class          : FMP4StsdBox(Sample Description Box)
-Description    : Level 6,第六级，
视频的编码类型、宽高、长度，音频的声道、采样等信息

* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4StsdBox : public FMP4FullBaseBox 
{
public:
	FMP4StsdBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwEntryCount);
        memcpy(FMP4FullBaseBox::m_acBoxType,"stsd",sizeof(FMP4FullBaseBox::m_acBoxType));//""

	}
    int SetParams(unsigned int i_dwTrakHandlerType) 
	{
        m_dwTrakHandlerType=i_dwTrakHandlerType;
        if(FMP4_VIDEO == m_dwTrakHandlerType)
        {
            FMP4FullBaseBox::m_dwBoxSize += m_VideoBox.m_dwBoxSize;
        }
        else if(FMP4_AUDIO == m_dwTrakHandlerType)
        {
            FMP4FullBaseBox::m_dwBoxSize += m_AudioBox.m_dwBoxSize;
        }
        else
        {
            FMP4_LOGE("FMP4StsdBox ToBits i_dwTrakHandlerType err %d,%d",m_dwTrakHandlerType,i_dwTrakHandlerType);
            return -1;
        }
        return 0;
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4StsdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen), m_dwEntryCount); /*  */
        iLen+=4;
        
        if(FMP4_VIDEO == m_dwTrakHandlerType)
        {
            iLen += m_VideoBox.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        }
        else if(FMP4_AUDIO == m_dwTrakHandlerType)
        {
            iLen += m_AudioBox.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        }
        
        return iLen;
    };
public:
	//根据track type ，下列选
	FMP4VideoBox m_VideoBox;
	FMP4AudioBox m_AudioBox;
private:
	//目前1个轨道(音频流轨道或者视频流轨道) 只需一个video box或audio boxs
	unsigned int m_dwEntryCount = 1;//后续可考虑多box情况

    unsigned int m_dwTrakHandlerType;
};

/*****************************************************************************
-Class          : FMP4SttsBox(Time To Sample Box)
-Description    : Level 6,第六级，
存储了sample的duration，描述了sample时序的映射方法，

* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4SttsBox : public FMP4FullBaseBox 
{
public:
	FMP4SttsBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwEntryCount);
        memcpy(FMP4FullBaseBox::m_acBoxType,"stts",sizeof(FMP4FullBaseBox::m_acBoxType));//""
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4SttsBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen), m_dwEntryCount); /* entry count */
        iLen+=4;
        
        return iLen;
    };
    
private:
	unsigned int m_dwEntryCount = 0;//sample_count  0 (sample类似帧的概念，一般只放一帧即sample_count默认0)

};

/*****************************************************************************
-Class          : FMP4StssBox(Sync Sample Box)
-Description    : Level 6,第六级，
确定media中的关键帧

* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4StssBox : public FMP4FullBaseBox 
{
public:
	FMP4StssBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwEntryCount);
        memcpy(FMP4FullBaseBox::m_acBoxType,"stss",sizeof(FMP4FullBaseBox::m_acBoxType));//""
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4StssBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen), m_dwEntryCount); /* entry count */
        iLen+=4;
        
        return iLen;
    };
    
private:
	unsigned int m_dwEntryCount = 0;//sample_count  0//sample_count  0 (sample类似帧的概念，一般只放一帧即sample_count默认0)

};

/*****************************************************************************
-Class          : FMP4CttsBox()
-Description    : Level 6,第六级，

* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4CttsBox : public FMP4FullBaseBox 
{
public:
	FMP4CttsBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwEntryCount);
        memcpy(FMP4FullBaseBox::m_acBoxType,"ctts",sizeof(FMP4FullBaseBox::m_acBoxType));//""
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4CttsBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen), m_dwEntryCount); /* entry count */
        iLen+=4;
        
        return iLen;
    };
    
private:
	unsigned int m_dwEntryCount = 0;//sample_count  0

};

/*****************************************************************************
-Class          : FMP4StscBox(Sample To Chunk Box)
-Description    : Level 6,第六级，

* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4StscBox : public FMP4FullBaseBox 
{
public:
	FMP4StscBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwEntryCount);
        memcpy(FMP4FullBaseBox::m_acBoxType,"stsc",sizeof(FMP4FullBaseBox::m_acBoxType));//""
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4StscBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen), m_dwEntryCount); /* entry count */
        iLen+=4;
        
        return iLen;
    };
    
private:
	unsigned int m_dwEntryCount = 0;//sample_count  0

};

/*****************************************************************************
-Class          : FMP4StszBox(Sample Size Box)
-Description    : Level 6,第六级，

* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4StszBox : public FMP4FullBaseBox 
{
public:
	FMP4StszBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwSampleSize)+ sizeof(m_dwSampleCount);
        memcpy(FMP4FullBaseBox::m_acBoxType,"stsz",sizeof(FMP4FullBaseBox::m_acBoxType));//""
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4StszBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen), m_dwSampleSize); /*  */
        iLen+=4;
        Write32BE((o_pbBuf+iLen), m_dwSampleCount); /*  */
        iLen+=4;
        
        return iLen;
    };
    
private:
	unsigned int m_dwSampleSize = 0;
	unsigned int m_dwSampleCount = 0;//(sample类似帧的概念，一般只放一帧即sample_count默认0)
};

/*****************************************************************************
-Class          : FMP4StcoBox(Chunk Offset Box)
-Description    : Level 6,第六级，

* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4StcoBox : public FMP4FullBaseBox 
{
public:
	FMP4StcoBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwEntryCount);
        memcpy(FMP4FullBaseBox::m_acBoxType,"stco",sizeof(FMP4FullBaseBox::m_acBoxType));//""
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4StcoBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen), m_dwEntryCount); /* entry count */
        iLen+=4;
        
        return iLen;
    };
    
private:
	unsigned int m_dwEntryCount = 0;//sample_count  0

};

/**********************************Level 5************************************/

/*****************************************************************************
-Class          : FMP4VmhdBox(Media Information Header Box)
-Description    : Level 5,第五级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4VmhdBox : public FMP4FullBaseBox
{
public:
	FMP4VmhdBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwGraphicsMode)+sizeof(m_dwR0);
        memcpy(FMP4FullBaseBox::m_acBoxType,"vmhd",sizeof(FMP4FullBaseBox::m_acBoxType));

		FMP4FullBaseBox::m_abFlags[2] = 1;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MdhdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen),m_dwGraphicsMode); /* reserved (graphics mode = copy) */
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwR0);
        iLen+=4;
        
        return iLen;
    };
private:
	unsigned int m_dwGraphicsMode = 0;
	unsigned int m_dwR0 = 0;
	//uint16_t opcolor[3] = { 0, 0, 0 };
};

/*****************************************************************************
-Class          : FMP4SmhdBox(SOUND Media Header Box)
-Description    : Level 5,第五级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4SmhdBox : public FMP4FullBaseBox
{
public:
	FMP4SmhdBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_wR0)+sizeof(m_wR1);
        memcpy(FMP4FullBaseBox::m_acBoxType,"smhd",sizeof(FMP4FullBaseBox::m_acBoxType));
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4SmhdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write16BE((o_pbBuf+iLen),m_wR0);
        iLen+=2;
        Write16BE((o_pbBuf+iLen),m_wR1);
        iLen+=2;
        
        return iLen;
    };
private:
	unsigned short m_wR0 = 0;/* reserved (balance, normally = 0) */
	unsigned short m_wR1 = 0;
};

/*****************************************************************************
-Class          : FMP4DinfBox(Data Information Box)
解释如何定位媒体信息，是一个container box。“dinf”一般包含一个“dref”
-Description    : Level 5,第五级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4DinfBox :public FMP4BaseBox
{
public:
	FMP4DinfBox() 
	{
        FMP4BaseBox::m_dwBoxSize += m_DrefBox.m_dwBoxSize;
        memcpy(FMP4BaseBox::m_acBoxType,"dinf",sizeof(FMP4BaseBox::m_acBoxType));
	};

    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MdhdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        iLen += m_DrefBox.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        
        return iLen;
    };
private:
	FMP4DrefBox m_DrefBox;
};
/*****************************************************************************
-Class          : FMP4StblBox(Sample Table Box)
-Description    : Level 5,第五级，
sample是媒体数据存储的单位，存储在media的chunk中，chunk和sample的长度均可互不相同
“stbl”包含了关于track中sample所有时间和位置的信息，以及sample的编解码等信息。
利用这个表，可以解释sample的时序、类型、大小以及在各自存储容器中的位置
“stsd”必不可少，且至少包含一个条目

* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4StblBox : public FMP4BaseBox 
{
public:
	FMP4StblBox()
	{
        memcpy(FMP4BaseBox::m_acBoxType,"stbl",sizeof(FMP4BaseBox::m_acBoxType));//""
	};
    int SetParams(unsigned int i_dwTrakHandlerType) 
	{
	    int iRet = -1;
	    iRet = m_Stsd.SetParams(i_dwTrakHandlerType);//先放前面，才能确定后面的大小
        m_dwTrakHandlerType=i_dwTrakHandlerType;
        FMP4BaseBox::m_dwBoxSize += m_Stsd.m_dwBoxSize;
        FMP4BaseBox::m_dwBoxSize += m_Stts.m_dwBoxSize;
        if(FMP4_VIDEO == m_dwTrakHandlerType)
        {
            FMP4BaseBox::m_dwBoxSize += m_Stss.m_dwBoxSize;//ffmpeg 无
        }
        FMP4BaseBox::m_dwBoxSize += m_Stsc.m_dwBoxSize;
        FMP4BaseBox::m_dwBoxSize += m_Stsz.m_dwBoxSize;
        FMP4BaseBox::m_dwBoxSize += m_Stco.m_dwBoxSize;
        return iRet;
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        

        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4StsdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        
        iLen += m_Stsd.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Stts.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        if(FMP4_VIDEO == m_dwTrakHandlerType)
        {
            iLen += m_Stss.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);//ffmpeg 无
        }
        iLen += m_Stsc.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Stsz.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Stco.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        
        return iLen;
    };
    
	FMP4StsdBox m_Stsd;
private:
	FMP4SttsBox m_Stts;//
	FMP4StssBox m_Stss;// video only
	//FMP4CttsBox ctts;//
	FMP4StscBox m_Stsc;//
	FMP4StszBox m_Stsz;//
	FMP4StcoBox m_Stco;//

    unsigned int m_dwTrakHandlerType;
};

/**********************************Level 4************************************/

/*****************************************************************************
-Class          : FMP4ElstBox()
-Description    : Level 4,第四级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4ElstBox : public FMP4FullBaseBox
{
public:
	FMP4ElstBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwEntryCount)+sizeof(m_dwSegmentDuration)+sizeof(m_dwMediaTime)+sizeof(m_wMediaRateInteger)+sizeof(m_wMediaRateFraction);
        memcpy(FMP4FullBaseBox::m_acBoxType,"elst",sizeof(FMP4FullBaseBox::m_acBoxType));
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4ElstBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen),m_dwEntryCount);/*  */
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwSegmentDuration);/*  */
        iLen+=4;
        Write32BE((o_pbBuf+iLen), m_dwMediaTime); /*  */
        iLen+=4;
        
        Write16BE((o_pbBuf+iLen), m_wMediaRateInteger); /*  */
        iLen+=2;
        Write16BE((o_pbBuf+iLen), m_wMediaRateFraction); /*  */
        iLen+=2;

        return iLen;
    };
    
private:
	unsigned int m_dwEntryCount = 1;
	unsigned int m_dwSegmentDuration = 0;
	unsigned int m_dwMediaTime = 0;
	unsigned short m_wMediaRateInteger = 1;
	unsigned short m_wMediaRateFraction = 0;
};

/*****************************************************************************
-Class          : FMP4MdhdBox(Media Header Box)
-Description    : Level 4,第四级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MdhdBox : public FMP4FullBaseBox
{
public:
	FMP4MdhdBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwCreationTime)+sizeof(m_dwModificationTime)+sizeof(m_dwTimeScale)+sizeof(m_dwDuration)+sizeof(m_abLanguage);
        memcpy(FMP4FullBaseBox::m_acBoxType,"mdhd",sizeof(FMP4FullBaseBox::m_acBoxType));
	};
	int SetParams(unsigned int i_dwTimeScale,unsigned int i_dwCreationTime=0,unsigned int i_dwModificationTime=0) 
	{
        FMP4_LOGW("FMP4MdhdBox SetParams %d\r\n",i_dwTimeScale);
		if(i_dwCreationTime != 0)
            m_dwCreationTime = i_dwCreationTime;
		if(i_dwModificationTime != 0)
            m_dwModificationTime = i_dwModificationTime;
        m_dwTimeScale = i_dwTimeScale;
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MdhdBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen),m_dwCreationTime);/* creation_time */
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwModificationTime);/* modification_time */
        iLen+=4;
        Write32BE((o_pbBuf+iLen), m_dwTimeScale); /* timescale ,ffmpeg video 12800,aac 44100采样率*/
        iLen+=4;
        
        Write32BE((o_pbBuf+iLen), m_dwDuration); /* duration */
        iLen+=4;
        memcpy(o_pbBuf+iLen,m_abLanguage,sizeof(m_abLanguage));
        iLen+=sizeof(m_abLanguage);

        return iLen;
    };

private:
	unsigned int m_dwCreationTime = time(NULL) + 0x7C25B080; // 1970 based -> 1904 based;;// seconds sine midnight, Jan. 1, 1904, UTC
	unsigned int m_dwModificationTime = time(NULL) + 0x7C25B080; // seconds sine midnight, Jan. 1, 1904, UTC
	unsigned int m_dwTimeScale = 0X15F90;// second  //m_dwTimeScale 表示整个mp4中的时间单位是1/1000s，即当前的时间单位为ms
	unsigned int m_dwDuration = 0;//            //比如，	tfhd中的default_sample_duration值为40，则表示当前帧持续时间为40ms
	unsigned char m_abLanguage[4] = {/* ISO-639-2/T language code */ 0x55, 0xc4, /* pre_defined (quality) */0x00, 0x00 };

};

/*****************************************************************************
-Class          : FMP4HdlrBox(Handler Reference Box)解释了媒体的播放过程信息
-Description    : Level 4,第四级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4HdlrBox : public FMP4FullBaseBox
{
public:
	FMP4HdlrBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwPreDefined)+sizeof(m_dwHandlerType)+sizeof(m_dwReserved)+sizeof(m_abHandlerDescr);
        memcpy(FMP4FullBaseBox::m_acBoxType,"hdlr",sizeof(FMP4FullBaseBox::m_acBoxType));
	};
	int SetParams(unsigned int i_dwHandlerType,char *i_strHandlerDescr) 
	{
        FMP4_LOGW("FMP4HdlrBox SetParams %x,%s\r\n",i_dwHandlerType,i_strHandlerDescr);
        m_dwHandlerType = i_dwHandlerType;
        snprintf(m_abHandlerDescr,sizeof(m_abHandlerDescr),"%s",i_strHandlerDescr);
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4HdlrBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen),m_dwPreDefined); /* pre_defined */
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwHandlerType);/* handler_type */
        iLen+=4;

        for(i=0;i<sizeof(m_dwReserved)/sizeof(unsigned int);i++)
        {
            Write32BE((o_pbBuf+iLen), m_dwReserved[i]); 
            iLen+=4;
        }
        memcpy(o_pbBuf+iLen,m_abHandlerDescr,sizeof(m_abHandlerDescr));
        iLen+=sizeof(m_abHandlerDescr);
        
        return iLen;
    };
private:
	unsigned int m_dwPreDefined = 0;
	unsigned int m_dwHandlerType = 0x76696465;// MOV_VIDEO/MOV_AUDIO
	unsigned int m_dwReserved[3] = { 0 };
	char  m_abHandlerDescr[13] = { 0 };// VideoHandler/SoundHandler/SubtitleHandler
};
/*****************************************************************************
-Class          : FMP4MinfBox(Media Information Box)存储了解释track媒体数据的handler-specific信息，
                media handler用这些信息将媒体时间映射到媒体数据并进行处理
-Description    : Level 4,第四级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MinfBox :public FMP4BaseBox
{
public:
	FMP4MinfBox()
	{
        memcpy(FMP4BaseBox::m_acBoxType,"minf",sizeof(FMP4BaseBox::m_acBoxType));//""
	};
    int SetParams(unsigned int i_dwTrakHandlerType) 
	{
	    int iRet = -1;
	    iRet = m_Stbl.SetParams(i_dwTrakHandlerType);//先放前面，才能确定后面的大小
        m_dwTrakHandlerType = i_dwTrakHandlerType;
        if(FMP4_VIDEO == m_dwTrakHandlerType)
        {
            FMP4BaseBox::m_dwBoxSize += m_Vmhd.m_dwBoxSize;//必须放在ToBits前，因为调用ToBits前就要用
        }
        else if(FMP4_AUDIO == m_dwTrakHandlerType)
        {
            FMP4BaseBox::m_dwBoxSize += m_Smhd.m_dwBoxSize;
        }
        else
        {
            FMP4_LOGE("FMP4MinfBox ToBits i_dwTrakHandlerType err %d,%d",m_dwTrakHandlerType,i_dwTrakHandlerType);
        }
        FMP4BaseBox::m_dwBoxSize += m_Dinf.m_dwBoxSize;
        FMP4BaseBox::m_dwBoxSize += m_Stbl.m_dwBoxSize;
        return iRet;
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        

        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4StsdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        if(FMP4_VIDEO == m_dwTrakHandlerType)
        {
            iLen += m_Vmhd.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        }
        else if(FMP4_AUDIO == m_dwTrakHandlerType)
        {
            iLen += m_Smhd.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        }
        iLen += m_Dinf.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Stbl.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        
        return iLen;
    };
    
	FMP4StblBox m_Stbl;
private:
	//vmhd、smhd、hmhd、nmhd 中选
	FMP4VmhdBox m_Vmhd;
	FMP4SmhdBox m_Smhd;
	
	FMP4DinfBox m_Dinf;


    unsigned int m_dwTrakHandlerType;
};


/**********************************Level 3************************************/

/*****************************************************************************
-Class          : FMP4TkhdBox( Track Header Box)
-Description    : Level 3,第三级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/

class FMP4TkhdBox : public FMP4FullBaseBox
{
public:
	FMP4TkhdBox() 
	{

        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwCreationTime)+sizeof(m_dwModificationTime)+sizeof(m_dwTrackID)+sizeof(m_abR0)+sizeof(m_dwDuration)+
        sizeof(m_abR1)+sizeof(m_wLayer)+sizeof(m_wAlternateGroup)+sizeof(m_wVolume)+sizeof(m_abR2)+sizeof(m_adwMatrix)+sizeof(m_dwWidth)+sizeof(m_dwHeight);
        memcpy(FMP4FullBaseBox::m_acBoxType,"tkhd",sizeof(FMP4FullBaseBox::m_acBoxType));
        
	};
	int SetParams(unsigned char i_bFlags,unsigned int i_dwTrackID,unsigned short i_wVolume,unsigned int i_dwWidth,unsigned int i_dwHeight,unsigned int i_dwCreationTime=0,unsigned int i_dwModificationTime=0) 
	{
        FMP4_LOGW("FMP4TkhdBox SetParams %d,%d,i_dwWidth %d,i_dwHeight %d\r\n",i_dwTrackID,i_wVolume,i_dwWidth,i_dwHeight);
		FMP4FullBaseBox::m_abFlags[2] = i_bFlags;
		if(i_dwCreationTime != 0)
            m_dwCreationTime = i_dwCreationTime;
		if(i_dwModificationTime != 0)
            m_dwModificationTime = i_dwModificationTime;
        m_dwTrackID = i_dwTrackID;
        m_wVolume = i_wVolume;
        m_dwWidth = i_dwWidth<< 16;
        m_dwHeight = i_dwHeight<< 16;
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TkhdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen),m_dwCreationTime);/* creation_time */
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwModificationTime);/* modification_time */
        iLen+=4;
        Write32BE((o_pbBuf+iLen), m_dwTrackID); /* track_ID */
        iLen+=4;
        memcpy(o_pbBuf+iLen,m_abR0,sizeof(m_abR0));/* reserved */
        iLen+=sizeof(m_abR0);
        
        Write32BE((o_pbBuf+iLen), m_dwDuration); /* duration */
        iLen+=4;
        memcpy(o_pbBuf+iLen,m_abR1,sizeof(m_abR1));/* reserved */
        iLen+=sizeof(m_abR1);
        Write16BE((o_pbBuf+iLen),m_wLayer); 
        iLen+=2;
        Write16BE((o_pbBuf+iLen),m_wAlternateGroup); 
        iLen+=2;
        Write16BE((o_pbBuf+iLen),m_wVolume); 
        iLen+=2;
        memcpy(o_pbBuf+iLen,m_abR2,sizeof(m_abR2));/* reserved */
        iLen+=sizeof(m_abR2);
        
        for(i=0;i<sizeof(m_adwMatrix)/sizeof(unsigned int);i++)
        {
            Write32BE((o_pbBuf+iLen), m_adwMatrix[i]); 
            iLen+=4;
        }
        
        Write32BE((o_pbBuf+iLen), m_dwWidth);/*track->av.video.width * 0x10000U*/ /* width */
        iLen+=4;
        Write32BE((o_pbBuf+iLen), m_dwHeight);/*track->av.video.height * 0x10000U*//* height */
        iLen+=4;
        
        return iLen;
    };
private:
	unsigned int m_dwCreationTime = time(NULL) + 0x7C25B080; // 1970 based -> 1904 based;;// seconds sine midnight, Jan. 1, 1904, UTC
	unsigned int m_dwModificationTime = time(NULL) + 0x7C25B080; // seconds sine midnight, Jan. 1, 1904, UTC
	unsigned int m_dwTrackID = 1;// cannot be zero
	unsigned char m_abR0[4] = { 0 };//reserved;
	unsigned int m_dwDuration = 0;// default uint64_MAX(by Movie Header Box timescale)
	unsigned char m_abR1[8] = { 0 };
	unsigned short m_wLayer = 0;
	unsigned short m_wAlternateGroup = 0;
	unsigned short m_wVolume = 0;// fixed point 8.8 number, 1.0 (0x0100) is full volume
	unsigned char m_abR2[2] = { 0 };
	unsigned int m_adwMatrix[9] = { 0x00010000/* u */,0,0,0/* v */,0x00010000,0,0 /* w */,0,0x40000000};// u,v,w
	unsigned int m_dwWidth = 0X07800000;// fixed-point 16.16 values
	unsigned int m_dwHeight = 0X04380000;// fixed-point 16.16 values
};

/*****************************************************************************
-Class          : FMP4EdtsBox()
-Description    : Level 3,第三级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4EdtsBox : public FMP4BaseBox
{
public:
	FMP4EdtsBox() 
	{
        memcpy(FMP4BaseBox::m_acBoxType,"edts",sizeof(FMP4BaseBox::m_acBoxType));//""
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        FMP4BaseBox::m_dwBoxSize += m_Elst.m_dwBoxSize;
        
        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4EdtsBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        iLen += m_Elst.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        
        return iLen;
    };

private:
	FMP4ElstBox m_Elst;
};

/*****************************************************************************
-Class          : FMP4MdiaBox(Media Box)
-Description    : Level 3,第三级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MdiaBox : public FMP4BaseBox
{
public:
	FMP4MdiaBox()
	{
        memcpy(FMP4BaseBox::m_acBoxType,"mdia",sizeof(FMP4BaseBox::m_acBoxType));//""
	};
    int SetParams(unsigned int i_dwTrakHandlerType) 
	{
	    int iRet = -1;
	    iRet = m_Minf.SetParams(i_dwTrakHandlerType);//先放前面，才能确定后面的大小
        FMP4BaseBox::m_dwBoxSize += m_Mdhd.m_dwBoxSize;//必须放在ToBits前，因为调用ToBits前就要用
        FMP4BaseBox::m_dwBoxSize += m_Hdlr.m_dwBoxSize;
        FMP4BaseBox::m_dwBoxSize += m_Minf.m_dwBoxSize;
        return iRet;
	}
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        
        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MdiaBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        iLen += m_Mdhd.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Hdlr.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Minf.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        
        return iLen;
    };

    FMP4MdhdBox m_Mdhd;
    FMP4HdlrBox m_Hdlr;
	FMP4MinfBox m_Minf;//m_Stbl FMP4StsdBox m_VideoBox
private:

};

/*****************************************************************************
-Class          : FMP4MehdBox()
-Description    : Level 3,第三级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MehdBox : public FMP4FullBaseBox
{
public:
	FMP4MehdBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_ddwFragmentDuration);
        memcpy(FMP4FullBaseBox::m_acBoxType,"mehd",sizeof(FMP4FullBaseBox::m_acBoxType));//""
        FMP4FullBaseBox::m_bVersion=1;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MvhdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);

        Write32BE((o_pbBuf+iLen),(unsigned int)(m_ddwFragmentDuration>> 32));/*  */
        iLen+=4;
        Write32BE((o_pbBuf+iLen),(unsigned int)m_ddwFragmentDuration);/*  */
        iLen+=4;
        return iLen;
    };
    
private:
	uint64_t m_ddwFragmentDuration = 0;
};
/*****************************************************************************
-Class          : FMP4TrexBox()
-Description    : Level 3,第三级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4TrexBox : public FMP4FullBaseBox
{
public:
	FMP4TrexBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwTrackId)+sizeof(m_dwSampleDescriptionIndex)+sizeof(m_dwSampleDuration)+sizeof(m_dwSampleSize)+sizeof(m_dwSampleFlags);
        memcpy(FMP4FullBaseBox::m_acBoxType,"trex",sizeof(FMP4FullBaseBox::m_acBoxType));//""
	};
	int SetParams(unsigned int i_dwTrackId) 
	{
        FMP4_LOGW("FMP4TrexBox SetParams %d,%d\r\n",m_dwTrackId,i_dwTrackId);
        m_dwTrackId = i_dwTrackId;
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TrexBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen),m_dwTrackId);
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwSampleDescriptionIndex);
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwSampleDuration);
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwSampleSize);
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwSampleFlags);
        iLen+=4;

        return iLen;
    };
private:
	unsigned int m_dwTrackId = 1;/* track_ID */
	unsigned int m_dwSampleDescriptionIndex = 1;/* default_sample_description_index */
	unsigned int m_dwSampleDuration = 0; /* default_sample_duration */
	unsigned int m_dwSampleSize = 0;/* default_sample_size */
	unsigned int m_dwSampleFlags = 0;/* default_sample_flags */
};

/*****************************************************************************
-Class          : FMP4TfhdBox(Track Fragment Header Box)
主要是对指定的 trak进行相关的默认设置
-Description    : Level 3,第三级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4TfhdBox : public FMP4FullBaseBox
{
public:
	FMP4TfhdBox() 
	{
        memcpy(FMP4FullBaseBox::m_acBoxType,"tfhd",sizeof(FMP4FullBaseBox::m_acBoxType));
	};
	int SetParams(unsigned int i_dwFlags,unsigned int i_dwTrackId,uint64_t i_ddwBaseDataOffset,unsigned int i_dwDefaultSampleDuration,unsigned int i_dwDefaultSampleSize,unsigned int i_dwDefaultSampleFlags) 
	{
        FMP4_LOGW("FMP4TfhdBox SetParams %d,%d, %d,%d, %d\r\n",i_dwFlags,i_dwTrackId,i_dwDefaultSampleDuration,i_dwDefaultSampleSize,i_dwDefaultSampleFlags);
        Write24BE(FMP4FullBaseBox::m_abFlags,i_dwFlags);
        m_dwTrackId = i_dwTrackId;
        
        m_ddwBaseDataOffset = i_ddwBaseDataOffset;
        m_dwDefaultSampleDuration = i_dwDefaultSampleDuration;
        m_dwDefaultSampleSize = i_dwDefaultSampleSize;
        m_dwDefaultSampleFlags = i_dwDefaultSampleFlags;

        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwTrackId);
        if (FMP4_TFHD_FLAG_BASE_DATA_OFFSET & i_dwFlags)
        {
            FMP4FullBaseBox::m_dwBoxSize += sizeof(m_ddwBaseDataOffset);
        }
        if (FMP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX & i_dwFlags)
        {
            FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwSampleDescriptionIndex);
        }
        if (FMP4_TFHD_FLAG_DEFAULT_DURATION & i_dwFlags)
        {
            FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwDefaultSampleDuration);
        }
        if (FMP4_TFHD_FLAG_DEFAULT_SIZE & i_dwFlags)
        {
            FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwDefaultSampleSize);
        }
        if (FMP4_TFHD_FLAG_DEFAULT_FLAGS & i_dwFlags)
        {
            FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwDefaultSampleFlags);
        }
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        unsigned int dwFlags = 0;
        Read24BE(FMP4FullBaseBox::m_abFlags,&dwFlags);

        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TrexBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);

        
        Write32BE((o_pbBuf+iLen),m_dwTrackId);
        iLen+=4;
        if (FMP4_TFHD_FLAG_BASE_DATA_OFFSET & dwFlags)
        {
            Write32BE((o_pbBuf+iLen),(unsigned int)(m_ddwBaseDataOffset >> 32));
            iLen+=4;
            Write32BE((o_pbBuf+iLen),m_ddwBaseDataOffset);
            iLen+=4;
        }
        if (FMP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX & dwFlags)
        {
            Write32BE((o_pbBuf+iLen),m_dwSampleDescriptionIndex);
            iLen+=4;
        }
        if (FMP4_TFHD_FLAG_DEFAULT_DURATION & dwFlags)
        {
            Write32BE((o_pbBuf+iLen),m_dwDefaultSampleDuration);
            iLen+=4;
        }
        if (FMP4_TFHD_FLAG_DEFAULT_SIZE & dwFlags)
        {
            Write32BE((o_pbBuf+iLen),m_dwDefaultSampleSize);
            iLen+=4;
        }
        if (FMP4_TFHD_FLAG_DEFAULT_FLAGS & dwFlags)
        {
            Write32BE((o_pbBuf+iLen),m_dwDefaultSampleFlags);
            iLen+=4;
        }

        return iLen;
    };
    static const unsigned int FMP4_TFHD_FLAG_BASE_DATA_OFFSET           = 0x00000001;
    static const unsigned int FMP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX	= 0x00000002;
    static const unsigned int FMP4_TFHD_FLAG_DEFAULT_DURATION           = 0x00000008;
    static const unsigned int FMP4_TFHD_FLAG_DEFAULT_SIZE               = 0x00000010;
    static const unsigned int FMP4_TFHD_FLAG_DEFAULT_FLAGS              = 0x00000020;
    static const unsigned int FMP4_TFHD_FLAG_DURATION_IS_EMPTY          = 0x00010000;
    static const unsigned int FMP4_TFHD_FLAG_DEFAULT_BASE_IS_MOOF       = 0x00020000;

private:
	unsigned int m_dwTrackId =1;
	uint64_t m_ddwBaseDataOffset = 0;// hton32(0X001D27B);
	unsigned int m_dwSampleDescriptionIndex =1;//data_reference_index//uint32_t sample_description_index = hton32(1); 
	unsigned int m_dwDefaultSampleDuration = 0;//hton32(0XE10);
	unsigned int m_dwDefaultSampleSize = 0;
	unsigned int m_dwDefaultSampleFlags = 0;//hton32(0X01010000);
};

/*****************************************************************************
-Class          : FMP4TfdtBox(Track Fragment Decode Time Box)
存放相关 sample 编码的绝对时间的。因为 fMP4 是流式的格式，
所以不像 MP4一样可以直接根据 sample 直接 seek 到具体位置。
这里就需要一个标准时间参考，来快速定位都某个具体的 fragment。
-Description    : Level 3,第三级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4TfdtBox : public FMP4FullBaseBox
{
public:
	FMP4TfdtBox() 
	{
        memcpy(FMP4FullBaseBox::m_acBoxType,"tfdt",sizeof(FMP4FullBaseBox::m_acBoxType));
	};

	int SetParams(uint64_t i_ddwBaseDecodeTime) //i_ddwBaseDataOffset
	{
        FMP4_LOGW("FMP4TfdtBox SetParams %lld \r\n",i_ddwBaseDecodeTime);
        m_ddwBaseMediaDecodeTime = i_ddwBaseDecodeTime;
        if (m_ddwBaseMediaDecodeTime > INT32_MAX)
        {
            FMP4FullBaseBox::m_bVersion = 1;
        }
        if (0 == FMP4FullBaseBox::m_bVersion)
        {
            FMP4FullBaseBox::m_dwBoxSize += 4;
        }
        else
        {
            FMP4FullBaseBox::m_dwBoxSize += 8;
        }
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;

        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TfdtBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);

        
        if (0 == FMP4FullBaseBox::m_bVersion)
        {
            Write32BE((o_pbBuf+iLen),(unsigned int)(m_ddwBaseMediaDecodeTime));
            iLen+=4;
        }
        else
        {
            Write32BE((o_pbBuf+iLen),(unsigned int)(m_ddwBaseMediaDecodeTime >> 32));
            iLen+=4;
            Write32BE((o_pbBuf+iLen),m_ddwBaseMediaDecodeTime);
            iLen+=4;
        }

        return iLen;
    };
private:	
	uint64_t m_ddwBaseMediaDecodeTime = 0;//// if version= 0 , decode time use uint32_t type.
};
/*****************************************************************************
-Class          : FMP4TrunBox(Track Fragment Run Box)
存储该 moof 里面相关的 sample 内容。
例如，每个 sample (每帧)的 size、 duration、offset等
-Description    : Level 3,第三级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/12/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4TrunBox : public FMP4FullBaseBox
{
public:
	FMP4TrunBox() 
	{
        memcpy(FMP4FullBaseBox::m_acBoxType,"trun",sizeof(FMP4FullBaseBox::m_acBoxType));
        FMP4FullBaseBox::m_bVersion = 1;
	};
	int SetParams(unsigned int i_dwDefaultSampleSize,unsigned int i_dwDefaultSampleDuration,unsigned int i_dwSampleCount,unsigned int i_dwDataOffset,T_FMP4SampleInfo * i_aptSampleInfo) //浅拷贝
	{
	    unsigned int dwFlags;
        if (i_dwSampleCount < 1 || NULL == i_aptSampleInfo) 
        {
            FMP4_LOGE("FMP4TrunBox SetParams empty err %d,%d\r\n",i_dwSampleCount,m_dwSampleCount);
            return -1;
        }
        FMP4_LOGW("FMP4TrunBox SetParams %d,%d, %d,%d, %d\r\n",i_dwSampleCount,i_dwDefaultSampleSize,i_dwDefaultSampleDuration,i_dwSampleCount,i_dwDataOffset);
        dwFlags = FMP4_TRUN_FLAG_DATA_OFFSET_PRESENT;
        if (i_aptSampleInfo[0].eFrameType == FMP4_VIDEO_KEY_FRAME)
            dwFlags |= FMP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT;
        for (int i = 0; i < i_dwSampleCount; i++)
        {
            if (i_aptSampleInfo[i].dwDataSize != i_dwDefaultSampleSize)
                dwFlags |= FMP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT;
            if (i_aptSampleInfo[i].dwSampleDuration != i_dwDefaultSampleDuration)
                dwFlags |= FMP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT;
            if (i_aptSampleInfo[i].ddwPTS != i_aptSampleInfo[i].ddwDTS)
                dwFlags |= FMP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT;
        }
        
        Write24BE(FMP4FullBaseBox::m_abFlags,dwFlags);
        m_dwSampleCount = i_dwSampleCount;
        
        m_dwDataOffset = i_dwDataOffset;//表示mdat box里的数据相对与moof开始(BoxSize字节开始)的偏移量
        m_aptSampleInfo = i_aptSampleInfo;

        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwSampleCount);
        if (FMP4_TRUN_FLAG_DATA_OFFSET_PRESENT & dwFlags)
        {
            FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwDataOffset);
        }
        if (FMP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT & dwFlags)
        {
            FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwFirstSampleFlags);
        }
        for (int i = 0; i < m_dwSampleCount; i++)
        {
            if (dwFlags & FMP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT)
            {
                FMP4FullBaseBox::m_dwBoxSize += sizeof(m_aptSampleInfo[i].dwSampleDuration);
            }
        
            if (dwFlags & FMP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT)
            {
                FMP4FullBaseBox::m_dwBoxSize += sizeof(m_aptSampleInfo[i].dwDataSize);
            }
        
            //assert(0 == (dwFlags & FMP4_TRUN_FLAG_SAMPLE_FLAGS_PRESENT));
            //mov_buffer_w32(&mov->io, 0); /* sample_flags */ 
        
            if (dwFlags & FMP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT)
            {
                FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwSampleCompositionTimeOffset);
            }
        }
        return 0;
	};
	int ModifyParams(unsigned int i_dwDataOffset) 
	{
        FMP4_LOGW("ModifyParams i_dwDataOffset%d,%d\r\n",i_dwDataOffset,m_dwDataOffset);
        m_dwDataOffset = i_dwDataOffset;//表示mdat box里的数据相对于moof开始(BoxSize字节的位置)的偏移量
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        unsigned int dwFlags = 0;

        if (m_dwSampleCount < 1) 
        {
            FMP4_LOGE("FMP4TrunBox ToBits empty err %d,%d\r\n",i_dwMaxBufLen,m_dwSampleCount);
            return iLen;
        }
        Read24BE(FMP4FullBaseBox::m_abFlags,&dwFlags);

        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TrunBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);

        Write32BE((o_pbBuf+iLen),m_dwSampleCount);
        iLen+=4;
        if (FMP4_TRUN_FLAG_DATA_OFFSET_PRESENT & dwFlags)
        {
            Write32BE((o_pbBuf+iLen),m_dwDataOffset);
            iLen+=4;
        }
        if (FMP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT & dwFlags)
        {
            Write32BE((o_pbBuf+iLen),m_dwFirstSampleFlags);//FMP4_TREX_FLAG_SAMPLE_DEPENDS_ON_I_PICTURE
            iLen+=4;
        }
        for (i = 0; i < m_dwSampleCount; i++)
        {
            if (dwFlags & FMP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT)
            {
                Write32BE((o_pbBuf+iLen),m_aptSampleInfo[i].dwSampleDuration);/* sample_duration */
                iLen+=4;
            }
    
            if (dwFlags & FMP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT)
            {
                Write32BE((o_pbBuf+iLen),m_aptSampleInfo[i].dwDataSize);/* sample_size */
                iLen+=4;
            }
    
            //assert(0 == (dwFlags & FMP4_TRUN_FLAG_SAMPLE_FLAGS_PRESENT));
            //mov_buffer_w32(&mov->io, 0); /* sample_flags */ 
    
            if (dwFlags & FMP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT)
            {
                m_dwSampleCompositionTimeOffset = (unsigned int)(m_aptSampleInfo[i].ddwPTS- m_aptSampleInfo[i].ddwDTS); /* sample_composition_time_offset */
                Write32BE((o_pbBuf+iLen),m_dwSampleCompositionTimeOffset);
                iLen+=4;
            }
        }

        return iLen;
    };
    static const unsigned int FMP4_TRUN_FLAG_DATA_OFFSET_PRESENT				          =	0x0001;
    static const unsigned int FMP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT			          =	0x0004;
    static const unsigned int FMP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT			          =	0x0100;
    static const unsigned int FMP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT				          =	0x0200;
    static const unsigned int FMP4_TRUN_FLAG_SAMPLE_FLAGS_PRESENT				          =	0x0400;
    static const unsigned int FMP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT =	0x0800;

private:	
	unsigned int m_dwSampleCount = 0;
	unsigned int m_dwDataOffset = 0;
	unsigned int m_dwFirstSampleFlags = FMP4_TREX_FLAG_SAMPLE_DEPENDS_ON_I_PICTURE;
	unsigned int m_dwSampleCompositionTimeOffset = 0;
	T_FMP4SampleInfo * m_aptSampleInfo;
	
};

/**********************************Level 2************************************/

/*****************************************************************************
-Class          : FMP4MvhdBox(Movie Header Box)
-Description    : Level 2,第二级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MvhdBox : public FMP4FullBaseBox
{
public:
	FMP4MvhdBox() 
	{
        FMP4FullBaseBox::m_dwBoxSize += sizeof(m_dwCreationTime)+sizeof(m_dwModificationTime)+sizeof(m_dwTimeScale)+sizeof(m_dwDuration)+sizeof(m_dwRate)+
        sizeof(m_wVolume)+sizeof(m_abR0)+sizeof(m_adwMatrix)+sizeof(m_adwPreDefined)+sizeof(m_dwNextTrackID);
        memcpy(FMP4FullBaseBox::m_acBoxType,"mvhd",sizeof(FMP4FullBaseBox::m_acBoxType));
	};
	int SetParams(unsigned int i_dwNextTrackID) 
	{
        FMP4_LOGW("FMP4MvhdBox SetParams %d,%d\r\n",m_dwNextTrackID,i_dwNextTrackID);
        m_dwNextTrackID = i_dwNextTrackID;//ffmpeg 2,media 3
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MvhdBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen),m_dwCreationTime);/* creation_time */
        iLen+=4;
        Write32BE((o_pbBuf+iLen),m_dwModificationTime);/* modification_time */
        iLen+=4;
        Write32BE((o_pbBuf+iLen), m_dwTimeScale); /* timescale */
        iLen+=4;
        Write32BE((o_pbBuf+iLen), m_dwDuration); /* duration */
        iLen+=4;

        Write32BE((o_pbBuf+iLen), m_dwRate); /* rate 1.0 */
        iLen+=4;
        Write16BE((o_pbBuf+iLen),m_wVolume); /* volume 1.0 = normal */
        iLen+=2;
        memcpy(o_pbBuf+iLen,m_abR0,sizeof(m_abR0));/* reserved */
        iLen+=sizeof(m_abR0);

        for(i=0;i<sizeof(m_adwMatrix)/sizeof(unsigned int);i++)
        {
            Write32BE((o_pbBuf+iLen), m_adwMatrix[i]); 
            iLen+=4;
        }
        for(i=0;i<sizeof(m_adwPreDefined)/sizeof(unsigned int);i++)
        {
            Write32BE((o_pbBuf+iLen), m_adwPreDefined[i]); 
            iLen+=4;
        }
        
        Write32BE((o_pbBuf+iLen), m_dwNextTrackID); /* Next track id */
        iLen+=4;
        
        return iLen;
    };
private:
    //根据协议规定长度定义每个变量大小
	unsigned int m_dwCreationTime = time(NULL) + 0x7C25B080; // 1970 based -> 1904 based;;// seconds sine midnight, Jan. 1, 1904, UTC
	unsigned int m_dwModificationTime = time(NULL) + 0x7C25B080; // seconds sine midnight, Jan. 1, 1904, UTC
	unsigned int m_dwTimeScale = 1000;// time-scale for the entire presentation, the number of time units that pass in one second
	unsigned int m_dwDuration = 0; // default UINT64_MAX(by timescale)
	unsigned int m_dwRate = 0X10000;/* rate 1.0 */
	unsigned short m_wVolume = 0X100;// fixed point 8.8 number, 1.0 (0x0100) is full volume
	unsigned char m_abR0[10] = { 0 };
	unsigned int m_adwMatrix[9] = { 0x00010000/* u */,0,0,0/* v */,0x00010000,0,0 /* w */,0,0x40000000};// u,v,w
	unsigned int m_adwPreDefined[6] = { 0,0,0,0,0,0 };/* reserved (preview time) *//* reserved (preview duration) *//* reserved (poster time) *//* reserved (selection time) *//* reserved (selection duration) *//* reserved (current time) */
	unsigned int m_dwNextTrackID = 1;//从1开始，每添加一条流则加1
};


/*****************************************************************************
-Class          : FMP4TrakBox(Track Box)包含了该track的媒体数据引用和描述
                “trak”必须包含一个“tkhd”和一个“mdia”，此外还有很多可选的box（略）
-Description    : Level 2,第二级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4TrakBox : public FMP4BaseBox
{
public:
	FMP4TrakBox()
	{
        memcpy(FMP4BaseBox::m_acBoxType,"trak",sizeof(FMP4BaseBox::m_acBoxType));
	};
	int SetParams(unsigned int i_dwTrakHandlerType,unsigned int i_dwTrackID,T_FMP4SampleInfo *ptVideoFrameInfo,T_FMP4SampleInfo *ptAudioFrameInfo) 
	{
        FMP4_LOGW("FMP4TrakBox SetParams %d %d \r\n",i_dwTrakHandlerType,i_dwTrackID);
        if(FMP4_VIDEO == i_dwTrakHandlerType)
        {
            if(NULL == ptVideoFrameInfo)
            {
                FMP4_LOGE("FMP4TrakBox SetParams NULL == ptVideoFrameInfo %d\r\n",i_dwTrakHandlerType);
                return -1;
            }
            m_Mdia.m_Mdhd.SetParams(1000);//表示整个mp4中的时间单位是1/1000s，即当前的时间单位为ms
            if(FMP4_OBJECT_TYPE_H264 == ptVideoFrameInfo->eEncType)
            {//level低的放前面，否则会影响level高中的boxsize大小
                m_Mdia.m_Minf.m_Stbl.m_Stsd.m_VideoBox.m_AvcCBox.SetParams(ptVideoFrameInfo->abEncExtraData,ptVideoFrameInfo->iEncExtraDataLen);
            }
            else if(FMP4_OBJECT_TYPE_H265 == ptVideoFrameInfo->eEncType)
            {
                m_Mdia.m_Minf.m_Stbl.m_Stsd.m_VideoBox.m_HvcCBox.SetParams(ptVideoFrameInfo->abEncExtraData,ptVideoFrameInfo->iEncExtraDataLen);
            }
            else
            {
                FMP4_LOGE("FMP4TrakBox SetParams ptVideoFrameInfo->eEncType err %d\r\n",ptVideoFrameInfo->eEncType);
                return -1;
            }
            m_Mdia.m_Minf.m_Stbl.m_Stsd.m_VideoBox.SetParams(ptVideoFrameInfo->eEncType,ptVideoFrameInfo->tVideoEncParam.dwWidth,ptVideoFrameInfo->tVideoEncParam.dwHeight);
            m_Mdia.m_Hdlr.SetParams(i_dwTrakHandlerType,(char *)"VideoHandler");
            m_Mdia.SetParams(i_dwTrakHandlerType);
            m_Tkhd.SetParams(FMP4_TKHD_FLAG_TRACK_ENABLE|FMP4_TKHD_FLAG_TRACK_IN_MOVIE,i_dwTrackID,
            0,ptVideoFrameInfo->tVideoEncParam.dwWidth,ptVideoFrameInfo->tVideoEncParam.dwHeight);
        }
        else if(FMP4_AUDIO == i_dwTrakHandlerType)
        {
            if(NULL == ptAudioFrameInfo)
            {
                FMP4_LOGE("FMP4TrakBox SetParams NULL == ptAudioFrameInfo %d\r\n",i_dwTrakHandlerType);
                return -1;
            }
            m_Mdia.m_Mdhd.SetParams(1000);//比如，	tfhd中的default_sample_duration值为40，则表示当前帧持续时间为40ms
            if(FMP4_OBJECT_TYPE_G711A == ptAudioFrameInfo->eEncType ||FMP4_OBJECT_TYPE_G711U == ptAudioFrameInfo->eEncType)
            {//level低的放前面，否则会影响level高中的boxsize大小
            }
            else if(FMP4_OBJECT_TYPE_AAC == ptAudioFrameInfo->eEncType ||FMP4_OBJECT_TYPE_MP3 == ptAudioFrameInfo->eEncType)
            {//level低的放前面，否则会影响level高中的boxsize大小
                m_Mdia.m_Minf.m_Stbl.m_Stsd.m_AudioBox.m_EsdsBox.SetParams(i_dwTrackID,ptAudioFrameInfo->eEncType,ptAudioFrameInfo->abEncExtraData,ptAudioFrameInfo->iEncExtraDataLen);
            }
            else
            {
                FMP4_LOGE("FMP4TrakBox SetParams ptAudioFrameInfo->eEncType err %d\r\n",ptAudioFrameInfo->eEncType);
                return -1;
            }
            m_Mdia.m_Minf.m_Stbl.m_Stsd.m_AudioBox.SetParams(ptAudioFrameInfo->eEncType,ptAudioFrameInfo->tAudioEncParam.dwChannels, 
            ptAudioFrameInfo->tAudioEncParam.dwBitsPerSample, ptAudioFrameInfo->dwSampleRate);
            m_Mdia.m_Hdlr.SetParams(i_dwTrakHandlerType,(char *)"SoundHandler");
            m_Mdia.SetParams(i_dwTrakHandlerType);
            m_Tkhd.SetParams(FMP4_TKHD_FLAG_TRACK_ENABLE|FMP4_TKHD_FLAG_TRACK_IN_MOVIE,i_dwTrackID,0x0100,0,0);
        }
        else
        {
            FMP4_LOGW("FMP4TrakBox SetParams err %d\r\n",i_dwTrakHandlerType);
        }
        FMP4BaseBox::m_dwBoxSize += m_Tkhd.m_dwBoxSize;//必须放在ToBits前，因为调用ToBits前就要用
        FMP4BaseBox::m_dwBoxSize += m_Mdia.m_dwBoxSize;
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        
        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TrakBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        iLen += m_Tkhd.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Mdia.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        
        return iLen;
    };
	FMP4TkhdBox m_Tkhd;
	//FMP4EdtsBox edts;//sample_count < 1 则注释，即fmp4注释掉
	FMP4MdiaBox m_Mdia;
private:

};

/*****************************************************************************
-Class          : FMP4MvexBox ()
-Description    : Level 2,第二级，
                子box SetParams会修改m_dwBoxSize导致不能常驻、多次使用，
                子box SetParams只能调用一次，故放构造函数中，因此FMP4MoovBox用完就要释放
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MvexBox : public FMP4BaseBox
{
public:
	FMP4MvexBox() 
	{
        memcpy(FMP4BaseBox::m_acBoxType,"mvex",sizeof(FMP4BaseBox::m_acBoxType));
        memset(m_pTrex,0,sizeof(m_pTrex));
	};
	~FMP4MvexBox() 
	{
        for(int i = 0;i<m_dwTrexCnt;i++)
        {
            if(NULL != m_pTrex[i])
            {
                delete m_pTrex[i];
                m_pTrex[i] = NULL;
            }
        }
	};
	int SetParams(unsigned int i_dwTrackCount,unsigned int * i_pdwTrackId) 
	{
	    int i = 0;
	    
        FMP4_LOGW("FMP4MvexBox SetParams %d,%d\r\n",m_dwTrexCnt,i_dwTrackCount);
        if(i_dwTrackCount > FMP4_MAX_TRAK_NUM)
        {
            FMP4_LOGE("FMP4MvexBox SetParams err %d,%d\r\n",m_dwTrexCnt,i_dwTrackCount);
            return -1;
        }
        for(i = 0;i<m_dwTrexCnt;i++)
        {
            if(NULL != m_pTrex[i])
            {
                delete m_pTrex[i];
                m_pTrex[i] = NULL;
            }
        }
        m_dwTrexCnt = i_dwTrackCount;
        for(i = 0;i<m_dwTrexCnt;i++)
        {
            m_pTrex[i] = new FMP4TrexBox();
            m_pTrex[i]->SetParams(i_pdwTrackId[i]);
        }

        FMP4BaseBox::m_dwBoxSize += m_Mehd.m_dwBoxSize;//ffmpeg 无
        for(i = 0;i<m_dwTrexCnt;i++)
        {
            FMP4BaseBox::m_dwBoxSize += m_pTrex[i]->m_dwBoxSize;
        }
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        
        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TrakBox ToBits err %d,%d\r\n",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        iLen += m_Mehd.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);////ffmpeg 无
        for(i = 0;i<m_dwTrexCnt;i++)
        {
            iLen += m_pTrex[i]->ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        }
        
        return iLen;
    };
    
private:
	FMP4MehdBox m_Mehd;

	FMP4TrexBox * m_pTrex[FMP4_MAX_TRAK_NUM];
	unsigned int m_dwTrexCnt = 0;//track count
};

/*****************************************************************************
-Class          : FMP4UdtaBox (user data)
-Description    : Level 2,第二级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4UdtaBox : public FMP4BaseBox
{
public:
	FMP4UdtaBox() 
	{
        FMP4BaseBox::m_dwBoxSize += m_iLen;
        memcpy(FMP4BaseBox::m_acBoxType,"udta",sizeof(FMP4BaseBox::m_acBoxType));
	};
	virtual ~FMP4UdtaBox() 
	{
        if(NULL != m_pData)
        {
            delete [] m_pData;
        }
	};
	int SetParams(char * data,int len) 
	{
        if(NULL == data || len <= 0)
        {
            FMP4_LOGE("FMP4UdtaBox SetParams err %d,%d",m_iLen,len);
        }
        if(NULL != m_pData)
        {
            delete [] m_pData;
        }
        m_pData = new char[len];
        m_iLen = len;
        memcpy(m_pData,data,m_iLen);
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        FMP4BaseBox::m_dwBoxSize += m_iLen;
        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4UdtaBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        memcpy(o_pbBuf+iLen,m_pData,m_iLen);
        iLen+=m_iLen;
        
        return iLen;
    };
private:
	char * m_pData=NULL;
	int m_iLen = 0;
};

/*****************************************************************************
-Class          : FMP4MfhdBox (mfhd)
-Description    : Level 2,第二级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MfhdBox : public FMP4FullBaseBox
{
public:
	FMP4MfhdBox() 
	{
        FMP4BaseBox::m_dwBoxSize += sizeof(m_dwSequenceNumber);
        memcpy(FMP4BaseBox::m_acBoxType,"mfhd",sizeof(FMP4BaseBox::m_acBoxType));
	};
	int SetParams(unsigned int i_dwSeqNum) 
	{
        FMP4_LOGW("FMP4MfhdBox SetParams i_dwSeqNum %d,%d\r\n",i_dwSeqNum,m_dwSequenceNumber);
        m_dwSequenceNumber = i_dwSeqNum;
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4FullBaseBox::m_dwBoxSize;//
        int iLen = 0;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MfhdBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        iLen += FMP4FullBaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        Write32BE((o_pbBuf+iLen),m_dwSequenceNumber);
        iLen+=4;
        
        return iLen;
    };

private:
	unsigned int m_dwSequenceNumber = 1;// start from 1
};

/*****************************************************************************
-Class          : FMP4TrafBox (Track Fragment Box)
存在 tfhd、tfdt、sdtp、trun 的容器
-Description    : Level 2,第二级，
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4TrafBox :public FMP4BaseBox
{
public:
	FMP4TrafBox() 
	{
        memcpy(FMP4BaseBox::m_acBoxType,"traf",sizeof(FMP4BaseBox::m_acBoxType));
	};
	int SetParams(T_FMP4TrackInfo *i_ptTrack,uint64_t i_ddwMoofOffset=0) 
	{
        FMP4_LOGW("FMP4TrafBox SetParams %d,%lld, %d,%d \r\n",i_ptTrack->dwTrackId,i_ddwMoofOffset,i_ptTrack->dwHandlerType,i_ptTrack->dwSampleCount);
        unsigned int dwFlags;
        unsigned int dwDefaultSampleFlags;
        unsigned int dwDefaultSampleDuration;
        unsigned int dwDefaultSampleSize;
        
        if(NULL == i_ptTrack ||NULL == i_ptTrack->aptSampleInfo)
        {
            FMP4_LOGE("FMP4TrafBox SetParams err %p\r\n",i_ptTrack);
            return -1;
        }
        dwFlags = FMP4TfhdBox::FMP4_TFHD_FLAG_DEFAULT_FLAGS;
        dwFlags |= FMP4TfhdBox::FMP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX;
        // ISO/IEC 23009-1:2014(E) 6.3.4.2 General format type (p93)
        // The 'moof' boxes shall use movie-fragment relative addressing for media data that 
        // does not use external data references, the flag 'default-base-is-moof' shall be set, 
        // and data-offset shall be used, i.e. base-data-offset-present shall not be used.
        //因此注释掉dwFlags |=FMP4TfhdBox::FMP4_TFHD_FLAG_BASE_DATA_OFFSET
        dwFlags |=FMP4TfhdBox::FMP4_TFHD_FLAG_DEFAULT_BASE_IS_MOOF;

        dwDefaultSampleFlags = FMP4_AUDIO == i_ptTrack->dwHandlerType ? FMP4_TREX_FLAG_SAMPLE_DEPENDS_ON_I_PICTURE : 
        (FMP4_TREX_FLAG_SAMPLE_IS_NO_SYNC_SAMPLE| FMP4_TREX_FLAG_SAMPLE_DEPENDS_ON_NOT_I_PICTURE);//可能反了?
        if (i_ptTrack->dwSampleCount > 0)
        {
            dwFlags |= FMP4TfhdBox::FMP4_TFHD_FLAG_DEFAULT_DURATION | FMP4TfhdBox::FMP4_TFHD_FLAG_DEFAULT_SIZE;
            dwDefaultSampleDuration = i_ptTrack->aptSampleInfo[0].dwSampleDuration;
            dwDefaultSampleSize =i_ptTrack->aptSampleInfo[0].dwDataSize;
        }
        else
        {
            dwFlags |= FMP4TfhdBox::FMP4_TFHD_FLAG_DURATION_IS_EMPTY;
            dwDefaultSampleDuration = 0; // not set
            dwDefaultSampleSize = 0; // not set
        }
        //i_ddwMoofOffset是指moof box数据开始的物理位置，但是由于没设BASE_DATA_OFFSET标志所以可以传0值
        m_Tfhd.SetParams(dwFlags,i_ptTrack->dwTrackId, i_ddwMoofOffset,dwDefaultSampleDuration, dwDefaultSampleSize,dwDefaultSampleFlags);
        m_Tfdt.SetParams(i_ptTrack->aptSampleInfo[0].ddwDTS-i_ptTrack->dwStartDts);
        m_Trun.SetParams(dwDefaultSampleSize,dwDefaultSampleDuration,i_ptTrack->dwSampleCount,i_ptTrack->aptSampleInfo[0].ddwDataOffset,i_ptTrack->aptSampleInfo);

        FMP4BaseBox::m_dwBoxSize += m_Tfhd.m_dwBoxSize;
        FMP4BaseBox::m_dwBoxSize += m_Tfdt.m_dwBoxSize;
        FMP4BaseBox::m_dwBoxSize += m_Trun.m_dwBoxSize;
        return 0;
	};
	int ModifyParams(T_FMP4TrackInfo *i_ptTrack) 
	{
        if(NULL == i_ptTrack ||NULL == i_ptTrack->aptSampleInfo)
        {
            FMP4_LOGE("FMP4TrafBox ModifyParams err %p\r\n",i_ptTrack);
            return -1;
        }
        m_Trun.ModifyParams(i_ptTrack->aptSampleInfo[0].ddwDataOffset);
        return 0;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TrafBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        iLen += m_Tfhd.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Tfdt.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        iLen += m_Trun.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        
        return iLen;
    };
private:	
	FMP4TfhdBox m_Tfhd;
	FMP4TfdtBox m_Tfdt;
	FMP4TrunBox m_Trun;
};

/**********************************Level 1************************************/

/*****************************************************************************
-Class          : FMP4FtypBox (File Type Box)
-Description    : Level 1,第一级，最顶层box
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4FtypBox  : public FMP4BaseBox 
{
public:
    FMP4FtypBox() 
    {
        int i = 0;
        m_apCompatibleBrands[0] = FMP4_BRAND_ISOM;
        m_apCompatibleBrands[1] = FMP4_BRAND_ISO2;
        m_apCompatibleBrands[2] = FMP4_BRAND_AVC1;
        m_apCompatibleBrands[3] = FMP4_BRAND_ISO6;
        m_apCompatibleBrands[4] = FMP4_BRAND_MP42;//apple FMP4_BRAND_MP42;ffmpeg media FMP4_BRAND_MP41
        //m_apCompatibleBrands[5] = FMP4_BRAND_MSDH;//ffmpeg 无，media 有
        
        FMP4BaseBox::m_dwBoxSize += strlen(m_strMajorBrand)+sizeof(m_dwMinorVersion);
        for(i =0;i<sizeof(m_apCompatibleBrands)/sizeof(const char *);i++)
        {
            FMP4BaseBox::m_dwBoxSize += strlen(m_apCompatibleBrands[i]);
        }
        memcpy(FMP4BaseBox::m_acBoxType,"ftyp",sizeof(FMP4BaseBox::m_acBoxType));
    };

    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            return iLen;
        }
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        memcpy(o_pbBuf+iLen,m_strMajorBrand,strlen(m_strMajorBrand));
        iLen+=strlen(m_strMajorBrand);
        Write32BE((o_pbBuf+iLen),m_dwMinorVersion);
        iLen+=4;
        for(int i =0;i<sizeof(m_apCompatibleBrands)/sizeof(const char *);i++)
        {
            memcpy(o_pbBuf+iLen,m_apCompatibleBrands[i],strlen(m_apCompatibleBrands[i]));
            iLen+=strlen(m_apCompatibleBrands[i]);
        }

        return iLen;
    };
public:
	const char * m_strMajorBrand = FMP4_BRAND_MP42;//apple FMP4_BRAND_MP42;ffmpeg FMP4_BRAND_ISOM;media FMP4_BRAND_MSDH
	unsigned int m_dwMinorVersion = 0X00000200;
	const char * m_apCompatibleBrands[5] ;//ffmpeg 5;media 6 //"isom iso2 avc1 iso6 mp41";//兼容不放FMP4_BRAND_HVC1也行,ios用FMP4_BRAND_MP42 代替MP41
};


/*****************************************************************************
-Class          : FMP4MoovBox(Movie Box 该box包含了文件媒体的metadata信息)
                包含1个“mvhd”和若干个“trak”track表示一个视频或音频序列(通道)
-Description    : Level 1,第一级，最顶层box
                 子box SetParams会修改m_dwBoxSize导致不能常驻、多次使用，
                 子box SetParams只能调用一次，故放构造函数中，因此FMP4MoovBox用完就要释放
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MoovBox : public FMP4BaseBox 
{
public:
	FMP4MoovBox(unsigned int i_dwTrackCount,unsigned int * i_pdwTrakHandlerType,T_FMP4SampleInfo *ptVideoFrameInfo,T_FMP4SampleInfo *ptAudioFrameInfo) 
	{
        memcpy(FMP4BaseBox::m_acBoxType,"moov",sizeof(FMP4BaseBox::m_acBoxType));
        if(i_dwTrackCount > FMP4_MAX_TRAK_NUM)
        {
            FMP4_LOGE("FMP4MoovBox    err %d,%d",m_dwTrakCnt,i_dwTrackCount);
            return;
        }
        if(i_dwTrackCount >= 2 && (NULL == ptVideoFrameInfo ||NULL == ptAudioFrameInfo))
        {
            FMP4_LOGE("FMP4MoovBox    err NULL %d,%p,%p\r\n",i_dwTrackCount,ptVideoFrameInfo,ptAudioFrameInfo);
            return;
        }
        unsigned int adwTrackId[FMP4_MAX_TRAK_NUM];
        memset(m_pTrak,0,sizeof(m_pTrak));
        m_dwTrakCnt = i_dwTrackCount;
        for(int i = 0;i<m_dwTrakCnt;i++)
        {
            m_pTrak[i] = new FMP4TrakBox();
            m_pTrak[i]->SetParams(i_pdwTrakHandlerType[i],i+1,ptVideoFrameInfo,ptAudioFrameInfo);
            adwTrackId[i]=i+1;
        }
        m_MvhdBox.SetParams(i_dwTrackCount+1);
        m_Mvex.SetParams(i_dwTrackCount,adwTrackId);
	};
	~FMP4MoovBox() 
	{
        for(int i = 0;i<m_dwTrakCnt;i++)
        {
            if(NULL != m_pTrak[i])
            {
                delete m_pTrak[i];
                m_pTrak[i] = NULL;
            }
        }
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        FMP4BaseBox::m_dwBoxSize += m_MvhdBox.m_dwBoxSize;
        for(i = 0;i<m_dwTrakCnt;i++)
        {
            FMP4BaseBox::m_dwBoxSize += m_pTrak[i]->m_dwBoxSize;
        }
        FMP4BaseBox::m_dwBoxSize += m_Mvex.m_dwBoxSize;
        //FMP4BaseBox::m_dwBoxSize += m_Udta.m_dwBoxSize;//用户数据暂无
        
        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4TrakBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        if(0 == m_dwTrakCnt)
        {
            FMP4_LOGE("FMP4TrakBox ToBits err m_dwTrakCnt %d\r\n",m_dwTrakCnt);
            return iLen;
        }

        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        iLen += m_MvhdBox.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        for(i = 0;i<m_dwTrakCnt;i++)
        {
            iLen += m_pTrak[i]->ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        }
        iLen += m_Mvex.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        //iLen += m_Udta.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);//用户数据暂无
        
        return iLen;
    };
    
public:
	FMP4MvhdBox m_MvhdBox;
	
	unsigned int m_dwTrakCnt = 0;//track count
	FMP4TrakBox * m_pTrak[FMP4_MAX_TRAK_NUM];

	FMP4MvexBox m_Mvex;
	FMP4UdtaBox m_Udta;//用户数据暂无
};


/*****************************************************************************
-Class          : FMP4MoofBox
-Description    : Level 1,第一级，最顶层box
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MoofBox : public FMP4BaseBox 
{
public:
	FMP4MoofBox(unsigned int i_dwSeqNum,unsigned int i_dwTrackCnt,T_FMP4TrackInfo * i_aptTrackInfo) //浅拷贝
	{
    	int i = 0;
    	
        memcpy(FMP4BaseBox::m_acBoxType,"moof",sizeof(FMP4BaseBox::m_acBoxType));
        m_dwTrackCnt = i_dwTrackCnt;
        m_aptTrackInfo = i_aptTrackInfo;
        m_Mfhd.SetParams(i_dwSeqNum);
        for(i = 0;i<m_dwTrackCnt;i++)
        {
            m_aTraf[i].SetParams(&i_aptTrackInfo[i]);
        }
        FMP4BaseBox::m_dwBoxSize += m_Mfhd.m_dwBoxSize;
        for(i = 0;i<m_dwTrackCnt;i++)
        {
            FMP4BaseBox::m_dwBoxSize += m_aTraf[i].m_dwBoxSize;
        }
	};
	int ModifyParams(unsigned int i_dwTrackCnt,T_FMP4TrackInfo * i_aptTrackInfo) 
	{
	    int iRet = -1;
        for(int i = 0;i<i_dwTrackCnt;i++)
        {
            iRet = m_aTraf[i].ModifyParams(&i_aptTrackInfo[i]);
        }
        return iRet;
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i ;
        
        
        dwMaxLen = FMP4BaseBox::m_dwBoxSize;
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MoofBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        iLen += m_Mfhd.ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        for(i = 0;i<m_dwTrackCnt;i++)
        {
            iLen += m_aTraf[i].ToBits(o_pbBuf+iLen,i_dwMaxBufLen-iLen);
        }
        
        return iLen;
    };
    
public:
	FMP4MfhdBox m_Mfhd;
	FMP4TrafBox m_aTraf[FMP4_MAX_TRAK_NUM];
private:
    unsigned int m_dwTrackCnt = 0;//track count
    T_FMP4TrackInfo * m_aptTrackInfo;
};
/*****************************************************************************
-Class          : FMP4MdatBox
-Description    : Level 1,第一级，最顶层box
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/11/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class FMP4MdatBox : public FMP4BaseBox
{
public:
	FMP4MdatBox(unsigned int i_dwTrackCnt,T_FMP4TrackInfo * i_aptTrackInfo) //浅拷贝
	{
        m_dwTrackCnt = i_dwTrackCnt;
        m_aptTrackInfo = i_aptTrackInfo;
        memcpy(FMP4BaseBox::m_acBoxType,"mdat",sizeof(FMP4BaseBox::m_acBoxType));
	};
    int ToBits(unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
    {
        unsigned int dwMaxLen = FMP4BaseBox::m_dwBoxSize;//
        int iLen = 0;
        int i,j ;
        
        for(i = 0;i<m_dwTrackCnt;i++)
        {
            for(j = 0;j<m_aptTrackInfo[i].dwSampleCount;j++)
            {
                FMP4BaseBox::m_dwBoxSize += m_aptTrackInfo[i].aptSampleInfo[j].dwDataSize;
            }
        }
        
        if(NULL == o_pbBuf || i_dwMaxBufLen < dwMaxLen)
        {
            FMP4_LOGE("FMP4MdatBox ToBits err %d,%d",i_dwMaxBufLen,dwMaxLen);
            return iLen;
        }
        
        iLen += FMP4BaseBox::ToBits(o_pbBuf,i_dwMaxBufLen);
        
        for(i = 0;i<m_dwTrackCnt;i++)
        {
            for(j = 0;j<m_aptTrackInfo[i].dwSampleCount;j++)
            {
                memcpy(o_pbBuf + iLen, m_aptTrackInfo[i].aptSampleInfo[j].pbData, m_aptTrackInfo[i].aptSampleInfo[j].dwDataSize);
                iLen += m_aptTrackInfo[i].aptSampleInfo[j].dwDataSize;
            }
        }
        
        return iLen;
    };

private:
    unsigned int m_dwTrackCnt = 0;//track count
    T_FMP4TrackInfo * m_aptTrackInfo;
};

/*****************************************************************************
-Fuction        : FMP4
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
FMP4::FMP4()
{
    m_iCurTrakNum = 2;
    memset(m_adwTrakHandlerType,0,sizeof(m_adwTrakHandlerType));
    m_adwTrakHandlerType[0]=FMP4_VIDEO;
    m_adwTrakHandlerType[1]=FMP4_AUDIO;
    m_dwSegmentBaseDecTime = 0;
    m_iFindFirstFrame = 0;
}

/*****************************************************************************
-Fuction        : FMP4
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
FMP4::FMP4(E_Fmp4StreamType eFmp4StreamType)
{
    m_iCurTrakNum = 2;
    memset(m_adwTrakHandlerType,0,sizeof(m_adwTrakHandlerType));
    m_adwTrakHandlerType[0]=FMP4_VIDEO;
    m_adwTrakHandlerType[1]=FMP4_AUDIO;
    if(FMP4_STREAM_TYPE_VIDEO_STREAM == eFmp4StreamType)
    {
        m_iCurTrakNum = 1;
        m_adwTrakHandlerType[0]=FMP4_VIDEO;
    }
    else if(FMP4_STREAM_TYPE_AUDIO_STREAM == eFmp4StreamType)
    {
        m_iCurTrakNum = 1;
        m_adwTrakHandlerType[0]=FMP4_AUDIO;
    }
    m_dwSegmentBaseDecTime = 0;
    m_iFindFirstFrame = 0;
}

/*****************************************************************************
-Fuction        : FMP4
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
FMP4::~FMP4()
{

}


/*****************************************************************************
-Fuction        : CreateHeader
-Description    : 
-Input          : 
-Output         : 
-Return         : -1 err >0 len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/12/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4::CreateHeader(list<T_Fmp4FrameInfo> * i_pFMP4Media,unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    char* pcBuf = NULL;
    int i = 0;
    T_FMP4SampleInfo tVideoFrameInfo;
    T_FMP4SampleInfo tAudioFrameInfo;
    T_Fmp4FrameInfo *ptFrameInfo = NULL;
    
    if(NULL == i_pFMP4Media ||NULL == o_pbBuf)
    {
        FMP4_LOGE("CreateHeader err NULL\r\n");
        return iRet;
    }
    FMP4FtypBox FtypBox;
    
    memset(&tVideoFrameInfo,0,sizeof(T_FMP4SampleInfo));
    memset(&tAudioFrameInfo,0,sizeof(T_FMP4SampleInfo));
    m_iCurTrakNum = 0;
    for(list<T_Fmp4FrameInfo>::iterator iter = i_pFMP4Media->begin();iter != i_pFMP4Media->end();iter++)
    {
        if(FMP4_VIDEO_KEY_FRAME == iter->eFrameType)
        {
            m_adwTrakHandlerType[m_iCurTrakNum]=FMP4_VIDEO;
            m_iCurTrakNum++;
            tVideoFrameInfo.eFrameType = FMP4_VIDEO_KEY_FRAME;//其余暂不需要设置
            switch(iter->eEncType)
            {
                case FMP4_ENC_H264:
                {
                    tVideoFrameInfo.eEncType=FMP4_OBJECT_TYPE_H264;
                    break;
                }
                case FMP4_ENC_H265:
                {
                    tVideoFrameInfo.eEncType=FMP4_OBJECT_TYPE_H265;
                    break;
                }
                default:
                {
                    FMP4_LOGE("CreateHeader FMP4_VIDEO_FRAME err eEncType %d",iter->eEncType);
                    return iRet;
                }
            }
            tVideoFrameInfo.dwSampleRate= iter->ddwSampleRate;
            tVideoFrameInfo.tVideoEncParam.dwWidth= iter->tVideoEncParam.dwWidth;
            tVideoFrameInfo.tVideoEncParam.dwHeight= iter->tVideoEncParam.dwHeight;
            tVideoFrameInfo.iEncExtraDataLen= iter->iEncExtraDataLen;
            memcpy(tVideoFrameInfo.abEncExtraData,iter->abEncExtraData,tVideoFrameInfo.iEncExtraDataLen);
            break;
        }
    }
    for(list<T_Fmp4FrameInfo>::iterator iter = i_pFMP4Media->begin();iter != i_pFMP4Media->end();iter++)
    {
        if(FMP4_AUDIO_FRAME == iter->eFrameType)
        {
            m_adwTrakHandlerType[m_iCurTrakNum]=FMP4_AUDIO;
            m_iCurTrakNum++;
            tAudioFrameInfo.eFrameType = FMP4_AUDIO_FRAME;//其余暂不需要设置
            switch(iter->eEncType)
            {
                case FMP4_ENC_G711A:
                {
                    tAudioFrameInfo.eEncType=FMP4_OBJECT_TYPE_G711A;
                    break;
                }
                case FMP4_ENC_G711U:
                {
                    tAudioFrameInfo.eEncType=FMP4_OBJECT_TYPE_G711U;
                    break;
                }
                case FMP4_ENC_AAC:
                {
                    tAudioFrameInfo.eEncType=FMP4_OBJECT_TYPE_AAC;
                    break;
                }
                default:
                {
                    FMP4_LOGE("CreateHeader FMP4_AUDIO_FRAME err eEncType %d",iter->eEncType);
                    return iRet;
                }
            }
            tAudioFrameInfo.dwSampleRate= iter->ddwSampleRate;
            tAudioFrameInfo.tAudioEncParam.dwChannels= iter->tAudioEncParam.dwChannels;
            tAudioFrameInfo.tAudioEncParam.dwBitsPerSample= iter->tAudioEncParam.dwBitsPerSample;
            tAudioFrameInfo.iEncExtraDataLen= iter->iEncExtraDataLen;
            memcpy(tAudioFrameInfo.abEncExtraData,iter->abEncExtraData,tAudioFrameInfo.iEncExtraDataLen);
            break;
        }
    }
    if(m_iCurTrakNum<1)
    {
        FMP4_LOGE("CreateHeader err m_iCurTrakNum\r\n");
        return iRet;
    }
    FMP4MoovBox MoovBox(m_iCurTrakNum,m_adwTrakHandlerType,&tVideoFrameInfo,&tAudioFrameInfo);

    iDataLen = FtypBox.ToBits(o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
    iDataLen+=MoovBox.ToBits(o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
    
    return iDataLen;
}



/*****************************************************************************
-Fuction        : CreateSegment
-Description    : 为了计算持续时间，i_pFMP4Media中最后一帧不打包，
-Input          : 
-Output         : 
-Return         : -1 err >0 len
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int FMP4::CreateSegment(list<T_Fmp4FrameInfo> * i_pFMP4Media,unsigned int i_dwSeqNum,unsigned char *o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    T_FMP4TrackInfo atTrackInfo[FMP4_MAX_TRAK_NUM];
    unsigned int dwTrackCnt=0;
    int i = 0,j=0;
    T_FMP4SampleInfo * aptVideoSampleInfo=NULL;
    T_FMP4SampleInfo * aptAudioSampleInfo=NULL;
    unsigned int dwVideoSampleCount=0;
    unsigned int dwAudioSampleCount=0;
    list<T_Fmp4FrameInfo>::iterator iter;
    uint64_t ddwLastVideoTimeStamp=0;
    uint64_t ddwLastAudioTimeStamp=0;
    uint64_t ddwMoofBoxStartOffset=0;//moof box数据开始的偏移位置
    uint64_t ddwDataOffset=0;//表示mdat box里的数据相对与moof开始(BoxSize字节开始)的偏移量
    
    if(NULL == i_pFMP4Media ||NULL == o_pbBuf ||i_pFMP4Media->size()<1)
    {
        FMP4_LOGE("CreateSegment err NULL\r\n");
        return iRet;
    }
    if(m_iCurTrakNum<1)
    {
        FMP4_LOGE("CreateSegment err m_iCurTrakNum\r\n");
        return iRet;
    }
    dwTrackCnt=0;
    memset(atTrackInfo,0,sizeof(atTrackInfo));
    for(i=0;i<m_iCurTrakNum&&dwTrackCnt<FMP4_MAX_TRAK_NUM;i++)
    {
        atTrackInfo[dwTrackCnt].dwHandlerType=m_adwTrakHandlerType[i];
        atTrackInfo[dwTrackCnt].dwTrackId=i+1;
        dwTrackCnt++;
    }
    dwVideoSampleCount=0;
    dwAudioSampleCount=0;
    for(list<T_Fmp4FrameInfo>::iterator iter = i_pFMP4Media->begin();iter != i_pFMP4Media->end();iter++)
    {
        if(FMP4_VIDEO_KEY_FRAME == iter->eFrameType ||FMP4_VIDEO_INNER_FRAME == iter->eFrameType)
        {
            dwVideoSampleCount++;
        }
        else if(FMP4_AUDIO_FRAME == iter->eFrameType)
        {
            dwAudioSampleCount++;
        }
    }
    aptVideoSampleInfo = (T_FMP4SampleInfo *)malloc(dwVideoSampleCount*sizeof(T_FMP4SampleInfo));
    if(NULL == aptVideoSampleInfo)
    {
        FMP4_LOGE("CreateSegment malloc err NULL\r\n");
        return iRet;
    }
    memset(aptVideoSampleInfo,0,dwVideoSampleCount*sizeof(T_FMP4SampleInfo));
    ddwLastVideoTimeStamp=0;
    for(iter = i_pFMP4Media->begin(),j=0;iter != i_pFMP4Media->end()&&j<(int)dwVideoSampleCount;iter++)
    {
        if(FMP4_VIDEO_KEY_FRAME == iter->eFrameType ||FMP4_VIDEO_INNER_FRAME == iter->eFrameType)
        {
            aptVideoSampleInfo[j].eFrameType = iter->eFrameType;
            aptVideoSampleInfo[j].ddwPTS = iter->ddwTimeStamp;//只有p帧
            aptVideoSampleInfo[j].ddwDTS = iter->ddwTimeStamp;
            aptVideoSampleInfo[j].pbData = iter->pbFrameStartPos;
            aptVideoSampleInfo[j].dwDataSize = iter->iFrameLen;
            aptVideoSampleInfo[j].ddwDataOffset = 0;//后续设置
            if(j-1>=0)
                aptVideoSampleInfo[j-1].dwSampleDuration=(unsigned int)(iter->ddwTimeStamp-ddwLastVideoTimeStamp);
            ddwLastVideoTimeStamp=iter->ddwTimeStamp;
            //aptVideoSampleInfo[j].dwSampleDuration = aptVideoSampleInfo[j-1].dwSampleDuration;
            switch(iter->eEncType)
            {
                case FMP4_ENC_H264:
                {
                    aptVideoSampleInfo[j].eEncType = FMP4_OBJECT_TYPE_H264;
                    break;
                }
                case FMP4_ENC_H265:
                {
                    aptVideoSampleInfo[j].eEncType = FMP4_OBJECT_TYPE_H265;
                    break;
                }
                default:
                {
                    FMP4_LOGE("CreateSegment FMP4_VIDEO_FRAME err eEncType %d\r\n",iter->eEncType);
                    free(aptVideoSampleInfo);
                    return iRet;
                }
            }
            aptVideoSampleInfo[j].dwSampleRate= iter->ddwSampleRate;
            aptVideoSampleInfo[j].tVideoEncParam.dwWidth= iter->tVideoEncParam.dwWidth;
            aptVideoSampleInfo[j].tVideoEncParam.dwHeight= iter->tVideoEncParam.dwHeight;
            aptVideoSampleInfo[j].iEncExtraDataLen= iter->iEncExtraDataLen;
            memcpy(aptVideoSampleInfo[j].abEncExtraData,iter->abEncExtraData,aptVideoSampleInfo[j].iEncExtraDataLen);
            j++;
        }
    }
    aptAudioSampleInfo = (T_FMP4SampleInfo *)malloc(dwAudioSampleCount*sizeof(T_FMP4SampleInfo));
    if(NULL == aptAudioSampleInfo)
    {
        FMP4_LOGE("CreateSegment malloc err NULL\r\n");
        free(aptVideoSampleInfo);
        return iRet;
    }
    memset(aptAudioSampleInfo,0,dwAudioSampleCount*sizeof(T_FMP4SampleInfo));
    ddwLastAudioTimeStamp=0;
    //for(i=0,j=0;i<(int)i_pFMP4Media->size()&&j<(int)dwAudioSampleCount;i++)
    for(iter = i_pFMP4Media->begin(),j=0;iter != i_pFMP4Media->end()&&j<(int)dwAudioSampleCount;iter++)
    {
        if(FMP4_AUDIO_FRAME == iter->eFrameType)
        {
            //ptFrameInfo = &i_pFMP4Media[i];
            aptAudioSampleInfo[j].eFrameType = iter->eFrameType;
            aptAudioSampleInfo[j].ddwPTS = iter->ddwTimeStamp;//只有p帧
            aptAudioSampleInfo[j].ddwDTS = iter->ddwTimeStamp;
            aptAudioSampleInfo[j].pbData = iter->pbFrameStartPos;
            aptAudioSampleInfo[j].dwDataSize = iter->iFrameLen;
            aptAudioSampleInfo[j].ddwDataOffset = 0;//后续设置
            if(j-1>=0)
            {
                aptAudioSampleInfo[j-1].dwSampleDuration=(unsigned int)(iter->ddwTimeStamp-ddwLastAudioTimeStamp);
                aptAudioSampleInfo[j].dwSampleDuration = aptAudioSampleInfo[j-1].dwSampleDuration;//音频可用上一帧的
            }
            ddwLastAudioTimeStamp=iter->ddwTimeStamp;
            switch(iter->eEncType)
            {
                case FMP4_ENC_G711A:
                {
                    aptAudioSampleInfo[j].eEncType = FMP4_OBJECT_TYPE_G711A;
                    break;
                }
                case FMP4_ENC_G711U:
                {
                    aptAudioSampleInfo[j].eEncType = FMP4_OBJECT_TYPE_G711U;
                    break;
                }
                case FMP4_ENC_AAC:
                {
                    aptAudioSampleInfo[j].eEncType=FMP4_OBJECT_TYPE_AAC;
                    break;
                }
                default:
                {
                    FMP4_LOGE("CreateSegment FMP4_AUDIO_FRAME err eEncType %d\r\n",iter->eEncType);
                    free(aptVideoSampleInfo);
                    free(aptAudioSampleInfo);
                    return iRet;
                }
            }
            aptAudioSampleInfo[j].dwSampleRate= iter->ddwSampleRate;
            aptAudioSampleInfo[j].tAudioEncParam.dwChannels= iter->tAudioEncParam.dwChannels;
            aptAudioSampleInfo[j].tAudioEncParam.dwBitsPerSample= iter->tAudioEncParam.dwBitsPerSample;
            aptAudioSampleInfo[j].iEncExtraDataLen= iter->iEncExtraDataLen;
            memcpy(aptAudioSampleInfo[j].abEncExtraData,iter->abEncExtraData,aptAudioSampleInfo[j].iEncExtraDataLen);
            j++;
        }
    }
    
    for(i=0;i<dwTrackCnt;i++)
    {
        if(0 == i)
        {
            atTrackInfo[i].aptSampleInfo=aptVideoSampleInfo;
            atTrackInfo[i].dwSampleCount=dwVideoSampleCount-1;//去掉当前帧，外层会保存，下次再打包
            if(0 == m_iFindFirstFrame)
            {
                m_dwSegmentBaseDecTime = atTrackInfo[i].aptSampleInfo[0].ddwDTS;//时间戳开始，便于内部使用相对时间戳
                m_iFindFirstFrame = 1;
            }
            atTrackInfo[i].dwStartDts=m_dwSegmentBaseDecTime;
        }
        else if(1 == i)
        {
            atTrackInfo[i].aptSampleInfo= aptAudioSampleInfo;
            atTrackInfo[i].dwSampleCount= dwAudioSampleCount;
            if(0 == m_iFindFirstFrame)
            {
                FMP4_LOGE("0 == m_iFindFirstFrame err %d\r\n",m_dwSegmentBaseDecTime);
            }
            atTrackInfo[i].dwStartDts=m_dwSegmentBaseDecTime;//音视频时钟同源
        }
    }

    
    FMP4MoofBox MoofBox(i_dwSeqNum,dwTrackCnt,atTrackInfo);
    FMP4MdatBox MdatBox(dwTrackCnt,atTrackInfo);
    ddwMoofBoxStartOffset=0;//表示mdat box里的数据相对于moof开始(BoxSize字节的位置)的偏移量
    ddwDataOffset=MoofBox.m_dwBoxSize+MdatBox.m_dwBoxSize-0;//moof box size+mdat header-0(ddwMoofBoxStartOffset)
    for(i=0;i<(int)dwTrackCnt;i++)
    {
        for(j=0;j<(int)atTrackInfo[i].dwSampleCount;j++)
        {
            atTrackInfo[i].aptSampleInfo[j].ddwDataOffset = ddwDataOffset;//如果是连续的内存，则只用设置第一个帧的数据地址
            ddwDataOffset+=atTrackInfo[i].aptSampleInfo[j].dwDataSize;
        }
    }
    MoofBox.ModifyParams(dwTrackCnt,atTrackInfo);
    iDataLen=0;
    iDataLen = MoofBox.ToBits(o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
    iDataLen+=MdatBox.ToBits(o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
    
    free(aptVideoSampleInfo);
    free(aptAudioSampleInfo);
    iRet = iDataLen;
    return iRet;
}














