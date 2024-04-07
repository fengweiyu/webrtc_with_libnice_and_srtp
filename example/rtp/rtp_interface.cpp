/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       rtp_interface.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "rtp_interface.h"
#include "Rtp.h"


/*****************************************************************************
-Fuction		: RtpInterface
-Description	: RtpInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpInterface :: RtpInterface(char * i_strPath)
{
    m_pRtp = new Rtp(i_strPath);
}

/*****************************************************************************
-Fuction		: RtpInterface
-Description	: RtpInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpInterface :: RtpInterface(unsigned char **m_ppPackets,int i_iMaxPacketNum,char * i_strPath)
{
    m_pRtp = new Rtp(m_ppPackets,i_iMaxPacketNum,i_strPath);
}

/*****************************************************************************
-Fuction		: ~RtpInterface
-Description	: ~RtpInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
RtpInterface :: ~RtpInterface()
{
    if(NULL !=m_pRtp)
    {
        delete ((Rtp *)m_pRtp);
    }
}

/*****************************************************************************
-Fuction		: ~RtpInterface
-Description	: ~RtpInterface
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface :: DeInit(unsigned char **m_ppPackets,int i_iMaxPacketNum)
{
    if(NULL !=m_pRtp)
    {
        return ((Rtp *)m_pRtp)->DeInit(m_ppPackets,i_iMaxPacketNum);
    }
    return -1;
}

/*****************************************************************************
-Fuction		: GetRtpData
-Description	: GetRtpData
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,其他表示rtp包个数
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface :: GetRtpData(unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum)
{
    return ((Rtp *)m_pRtp)->GetRtpData(o_ppbPacketBuf,o_aiEveryPacketLen,i_iPacketBufMaxNum);
}

/*****************************************************************************
-Fuction		: GetRtpData
-Description	: GetRtpData
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,其他表示rtp包个数
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface :: RemoveH264EmulationBytes(unsigned char *o_pbNaluBuf,int i_iMaxNaluBufLen,unsigned char *i_pbNaluBuf,int i_iNaluLen)
{
    return -1;
}

/*****************************************************************************
-Fuction		: VideoHandle::GetSPS_PPS
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface::GetSPS_PPS(unsigned char *o_pbSpsBuf,int *o_piSpsBufLen,unsigned char *o_pbPpsBuf,int *o_piPpsBufLen)
{
    return ((Rtp *)m_pRtp)->GetSPS_PPS(o_pbSpsBuf,o_piSpsBufLen,o_pbPpsBuf,o_piPpsBufLen);
}


/*****************************************************************************
-Fuction		: VideoHandle::GetSPS_PPS
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface::GetSSRC(unsigned int *o_pdwVideoSSRC,unsigned int *o_pdwAudioSSRC)
{
	return ((Rtp *)m_pRtp)->GetSSRC(o_pdwVideoSSRC,o_pdwAudioSSRC);
}

/*****************************************************************************
-Fuction		: GetRtpPackets
-Description	: GetRtpPackets
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,其他表示rtp包个数
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface :: GetRtpPackets(void *m_ptFrame,unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iPacketBufMaxNum)
{
    return ((Rtp *)m_pRtp)->GetRtpPackets((T_MediaFrameInfo *)m_ptFrame,o_ppbPacketBuf,o_aiEveryPacketLen,i_iPacketBufMaxNum);
}
/*****************************************************************************
-Fuction		: Rtp
-Description	: //0 否 ,1 是
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface::IsRtp(char *buf, int size) 
{
    return ((Rtp *)m_pRtp)->IsRtp(buf,size);
}

/*****************************************************************************
-Fuction		: Rtp
-Description	: //0 否 ,1 是
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int RtpInterface::IsRtcp(char *buf, int size) 
{
    return ((Rtp *)m_pRtp)->IsRtcp(buf,size);
}

