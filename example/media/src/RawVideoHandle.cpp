/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	RawVideoHandle.cpp
* Description		: 	RawVideoHandle operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "MediaAdapter.h"
#include "RawVideoHandle.h"
#include <string.h>
#include <iostream>


using std::cout;//需要<iostream>
using std::endl;

#define VIDEO_H264_FRAME_INTERVAL 40
#define VIDEO_H264_SAMPLE_RATE 90000

#define VIDEO_H265_FRAME_INTERVAL 40
#define VIDEO_H265_SAMPLE_RATE 90000


#define BIT(ptr, off) (((ptr)[(off) / 8] >> (7 - ((off) % 8))) & 0x01)

/*****************************************************************************
-Fuction		: RawVideoHandle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RawVideoHandle::RawVideoHandle()
{
}
/*****************************************************************************
-Fuction		: ~RawVideoHandle
-Description	: ~RawVideoHandle
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RawVideoHandle::~RawVideoHandle()
{
}

/*****************************************************************************
-Fuction        : RtmpMediaHandle
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RawVideoHandle::SpsToH264Resolution(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_RawH265Extradata *o_ptH265Extradata)
{
    int iRet = -1;
    unsigned char abSodbSPS[VIDEO_SPS_MAX_SIZE];
    int iSodbLen = 0;
    
    unsigned char id;
    unsigned char profile_idc;
    unsigned char level_idc;
    unsigned char constraint_set_flags = 0;
    unsigned char chroma_format_idc;
    unsigned char bit_depth_luma;
    unsigned char frame_mbs_only_flag;
    unsigned char seq_scaling_matrix_present_flag,seq_scaling_list_present_flag;
    unsigned char frame_cropping_flag;
    int delta_scale, lastScale = 8, nextScale = 8;
    int sizeOfScalingList;
    int iBit;//偏移第几位
    int i ,j,num_ref_frames_in_pic_order_cnt_cycle;
    unsigned int pic_order_cnt_type;
    unsigned int pic_width_in_mbs_minus1,pic_height_in_map_units_minus1;
    unsigned int frame_crop_left_offset,frame_crop_right_offset,frame_crop_top_offset,frame_crop_bottom_offset;

    
    if(NULL == i_pbSpsData || NULL == o_ptH265Extradata || 0 >= i_wSpsLen)
    {
        MH_LOGE("SpsToH265Extradata NULL %d \r\n", i_wSpsLen);
        return iRet;
    }
    memset(abSodbSPS,0,sizeof(abSodbSPS));
    iSodbLen = DecodeEBSP(i_pbSpsData, i_wSpsLen, abSodbSPS);
    if (iSodbLen < 12+3)
        return iRet;
    profile_idc = abSodbSPS[1] ;
    iBit = 2*8;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 0; // constraint_set0_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 1; // constraint_set1_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 2; // constraint_set2_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 3; // constraint_set3_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 4; // constraint_set4_flag
    iBit++;
    constraint_set_flags |= BIT(abSodbSPS, iBit) << 5; // constraint_set5_flag
    iBit++;
    iBit+=2;
    level_idc = abSodbSPS[3] ;
    iBit+=8;
    id=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
    if (profile_idc == 100 || profile_idc == 110 ||profile_idc == 122 || profile_idc == 244 || profile_idc ==  44 ||profile_idc == 83 || 
    profile_idc == 86 || profile_idc == 118 ||profile_idc == 128 || profile_idc == 138 || profile_idc == 139 ||profile_idc == 134) 
    {
        chroma_format_idc=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
        if (chroma_format_idc == 3) 
        {
            iBit++; // separate_colour_plane_flag
        }
        bit_depth_luma = H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit) + 8;
        H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // bit_depth_chroma_minus8
        iBit++; // qpprime_y_zero_transform_bypass_flag
        seq_scaling_matrix_present_flag = BIT(abSodbSPS, iBit);// seq_scaling_matrix_present_flag
        iBit++;
        if (seq_scaling_matrix_present_flag) 
        { 
            for (i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); i++) 
            {
                seq_scaling_list_present_flag = BIT(abSodbSPS, iBit); // seq_scaling_list_present_flag
                iBit++;
                if (!seq_scaling_list_present_flag)
                    continue;
                lastScale = 8;
                nextScale = 8;
                sizeOfScalingList = i < 6 ? 16 : 64;
                for (j = 0; j < sizeOfScalingList; j++) 
                {
                    if (nextScale != 0) 
                    {
                        delta_scale = H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
                        nextScale = (lastScale + delta_scale) & 0xff;
                    }
                    lastScale = nextScale == 0 ? lastScale : nextScale;
                }
            }
        }
    }
    else
    {
        chroma_format_idc = 1;
        bit_depth_luma = 8;
    }
    H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // log2_max_frame_num_minus4
    pic_order_cnt_type = H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
    if (pic_order_cnt_type == 0) 
    {
        H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // log2_max_pic_order_cnt_lsb_minus4
    } 
    else if (pic_order_cnt_type == 1) 
    {
        iBit++;    // delta_pic_order_always_zero
        H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // offset_for_non_ref_pic
        H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // offset_for_top_to_bottom_field
        num_ref_frames_in_pic_order_cnt_cycle = (int)H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit);
        for (i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
            H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // offset_for_ref_frame
    }
    H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // max_num_ref_frames
    iBit++; // gaps_in_frame_num_value_allowed_flag
    pic_width_in_mbs_minus1=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // pic_width_in_mbs_minus1
    pic_height_in_map_units_minus1=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // pic_height_in_map_units_minus1
    frame_mbs_only_flag = BIT(abSodbSPS, iBit);
    iBit++;
    if (!frame_mbs_only_flag)
        iBit++; // mb_adaptive_frame_field_flag
    iBit++; // direct_8x8_inference_flag
    frame_cropping_flag=BIT(abSodbSPS, iBit);// frame_cropping_flag
    iBit++;
    if (frame_cropping_flag) 
    { 
        frame_crop_left_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // frame_crop_left_offset
        frame_crop_right_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // frame_crop_right_offset
        frame_crop_top_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // frame_crop_top_offset
        frame_crop_bottom_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &iBit); // frame_crop_bottom_offset
    }
    
    // 宽高计算公式
    o_ptH265Extradata->pic_width = (pic_width_in_mbs_minus1+1) * 16;
    o_ptH265Extradata->pic_height = (2 - frame_mbs_only_flag)* (pic_height_in_map_units_minus1 +1) * 16;
    if(frame_cropping_flag)
    {
        unsigned int crop_unit_x;
        unsigned int crop_unit_y;
        if (0 == chroma_format_idc) // monochrome
        {
            crop_unit_x = 1;
            crop_unit_y = 2 - frame_mbs_only_flag;
        }
        else if (1 == chroma_format_idc) // 4:2:0
        {
            crop_unit_x = 2;
            crop_unit_y = 2 * (2 - frame_mbs_only_flag);
        }
        else if (2 == chroma_format_idc) // 4:2:2
        {
            crop_unit_x = 2;
            crop_unit_y = 2 - frame_mbs_only_flag;
        }
        else // 3 == sps.chroma_format_idc   // 4:4:4
        {
            crop_unit_x = 1;
            crop_unit_y = 2 -frame_mbs_only_flag;
        }
        
        o_ptH265Extradata->pic_width -= crop_unit_x * (frame_crop_left_offset + frame_crop_right_offset);
        o_ptH265Extradata->pic_height -= crop_unit_y * (frame_crop_top_offset + frame_crop_bottom_offset);
    }

    return 0;
}

/*****************************************************************************
-Fuction        : SpsToH265Extradata
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RawVideoHandle::SpsToH265Extradata(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_RawH265Extradata *o_ptH265Extradata)
{
    int iRet = -1;
    unsigned char abSodbSPS[VIDEO_SPS_MAX_SIZE];
    int iSodbLen = 0;
    unsigned char sps;
    unsigned char sps_max_sub_layers_minus1;
    unsigned char sps_temporal_id_nesting_flag;
    unsigned char conformance_window_flag;
    int n;
    unsigned int pic_width_in_luma_samples;
    unsigned int pic_height_in_luma_samples;
    unsigned int conf_win_left_offset;
    unsigned int conf_win_right_offset;
    unsigned int conf_win_top_offset;
    unsigned int conf_win_bottom_offset;
    unsigned int sub_width,sub_height;
    unsigned char separate_colour_plane_flag = 0;
    
    if(NULL == i_pbSpsData || NULL == o_ptH265Extradata || 0 >= i_wSpsLen)
    {
        MH_LOGE("SpsToH265Extradata NULL %d \r\n", i_wSpsLen);
        return iRet;
    }
    memset(abSodbSPS,0,sizeof(abSodbSPS));
    iSodbLen = DecodeEBSP(i_pbSpsData, i_wSpsLen, abSodbSPS);
    if (iSodbLen < 12+3)
        return iRet;
    sps_max_sub_layers_minus1 = (abSodbSPS[2] >> 1) & 0x07;
    sps_temporal_id_nesting_flag = abSodbSPS[2] & 0x01;
    n = HevcProfileTierLevel(abSodbSPS + 3, iSodbLen - 3, sps_max_sub_layers_minus1, o_ptH265Extradata);
    if (n <= 0)
        return iRet;
    n = (n + 3) * 8;
    
    sps = (unsigned char)H264ReadBitByUE(abSodbSPS, iSodbLen, &n);
    o_ptH265Extradata->chromaFormat = (unsigned char)H264ReadBitByUE(abSodbSPS, iSodbLen, &n);
    if (3 == o_ptH265Extradata->chromaFormat)
    {
        separate_colour_plane_flag=BIT(abSodbSPS, n);
        n++;
    }
    pic_width_in_luma_samples=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // pic_width_in_luma_samples
    pic_height_in_luma_samples=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // pic_height_in_luma_samples
    conformance_window_flag = BIT(abSodbSPS, n); 
    n++; // conformance_window_flag
    if (conformance_window_flag)
    {
        conf_win_left_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // conf_win_left_offset
        conf_win_right_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // conf_win_right_offset
        conf_win_top_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // conf_win_top_offset
        conf_win_bottom_offset=H264ReadBitByUE(abSodbSPS, iSodbLen, &n); // conf_win_bottom_offset
    }
    o_ptH265Extradata->bitDepthLumaMinus8 = (unsigned char)H264ReadBitByUE(abSodbSPS, iSodbLen, &n);
    o_ptH265Extradata->bitDepthChromaMinus8 = (unsigned char)H264ReadBitByUE(abSodbSPS, iSodbLen, &n);
    
    o_ptH265Extradata->pic_width = pic_width_in_luma_samples;
    o_ptH265Extradata->pic_height = pic_height_in_luma_samples;
    if (conformance_window_flag)
    {
        sub_width=((1==o_ptH265Extradata->chromaFormat)||(2 == o_ptH265Extradata->chromaFormat))&&(0==separate_colour_plane_flag)?2:1;
        sub_height=(1==o_ptH265Extradata->chromaFormat)&& (0 == separate_colour_plane_flag)?2:1;
        o_ptH265Extradata->pic_width -= (sub_width*conf_win_right_offset + sub_width*conf_win_left_offset);
        o_ptH265Extradata->pic_height -= (sub_height*conf_win_bottom_offset + sub_height*conf_win_top_offset);
    }
    return 0;
}

/*****************************************************************************
-Fuction        : DecodeEBSP
-Description    : 脱壳操作
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RawVideoHandle::DecodeEBSP(unsigned char* nalu, int bytes, unsigned char* sodb)
{
    int i, j;
    for (j = i = 0; i < bytes; i++)
    {
        if (i + 2 < bytes && 0 == nalu[i] && 0 == nalu[i + 1] && 0x03 == nalu[i + 2])
        {
            sodb[j++] = nalu[i];
            sodb[j++] = nalu[i + 1];
            i += 2;
        }
        else
        {
            sodb[j++] = nalu[i];
        }
    }
    return j;
}

/*****************************************************************************
-Fuction        : H264ReadBitByUE 读字节数据
-Description    : 指数哥伦布编码，ue(v)的解码
第一步：每次读取一个比特，如果是0就继续读，直至读到1为止，
此时读取比特的个数即leadingZeroBits
第二步：从第一步读到的比特1后，再顺序读leadingZeroBits       
个比特
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
unsigned int RawVideoHandle::H264ReadBitByUE(unsigned char* data, int bytes, int* offset)
{
    int bit, i;
    int leadingZeroBits = -1;

    for (bit = 0; !bit && *offset / 8 < bytes; ++leadingZeroBits)
    {
        bit = (data[*offset / 8] >> (7 - (*offset % 8))) & 0x01;
        ++*offset;
    }

    bit = 0;
    //assert(leadingZeroBits < 32);

    for (i = 0; i < leadingZeroBits && *offset / 8 < bytes; i++)
    {
        bit = (bit << 1) | ((data[*offset / 8] >> (7 - (*offset % 8))) & 0x01);
        ++*offset;
    }

    return (unsigned int)((1 << leadingZeroBits) - 1 + bit);
}

/*****************************************************************************
-Fuction        : hevc_profile_tier_level
-Description    : 脱壳操作
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int RawVideoHandle::HevcProfileTierLevel(unsigned char* nalu, int bytes, unsigned char maxNumSubLayersMinus1,T_RawH265Extradata* hevc)
{
    int n;
    unsigned char i;
    unsigned char sub_layer_profile_present_flag[8];
    unsigned char sub_layer_level_present_flag[8];

    if (bytes < 12)
        return -1;

    hevc->general_profile_space = (nalu[0] >> 6) & 0x03;
    hevc->general_tier_flag = (nalu[0] >> 5) & 0x01;
    hevc->general_profile_idc = nalu[0] & 0x1f;

    hevc->general_profile_compatibility_flags = 0;
    hevc->general_profile_compatibility_flags |= nalu[1] << 24;
    hevc->general_profile_compatibility_flags |= nalu[2] << 16;
    hevc->general_profile_compatibility_flags |= nalu[3] << 8;
    hevc->general_profile_compatibility_flags |= nalu[4];

    hevc->general_constraint_indicator_flags = 0;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[5]) << 40;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[6]) << 32;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[7]) << 24;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[8]) << 16;
    hevc->general_constraint_indicator_flags |= ((uint64_t)nalu[9]) << 8;
    hevc->general_constraint_indicator_flags |= nalu[10];

    hevc->general_level_idc = nalu[11];
    if (maxNumSubLayersMinus1 < 1)
        return 12;

    if (bytes < 14)
        return -1; // error

    for (i = 0; i < maxNumSubLayersMinus1; i++)
    {
        sub_layer_profile_present_flag[i] = BIT(nalu, 12 * 8 + i * 2);
        sub_layer_level_present_flag[i] = BIT(nalu, 12 * 8 + i * 2 + 1);
    }

    n = 12 + 2;
    for (i = 0; i < maxNumSubLayersMinus1; i++)
    {
        if(sub_layer_profile_present_flag[i])
            n += 11;
        if (sub_layer_level_present_flag[i])
            n += 1;
    }

    return bytes < n ? n : -1;
}

char * H264Handle::m_strVideoFormatName=(char *)VIDEO_ENC_FORMAT_H264_NAME;
/*****************************************************************************
-Fuction		: H264Handle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H264Handle::H264Handle()
{
	memset(&m_tVideoEncodeParam,0,sizeof(T_VideoEncodeParam));
	memset(&m_tMediaInfo,0,sizeof(T_MediaInfo));
}
/*****************************************************************************
-Fuction		: ~H264Handle
-Description	: ~H264Handle
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H264Handle::~H264Handle()
{
}


/*****************************************************************************
-Fuction		: VideoHandle::Init
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H264Handle::Init(char *i_strPath)
{
    int iRet=FALSE;
    if(NULL == i_strPath)
    {
        cout<<"Init NULL"<<endl;
        return iRet;
    }
    m_tMediaInfo.dwVideoSampleRate = VIDEO_H264_SAMPLE_RATE;
    m_tMediaInfo.eVideoEncType = MEDIA_ENCODE_TYPE_H264;
    m_tMediaInfo.eStreamType = STREAM_TYPE_VIDEO_STREAM;
    iRet = TRUE;
	return iRet;
}

/*****************************************************************************
-Fuction		: VideoHandle::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H264Handle::GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam)
{
    int iRet=FALSE;
    int iFramMark = 0;
    unsigned char *pcFrameStartPos = NULL;
    unsigned char *pcNaluStartPos = NULL;
    unsigned char *pcNaluEndPos = NULL;
    unsigned char *pcFrameData = NULL;
	int iRemainDataLen = 0;
    unsigned char bNaluType = 0;
    
	if(m_ptMediaFrameParam == NULL ||m_ptMediaFrameParam->iFrameBufLen <= 4)
	{
        cout<<"GetNextFrame err:"<<m_ptMediaFrameParam->iFrameBufLen<<endl;
        return iRet;
	}
	
	pcFrameData = m_ptMediaFrameParam->pbFrameBuf;
	iRemainDataLen = m_ptMediaFrameParam->iFrameBufLen;
    m_ptMediaFrameParam->dwNaluCount = 0;
    m_ptMediaFrameParam->iFrameLen = 0;
    while(iRemainDataLen > 0)
    {
        if (iRemainDataLen >= 3 && pcFrameData[0] == 0 && pcFrameData[1] == 0 && pcFrameData[2] == 1)
        {
            if(pcNaluStartPos != NULL)
            {
                pcNaluEndPos = pcFrameData;//此时是一个nalu的结束
            }
            else
            {
                pcNaluStartPos = pcFrameData;//此时是一个nalu的开始
                bNaluType = pcNaluStartPos[3] & 0x1f;
            }
            if(pcNaluEndPos != NULL)
            {
                if(pcFrameStartPos == NULL)
                {
                    pcFrameStartPos = pcNaluStartPos;
                    m_ptMediaFrameParam->pbFrameStartPos = pcFrameStartPos;
                }
                m_ptMediaFrameParam->iFrameLen += (pcNaluEndPos - pcNaluStartPos);
                m_ptMediaFrameParam->a_dwNaluEndOffset[m_ptMediaFrameParam->dwNaluCount] = (pcNaluEndPos - pcFrameStartPos);
                m_ptMediaFrameParam->dwNaluCount++;
                if(pcNaluEndPos - pcNaluStartPos > 3)
                {
                    switch(bNaluType)//取nalu类型
                    {
                        case 0x7:
                        {
                            memset(m_tVideoEncodeParam.abSPS,0,sizeof(m_tVideoEncodeParam.abSPS));
                            m_tVideoEncodeParam.iSizeOfSPS = pcNaluEndPos - pcNaluStartPos - 3;//包括类型减3
                            memcpy(m_tVideoEncodeParam.abSPS,pcNaluStartPos+3,m_tVideoEncodeParam.iSizeOfSPS);
                            break;
                        }
                        case 0x8:
                        {
                            memset(m_tVideoEncodeParam.abPPS,0,sizeof(m_tVideoEncodeParam.abPPS));
                            m_tVideoEncodeParam.iSizeOfPPS = pcNaluEndPos - pcNaluStartPos - 3;//包括类型减3
                            memcpy(m_tVideoEncodeParam.abPPS,pcNaluStartPos+3,m_tVideoEncodeParam.iSizeOfPPS);
                            break;
                        }
                        case 0x1:
                        {
                            m_ptMediaFrameParam->eFrameType = MEDIA_FRAME_TYPE_VIDEO_P_FRAME;
                            iFramMark = 1;
                            break;
                        }
                        case 0x5:
                        {
                            m_ptMediaFrameParam->eFrameType = MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
                            iFramMark = 1;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
                pcNaluStartPos = pcNaluEndPos;//上一个nalu的结束为下一个nalu的开始
                bNaluType = pcNaluStartPos[3] & 0x1f;
                pcNaluEndPos = NULL;
                if(0 != iFramMark)
                {
                    //时间戳的单位是1/VIDEO_H264_SAMPLE_RATE(s),频率的倒数
                    m_ptMediaFrameParam->dwTimeStamp += VIDEO_H264_FRAME_INTERVAL*VIDEO_H264_SAMPLE_RATE/1000;
                    break;
                }
            }
            pcFrameData += 3;
            iRemainDataLen -= 3;
        }
        else if (iRemainDataLen >= 4 && pcFrameData[0] == 0 && pcFrameData[1] == 0 && pcFrameData[2] == 0 && pcFrameData[3] == 1)
        {
            if(pcNaluStartPos != NULL)
            {
                pcNaluEndPos = pcFrameData;
            }
            else
            {
                pcNaluStartPos = pcFrameData;
                bNaluType = pcNaluStartPos[4] & 0x1f;
            }
            if(pcNaluEndPos != NULL)
            {
                if(pcFrameStartPos == NULL)
                {
                    pcFrameStartPos = pcNaluStartPos;
                    m_ptMediaFrameParam->pbFrameStartPos = pcFrameStartPos;
                }
                m_ptMediaFrameParam->iFrameLen += (pcNaluEndPos - pcNaluStartPos);
                m_ptMediaFrameParam->a_dwNaluEndOffset[m_ptMediaFrameParam->dwNaluCount] = (pcNaluEndPos - pcFrameStartPos);
                m_ptMediaFrameParam->dwNaluCount++;
                if(pcNaluEndPos - pcNaluStartPos > 4)
                {
                    switch(bNaluType)//取nalu类型
                    {
                        case 0x7:
                        {
                            memset(m_tVideoEncodeParam.abSPS,0,sizeof(m_tVideoEncodeParam.abSPS));
                            m_tVideoEncodeParam.iSizeOfSPS = pcNaluEndPos - pcNaluStartPos - 4;//包括类型减4开始码
                            memcpy(m_tVideoEncodeParam.abSPS,pcNaluStartPos+4,m_tVideoEncodeParam.iSizeOfSPS);
                            break;
                        }
                        case 0x8:
                        {
                            memset(m_tVideoEncodeParam.abPPS,0,sizeof(m_tVideoEncodeParam.abPPS));
                            m_tVideoEncodeParam.iSizeOfPPS = pcNaluEndPos - pcNaluStartPos - 4;//包括类型减4
                            memcpy(m_tVideoEncodeParam.abPPS,pcNaluStartPos+4,m_tVideoEncodeParam.iSizeOfPPS);
                            break;
                        }
                        case 0x1:
                        {
                            m_ptMediaFrameParam->eFrameType = MEDIA_FRAME_TYPE_VIDEO_P_FRAME;
                            iFramMark = 1;
                            break;
                        }
                        case 0x5:
                        {
                            m_ptMediaFrameParam->eFrameType = MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
                            iFramMark = 1;//i p b nalu才表示一帧的结束
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
                pcNaluStartPos = pcNaluEndPos;
                bNaluType = pcNaluStartPos[4] & 0x1f;
                pcNaluEndPos = NULL;
                if(0 != iFramMark)
                {
                    //时间戳的单位是1/VIDEO_H264_SAMPLE_RATE(s),频率的倒数
                    m_ptMediaFrameParam->dwTimeStamp += VIDEO_H264_FRAME_INTERVAL*VIDEO_H264_SAMPLE_RATE/1000;
                    break;
                }
            }
            pcFrameData += 4;
            iRemainDataLen -= 4;
        }
        else
        {
            pcFrameData ++;
            iRemainDataLen --;
        }
    }
	if(NULL != m_ptMediaFrameParam->pbFrameStartPos)
	{
        m_ptMediaFrameParam->iFrameProcessedLen += m_ptMediaFrameParam->pbFrameStartPos - m_ptMediaFrameParam->pbFrameBuf + m_ptMediaFrameParam->iFrameLen;
	}
    if(0 != iFramMark)
    {
        iRet = TRUE;
    }

	return iRet;
}
/*****************************************************************************
-Fuction		: VideoHandle::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H264Handle::GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam)
{
    int iRet=FALSE;

	if(o_ptVideoEncodeParam == NULL)
	{
        cout<<"GetVideoEncParam NULL"<<endl;
        return iRet;
	}

    memcpy(o_ptVideoEncodeParam,&m_tVideoEncodeParam,sizeof(T_VideoEncodeParam));
    return TRUE;
}
/*****************************************************************************
-Fuction		: VideoHandle::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H264Handle::GetMediaInfo(T_MediaInfo *o_ptMediaInfo)
{
    int iRet=FALSE;

	if(o_ptMediaInfo == NULL)
	{
        cout<<"GetMediaInfo NULL"<<endl;
        return iRet;
	}

    memcpy(o_ptMediaInfo,&m_tMediaInfo,sizeof(T_MediaInfo));
    return TRUE;
}


/*****************************************************************************
-Fuction		: VideoHandle::RemoveH264EmulationBytes
-Description	: 去掉h264中防止竞争的字节（脱壳操作）
-Input			: i_pbNaluBuf i_iNaluLen i_iMaxNaluBufLen
-Output 		: o_pbNaluBuf
-Return 		: iNaluLen //返回脱壳操作后的长度
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H264Handle::RemoveH264EmulationBytes(unsigned char *o_pbNaluBuf,int i_iMaxNaluBufLen,unsigned char *i_pbNaluBuf,int i_iNaluLen)
{
    int iNaluLen=0;
    
    int i = 0;
    while (i < i_iNaluLen && iNaluLen+1 < i_iMaxNaluBufLen) 
    {
      if (i+2 < i_iNaluLen && i_pbNaluBuf[i] == 0 && i_pbNaluBuf[i+1] == 0 && i_pbNaluBuf[i+2] == 3) 
      {
        o_pbNaluBuf[iNaluLen] = o_pbNaluBuf[iNaluLen+1] = 0;
        iNaluLen += 2;
        i += 3;
      } 
      else 
      {
        o_pbNaluBuf[iNaluLen] = i_pbNaluBuf[i];
        iNaluLen += 1;
        i += 1;
      }
    }
    
    return iNaluLen;
}

/*****************************************************************************
-Fuction        : ParseNaluFromFrame
-Description    : ParseNaluFromFrame
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int H264Handle::GetFrame(T_MediaFrameInfo *m_ptFrame)
{
    int iRet = -1;
    unsigned char *pcNaluStartPos = NULL;
    unsigned char *pcNaluEndPos = NULL;
    unsigned char *pcFrameData = NULL;
    int iRemainDataLen = 0;
    unsigned char bNaluType = 0;
    unsigned char bStartCodeLen = 0;
    
    if(NULL == m_ptFrame || NULL == m_ptFrame->pbFrameBuf ||m_ptFrame->iFrameBufLen <= 4)
    {
        MH_LOGE("GetFrame NULL %d\r\n", m_ptFrame->iFrameBufLen);
        return iRet;
    }
	
	pcFrameData = m_ptFrame->pbFrameBuf;
	iRemainDataLen = m_ptFrame->iFrameBufLen;
    m_ptFrame->dwNaluCount = 0;
    m_ptFrame->iFrameLen = 0;
    while(iRemainDataLen > 0)
    {
        if (iRemainDataLen >= 3 && pcFrameData[0] == 0 && pcFrameData[1] == 0 && pcFrameData[2] == 1)
        {
            if(pcNaluStartPos != NULL)
            {
                pcNaluEndPos = pcFrameData;//此时是一个nalu的结束
            }
            else
            {
                pcNaluStartPos = pcFrameData;//此时是一个nalu的开始
                bStartCodeLen = 3;
                bNaluType = pcNaluStartPos[3] & 0x1f;
            }
            if(pcNaluEndPos != NULL)
            {
                if(pcNaluEndPos - pcNaluStartPos > 3)
                {
                    iRet=SetH264NaluData(bNaluType,bStartCodeLen,pcNaluStartPos,pcNaluEndPos - pcNaluStartPos,m_ptFrame);
                }
                pcNaluStartPos = pcNaluEndPos;//上一个nalu的结束为下一个nalu的开始
                bStartCodeLen = 3;
                bNaluType = pcNaluStartPos[3] & 0x1f;
                pcNaluEndPos = NULL;
                if(iRet == 0)
                {
                    break;//解析出一帧则退出
                }
            }
            pcFrameData += 3;
            iRemainDataLen -= 3;
        }
        else if (iRemainDataLen >= 4 && pcFrameData[0] == 0 && pcFrameData[1] == 0 && pcFrameData[2] == 0 && pcFrameData[3] == 1)
        {
            if(pcNaluStartPos != NULL)
            {
                pcNaluEndPos = pcFrameData;
            }
            else
            {
                pcNaluStartPos = pcFrameData;
                bStartCodeLen = 4;
                bNaluType = pcNaluStartPos[4] & 0x1f;
            }
            if(pcNaluEndPos != NULL)
            {
                if(pcNaluEndPos - pcNaluStartPos > 4)
                {
                    iRet=SetH264NaluData(bNaluType,bStartCodeLen,pcNaluStartPos,pcNaluEndPos - pcNaluStartPos,m_ptFrame);
                }
                pcNaluStartPos = pcNaluEndPos;
                bStartCodeLen = 4;
                bNaluType = pcNaluStartPos[4] & 0x1f;
                pcNaluEndPos = NULL;
                if(iRet == 0)
                {
                    break;//解析出一帧则退出
                }
            }
            pcFrameData += 4;
            iRemainDataLen -= 4;
        }
        else
        {
            pcFrameData ++;
            iRemainDataLen --;
        }
    }
    if(pcNaluStartPos != NULL && iRet != 0)
    {
        iRet=SetH264NaluData(bNaluType,bStartCodeLen,pcNaluStartPos,pcFrameData - pcNaluStartPos,m_ptFrame);
        if(iRet < 0)
        {
            MH_LOGE("SetH264NaluData err %d %d\r\n", m_ptFrame->dwNaluCount,m_ptFrame->iFrameLen);
            return iRet;
        }
    }
    
	if(NULL != m_ptFrame->pbFrameStartPos)
	{
        m_ptFrame->iFrameProcessedLen += m_ptFrame->pbFrameStartPos - m_ptFrame->pbFrameBuf + m_ptFrame->iFrameLen;
	}
    return iRet;
}


/*****************************************************************************
-Fuction        : RtmpMediaHandle
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int H264Handle::SetH264NaluData(unsigned char i_bNaluType,unsigned char i_bStartCodeLen,unsigned char *i_pbNaluData,int i_iNaluDataLen,T_MediaFrameInfo *m_ptFrame)
{
    int iRet = -1;
    unsigned char * pbNaluData = NULL;//去掉00 00 00 01
    int iNaluDataLen=0;
    E_MediaFrameType eFrameType = MEDIA_FRAME_TYPE_UNKNOW;
    
    if(NULL == i_pbNaluData || NULL == m_ptFrame)
    {
        MH_LOGE("SetH264NaluData NULL %d \r\n", i_iNaluDataLen);
        return iRet;
    }
    
    if(m_ptFrame->pbFrameStartPos == NULL)
    {
        m_ptFrame->pbFrameStartPos = i_pbNaluData;
    }
    m_ptFrame->iFrameLen += i_iNaluDataLen;
    m_ptFrame->atNaluInfo[m_ptFrame->dwNaluCount].pbData= i_pbNaluData;
    m_ptFrame->atNaluInfo[m_ptFrame->dwNaluCount].dwDataLen= i_iNaluDataLen;
    m_ptFrame->dwNaluCount++;

    iNaluDataLen = i_iNaluDataLen-i_bStartCodeLen;//包括类型减开始码
    pbNaluData = i_pbNaluData+i_bStartCodeLen;
    switch(i_bNaluType)//取nalu类型
    {
        case 0x7:
        {
            memset(m_ptFrame->tVideoEncodeParam.abSPS,0,sizeof(m_ptFrame->tVideoEncodeParam.abSPS));
            m_ptFrame->tVideoEncodeParam.iSizeOfSPS= iNaluDataLen;//包括类型(减3开始码)
            memcpy(m_ptFrame->tVideoEncodeParam.abSPS,pbNaluData,m_ptFrame->tVideoEncodeParam.iSizeOfSPS);

            if(0 == m_ptFrame->dwWidth && 0 == m_ptFrame->dwHeight)
            {
                T_RawH265Extradata tH265Extradata;
                memset(&tH265Extradata,0,sizeof(T_RawH265Extradata));
                m_oRawVideoHandle.SpsToH264Resolution(m_ptFrame->tVideoEncodeParam.abSPS,m_ptFrame->tVideoEncodeParam.iSizeOfSPS,&tH265Extradata);
                m_ptFrame->dwWidth = tH265Extradata.pic_width;
                m_ptFrame->dwHeight = tH265Extradata.pic_height;
            }
            break;
        }
        case 0x8:
        {
            memset(m_ptFrame->tVideoEncodeParam.abPPS,0,sizeof(m_ptFrame->tVideoEncodeParam.abPPS));
            m_ptFrame->tVideoEncodeParam.iSizeOfPPS = iNaluDataLen;//包括类型减3开始码
            memcpy(m_ptFrame->tVideoEncodeParam.abPPS,pbNaluData,m_ptFrame->tVideoEncodeParam.iSizeOfPPS);
            break;
        }
        case 0x1:
        {
            eFrameType = MEDIA_FRAME_TYPE_VIDEO_P_FRAME;
            break;
        }
        case 0x5:
        {
            eFrameType = MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
            break;
        }
        default:
        {
            break;
        }
    }

    if(MEDIA_FRAME_TYPE_UNKNOW != eFrameType)
    {
        if(STREAM_TYPE_UNKNOW == m_ptFrame->eStreamType)//文件的时候才需要赋值，数据流的时候外部会赋值以外部为准
        {
            m_ptFrame->eFrameType = eFrameType;
            m_ptFrame->dwTimeStamp += VIDEO_H264_FRAME_INTERVAL;//VIDEO_H264_FRAME_INTERVAL*VIDEO_H264_SAMPLE_RATE/1000;
            m_ptFrame->dwSampleRate= VIDEO_H264_SAMPLE_RATE;
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_H264;
        }
        iRet = 0;//解析出一帧则退出
    }
    return iRet;
}

char * H265Handle::m_strVideoFormatName=(char *)VIDEO_ENC_FORMAT_H265_NAME;
/*****************************************************************************
-Fuction		: H264Handle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H265Handle::H265Handle()
{
	memset(&m_tVideoEncodeParam,0,sizeof(T_VideoEncodeParam));
	memset(&m_tMediaInfo,0,sizeof(T_MediaInfo));
	
}
/*****************************************************************************
-Fuction		: ~H264Handle
-Description	: ~H264Handle
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
H265Handle::~H265Handle()
{
}
/*****************************************************************************
-Fuction		: VideoHandle::Init
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H265Handle::Init(char *i_strPath)
{
    int iRet=FALSE;
    if(NULL == i_strPath)
    {
        cout<<"Init NULL"<<endl;
        return iRet;
    }
    iRet = TRUE;
    m_tMediaInfo.dwVideoSampleRate = VIDEO_H265_SAMPLE_RATE;
    m_tMediaInfo.eVideoEncType = MEDIA_ENCODE_TYPE_H265;
    m_tMediaInfo.eStreamType = STREAM_TYPE_VIDEO_STREAM;
	return iRet;
}

/*****************************************************************************
-Fuction		: VideoHandle::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H265Handle::GetNextFrame(T_MediaFrameParam *m_ptMediaFrameParam)
{
    int iRet=FALSE;
    int iFramMark = 0;
    unsigned char *pcFrameStartPos = NULL;
    unsigned char *pcNaluStartPos = NULL;
    unsigned char *pcNaluEndPos = NULL;
    unsigned char *pcFrameData = NULL;
	int iRemainDataLen = 0;
    unsigned char bNaluType = 0;
    
	if(m_ptMediaFrameParam == NULL ||m_ptMediaFrameParam->iFrameBufLen <= 4)
	{
        cout<<"GetNextFrame err:"<<m_ptMediaFrameParam->iFrameBufLen<<endl;
        return iRet;
	}
	
	pcFrameData = m_ptMediaFrameParam->pbFrameBuf;
	iRemainDataLen = m_ptMediaFrameParam->iFrameBufLen;
    m_ptMediaFrameParam->dwNaluCount = 0;
    m_ptMediaFrameParam->iFrameLen = 0;
    while(iRemainDataLen > 0)
    {
        if (iRemainDataLen >= 4 && pcFrameData[0] == 0 && pcFrameData[1] == 0 && pcFrameData[2] == 0 && pcFrameData[3] == 1)
        {
            if(pcNaluStartPos != NULL)
            {
                pcNaluEndPos = pcFrameData;
            }
            else
            {
                pcNaluStartPos = pcFrameData;
                bNaluType = (pcNaluStartPos[4] & 0x7E)>>1;//取nalu类型
            }
            if(pcNaluEndPos != NULL)
            {
                if(pcFrameStartPos == NULL)
                {
                    pcFrameStartPos = pcNaluStartPos;
                    m_ptMediaFrameParam->pbFrameStartPos = pcFrameStartPos;
                }
                m_ptMediaFrameParam->iFrameLen += (pcNaluEndPos - pcNaluStartPos);
                m_ptMediaFrameParam->a_dwNaluEndOffset[m_ptMediaFrameParam->dwNaluCount] = (pcNaluEndPos - pcFrameStartPos);
                m_ptMediaFrameParam->dwNaluCount++;
                if(pcNaluEndPos - pcNaluStartPos > 4)
                {
                    if(bNaluType >= 0 && bNaluType <= 9)// p slice 片
                    {
                        m_ptMediaFrameParam->eFrameType = MEDIA_FRAME_TYPE_VIDEO_P_FRAME;
                        iFramMark = 1;//i p b nalu才表示一帧的结束
                    }
                    else if(bNaluType >= 16 && bNaluType <= 21)// IRAP 等同于i帧
                    {
                        m_ptMediaFrameParam->eFrameType = MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
                        iFramMark = 1;//i p b nalu才表示一帧的结束
                    }
                    else if(bNaluType == 32)//VPS
                    {
                        memset(m_tVideoEncodeParam.abVPS,0,sizeof(m_tVideoEncodeParam.abVPS));
                        m_tVideoEncodeParam.iSizeOfVPS = pcNaluEndPos - pcNaluStartPos - 4;//包括类型减4
                        memcpy(m_tVideoEncodeParam.abVPS,pcNaluStartPos+4,m_tVideoEncodeParam.iSizeOfVPS);
                    }
                    else if(bNaluType == 33)//SPS
                    {
                        memset(m_tVideoEncodeParam.abSPS,0,sizeof(m_tVideoEncodeParam.abSPS));
                        m_tVideoEncodeParam.iSizeOfSPS = pcNaluEndPos - pcNaluStartPos - 4;//包括类型减4
                        memcpy(m_tVideoEncodeParam.abSPS,pcNaluStartPos+4,m_tVideoEncodeParam.iSizeOfSPS);
                    }
                    else if(bNaluType == 34)//PPS
                    {
                        memset(m_tVideoEncodeParam.abPPS,0,sizeof(m_tVideoEncodeParam.abPPS));
                        m_tVideoEncodeParam.iSizeOfPPS = pcNaluEndPos - pcNaluStartPos - 4;//包括类型减4
                        memcpy(m_tVideoEncodeParam.abPPS,pcNaluStartPos+4,m_tVideoEncodeParam.iSizeOfPPS);
                    }
                }
                pcNaluStartPos = pcNaluEndPos;
                bNaluType = (pcNaluStartPos[4] & 0x7E)>>1;//取nalu类型
                pcNaluEndPos = NULL;
                if(0 != iFramMark)
                {
                    //时间戳的单位是1/VIDEO_H265_SAMPLE_RATE(s),频率的倒数
                    m_ptMediaFrameParam->dwTimeStamp += VIDEO_H265_FRAME_INTERVAL*VIDEO_H265_SAMPLE_RATE/1000;
                    break;
                }
            }
            pcFrameData += 4;
            iRemainDataLen -= 4;
        }
        else
        {
            pcFrameData ++;
            iRemainDataLen --;
        }
    }
	if(NULL != m_ptMediaFrameParam->pbFrameStartPos)
	{
        m_ptMediaFrameParam->iFrameProcessedLen += m_ptMediaFrameParam->pbFrameStartPos - m_ptMediaFrameParam->pbFrameBuf + m_ptMediaFrameParam->iFrameLen;
	}
    if(0 != iFramMark)
    {
        iRet = TRUE;
    }
	return iRet;
}
/*****************************************************************************
-Fuction		: VideoHandle::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H265Handle::GetVideoEncParam(T_VideoEncodeParam *o_ptVideoEncodeParam)
{
    int iRet=FALSE;

	if(o_ptVideoEncodeParam == NULL)
	{
        cout<<"GetVideoEncParam NULL"<<endl;
        return iRet;
	}

    memcpy(o_ptVideoEncodeParam,&m_tVideoEncodeParam,sizeof(T_VideoEncodeParam));
    return TRUE;
}
/*****************************************************************************
-Fuction		: VideoHandle::GetNextVideoFrame
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int H265Handle::GetMediaInfo(T_MediaInfo *o_ptMediaInfo)
{
    int iRet=FALSE;

	if(o_ptMediaInfo == NULL)
	{
        cout<<"GetMediaInfo NULL"<<endl;
        return iRet;
	}

    memcpy(o_ptMediaInfo,&m_tMediaInfo,sizeof(T_MediaInfo));
    return TRUE;
}

/*****************************************************************************
-Fuction        : ParseNaluFromFrame
-Description    : ParseNaluFromFrame
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int H265Handle::GetFrame(T_MediaFrameInfo *m_ptFrame)
{
    int iRet = -1;
    unsigned char *pcNaluStartPos = NULL;
    unsigned char *pcNaluEndPos = NULL;
    unsigned char *pcFrameData = NULL;
    int iRemainDataLen = 0;
    unsigned char bNaluType = 0;
    unsigned char bStartCodeLen = 0;


    if(NULL == m_ptFrame || NULL == m_ptFrame->pbFrameBuf ||m_ptFrame->iFrameBufLen <= 4)
    {
        MH_LOGE("GetFrame NULL %d\r\n", m_ptFrame->iFrameBufLen);
        return iRet;
    }
	
	pcFrameData = m_ptFrame->pbFrameBuf;
	iRemainDataLen = m_ptFrame->iFrameBufLen;
    m_ptFrame->dwNaluCount = 0;
    m_ptFrame->iFrameLen = 0;
    while(iRemainDataLen > 0)
    {
        if (iRemainDataLen >= 3 && pcFrameData[0] == 0 && pcFrameData[1] == 0 && pcFrameData[2] == 1)
        {
            if(pcNaluStartPos != NULL)
            {
                pcNaluEndPos = pcFrameData;//此时是一个nalu的结束
            }
            else
            {
                pcNaluStartPos = pcFrameData;//此时是一个nalu的开始
                bStartCodeLen = 3;
                bNaluType = (pcNaluStartPos[bStartCodeLen] & 0x7E)>>1;//取nalu类型
            }
            if(pcNaluEndPos != NULL)
            {
                if(pcNaluEndPos - pcNaluStartPos > 3)
                {
                    iRet=SetH265NaluData(bNaluType,bStartCodeLen,pcNaluStartPos,pcNaluEndPos - pcNaluStartPos,m_ptFrame);
                }
                pcNaluStartPos = pcNaluEndPos;//上一个nalu的结束为下一个nalu的开始
                bStartCodeLen = 3;
                bNaluType = (pcNaluStartPos[bStartCodeLen] & 0x7E)>>1;//取nalu类型
                pcNaluEndPos = NULL;
                if(iRet == 0)
                {
                    break;//解析出一帧则退出
                }
            }
            pcFrameData += 3;
            iRemainDataLen -= 3;
        }
        else if (iRemainDataLen >= 4 && pcFrameData[0] == 0 && pcFrameData[1] == 0 && pcFrameData[2] == 0 && pcFrameData[3] == 1)
        {
            if(pcNaluStartPos != NULL)
            {
                pcNaluEndPos = pcFrameData;
            }
            else
            {
                pcNaluStartPos = pcFrameData;
                bStartCodeLen = 4;
                bNaluType = (pcNaluStartPos[4] & 0x7E)>>1;//取nalu类型
            }
            if(pcNaluEndPos != NULL)
            {
                if(pcNaluEndPos - pcNaluStartPos > 4)
                {
                    iRet = SetH265NaluData(bNaluType,bStartCodeLen,pcNaluStartPos,pcNaluEndPos - pcNaluStartPos,m_ptFrame);//包括类型减4//去掉00 00 00 01
                }
                pcNaluStartPos = pcNaluEndPos;
                bStartCodeLen = 4;
                bNaluType = (pcNaluStartPos[4] & 0x7E)>>1;//取nalu类型
                pcNaluEndPos = NULL;
                if(iRet == 0)
                {
                    break;//解析出一帧则退出
                }
            }
            pcFrameData += 4;
            iRemainDataLen -= 4;
        }
        else
        {
            pcFrameData ++;
            iRemainDataLen --;
        }
    }
    if(pcNaluStartPos != NULL && iRet != 0)
    {
        iRet=SetH265NaluData(bNaluType,bStartCodeLen,pcNaluStartPos,pcFrameData - pcNaluStartPos,m_ptFrame);//包括类型减4开始码
        if(iRet < 0)
        {
            MH_LOGE("SetH265NaluData err bNaluType%d bStartCodeLen%d dwNaluCount%d iFrameLen%d\r\n", bNaluType,bStartCodeLen,m_ptFrame->dwNaluCount,m_ptFrame->iFrameLen);
            return iRet;
        }
    }
	if(NULL != m_ptFrame->pbFrameStartPos)
	{
        m_ptFrame->iFrameProcessedLen += m_ptFrame->pbFrameStartPos - m_ptFrame->pbFrameBuf + m_ptFrame->iFrameLen;
	}
    return iRet;
}
/*****************************************************************************
-Fuction        : SetH265NaluData
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int H265Handle::SetH265NaluData(unsigned char i_bNaluType,unsigned char i_bStartCodeLen,unsigned char *i_pbNaluData,int i_iNaluDataLen,T_MediaFrameInfo *m_ptFrame)
{
    int iRet = -1;
    unsigned char * pbNaluData = NULL;//去掉00 00 00 01
    int iNaluDataLen=0;
    E_MediaFrameType eFrameType = MEDIA_FRAME_TYPE_UNKNOW;
    
    if(NULL == i_pbNaluData || NULL == m_ptFrame)
    {
        MH_LOGE("SetH265NaluData NULL %d \r\n", iRet);
        return iRet;
    }
    
    if(m_ptFrame->pbFrameStartPos == NULL)
    {
        m_ptFrame->pbFrameStartPos = i_pbNaluData;
    }
    m_ptFrame->iFrameLen += i_iNaluDataLen;
    m_ptFrame->atNaluInfo[m_ptFrame->dwNaluCount].pbData= i_pbNaluData;
    m_ptFrame->atNaluInfo[m_ptFrame->dwNaluCount].dwDataLen= i_iNaluDataLen;
    m_ptFrame->dwNaluCount++;

    iNaluDataLen = i_iNaluDataLen-i_bStartCodeLen;//包括类型减开始码
    pbNaluData = i_pbNaluData+i_bStartCodeLen;

    if(i_bNaluType >= 0 && i_bNaluType <= 9)// p slice 片
    {
        eFrameType = MEDIA_FRAME_TYPE_VIDEO_P_FRAME;
    }
    else if(i_bNaluType >= 16 && i_bNaluType <= 21)// IRAP 等同于i帧
    {
        eFrameType = MEDIA_FRAME_TYPE_VIDEO_I_FRAME;
    }
    else if(i_bNaluType == 32)//VPS
    {
        memset(m_ptFrame->tVideoEncodeParam.abVPS,0,sizeof(m_ptFrame->tVideoEncodeParam.abVPS));
        m_ptFrame->tVideoEncodeParam.iSizeOfVPS= iNaluDataLen;//包括类型减4
        memcpy(m_ptFrame->tVideoEncodeParam.abVPS,pbNaluData,m_ptFrame->tVideoEncodeParam.iSizeOfVPS);
    }
    else if(i_bNaluType == 33)//SPS
    {
        memset(m_ptFrame->tVideoEncodeParam.abSPS,0,sizeof(m_ptFrame->tVideoEncodeParam.abSPS));
        m_ptFrame->tVideoEncodeParam.iSizeOfSPS= iNaluDataLen;//包括类型减4
        memcpy(m_ptFrame->tVideoEncodeParam.abSPS,pbNaluData,m_ptFrame->tVideoEncodeParam.iSizeOfSPS);

        if(0 == m_ptFrame->dwWidth && 0 == m_ptFrame->dwHeight)
        {
            T_RawH265Extradata tH265Extradata;
            memset(&tH265Extradata,0,sizeof(T_RawH265Extradata));
            m_oRawVideoHandle.SpsToH265Extradata(m_ptFrame->tVideoEncodeParam.abSPS,m_ptFrame->tVideoEncodeParam.iSizeOfSPS,&tH265Extradata);
            m_ptFrame->dwWidth = tH265Extradata.pic_width;
            m_ptFrame->dwHeight = tH265Extradata.pic_height;
        }
    }
    else if(i_bNaluType == 34)//PPS
    {
        memset(m_ptFrame->tVideoEncodeParam.abPPS,0,sizeof(m_ptFrame->tVideoEncodeParam.abPPS));
        m_ptFrame->tVideoEncodeParam.iSizeOfPPS= iNaluDataLen;//包括类型减4
        memcpy(m_ptFrame->tVideoEncodeParam.abPPS,pbNaluData,m_ptFrame->tVideoEncodeParam.iSizeOfPPS);
    }
    
    if(MEDIA_FRAME_TYPE_UNKNOW != eFrameType)
    {
        if(STREAM_TYPE_UNKNOW == m_ptFrame->eStreamType)//文件的时候才需要赋值，数据流的时候外部会赋值以外部为准
        {
            m_ptFrame->eFrameType = eFrameType;
            m_ptFrame->dwTimeStamp += VIDEO_H265_FRAME_INTERVAL;//VIDEO_H265_FRAME_INTERVAL*VIDEO_H264_SAMPLE_RATE/1000;
            m_ptFrame->dwSampleRate= VIDEO_H265_SAMPLE_RATE;
            m_ptFrame->eEncType = MEDIA_ENCODE_TYPE_H265;
        }
        iRet = 0;//解析出一帧则退出
    }
    return iRet;
}

