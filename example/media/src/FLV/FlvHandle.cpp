/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	FlvHandle.cpp
* Description		: 	FlvHandle operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "MediaAdapter.h"
#include "FlvHandle.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>


using std::cout;//需要<iostream>
using std::endl;


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
FlvHandle::FlvHandle()
{
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
FlvHandle::~FlvHandle()
{
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
int FlvHandle::SpsToH265Extradata(unsigned char *i_pbSpsData,unsigned short i_wSpsLen,T_FlvH265Extradata *o_ptH265Extradata)
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
int FlvHandle::DecodeEBSP(unsigned char* nalu, int bytes, unsigned char* sodb)
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
unsigned int FlvHandle::H264ReadBitByUE(unsigned char* data, int bytes, int* offset)
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
int FlvHandle::HevcProfileTierLevel(unsigned char* nalu, int bytes, unsigned char maxNumSubLayersMinus1,T_FlvH265Extradata* hevc)
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

