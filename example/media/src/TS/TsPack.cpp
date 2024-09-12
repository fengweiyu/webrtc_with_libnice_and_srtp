/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	TsPack.cpp
* Description		: 	TsPack operation center
ts�ļ���Ϊ���㣺ts�㣨Transport Stream����pes�㣨Packet Elemental Stream����es�㣨Elementary Stream����
es���������Ƶ���ݣ�pes����������Ƶ�����ϼ���ʱ����ȶ�����֡��˵����Ϣ��
ts������pes���ϼ�����������ʶ��ʹ���ı�Ҫ��Ϣ��
ts���Ϊ�������֣�ts header��adaptation field��payload
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "MediaAdapter.h"
#include "TsPack.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>


using std::cout;//��Ҫ<iostream>
using std::endl;



#define PES_BUF_MAX_LEN (1024*1024)
#define TS_MEDIA_BUF_MAX_LEN (3*1024*1024)










#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define Write16BE(p,value) \
do{ \
    p[0] = (unsigned char)((value >> 8) & 0xFF);  \
    p[1] = (unsigned char)((value) & 0xFF);    \
}while(0)

#define Write32BE(p,value) \
do{ \
    p[0] = (unsigned char)((value >> 24) & 0xFF); \
    p[1] = (unsigned char)((value >> 16) & 0xFF); \
    p[2] = (unsigned char)((value >> 8) & 0xFF);  \
    p[3] = (unsigned char)((value) & 0xFF);    \
}while(0)


#define	TS_PACKET_SIZE				188
#define	TS_PACKET_HEADER_SIZE		4
#define	TS_SYNC_BYTE				0x47


#define	TS_PAT_PID					0x000//�淶����
#define	TS_PMT_PID					0x20 //�Լ�����
#define	TS_VIDEO_PID				0x21//�Լ�����
#define	TS_AUDIO_PID              0x101//�Լ�����

#define	PMT_STREAM_TYPE_H264	0x1B//VIDEO h.264�����Ӧ0x1b
#define	PMT_STREAM_TYPE_H265	0x24
#define	PMT_STREAM_TYPE_AAC	0x0F
#define	PMT_STREAM_TYPE_MP3	0x03//h.264�����Ӧ0x1b��aac�����Ӧ0x0f��mp3�����Ӧ0x03


#define	TS_VIDEO_STREAM_ID			0xE0 //�淶����
#define	TS_AUDIO_STREAM_ID			0xC0 //�淶����





//ts���Ϊ�������֣�ts header��adaptation field��payload��ts header�̶�4���ֽڣ�adaptation field���ܴ���Ҳ���ܲ����ڣ���Ҫ�����Ǹ�����188�ֽڵ���������䣻payload��pes����
//TS ��ͷ
typedef struct TsHeader
{
	unsigned char bSyncByte;         //ͬ���ֽ�, �̶�Ϊ0x47,��ʾ�������һ��TS����,ͬ���ֽڣ��̶�Ϊ0x47
	unsigned char TransportErrorIndicator : 1;  //��������ָʾ��   ,�������ָʾ����������tsͷ��adapt�����һ�������ֽڣ�ͨ����Ϊ0������ֽ�����adapt�򳤶���
	unsigned char PayloadUnitStartIndicator : 1; //��Ч���ص�Ԫ��ʼָʾ��,���ص�Ԫ��ʼ��ʾ����һ�����������ݰ���ʼʱ���Ϊ1(ֵΪ1��pes���ʾ�ְ���ʼ��psi���ʾ������ʼ�ֽڻ���1�������ֽ�point_field)
	unsigned char TransportPriority : 1; //��������, 1��ʾ�����ȼ�,������ƿ����õ��������ò���,�������ȼ���0Ϊ�����ȼ���1Ϊ�����ȼ���ͨ��ȡ0
	unsigned short PID : 13;        //PID,pidֵ(Packet ID���룬Ψһ�ĺ����Ӧ��ͬ�İ�)
	unsigned char TransportScramblingControl : 2;         //������ſ���,������ſ��ƣ�00��ʾδ����
	unsigned char AdaptationFieldControl : 2;  //����Ӧ���� 01������Ч���أ�10���������ֶΣ�11���е����ֶκ���Ч���أ��ȵ����ֶ�Ȼ����Ч���ء�Ϊ00�����������д��� ,�Ƿ��������Ӧ������00����������01��Ϊ������Ӧ�򣬽�����Ч���أ���10��Ϊ��������Ӧ������Ч���أ���11��Ϊͬʱ��������Ӧ�����Ч���ء�
	unsigned char ContinuityCounter : 4; //���������� һ��4bit�ļ���������Χ0-15 ,��������������0-f����ʼֵ��һ��ȡ0����������������
}T_TsHeader;

//����Ӧ�α�־			
typedef struct TsAdaptationField
{
	unsigned char bAdaptationFieldLength;                 	//����Ӧ�γ���,����Ӧ�򳤶ȣ�������ֽ���(��������ǰ��һ�ֽ�)
    //flag ȡ0x50��ʾ����PCR��0x40��ʾ������PCR
	unsigned char DiscontinutyIndicator : 1;       		//������ǰ����������Ĳ�����״̬Ϊ��
	unsigned char RandomAccessIndicator : 1;      		//������һ������ͬPID��PES����Ӧ�ú���PTS�ֶκ�һ��ԭʼ�����ʵ�
	unsigned char ElementaryStreamPriorityIndicator : 1;  //���ȼ�
	unsigned char FlagPCR : 1;						//����pcr�ֶ�
	unsigned char FlagOPCR : 1;                    //����opcr�ֶ�
	unsigned char SplicingPointFlag : 1;				//ƴ�ӵ��־       
	unsigned char TransportPrivateDataFlag : 1;		//˽���ֽ�
	unsigned char AdaptationFieldExtensionFlag : 1;	//�����ֶ�����չ

	unsigned long long ddwPCR;  //5B(ʵ�ʿ���ռ6�ֽ�) ����Ӧ�����õ��ĵ�pcr,5B Program Clock Reference����Ŀʱ�Ӳο������ڻָ���������һ�µ�ϵͳʱ��ʱ��STC��System Time Clock����

	unsigned long long ddwOPCR;  //����Ӧ�����õ��ĵ�opcr
	unsigned char bSpliceCountdown;
	
	//unsigned char private_data_len;//stuffing_bytes ����ֽڣ�xB ȡֵ0xff
	//unsigned char private_data[256];
}T_TsAdaptationField;



//PAT�ṹ�壺��Ŀ��ر�
typedef struct TsPAT
{
	unsigned char bTableID;                  			//�̶�Ϊ0x00 ����־�Ǹñ���PAT
	unsigned char SectionSyntaxIndicator : 1;   	//���﷨��־λ���̶�Ϊ1
	unsigned char Zero : 1;                      			//0 
	unsigned char Reserved1 : 2;                		//����λ,�̶�Ϊ11
	unsigned short SectionLength : 12;           		//��ʾ����ֽں������ݵĳ��ȣ�����CRC32
	unsigned short wTransportStreamID;      	//�ô�������ID��������һ��������������·���õ���,������ID���̶�Ϊ0x0001
	unsigned char Reserved2 : 2;                		//����λ,�̶�Ϊ11
	unsigned char VersionNumber : 5;           		//��Χ0-31����ʾPAT�İ汾��,�汾�ţ��̶�Ϊ00000�����PAT�б仯��汾�ż�1
	unsigned char CurrentNextIndicator : 1;   	//���͵�PAT�ǵ�ǰ��Ч������һ��PAT��Ч,�̶�Ϊ1����ʾ���PAT������ã����Ϊ0��Ҫ�ȴ���һ��PAT��
	unsigned char bSectionNumber;          		//�ֶεĺ��롣PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�
	unsigned char bLastSectionNumber;      	//���һ���ֶεĺ���
	//�����濪ʼʵ������һ���ظ�ѭ���Ľṹ
	//ֻ���ڵ�ǰ��Ӧ���У�ֻ��һ����Ŀ//�������������һ���򻯴���
	unsigned short wProgramNumber;          	//��Ŀ��,��Ŀ��Ϊ0x0000ʱ��ʾ����NIT����Ŀ��Ϊ0x0001ʱ,��ʾ����PMT
	unsigned char Reserved3 : 3;             		//����λ,�̶�Ϊ111
	                        //������Ϣ��NIT����PID,��Ŀ��Ϊ0ʱ��Ӧ��PIDΪnetwork_PID,�����в����� networke_pid
	unsigned int PID : 13;           	//��Ŀӳ����PID����Ŀ�Ŵ���0ʱ��Ӧ��PID��ÿ����Ŀ��Ӧһ��;��Ŀ�Ŷ�Ӧ���ݵ�PIDֵ
    //����ѭ��
    
	unsigned int dwCRC;             		//CRC32У����,ǰ�����ݵ�CRC32У����
}T_TsPAT;

//PMT�ṹ�壺��Ŀӳ���
typedef struct TsPMT
{
	unsigned char bTableID;                 			//�̶�Ϊ0x02, ��ʾPMT��
	unsigned char SectionSyntaxIndicator : 1;  	//�̶�Ϊ0x01
	unsigned char Zero : 1;                       			//0x00
	unsigned char Reserved1 : 2;                		//�̶�Ϊ11
	unsigned short SectionLength : 12;             		//������λbit��Ϊ00����ָʾ�ε�byte�����ɶγ�����ʼ������CRC���������ݵĳ���

	unsigned short wProgramNumber;            	// ָ���ý�Ŀ��Ӧ�ڿ�Ӧ�õ�Program map PID,Ƶ�����룬��ʾ��ǰ��PMT��������Ƶ����ȡֵ0x0001
	unsigned char Reserved2 : 2;                 		//0x03 �̶�Ϊ11
	unsigned char VersionNumber : 5;             		//ָ��TS����Program map section�İ汾��,�汾�ţ��̶�Ϊ00000�����PAT�б仯��汾�ż�1
	unsigned char CurrentNextIndicator : 1;     	//����λ��1ʱ����ǰ���͵�Program map section���ã�����λ��0ʱ��ָʾ��ǰ���͵�Program map section�����ã���һ��TS����Program map section��Ч��
	unsigned char bSectionNumber;            		//�̶�Ϊ0x00
	unsigned char bLastSectionNumber;       	//�̶�Ϊ0x00
	unsigned char Reserved3 : 3;                		//0x07,�̶�Ϊ111
	unsigned short PCR_PID : 13;    //ָ��TS����PIDֵ����TS������PCR�򣬸�PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ���������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��(PCR(��Ŀ�ο�ʱ��)����TS�����PID��ָ��Ϊ��ƵPID,(PIDֵ(PCR_PID��ֵ)��Ӧ��TS������PCR��))
	unsigned char Reserved4 : 4;                		//Ԥ��Ϊ0x0F,�̶�Ϊ1111
	unsigned short ProgramInfoLength : 12;       	//ǰ��λbitΪ00������ָ���������Խ�Ŀ��Ϣ��������byte����(��Ŀ������Ϣ��ָ��Ϊ0x000��ʾû��)

	//�����濪ʼʵ������һ���ظ�ѭ���Ľṹ
	//�����������ʹ������·������Ϣ��//����ȫ��ö�ٳ�����
	//����һ
	unsigned char bStreamTypeVideo; //ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��(�����ͣ���־��Video����Audio�����������ݣ�h.264�����Ӧ0x1b��aac�����Ӧ0x0f��mp3�����Ӧ0x03)
	unsigned char Reserved5Video : 3;           	//0x07 ,�̶�Ϊ111
	unsigned short ElementaryPidVideo : 13;       	//����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��(��stream_type��Ӧ��PID)
	unsigned char Reserved6Video : 4;          	//0x0F,�̶�Ϊ1111
	unsigned short EsInfoLengthVideo : 12;      	//ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��(������Ϣ��ָ��Ϊ0x000��ʾû��)
	//������	
	unsigned char bStreamTypeAudio;//ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��(�����ͣ���־��Video����Audio�����������ݣ�h.264�����Ӧ0x1b��aac�����Ӧ0x0f��mp3�����Ӧ0x03)
	unsigned char Reserved5Audio : 3;           	//0x07,�̶�Ϊ111
	unsigned short ElementaryPidAudio : 13;       	//����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��(��stream_type��Ӧ��PID)
	unsigned char Reserved6Audio : 4;          	//0x0F,�̶�Ϊ1111
	unsigned short EsInfoLengthAudio : 12;      	//ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��(������Ϣ��ָ��Ϊ0x000��ʾû��)
    //����ѭ��

	unsigned int dwCRC;                    	//CRC32У����,ǰ�����ݵ�CRC32У����
}T_TsPMT;




//PES���ṹ�壬����PES��ͷ, 19 ���ֽ�
typedef struct PesHeader
{
    unsigned char   abStartCode[3];	//��ʼ��0x000001
    unsigned char   bStreamId;                		//�����������ͺͱ��
    unsigned short  wPesPacketLen;      	//������,����֡���ݵĳ��ȣ�����Ϊ0,Ҫ�Լ���,���16λ�������������Ҫ�Լ���
    unsigned char   MarkerBit : 2;                 		//�����ǣ�'10'
    unsigned char   ScramblingControl : 2;		//pes����Ч�غɵļ��ŷ�ʽ
    unsigned char   Priority : 1;				//��Ч���ص����ȼ�
    unsigned char   DataAlignmentIndicator : 1;   	//�������Ϊ1����PES����ͷ�����������Ƶ����Ƶsyncword��ʼ�Ĵ���(��Ƶ�﷨��Ԫ����Ƶͬ���� 00 00 00 01)��
    unsigned char   Copyright : 1;                  		// 1:����Ȩ������0������
    unsigned char   OriginalOrCopy : 1;          		// 1;��Ч������ԭʼ�ģ�0����Ч����ʱ������
    unsigned char   PtsDtsFlags : 2;              		//'10'��PTS�ֶδ��ڣ���11����PTD��DTS�����ڣ���00������û�У���01�������á�
    unsigned char   EscrFlag : 1;                  		// 1:escr��׼�ֶ� �� escr��չ�ֶξ����ڣ�0�����κ�escr�ֶδ���
    unsigned char   EsRateFlag : 1;               		// 1:es_rate�ֶδ��ڣ�0 ��������
    unsigned char   DsmTrickModeFlag : 1;        	// 1;8�����ؽӷ�ʽ�ֶδ��ڣ�0 ��������
    unsigned char   AdditionalCopyInfoFlag : 1;  	// 1:additional_copy_info���ڣ�0: ������
    unsigned char   PesCrcFlag : 1;               		// 1:crc�ֶδ��ڣ�0��������
    unsigned char   PesExtensionFlag : 1;         	// 1:��չ�ֶδ��ڣ�0:������
    unsigned char   bPesHeaderDataLength;    	//�������ݵĳ��ȣ�
    unsigned char   abPTS[5];					//ptsdts��10���ֽ�
    unsigned char   abDTS[5];					//
}T_PesHeader;


/*****************************************************************************
-Fuction		: TsPack
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TsPack::TsPack()
{
    m_bContinuityCounterPAT=0;
    m_bContinuityCounterPMT=0;
    m_bContinuityCounterVideo=0;
    m_bContinuityCounterAudio=0;
    m_pbBufPES=NULL;
    m_pbMediaData = new unsigned char[TS_MEDIA_BUF_MAX_LEN];
    m_MediaList.clear();
    m_iCurMediaDataLen = 0;
    m_iFindedKeyFrame = 0;

    m_ddwSegmentPTS=0;
    m_ddwSegmentDuration=0;
    
}
/*****************************************************************************
-Fuction		: ~TsPack
-Description	: ~TsPack
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TsPack::~TsPack()
{
    if(NULL!= m_pbBufPES)
    {
        delete[] m_pbBufPES;
    }
    if(NULL!= m_pbMediaData)
    {
        delete[] m_pbMediaData;
        m_iCurMediaDataLen = 0;
    }
    if(m_MediaList.size()>0)
    {
        DelAllFrame();
    }
}


/*****************************************************************************
-Fuction        : GetMuxData
-Description    : 
-Input          : 
-Output         : 
-Return         : <0 err,0 need more data,>0 datalen
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::GetMuxData(T_MediaFrameInfo * i_ptFrameInfo,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen,int i_iForcePack)
{
    int iRet = -1;
    int iDataLen = 0;
    int64_t m_ddwSegmentMinPTS=0;
    int64_t m_ddwSegmentMaxPTS=0;
    list<T_MediaFrameInfo>::iterator iter;
    T_MediaFrameInfo * ptFrameInfo = NULL;
    int iVideoStreamType = 0;
    int iAudioStreamType = 0;


    if(NULL == i_ptFrameInfo ||NULL == i_ptFrameInfo->pbFrameStartPos ||NULL == o_pbBuf)
    {
        MH_LOGE("GetMuxData err NULL\r\n");
        return iRet;
    }
    
    if(m_iFindedKeyFrame==0&&i_ptFrameInfo->eFrameType!=MEDIA_FRAME_TYPE_VIDEO_I_FRAME)
    {
        MH_LOGW("Skip frame:%d\r\n",i_ptFrameInfo->eFrameType);//�ڲ������ʼʱ��ʹ�õ�һ��i֡Ϊ�ο���׼ʱ��
        return 0;//���Ե�һ֡����i֡������֡Ҫ���˵�
    }
    if(m_iFindedKeyFrame!=0 && m_MediaList.size()>1 && (i_ptFrameInfo->eFrameType==MEDIA_FRAME_TYPE_VIDEO_I_FRAME ||0 != i_iForcePack))
    {
        for (iter = m_MediaList.begin(); iter != m_MediaList.end(); ++iter)
        {
            ptFrameInfo=(T_MediaFrameInfo *)&(*iter);
            if (ptFrameInfo->eEncType == MEDIA_ENCODE_TYPE_H264)
            {
                iVideoStreamType=PMT_STREAM_TYPE_H264;
            }
            else if (ptFrameInfo->eEncType == MEDIA_ENCODE_TYPE_H265)
            {
                iVideoStreamType=PMT_STREAM_TYPE_H265;
            }
            else if (ptFrameInfo->eEncType == MEDIA_ENCODE_TYPE_AAC)
            {
                iAudioStreamType=PMT_STREAM_TYPE_AAC;
            }
            else if (ptFrameInfo->eEncType == MEDIA_ENCODE_TYPE_MP3)
            {
                iAudioStreamType=PMT_STREAM_TYPE_MP3;
            }
        }
        if(0 == iVideoStreamType && 0 == iAudioStreamType)
        {
            MH_LOGE("0 == iVideoStreamType && 0 ==iAudioStreamType %d err\r\n",i_ptFrameInfo->eEncType);
            return iRet;
        }
        for (iter = m_MediaList.begin(); iter != m_MediaList.end(); ++iter)
        {
            ptFrameInfo=(T_MediaFrameInfo *)&(*iter);
            //����ǰ֡����TS  ����װ
            iRet=FrameToTS(ptFrameInfo,iVideoStreamType,iAudioStreamType,0==iAudioStreamType?0:1,o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
            if(iRet <= 0)
            {
                MH_LOGE("FrameToTS err\r\n");
                return iRet;
            }
            iDataLen+=iRet;
        }

        m_ddwSegmentPTS=0;
        m_ddwSegmentDuration=0;
        iter = m_MediaList.begin();//front
        m_ddwSegmentMinPTS=(int64_t)iter->dwTimeStamp;
        m_ddwSegmentMaxPTS=(int64_t)i_ptFrameInfo->dwTimeStamp;
        m_ddwSegmentPTS=m_ddwSegmentMinPTS;
        if(m_ddwSegmentMaxPTS-m_ddwSegmentMinPTS>0)
            m_ddwSegmentDuration=(m_ddwSegmentMaxPTS-m_ddwSegmentMinPTS);
        DelAllFrame();
    }
    SaveFrame(i_ptFrameInfo);
    if(i_ptFrameInfo->eFrameType==MEDIA_FRAME_TYPE_VIDEO_I_FRAME)
    {
        m_iFindedKeyFrame=1;
    }
    return iDataLen;
}


/*****************************************************************************
-Fuction        : FMP4Handle
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::GetDurationAndPTS(int64_t *o_ddwSegmentDuration,int64_t *o_ddwSegmentPTS)
{
    int iRet = -1;
    
    if(NULL == o_ddwSegmentDuration ||NULL == o_ddwSegmentPTS)
    {
        MH_LOGE("GetDurationAndPTS err NULL\r\n");
        return iRet;
    }
    *o_ddwSegmentDuration=m_ddwSegmentDuration;
    *o_ddwSegmentPTS=m_ddwSegmentPTS;

    return 0;
}



/*****************************************************************************
-Fuction        : SaveFrame
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::SaveFrame(T_MediaFrameInfo * i_ptFrameInfo)
{
    int iRet = -1;
    int i = 0;
    T_MediaFrameInfo tFrameInfo;
    unsigned char* pbVideoData = NULL;
    unsigned char *pbNaluBuf=NULL;
    int iNaluLen=0;
    
    if(NULL == i_ptFrameInfo ||NULL == i_ptFrameInfo->pbFrameStartPos ||i_ptFrameInfo->iFrameLen <= 0)
    {
        MH_LOGE("SaveFrame err NULL\r\n");
        return iRet;
    }
    if(i_ptFrameInfo->iFrameLen + m_iCurMediaDataLen > TS_MEDIA_BUF_MAX_LEN)
    {
        MH_LOGE("SaveFrame err %d,%d\r\n",i_ptFrameInfo->iFrameLen,m_iCurMediaDataLen);
        return iRet;
    }
    memset(&tFrameInfo,0,sizeof(T_MediaFrameInfo));
    memcpy(&tFrameInfo,i_ptFrameInfo,sizeof(T_MediaFrameInfo));
    
    tFrameInfo.pbFrameStartPos = m_pbMediaData+m_iCurMediaDataLen;
    memcpy(tFrameInfo.pbFrameStartPos,i_ptFrameInfo->pbFrameStartPos,tFrameInfo.iFrameLen);
    m_iCurMediaDataLen+=tFrameInfo.iFrameLen;

    m_MediaList.push_back(tFrameInfo);
    return 0;
}


/*****************************************************************************
-Fuction        : DelAllFrame
-Description    : demux muxer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::DelAllFrame()
{
    m_MediaList.clear();
    m_iCurMediaDataLen = 0;
    return 0;
}


/*****************************************************************************
-Fuction        : FrameToTS
-Description    : 
-Input          : i_iEnableAudio=0��i_iAudioStreamType��Ϊ����ֵ
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::FrameToTS(T_MediaFrameInfo * i_ptFrameInfo,int i_iVideoStreamType,int i_iAudioStreamType,int i_iEnableAudio,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    unsigned char * pbBuf = NULL;
    unsigned int dwPID = 0;
    int iLenPMT = 0;
    int iLenPES = 0;
    int iLenProcessedPES = 0;
    int iLenTsPES = 0;
    int iLenTsPacketPES = 0;

    if(NULL == i_ptFrameInfo ||NULL == o_pbBuf)
    {
        MH_LOGE("GetMuxData err NULL\r\n");
        return iRet;
    }

    if(NULL == m_pbBufPES)
    {
        m_pbBufPES = new unsigned char [PES_BUF_MAX_LEN];
    }
    if(NULL == m_pbBufPES)
    {
        MH_LOGE("NULL == m_pbBufPES err\r\n");
        return iRet;
    }
    if (i_ptFrameInfo->eFrameType == MEDIA_FRAME_TYPE_AUDIO_FRAME)
    {
        dwPID=TS_AUDIO_PID;
    }
    else if (i_ptFrameInfo->eFrameType == MEDIA_FRAME_TYPE_VIDEO_I_FRAME || 
    i_ptFrameInfo->eFrameType == MEDIA_FRAME_TYPE_VIDEO_P_FRAME || 
    i_ptFrameInfo->eFrameType == MEDIA_FRAME_TYPE_VIDEO_B_FRAME)
    {
        dwPID=TS_VIDEO_PID;
    }
    else
    {
        MH_LOGE("i_ptFrameInfo->eFrameType %d err\r\n",i_ptFrameInfo->eFrameType);
        return -1;
    }

    //�������I  ֡�����PAT/PMT ��
    if (i_ptFrameInfo->eFrameType == MEDIA_FRAME_TYPE_VIDEO_I_FRAME)
    {
        iRet=GetTsHeader(TS_PAT_PID, 0x01, 0x01,o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
        if(iRet <= 0)
        {
            MH_LOGE("GetTsHeader err\r\n");
            return iRet;
        }
        iDataLen+=iRet;
        o_pbBuf[iDataLen] = 0; //payload_unit_start_indicator��ֵΪ1��psi���ʾ������ʼ�ֽڻ���1�������ֽ�point_field����ʾPSI Section����ʼ��
        iDataLen++; //point_fieldΪ0
        iRet=GeneratePAT(o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
        if(iRet <= 0)
        {
            MH_LOGE("GeneratePAT err\r\n");
            return iRet;
        }
        iDataLen+=iRet;
        if(iDataLen > TS_PACKET_SIZE)
        {
            MH_LOGE("iDataLen%d > TS_PACKET_SIZE err\r\n",iDataLen);
            return -1;
        }
        memset(o_pbBuf+iDataLen, 0xFF, TS_PACKET_SIZE-iDataLen);
        iDataLen+=TS_PACKET_SIZE-iDataLen;
        
        iRet=GetTsHeader(TS_PMT_PID, 0x01, 0x01,o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
        if(iRet <= 0)
        {
            MH_LOGE("GetTsHeader err\r\n");
            return iRet;
        }
        iDataLen+=iRet;
        iLenPMT=iRet;
        o_pbBuf[iDataLen] = 0; //payload_unit_start_indicator��ֵΪ1��psi���ʾ������ʼ�ֽڻ���1�������ֽ�point_field����ʾPSI Section����ʼ��
        iDataLen++; //point_fieldΪ0
        iLenPMT++;
        iRet=GeneratePMT(o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen,i_iVideoStreamType,i_iAudioStreamType,i_iEnableAudio);
        if(iRet <= 0)
        {
            MH_LOGE("GeneratePAT err\r\n");
            return iRet;
        }
        iDataLen+=iRet;
        iLenPMT+=iRet;
        if(iLenPMT > TS_PACKET_SIZE)
        {
            MH_LOGE("GeneratePMT iLenPMT%d > TS_PACKET_SIZE err\r\n",iLenPMT);
            return -1;
        }
        memset(o_pbBuf+iDataLen, 0xFF, TS_PACKET_SIZE-iLenPMT);
        iDataLen+=TS_PACKET_SIZE-iLenPMT;
    }
    
    //����ǰ֡����TS  ����װ
    memset(m_pbBufPES,0,PES_BUF_MAX_LEN);
    iRet=GeneratePES(i_ptFrameInfo,m_pbBufPES,PES_BUF_MAX_LEN);
    if(iRet <= 0)
    {
        MH_LOGE("GeneratePES err\r\n");
        return iRet;
    }
    iLenPES=iRet;
    iLenProcessedPES=0;
    
    //����TS  ��Ƭ�ְ�(��һ��)
    iRet=GetTsHeader(dwPID, 0x01, 0x03,o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
    if(iRet <= 0)
    {
        MH_LOGE("GetTsHeader dwPID err\r\n");
        return iRet;
    }
    iDataLen+=iRet;
    iLenTsPES=iRet;
    iRet=GetTsAdaptationField(iLenPES,i_ptFrameInfo->dwTimeStamp,1, o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
    if(iRet <= 0)
    {
        MH_LOGE("GetTsAdaptationField err\r\n");
        return iRet;
    }
    iDataLen+=iRet;
    iLenTsPES+=iRet;
    if(iLenTsPES >= TS_PACKET_SIZE)
    {
        MH_LOGE("iLenTsPES%d >= TS_PACKET_SIZE err\r\n",iLenTsPES);
        return iRet;
    }
    
    iLenTsPacketPES=TS_PACKET_SIZE-iLenTsPES;
    if(iLenTsPacketPES > i_dwMaxBufLen-iDataLen)
    {
        MH_LOGE("iLenTsPacketPES%d > i_dwMaxBufLen%d-iDataLen%d err\r\n",iLenTsPacketPES, i_dwMaxBufLen,iDataLen);
        return iRet;
    }
	memcpy(o_pbBuf+iDataLen, m_pbBufPES+iLenProcessedPES, iLenTsPacketPES);
    iDataLen+=iLenTsPacketPES;
    iLenProcessedPES+=iLenTsPacketPES;
    while(iLenPES-iLenProcessedPES>0)
    {
		if (iLenPES-iLenProcessedPES >= TS_PACKET_SIZE-TS_PACKET_HEADER_SIZE)
		{//�м��
			//TS ��ͷ
            iRet=GetTsHeader(dwPID, 0x00, 0x01,o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
            if(iRet <= 0)
            {
                MH_LOGE("GetTsHeader 0x01 err\r\n");
                return iRet;
            }
            iDataLen+=iRet;
            iLenTsPacketPES=TS_PACKET_SIZE-TS_PACKET_HEADER_SIZE;
            if(iLenTsPacketPES > i_dwMaxBufLen-iDataLen)
            {
                MH_LOGE("iLenPES iLenTsPacketPES%d > i_dwMaxBufLen%d-iDataLen%d err\r\n",iLenTsPacketPES, i_dwMaxBufLen,iDataLen);
                return iRet;
            }
            memcpy(o_pbBuf+iDataLen, m_pbBufPES+iLenProcessedPES, iLenTsPacketPES);
            iDataLen+=iLenTsPacketPES;
            iLenProcessedPES+=iLenTsPacketPES;
		}
        else
        {//ĩβ��
            //TS ��ͷ
            iRet=GetTsHeader(dwPID, 0x00, 0x03,o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
            if(iRet <= 0)
            {
                MH_LOGE("GetTsHeader 0x03 err\r\n");
                return iRet;
            }
            iDataLen+=iRet;
            iLenTsPES=iRet;
            iRet=GetTsAdaptationField(iLenPES-iLenProcessedPES,i_ptFrameInfo->dwTimeStamp,0, o_pbBuf+iDataLen,i_dwMaxBufLen-iDataLen);
            if(iRet <= 0)
            {
                MH_LOGE("GetTsAdaptationField err\r\n");
                return iRet;
            }
            iDataLen+=iRet;
            iLenTsPES+=iRet;
            if(iLenTsPES >= TS_PACKET_SIZE)
            {
                MH_LOGE("iLenTsPES%d >= TS_PACKET_SIZE err\r\n",iLenTsPES);
                return iRet;
            }
            iLenTsPacketPES=TS_PACKET_SIZE-iLenTsPES;
            if(iLenTsPacketPES > i_dwMaxBufLen-iDataLen)
            {
                MH_LOGE("iLenTsPacketPES%d > i_dwMaxBufLen%d-iDataLen%d err\r\n",iLenTsPacketPES, i_dwMaxBufLen,iDataLen);
                return iRet;
            }
            memcpy(o_pbBuf+iDataLen, m_pbBufPES+iLenProcessedPES, iLenTsPacketPES);
            iDataLen+=iLenTsPacketPES;
            iLenProcessedPES+=iLenTsPacketPES;
        }
    }

    return iDataLen;
}


/*****************************************************************************
-Fuction        : GetTsHeader
-Description    : (Program Association Table����Ŀ������)
PID = 0x00,��Ч���ص�Ԫ��ʼָʾ��

-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::GetTsHeader(unsigned int i_dwPID, unsigned char i_bPayloadUnitStartIndicator, unsigned char i_bAdaptationFieldControl,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    T_TsHeader tTsHeader;
    unsigned char * pbBuf = NULL;

    
    if(NULL == o_pbBuf ||i_dwMaxBufLen <TS_PACKET_HEADER_SIZE)
    {
        MH_LOGE("GetTsHeader err %d NULL\r\n",i_dwMaxBufLen);
        return iRet;
    }

    pbBuf = o_pbBuf;
    
    memset(&tTsHeader,0,sizeof(T_TsHeader));
	tTsHeader.bSyncByte= TS_SYNC_BYTE;
	tTsHeader.TransportErrorIndicator= 0x00;
	tTsHeader.PayloadUnitStartIndicator = i_bPayloadUnitStartIndicator;
	tTsHeader.TransportPriority = 0x00;
	tTsHeader.PID = i_dwPID;
	tTsHeader.TransportScramblingControl = 0x00;
	tTsHeader.AdaptationFieldControl = i_bAdaptationFieldControl;
	if (i_dwPID == TS_PAT_PID)             //����pat�İ�
	{
		tTsHeader.ContinuityCounter= (m_bContinuityCounterPAT % 16);
		m_bContinuityCounterPAT++;
	}
	else if (i_dwPID == TS_PMT_PID)        //����pmt�İ�
	{
		tTsHeader.ContinuityCounter = (m_bContinuityCounterPMT % 16);
		m_bContinuityCounterPMT++;
	}
	else if (i_dwPID == TS_VIDEO_PID)      
	{
		tTsHeader.ContinuityCounter = (m_bContinuityCounterVideo % 16);
		m_bContinuityCounterVideo++;
	}
	else if (i_dwPID == TS_AUDIO_PID)
	{
		tTsHeader.ContinuityCounter = (m_bContinuityCounterAudio % 16);
		m_bContinuityCounterAudio++;
	}
	else  //���������������չ
	{
		MH_LOGE("[error]:continuity_counter error packet\n");
	}


	pbBuf[0] = tTsHeader.bSyncByte;
	pbBuf[1] = tTsHeader.TransportErrorIndicator << 7 | tTsHeader.PayloadUnitStartIndicator << 6 | tTsHeader.TransportPriority << 5 | ((tTsHeader.PID >> 8) & 0x1f);
	pbBuf[2] = (tTsHeader.PID & 0x00ff);
	pbBuf[3] = tTsHeader.TransportScramblingControl << 6 | tTsHeader.AdaptationFieldControl << 4 | tTsHeader.ContinuityCounter;
    iDataLen = 4;

    return iDataLen;
}


/*****************************************************************************
-Fuction        : GetTsAdaptationField
-Description    : �ֶ�Ŀ����Ϊ�����188�ֽ�����Ч������Ϊ0xff��
pcr�ǽ�Ŀʱ�Ӳο���pcr��dts��pts���Ƕ�ͬһ��ϵͳʱ�ӵĲ���ֵ��
pcr�ǵ����ģ���˿��Խ�������Ϊdtsֵ����Ƶ���ݲ���Ҫpcr��
���û���ֶΣ�ipad�ǿ��Բ��ŵģ���vlc�޷����š�
���ts��ʱPAT��PMT����û��adaptation field�ģ������ĳ���(�����)ֱ�Ӳ�0xff���ɡ�
��Ƶ������Ƶ������Ҫ��adaptation field��
ͨ������һ��֡�ĵ�һ��ts�������һ��ts����м��ts������
-Input          : bFlagPCR 0 no pcr (���һ��ts����),1 with pcr(֡�ĵ�һ��ts��)
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::GetTsAdaptationField(int i_iLenPES,unsigned int i_dwTimeStamp,unsigned char i_bFlagPCR,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    T_TsAdaptationField tTsAdaptationField;
    unsigned char * pbBuf = NULL;
	unsigned char bAdaptiveFlags = 0;       //����Ӧ�εı�־
    unsigned char bCurrentAdaptiveLength = 0;
    int iFillLen = 0;
    unsigned char bAdaptationFieldHeaderLen = 0;


    if(NULL == o_pbBuf ||i_dwMaxBufLen <1)
    {
        MH_LOGE("GetTsAdaptationField err %d NULL\r\n",i_dwMaxBufLen);
        return iRet;
    }
    if(i_iLenPES <= 0)
    {
        MH_LOGE("GetTsAdaptationField err %d i_iLenPES\r\n",i_iLenPES);
        return iRet;
    }
    pbBuf = o_pbBuf;
    
	//��д����Ӧ��
	bAdaptationFieldHeaderLen= i_bFlagPCR==0?1:7;//7 sizeof(bAdaptiveFlags)+sizeof(PCR)
	if(i_iLenPES < TS_PACKET_SIZE - TS_PACKET_HEADER_SIZE - 1 - bAdaptationFieldHeaderLen)//- 1 tTsAdaptationField.bAdaptationFieldLengthռ��1�ֽ�
	{//��С֡���ݴ���
	    iFillLen = TS_PACKET_SIZE - TS_PACKET_HEADER_SIZE - 1 - bAdaptationFieldHeaderLen - i_iLenPES;
	}
	
	bCurrentAdaptiveLength = iFillLen;
	memset(&tTsAdaptationField,0,sizeof(T_TsAdaptationField));
	tTsAdaptationField.bAdaptationFieldLength = bAdaptationFieldHeaderLen; //7//ռ��7λ,����Ӧ�򳤶ȣ�������ֽ���

	tTsAdaptationField.DiscontinutyIndicator= 0;
	tTsAdaptationField.RandomAccessIndicator= 0;
	tTsAdaptationField.ElementaryStreamPriorityIndicator= 0;
	tTsAdaptationField.FlagPCR= i_bFlagPCR;                                          //ֻ�õ����
	tTsAdaptationField.FlagOPCR= 0;
	tTsAdaptationField.SplicingPointFlag= 0;
	tTsAdaptationField.TransportPrivateDataFlag= 0;
	tTsAdaptationField.AdaptationFieldExtensionFlag= 0;

	//��Ҫ�Լ���
	tTsAdaptationField.ddwPCR= i_dwTimeStamp * 90 * 300;	//��27MHZ ��ʱ��	
	tTsAdaptationField.ddwOPCR= 0;
	tTsAdaptationField.bSpliceCountdown= 0;
	//tTsAdaptationField.private_data_len = 0;

#if 0
	//��д����Ӧ��,//��д����Ӧ�α�־֡β//�����ӿ��Ѿ����������һ��ts������ӵ����ֶεĴ���,ֻ��i_bFlagPCR=0
	tTsAdaptationField.bAdaptationFieldLength = 1;                          ////ռ��1λ��־���õ�λ
	tTsAdaptationField.DiscontinutyIndicator = 0;
	tTsAdaptationField.RandomAccessIndicator = 0;
	tTsAdaptationField.ElementaryStreamPriorityIndicator = 0;
	tTsAdaptationField.FlagPCR = 0;                                          //ֻ�õ����
	tTsAdaptationField.FlagOPCR = 0;
	tTsAdaptationField.SplicingPointFlag = 0;
	tTsAdaptationField.TransportPrivateDataFlag = 0;
	tTsAdaptationField.AdaptationFieldExtensionFlag = 0;
	//��Ҫ�Լ���
	tTsAdaptationField.ddwPCR = 0;
	tTsAdaptationField.ddwOPCR = 0;
	tTsAdaptationField.bSpliceCountdown = 0;
	//tTsAdaptationField.private_data_len = 0;
#endif


	//��д����Ӧ�ֶ�
	if (tTsAdaptationField.bAdaptationFieldLength > 0)
	{
        //TS �����ֶγ���(����ټ�)
        //pbBuf[0] = tTsAdaptationField.bAdaptationFieldLength; //����Ӧ�ֶεĳ���
        pbBuf += 1;
		//����������Ӧ�εı�־�ֽ�(ռ1���ֽ�)
		pbBuf += 1;
		bCurrentAdaptiveLength += 1;

		if (tTsAdaptationField.DiscontinutyIndicator)
		{
			bAdaptiveFlags |= 0x80;
		}
		if (tTsAdaptationField.RandomAccessIndicator)
		{
			bAdaptiveFlags |= 0x40;
		}
		if (tTsAdaptationField.ElementaryStreamPriorityIndicator)
		{
			bAdaptiveFlags |= 0x20;
		}
		if (tTsAdaptationField.FlagPCR)//0
		{
			unsigned long long ddwBasePCR;
			unsigned int dwExtPCR;

			ddwBasePCR = (tTsAdaptationField.ddwPCR / 300);
			dwExtPCR = (tTsAdaptationField.ddwPCR % 300);
			bAdaptiveFlags |= 0x10;
			pbBuf[0] = (ddwBasePCR >> 25) & 0xff;
			pbBuf[1] = (ddwBasePCR >> 17) & 0xff;
			pbBuf[2] = (ddwBasePCR >> 9) & 0xff;
			pbBuf[3] = (ddwBasePCR >> 1) & 0xff;
			pbBuf[4] = ddwBasePCR << 7 | dwExtPCR >> 8 | 0x7e;
			pbBuf[5] = (dwExtPCR) & 0xff;
			pbBuf += 6;
			bCurrentAdaptiveLength += 6;
		}
		if (tTsAdaptationField.FlagOPCR)
		{
			unsigned long long ddwBaseOPCR;
			unsigned int dwExtOPCR;

			ddwBaseOPCR = (tTsAdaptationField.ddwOPCR / 300);
			dwExtOPCR = (tTsAdaptationField.ddwOPCR % 300);
			bAdaptiveFlags |= 0x08;
			pbBuf[0] = (ddwBaseOPCR >> 25) & 0xff;
			pbBuf[1] = (ddwBaseOPCR >> 17) & 0xff;
			pbBuf[2] = (ddwBaseOPCR >> 9) & 0xff;
			pbBuf[3] = (ddwBaseOPCR >> 1) & 0xff;
			pbBuf[4] = ((ddwBaseOPCR << 7) & 0x80) | ((dwExtOPCR >> 8) & 0x01);
			pbBuf[5] = (dwExtOPCR) & 0xff;
			pbBuf += 6;
			bCurrentAdaptiveLength += 6;
		}
		if (tTsAdaptationField.SplicingPointFlag)
		{
			pbBuf[0] = tTsAdaptationField.bSpliceCountdown;

			bAdaptiveFlags |= 0x04;
			pbBuf += 1;
			bCurrentAdaptiveLength += 1;
		}
		/*if (tTsAdaptationField.private_data_len > 0)
		{
			bAdaptiveFlags |= 0x02;
			if ((1 + tTsAdaptationField.private_data_len) > (int)(max_adaptive_size - CurrentAdaptiveLength))
			{
				printf("private_data_len error !\n");
				return -1;
			}
            pointer_ts[0] = tTsAdaptationField.private_data_len;
            pointer_ts += 1;
            memcpy(pointer_ts, tTsAdaptationField.private_data, tTsAdaptationField.private_data_len);
            pointer_ts += tTsAdaptationField.private_data_len;
            CurrentAdaptiveLength += (1 + tTsAdaptationField.private_data_len);
		}*/
		if (tTsAdaptationField.AdaptationFieldExtensionFlag)
		{
			bAdaptiveFlags |= 0x01;
			pbBuf[1] = 1;
			pbBuf[2] = 0;
			pbBuf += 2;
			bCurrentAdaptiveLength += 2;
		}
		o_pbBuf[0] = bCurrentAdaptiveLength;                        //
		o_pbBuf[1] = bAdaptiveFlags;                        //����־�����ڴ�
		
        
        //TS �����ֶ�(�����)
        if(iFillLen > 0)
        {
            memset(pbBuf, 0xFF, iFillLen);
            pbBuf += iFillLen;
        }
        iDataLen = pbBuf-o_pbBuf;
	}

    return iDataLen;
}


/*****************************************************************************
-Fuction        : GeneratePAT
-Description    : (Program Association Table����Ŀ������)
PID = 0x00,��Ч���ص�Ԫ��ʼָʾ��

-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::GeneratePAT(unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    T_TsPAT tPAT;
    unsigned char * pbBuf = NULL;
    unsigned int dwCRC = 0xFFFFFFFF;

    
    if(NULL == o_pbBuf ||i_dwMaxBufLen <16)
    {
        MH_LOGE("GeneratePAT err %d NULL\r\n",i_dwMaxBufLen);
        return iRet;
    }

    pbBuf = o_pbBuf;
    
    memset(&tPAT,0,sizeof(T_TsPAT));
	tPAT.bTableID= 0x00;
	tPAT.SectionSyntaxIndicator= 0x01;
	tPAT.Zero= 0x00;
	tPAT.Reserved1= 0x03;                              	//����Ϊ11��
	tPAT.SectionLength= 0x0d;                         	//pat�ṹ�峤�� 16���ֽڼ�ȥ�����3���ֽ�
	tPAT.wTransportStreamID= 0x01;			//
	tPAT.Reserved2= 0x03;					//����Ϊ11��
	tPAT.VersionNumber= 0x00;
	tPAT.CurrentNextIndicator= 0x01;			//��ǰ��pat ��Ч
	tPAT.bSectionNumber= 0x00;				//һ��pat ��һ��ts�������ܹ�����
	tPAT.bLastSectionNumber= 0x00;
	tPAT.wProgramNumber= 0x01;
	tPAT.Reserved3= 0x07;         				//����Ϊ111��
	tPAT.PID= TS_PMT_PID;        	//PMT��PID

	pbBuf[0] = tPAT.bTableID;
	pbBuf[1] = tPAT.SectionSyntaxIndicator << 7 | tPAT.Zero << 6 | tPAT.Reserved1 << 4 | ((tPAT.SectionLength >> 8) & 0x0F);
	pbBuf[2] = tPAT.SectionLength & 0x00FF;
	pbBuf[3] = tPAT.wTransportStreamID >> 8;
	pbBuf[4] = tPAT.wTransportStreamID & 0x00FF;
	pbBuf[5] = tPAT.Reserved2 << 6 | tPAT.VersionNumber << 1 | tPAT.CurrentNextIndicator;
	pbBuf[6] = tPAT.bSectionNumber;
	pbBuf[7] = tPAT.bLastSectionNumber;
	pbBuf[8] = tPAT.wProgramNumber >> 8;
	pbBuf[9] = tPAT.wProgramNumber & 0x00FF;
	pbBuf[10] = tPAT.Reserved3 << 5 | ((tPAT.PID >> 8) & 0x0F);
	pbBuf[11] = tPAT.PID & 0x00FF;
    iDataLen = 12;
    dwCRC=CalcCrc32(pbBuf,(unsigned int)iDataLen);
	pbBuf += iDataLen;
    Write32BE(pbBuf,dwCRC);
    iDataLen += 4;

    return iDataLen;
}


/*****************************************************************************
-Fuction        : GeneratePMT
-Description    : (Program Association Table����Ŀ������)
PID = 0x00,��Ч���ص�Ԫ��ʼָʾ��

-Input          : i_iEnableAudio=0��i_iAudioStreamType��Ϊ����ֵ
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::GeneratePMT(unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen,int i_iVideoStreamType,int i_iAudioStreamType,int i_iEnableAudio)
{
    int iRet = -1;
    int iDataLen = 0;
    T_TsPMT tPMT;
    unsigned char * pbBuf = NULL;
    unsigned int dwCRC = 0xFFFFFFFF;
    int iEnableAudio = 0;

    
    if(NULL == o_pbBuf ||i_dwMaxBufLen <29)
    {
        MH_LOGE("GeneratePAT err %d NULL\r\n",i_dwMaxBufLen);
        return iRet;
    }

    pbBuf = o_pbBuf;
    iEnableAudio=i_iEnableAudio>0?1:0;
    
	//����PMT ��
	tPMT.bTableID= 0x02;
	tPMT.SectionSyntaxIndicator= 0x01;
	tPMT.Zero= 0x00;
	tPMT.Reserved1= 0x03;
	tPMT.SectionLength= 12 + 5 + 5 * iEnableAudio+4-3;	//PMT�ṹ�峤�� 16 + 5 + 5���ֽڼ�ȥ�����3���ֽ�
	tPMT.wProgramNumber = 01;				//ֻ��һ����Ŀ
	tPMT.Reserved2= 0x03;
	tPMT.VersionNumber= 0x00;
	tPMT.CurrentNextIndicator= 0x01;			//��ǰ��PMT��Ч	
	tPMT.bSectionNumber= 0x00;
	tPMT.bLastSectionNumber= 0x00;
	tPMT.Reserved3= 0x07;
	tPMT.PCR_PID = TS_VIDEO_PID;				//��ƵPID                                   
	tPMT.Reserved4= 0x0F;
	tPMT.ProgramInfoLength = 0x00;			//������ ��Ŀ��Ϣ����
	
	tPMT.bStreamTypeVideo= i_iVideoStreamType;	 	//��Ƶ������
	tPMT.Reserved5Video= 0x07;
	tPMT.ElementaryPidVideo= TS_VIDEO_PID;	//��Ƶ��PID
	tPMT.Reserved6Video= 0x0F;
	tPMT.EsInfoLengthVideo= 0x00;               	//��Ƶ�޸���������Ϣ
	tPMT.bStreamTypeAudio= i_iAudioStreamType;		//��Ƶ����
	tPMT.Reserved5Audio= 0x07;
	tPMT.ElementaryPidAudio= TS_AUDIO_PID;	//��ƵPID 
	tPMT.Reserved6Audio= 0x0F;
	tPMT.EsInfoLengthAudio= 0x00;              	//��Ƶ�޸���������Ϣ

	pbBuf[0] = tPMT.bTableID;
	pbBuf[1] = tPMT.SectionSyntaxIndicator << 7 | tPMT.Zero << 6 | tPMT.Reserved1 << 4 | ((tPMT.SectionLength >> 8) & 0x0F);
	pbBuf[2] = tPMT.SectionLength & 0x00FF;
	pbBuf[3] = tPMT.wProgramNumber >> 8;
	pbBuf[4] = tPMT.wProgramNumber & 0x00FF;
	pbBuf[5] = tPMT.Reserved2 << 6 | tPMT.VersionNumber << 1 | tPMT.CurrentNextIndicator;
	pbBuf[6] = tPMT.bSectionNumber;
	pbBuf[7] = tPMT.bLastSectionNumber;
	pbBuf[8] = tPMT.Reserved3 << 5 | ((tPMT.PCR_PID >> 8) & 0x1F);
	pbBuf[9] = tPMT.PCR_PID & 0x0FF;
	pbBuf[10] = tPMT.Reserved4 << 4 | ((tPMT.ProgramInfoLength >> 8) & 0x0F);
	pbBuf[11] = tPMT.ProgramInfoLength & 0xFF;
	
	pbBuf[12] = tPMT.bStreamTypeVideo;                               //��Ƶ����stream_type
	pbBuf[13] = tPMT.Reserved5Video << 5 | ((tPMT.ElementaryPidVideo >> 8) & 0x1F);
	pbBuf[14] = tPMT.ElementaryPidVideo & 0x00FF;
	pbBuf[15] = tPMT.Reserved6Video << 4 | ((tPMT.EsInfoLengthVideo >> 8) & 0x0F);
	pbBuf[16] = tPMT.EsInfoLengthVideo & 0x0FF;
	iDataLen += 17;
	if (iEnableAudio)
	{
		pbBuf[17] = tPMT.bStreamTypeAudio;                               //��Ƶ����stream_type
		pbBuf[18] = tPMT.Reserved5Audio << 5 | ((tPMT.ElementaryPidAudio >> 8) & 0x1F);
		pbBuf[19] = tPMT.ElementaryPidAudio & 0x00FF;
		pbBuf[20] = tPMT.Reserved6Audio << 4 | ((tPMT.EsInfoLengthAudio >> 8) & 0x0F);
		pbBuf[21] = tPMT.EsInfoLengthAudio & 0x0FF;
		iDataLen += 5;
	}
    dwCRC=CalcCrc32(pbBuf,(unsigned int)iDataLen);
	pbBuf += iDataLen;
    Write32BE(pbBuf,dwCRC);
    iDataLen += 4;

    return iDataLen;
}


/*****************************************************************************
-Fuction        : GetMuxData
-Description    : T_MediaFrameInfo ���������Ż�Ϊ�¶���Ľṹ�壬Ȼ�����ϲ�ת��
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::GeneratePES(T_MediaFrameInfo * i_ptFrameInfo,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    int iDataLen = 0;
    unsigned char abAUD[7] = { 0, };
    int iLenAUD = 0;
    unsigned char abH264AUD[6] = { 0x00, 0x00, 0x00, 0x01,0x09,0xf0 };
    unsigned char abH265AUD[7] = { 0x00, 0x00, 0x00, 0x01, 0x46, 0x01, 0x50 };


    if(NULL == i_ptFrameInfo ||NULL == o_pbBuf ||i_dwMaxBufLen < 19+7)
    {
        MH_LOGE("FrameToPES err %d NULL\r\n",i_dwMaxBufLen);
        return iRet;
    }


    memset(o_pbBuf,0,i_dwMaxBufLen);
    iRet = GetPesHeader(i_ptFrameInfo,i_ptFrameInfo->iFrameLen,o_pbBuf,i_dwMaxBufLen);
    if(iRet <= 0)
    {
        MH_LOGE("GetPesHeader err\r\n");
        return iRet;
    }
    iDataLen = iRet;
     
    //es���aud,
	if (i_ptFrameInfo->eEncType == MEDIA_ENCODE_TYPE_H264)
    {
        memcpy(abAUD, abH264AUD, sizeof(abH264AUD));
        iLenAUD = sizeof(abH264AUD);
    }
    else if (i_ptFrameInfo->eEncType == MEDIA_ENCODE_TYPE_H265)
    {
        memcpy(abAUD, abH265AUD, sizeof(abH265AUD));
        iLenAUD = sizeof(abH265AUD);
    }
    memcpy(o_pbBuf+iDataLen, abAUD, iLenAUD);
    iDataLen += iLenAUD;

    
    if(i_ptFrameInfo->iFrameLen > i_dwMaxBufLen-iDataLen)
    {
        MH_LOGE("i_ptFrameInfo->iFrameLen%d > i_dwMaxBufLen%d-iDataLen%d err\r\n",i_ptFrameInfo->iFrameLen, i_dwMaxBufLen,iDataLen);
        return iRet;
    }
    memcpy(o_pbBuf+iDataLen, i_ptFrameInfo->pbFrameStartPos, i_ptFrameInfo->iFrameLen);
    iDataLen += i_ptFrameInfo->iFrameLen;
    
    return iDataLen;
}


/*****************************************************************************
-Fuction        : GetPesHeader
-Description    : PTS�������ٵ���
-Input          : i_iFrameLen 65536
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TsPack::GetPesHeader(T_MediaFrameInfo * i_ptFrameInfo,int i_iFrameLen,unsigned char * o_pbBuf,unsigned int i_dwMaxBufLen)
{
    int iRet = -1;
    T_PesHeader tPesHeader;
    int iDataLen = 0;
    unsigned char * pbBuf = NULL;
    unsigned char bTimeLen = 0;

    
    if(NULL == i_ptFrameInfo ||NULL == o_pbBuf ||i_dwMaxBufLen < 19)
    {
        MH_LOGE("GetPesHeader err %d NULL\r\n",i_dwMaxBufLen);
        return iRet;
    }

	//��װPES  ��Ϣ
	memset(&tPesHeader,0,sizeof(T_PesHeader));
	memset(tPesHeader.abStartCode,0,sizeof(tPesHeader.abStartCode));
	tPesHeader.abStartCode[2] = 0x01;
	if (i_ptFrameInfo->eFrameType == MEDIA_FRAME_TYPE_AUDIO_FRAME)
	{
		tPesHeader.bStreamId= TS_AUDIO_STREAM_ID;
	}
	else
	{
		tPesHeader.bStreamId = TS_VIDEO_STREAM_ID;           	//E0~EF��ʾ����Ƶ��,C0~DF����Ƶ,H264-- E0
	}

	tPesHeader.wPesPacketLen= 0;//(length + 8);    		//һ֡���ݵĳ��� ������ PES��ͷ ,���8 �� ����Ӧ�ĳ���,��0 �����Զ�����
	if (i_iFrameLen > 0xFFFF)                                    //���һ֡���ݵĴ�С��������
	{
        MH_LOGD("i_iFrameLen %d > 0xFFFF\r\n",i_iFrameLen);//����ʱ�����nalu��ʼ����в�������õ�������֡
		tPesHeader.wPesPacketLen = 0x00;//������ʼ���nalu���ݾ���nalu��һ���ְ�
	}
	tPesHeader.MarkerBit= 0x02;
	tPesHeader.ScramblingControl= 0x00;                      	//��ѡ�ֶ� ���ڣ�������
	tPesHeader.Priority= 0x00;
	tPesHeader.DataAlignmentIndicator= 0x01;//PES����ͷ�����������Ƶ����Ƶ��ͬ���� 00 00 00 01
	tPesHeader.Copyright= 0x00;
	tPesHeader.OriginalOrCopy = 0x00;
	if (i_ptFrameInfo->eFrameType == MEDIA_FRAME_TYPE_AUDIO_FRAME)
	{
        tPesHeader.PtsDtsFlags= 0x03;//0x02;        //0x02 ֻ����pts
	}
	else
	{
        tPesHeader.PtsDtsFlags= 0x03;                 //0x03 PTS ��DTS ����
	}
	tPesHeader.EscrFlag= 0x00;
	tPesHeader.EsRateFlag= 0x00;
	tPesHeader.DsmTrickModeFlag= 0x00;
	tPesHeader.AdditionalCopyInfoFlag= 0x00;
	tPesHeader.PesCrcFlag= 0x00;
	tPesHeader.PesExtensionFlag= 0x00;
	tPesHeader.bPesHeaderDataLength = 0x0a;                       //�������ݵĳ��ȣ�ȡֵ5��10,��������� ������	PTS�� DTS��ռ���ֽ���

	//����PTS/DTS
	memset(tPesHeader.abPTS, 0, sizeof(tPesHeader.abPTS));
	uint64_t scr = (i_ptFrameInfo->dwTimeStamp* 90 + 0) & 0x1ffffffffULL;			//��90KHZ ��ʱ��	

	//�����Ż���ֱ��ʹ��src��������ô����λ
	uint64_t PTS = scr;// + 60000;
	tPesHeader.abPTS[0] = (((0x3 << 4) | ((PTS >> 29) & 0x0E) | 0x01) & 0xff);
	tPesHeader.abPTS[1] = (((((PTS >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
	tPesHeader.abPTS[2] = ((((PTS >> 14) & 0xfffe) | 0x01) & 0xff);
	tPesHeader.abPTS[3] = (((((PTS << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
	tPesHeader.abPTS[4] = ((((PTS << 1) & 0xfffe) | 0x01) & 0xff);
	uint64_t DTS = scr;// + 50000;	
	tPesHeader.abDTS[0] = (((0x1 << 4) | ((DTS >> 29) & 0x0E) | 0x01) & 0xff);
	tPesHeader.abDTS[1] = (((((DTS >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
	tPesHeader.abDTS[2] = ((((DTS >> 14) & 0xfffe) | 0x01) & 0xff);
	tPesHeader.abDTS[3] = ((((DTS << 1) & 0xfffe) | 0x01) >> 8) & 0xff;
	tPesHeader.abDTS[4] = (((DTS << 1) & 0xfffe) | 0x01) & 0xff;


	//TS ��Ч�غɲ���(PES ��ͷ)
	pbBuf=o_pbBuf;
    bTimeLen = tPesHeader.PtsDtsFlags == 0x03?(sizeof(tPesHeader.abPTS)+sizeof(tPesHeader.abDTS)):sizeof(tPesHeader.abPTS);
	memcpy(pbBuf,tPesHeader.abStartCode,sizeof(tPesHeader.abStartCode));
	pbBuf[3] =tPesHeader.bStreamId;
	pbBuf[4] = (tPesHeader.wPesPacketLen >> 8) & 0xFF;
	pbBuf[5] = tPesHeader.wPesPacketLen & 0xFF;
	pbBuf[6] = tPesHeader.MarkerBit << 6 | tPesHeader.ScramblingControl << 4 | tPesHeader.Priority << 3 |
		tPesHeader.DataAlignmentIndicator << 2 | tPesHeader.Copyright << 1 | tPesHeader.OriginalOrCopy;
	pbBuf[7] = tPesHeader.PtsDtsFlags << 6 | tPesHeader.EscrFlag << 5 | tPesHeader.EsRateFlag << 4 |
		tPesHeader.DsmTrickModeFlag << 3 | tPesHeader.AdditionalCopyInfoFlag << 2 | tPesHeader.PesCrcFlag << 1 | tPesHeader.PesExtensionFlag;
	pbBuf[8] = bTimeLen;
	pbBuf += 9;
	//TS ��Ч�غɲ���(PTS ��DTS)
	if (0x03 == tPesHeader.PtsDtsFlags)
	{
        memcpy(pbBuf, tPesHeader.abPTS, sizeof(tPesHeader.abPTS));
        pbBuf += sizeof(tPesHeader.abPTS);
    	memcpy(pbBuf, tPesHeader.abDTS, sizeof(tPesHeader.abDTS));
        pbBuf += sizeof(tPesHeader.abDTS);
	}
	else
	{
        memcpy(pbBuf, tPesHeader.abPTS, sizeof(tPesHeader.abPTS));
        pbBuf += sizeof(tPesHeader.abPTS);
	}

    iDataLen = pbBuf-o_pbBuf;


    return iDataLen;
}

/*****************************************************************************
-Fuction        : CalcCrc32
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
unsigned int TsPack::CalcCrc32(unsigned char * i_pbData,unsigned int i_dwDataLen)
{
    static unsigned int s_adwCrcTable[256] = {
      0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
      0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
      0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
      0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
      0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
      0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
      0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
      0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
      0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
      0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
      0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
      0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
      0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
      0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
      0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
      0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
      0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
      0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
      0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
      0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
      0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
      0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
      0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
      0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
      0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
      0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
      0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
      0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
      0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
      0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
      0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
      0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
      0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
      0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
      0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
      0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
      0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
      0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
      0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
      0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
      0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
      0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
      0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};
	unsigned int i;
	unsigned int dwCRC = 0xffffffff;

	for (i = 0; i < i_dwDataLen; i++)
	{
		dwCRC = (dwCRC << 8) ^ s_adwCrcTable[((dwCRC >> 24) ^ *i_pbData++) & 0xff];
	}
	return dwCRC;
}

