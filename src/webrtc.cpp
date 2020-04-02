/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc.cpp
* Description           : 	
浏览器发起请求是offer,提供视频流播放是answer(服务端)
使用cJson组装和解析协议

* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "webrtc.h"
#include <sys/time.h>
#include "cJSON.h"


/*****************************************************************************
-Fuction        : WebRTC
-Description    : 底层三者关系比较乱，暂时回调后续考虑用设计模式
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRTC::WebRTC(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling):m_Libnice(i_strStunAddr,i_dwStunPort,i_eControlling),m_Srtp()
{
    m_pDtlsOnlyHandshake = NULL;
    memset(&m_tDtlsOnlyHandshakeCb,0,sizeof(T_DtlsOnlyHandshakeCb));
    m_tDtlsOnlyHandshakeCb.SendDataOut=&WebRTC::SendVideoDataOutCb;
    m_tDtlsOnlyHandshakeCb.pObj=&m_Libnice;
    m_pDtlsOnlyHandshake = new DtlsOnlyHandshake(m_tDtlsOnlyHandshakeCb);
    
    memset(&m_tLibniceCb,0,sizeof(T_LibniceCb));//改回调为传入对象的方法,回调必须带对象指针参数(回调函数要访问对象里的内容)
    m_tLibniceCb.Handshake= &WebRTC::HandshakeCb;//回调函数必须static,
    m_tLibniceCb.HandleRecvData= &WebRTC::HandleRecvDataCb;//如果函数内调用对象里的则必须改c函数传对象指针
    m_tLibniceCb.pObjCb=m_pDtlsOnlyHandshake;//不再libnice类里面做指针转换,这是防止底层模块相互依赖
    m_Libnice.SetCallback(&m_tLibniceCb);

    m_iSrtpCreatedFlag = 0;
    m_iSendReadyFlag = 0;
    m_pSctp = NULL;
    memset(&m_tSctpCb,0,sizeof(m_tSctpCb));
    m_tSctpCb.SendToOut = NULL;//没有应用数据通过sctp发暂时不用
    m_tSctpCb.RecvFromOut = NULL;
    m_pSctp = new Sctp(&m_tSctpCb);


    m_pDtlsOnlyHandshake->Init();
    m_pDtlsOnlyHandshake->Create();
}
/*****************************************************************************
-Fuction        : ~WebRTC
-Description    : ~WebRTC
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRTC::~WebRTC()
{
    m_iSrtpCreatedFlag = 0;
    
    delete m_pDtlsOnlyHandshake;
}
/*****************************************************************************
-Fuction        : Proc
-Description    : Proc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::Proc()
{
    return m_Libnice.LibniceProc();
}
/*****************************************************************************
-Fuction        : HandleCandidateMsg
-Description    : Offer消息必须是是在Candidate之前的，有这样的时序要求
这是webrtc抓包发现的，所以不符合这个时序则返回错误
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::HandleCandidateMsg(char * i_strCandidateMsg)
{
    int iRet = -1;
    cJSON * ptCandidateJson = NULL;
    cJSON * ptNode = NULL;
    char acRemoteCandidate[1024];
    
    if(NULL == i_strCandidateMsg)
    {
        printf("HandleOfferMsg NULL \r\n");
        return iRet;
    }
    ptCandidateJson = cJSON_Parse(i_strCandidateMsg);
    if(NULL != ptCandidateJson)
    {
        ptNode = cJSON_GetObjectItem(ptCandidateJson,"candidate");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(sizeof(acRemoteCandidate)<strlen(ptNode->valuestring))
            {
                printf("cJSON_GetObjectItem candidate err \r\n");
            }
            else
            {
                memset(acRemoteCandidate,0,sizeof(acRemoteCandidate));
                strncpy(acRemoteCandidate,ptNode->valuestring,sizeof(acRemoteCandidate));
                iRet=0;
            }
            ptNode = NULL;
        }
        cJSON_Delete(ptCandidateJson);
    }
    if(0 != iRet)
    {
    }
    else
    {
        iRet=m_Libnice.SetRemoteCandidateAndSDP(acRemoteCandidate);//
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : SendProtectedRtp
-Description    : SendProtectedRtp
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen)
{
    int iRet = -1;
    T_PolicyInfo tPolicyInfo;
    int iProtectRtpLen;
    if(NULL == i_acRtpBuf)
    {
        printf("SendProtectedRtp NULL \r\n");
        return iRet;
    }
    
    if(0 == m_iSrtpCreatedFlag)
    {
        memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
        m_pDtlsOnlyHandshake->GetPolicyInfo(&tPolicyInfo);
        m_Srtp.Create(tPolicyInfo.key, sizeof(tPolicyInfo.key), SRTP_SSRC_PROTECT);
        m_iSrtpCreatedFlag = 1;
    }
    iProtectRtpLen = i_iRtpBufLen;
    m_Srtp.ProtectRtp(i_acRtpBuf,&iProtectRtpLen,i_iRtpBufLen);
    iRet=m_Libnice.SendVideoData(i_acRtpBuf, iProtectRtpLen);

    return iRet;
}
/*****************************************************************************
-Fuction        : GetSendReadyFlag
-Description    : -1不可发送,0准备好通道可以发送
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GetSendReadyFlag()
{
    int iRet = -1;
    T_PolicyInfo tPolicyInfo;

    if(0!= m_Libnice.GetSendReadyFlag())
    {
        memset(&tPolicyInfo,0,sizeof(T_PolicyInfo));
        iRet=m_pDtlsOnlyHandshake->GetPolicyInfo(&tPolicyInfo);//获取成功表示通道协商成功了
    }
    if(iRet == 0 && m_iSendReadyFlag == 0)
    {
        m_pSctp->Init();
        m_iSendReadyFlag = 1;
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : 
// 'setup' attribute can take in an offer/answer exchange:
//       Offer      Answer
//      ________________
//      active     passive / holdconn
//      passive    active / holdconn
//      actpass    active / passive / holdconn
//      holdconn   holdconn



-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen)
{
	int iRet=-1;
    struct timeval tCreateTime;
    T_LocalCandidate tLocalCandidate;
    char strLocalFingerprint[160];
    char strCandidate[128];
    const char *strStreamType="video";
    string strSdpFmt("");
    int i=0;
    
    if (o_strSDP == NULL || NULL==i_ptVideoInfo || i_iSdpMaxLen <= 0) 
    {
		printf("GenerateLocalSDP NULL\r\n");
		return iRet;
    }
    memset(&tLocalCandidate,0,sizeof(T_LocalCandidate));
    m_Libnice.GetLocalCandidate(&tLocalCandidate);
	if(tLocalCandidate.iGatheringDoneFlag == 0)
	{
		printf("GenerateLocalSDP err\r\n");
		return iRet;
	}
    memset(&tCreateTime,0,sizeof(struct timeval));
    gettimeofday(&tCreateTime, NULL);
    memset(strLocalFingerprint,0,sizeof(strLocalFingerprint));
    m_pDtlsOnlyHandshake->GetLocalFingerprint(strLocalFingerprint,sizeof(strLocalFingerprint));
    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,给定了会话的发起者信息
        "s=ywf webrtc\r\n"//s=<sessionname> ;给定了Session Name
        "t=0 0\r\na=group:BUNDLE %d\r\n"//t=<start time><stop time> ;Time 与sdpMLineIndex sdpMid里的一致
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u UDP/TLS/RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//与sdpMLineIndex sdpMid里的一致
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=ice-options:trickle\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:actpass\r\n"
        "a=connection:new\r\n"
        "a=rtpmap:%d %s/%d\r\n"
        "a=ssrc:%ld cname:ywf%s\r\n"
        "a=ssrc:%ld msid:janus janusa0\r\n"
        "a=ssrc:%ld mslabel:janus\r\n"
        "a=ssrc:%ld label:janusa0\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {//目前为1个，多个也是失败
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp")&&NULL!= strstr(tLocalCandidate.strCandidateData[i],"."))
        {//后续可以优化为全部取出来放到数组中,sdp中的ip填0.0.0.0
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
            break;
        }
    }

    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,tLocalCandidate.strIP[i-1],
        i_ptVideoInfo->iID,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->ucRtpPayloadType,
        tLocalCandidate.strIP[i-1],//"0.0.0.0"还是失败，多个也是失败
        i_ptVideoInfo->iID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->ucRtpPayloadType,i_ptVideoInfo->pstrFormatName,i_ptVideoInfo->dwTimestampFrequency,
        tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec);

	return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : GenerateLocalSDP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::GenerateLocalCandidateMsg(T_VideoInfo *i_ptVideoInfo,char * o_strCandidateMsg,int i_iCandidateMaxLen)
{
    int iRet = -1;
    cJSON * ptNode = NULL;
    T_LocalCandidate tLocalCandidate;
    char strCandidate[128];
    int i;
    char strID[4];
    
    if(i_iCandidateMaxLen <= 0||NULL == o_strCandidateMsg ||NULL==i_ptVideoInfo)
    {
        printf("WebRtcOffer GenerateLocalMsg NULL \r\n");
        return iRet;
    }
    memset(&tLocalCandidate,0,sizeof(T_LocalCandidate));
    m_Libnice.GetLocalCandidate(&tLocalCandidate);
	if(tLocalCandidate.iGatheringDoneFlag == 0)
	{
		printf("GenerateLocalSDP err\r\n");
		return iRet;
	}
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {//选出最优的一个
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp")&&NULL!= strstr(tLocalCandidate.strCandidateData[i],"."))
        {//后续可以优化为全部取出来放到数组中,sdp中的ip填0.0.0.0
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"%s",
            tLocalCandidate.strCandidateData[i]);
            break;
        }
    }
    snprintf(strID,sizeof(strID),"%d",i_ptVideoInfo->iID);
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"candidate",strCandidate);
    cJSON_AddNumberToObject(root,"sdpMLineIndex",i_ptVideoInfo->iID);
    cJSON_AddStringToObject(root,"sdpMid",strID);
    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet=snprintf(o_strCandidateMsg,i_iCandidateMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);
    return iRet;
}


/*****************************************************************************
-Fuction        : HandshakeCb
-Description    : HandshakeCb
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
void WebRTC::HandshakeCb(void * pArg)
{
    DtlsOnlyHandshake *pDtlsOnlyHandshake=NULL;
    if(NULL!=pArg)//防止该静态函数对本对象的依赖,
    {//所以不直接用m_pDtlsOnlyHandshake->Handshake();
        pDtlsOnlyHandshake = (DtlsOnlyHandshake *)pArg;
        pDtlsOnlyHandshake->Handshake();
        
    }
}

/*****************************************************************************
-Fuction        : HandshakeCb
-Description    : HandshakeCb
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
void WebRTC::HandleRecvDataCb(char * i_acData,int i_iLen,void * pArg)
{
    DtlsOnlyHandshake *pDtlsOnlyHandshake=NULL;
    if (IsDtls(i_acData))
    {
        if(NULL!=pArg)//防止该静态函数对本对象的依赖,
        {//所以不直接用m_pDtlsOnlyHandshake->Handshake();
            pDtlsOnlyHandshake = (DtlsOnlyHandshake *)pArg;
            pDtlsOnlyHandshake->HandleRecvData(i_acData,i_iLen);
            
        }
    }
}

/*****************************************************************************
-Fuction        : SendDataOutCb
-Description    : SendDataOutCb
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRTC::SendVideoDataOutCb(char * i_acData,int i_iLen,void * pArg)
{
    int iRet=-1;
    Libnice *pLibnice=NULL;
    if(NULL!=pArg)//防止该静态函数对本对象的依赖,
    {//所以不直接用m_pDtlsOnlyHandshake->Handshake();
        pLibnice = (Libnice *)pArg;
        iRet=pLibnice->SendVideoData(i_acData,i_iLen);
    }
    printf("WebRTC::SendDataOutCb:%d\r\n",iRet);
    return iRet;
}

/*****************************************************************************
-Fuction        : IsDtls
-Description    : IsDtls
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
bool WebRTC::IsDtls(char *buf) 
{
    return ((*buf >= 20) && (*buf <= 64));
}


/*****************************************************************************
-Fuction        : WebRtcOffer
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcOffer::WebRtcOffer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling):WebRTC(i_strStunAddr,i_dwStunPort,i_eControlling)
{
}

/*****************************************************************************
-Fuction        : ~WebRtcOffer
-Description    : ~WebRtcOffer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcOffer::~WebRtcOffer()
{
}

/*****************************************************************************
-Fuction        : HandleMsg
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcOffer::HandleMsg(char * i_strAnswerMsg)
{
    int iRet = -1;
    cJSON * ptAnswerJson = NULL;
    cJSON * ptNode = NULL;
    char acRemoteSDP[5*1024];
    
    if(NULL == i_strAnswerMsg)
    {
        printf("WebRtcOffer HandleMsg NULL \r\n");
        return iRet;
    }
    ptAnswerJson = cJSON_Parse(i_strAnswerMsg);
    if(NULL != ptAnswerJson)
    {
        ptNode = cJSON_GetObjectItem(ptAnswerJson,"type");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(0 == strcmp(ptNode->valuestring,"answer"))
            {
                iRet = 0;
            }
            ptNode = NULL;
        }
        ptNode = cJSON_GetObjectItem(ptAnswerJson,"sdp");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(sizeof(acRemoteSDP)<strlen(ptNode->valuestring))
            {
                printf("cJSON_GetObjectItem sdp err \r\n");
            }
            memset(acRemoteSDP,0,sizeof(acRemoteSDP));
            strncpy(acRemoteSDP,ptNode->valuestring,sizeof(acRemoteSDP));
            ptNode = NULL;
        }
        cJSON_Delete(ptAnswerJson);
    }
    if(0 != iRet)
    {
    }
    else
    {
        iRet=m_Libnice.SaveRemoteSDP(acRemoteSDP);
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalMsg
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcOffer::GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strOfferMsg,int i_iOfferMaxLen)
{
    int iRet = -1;
    cJSON * ptNode = NULL;
    char acLocalSDP[5*1024];
    
    if(i_iOfferMaxLen <= 0||NULL == o_strOfferMsg ||NULL==i_ptVideoInfo)
    {
        printf("WebRtcOffer GenerateLocalMsg NULL \r\n");
        return iRet;
    }
    memset(acLocalSDP,0,sizeof(acLocalSDP));
    //m_Libnice.GetLocalSDP(acLocalSDP,sizeof(acLocalSDP));//local sdp缺少信息只好自己组包
    iRet=GenerateLocalSDP(i_ptVideoInfo,acLocalSDP,sizeof(acLocalSDP));
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"sdp",acLocalSDP);
    cJSON_AddStringToObject(root,"type","offer");
    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        snprintf(o_strOfferMsg,i_iOfferMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : 
// From RFC 4145, section-4.1, The following are the values that the
// 'setup' attribute can take in an offer/answer exchange:
//       Offer      Answer
//      ________________
//      active     passive / holdconn
//      passive    active / holdconn
//      actpass    active / passive / holdconn
//      holdconn   holdconn




-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcOffer::GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen)
{
	int iRet=-1;
    struct timeval tCreateTime;
    T_LocalCandidate tLocalCandidate;
    char strLocalFingerprint[160];
    char strCandidate[128];
    const char *strStreamType="video";
    string strSdpFmt("");
    int i=0;
    
    if (o_strSDP == NULL || NULL==i_ptVideoInfo || i_iSdpMaxLen <= 0) 
    {
		printf("GenerateLocalSDP NULL\r\n");
		return iRet;
    }
    memset(&tLocalCandidate,0,sizeof(T_LocalCandidate));
    m_Libnice.GetLocalCandidate(&tLocalCandidate);
	if(tLocalCandidate.iGatheringDoneFlag == 0)
	{
		printf("GenerateLocalSDP err\r\n");
		return iRet;
	}
    memset(&tCreateTime,0,sizeof(struct timeval));
    gettimeofday(&tCreateTime, NULL);
    memset(strLocalFingerprint,0,sizeof(strLocalFingerprint));
    m_pDtlsOnlyHandshake->GetLocalFingerprint(strLocalFingerprint,sizeof(strLocalFingerprint));
#if 0    
    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,给定了会话的发起者信息
        "s=ywf webrtc\r\n"//s=<sessionname> ;给定了Session Name
        "t=0 0\r\na=group:BUNDLE %d\r\n"//t=<start time><stop time> ;Time 与sdpMLineIndex sdpMid里的一致
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u UDP/TLS/RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//与sdpMLineIndex sdpMid里的一致
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=ice-options:trickle\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:actpass\r\n"
        "a=connection:new\r\n"
        "a=rtpmap:%d %s/%d\r\n"
        "a=ssrc:%ld cname:ywf%s\r\n"
        "a=ssrc:%ld msid:janus janusa0\r\n"
        "a=ssrc:%ld mslabel:janus\r\n"
        "a=ssrc:%ld label:janusa0\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {//目前为1个，多个也是失败
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp")&&NULL!= strstr(tLocalCandidate.strCandidateData[i],"."))
        {//后续可以优化为全部取出来放到数组中,sdp中的ip填0.0.0.0
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
            break;
        }
    }
    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,tLocalCandidate.strIP[i-1],
        i_ptVideoInfo->iID,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->ucRtpPayloadType,
        tLocalCandidate.strIP[i-1],//"0.0.0.0"还是失败，多个也是失败
        i_ptVideoInfo->iID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->ucRtpPayloadType,i_ptVideoInfo->pstrFormatName,i_ptVideoInfo->dwTimestampFrequency,
        tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec);
#endif

    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,给定了会话的发起者信息
        "s=ywf webrtc\r\n"//s=<sessionname> ;给定了Session Name
        "t=0 0\r\na=group:BUNDLE %d %d\r\n"//t=<start time><stop time> ;BUNDLE 与sdpMLineIndex sdpMid里的一致
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//与sdpMLineIndex sdpMid里的一致
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=ice-options:trickle\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:actpass\r\n"
        "a=connection:new\r\n"
        "a=rtpmap:%d %s/%d\r\n"
        "a=ssrc:%ld cname:ywf%s\r\n"
        "a=ssrc:%ld msid:janus janusa0\r\n"
        "a=ssrc:%ld mslabel:janus\r\n"
        "a=ssrc:%ld label:janusa0\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {
        memset(strCandidate,0,sizeof(strCandidate));
        snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
        strSdpFmt.append(strCandidate);
    }
    strSdpFmt.append("m=%s %u DTLS/SCTP\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//与sdpMLineIndex sdpMid里的加1
        "a=sendrecv\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:actpass\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {
        memset(strCandidate,0,sizeof(strCandidate));
        snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
        strSdpFmt.append(strCandidate);
    }
    
    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,"0.0.0.0",
        i_ptVideoInfo->iID,i_ptVideoInfo->iID+1,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->ucRtpPayloadType,
        "0.0.0.0",//"0.0.0.0"还是失败，多个也是失败
        i_ptVideoInfo->iID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->ucRtpPayloadType,i_ptVideoInfo->pstrFormatName,i_ptVideoInfo->dwTimestampFrequency,
        tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec,
        "application",i_ptVideoInfo->wPortNumForSDP,
        "0.0.0.0",
        i_ptVideoInfo->iID+1,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint);

	return iRet;
}
/*****************************************************************************
-Fuction        : WebRtcAnswer
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcAnswer::WebRtcAnswer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling):WebRTC(i_strStunAddr,i_dwStunPort,i_eControlling)
{
}

/*****************************************************************************
-Fuction        : ~~WebRtcAnswer
-Description    : ~~WebRtcAnswer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
WebRtcAnswer::~WebRtcAnswer()
{
}

/*****************************************************************************
-Fuction        : HandleOfferMsg
-Description    : 收到offer还是发wait消息，只有收到Candidate才发answer
消息
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::HandleMsg(char * i_strOfferMsg)
{
    int iRet = -1;
    cJSON * ptOfferJson = NULL;
    cJSON * ptNode = NULL;
    char acRemoteSDP[5*1024];
    
    if(NULL == i_strOfferMsg)
    {
        printf("HandleOfferMsg NULL \r\n");
        return iRet;
    }
    ptOfferJson = cJSON_Parse(i_strOfferMsg);
    if(NULL != ptOfferJson)
    {
        ptNode = cJSON_GetObjectItem(ptOfferJson,"type");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(0 == strcmp(ptNode->valuestring,"offer"))
            {
                iRet = 0;
            }
            ptNode = NULL;
        }
        ptNode = cJSON_GetObjectItem(ptOfferJson,"sdp");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            if(sizeof(acRemoteSDP)<strlen(ptNode->valuestring))
            {
                printf("cJSON_GetObjectItem sdp err \r\n");
            }
            memset(acRemoteSDP,0,sizeof(acRemoteSDP));
            strncpy(acRemoteSDP,ptNode->valuestring,sizeof(acRemoteSDP));
            ptNode = NULL;
        }
        cJSON_Delete(ptOfferJson);
    }
    if(0 != iRet)
    {
    }
    else
    {
        iRet=m_Libnice.SaveRemoteSDP(acRemoteSDP);
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalMsg
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strAnswerMsg,int i_iAnswerMaxLen)
{
    int iRet = -1;
    cJSON * ptNode = NULL;
    char acLocalSDP[5*1024];
    
    if(i_iAnswerMaxLen <= 0||NULL == o_strAnswerMsg ||NULL==i_ptVideoInfo)
    {
        printf("GetAnswerMsg NULL \r\n");
        return iRet;
    }
    memset(acLocalSDP,0,sizeof(acLocalSDP));
    //m_Libnice.GetLocalSDP(acLocalSDP,sizeof(acLocalSDP));//local sdp缺少信息只好自己组包
    iRet=GenerateLocalSDP(i_ptVideoInfo,acLocalSDP,sizeof(acLocalSDP));
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"sdp",acLocalSDP);
    cJSON_AddStringToObject(root,"type","answer");
    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        snprintf(o_strAnswerMsg,i_iAnswerMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);
    return iRet;
}

/*****************************************************************************
-Fuction        : GenerateLocalSDP
-Description    : 
// From RFC 4145, section-4.1, The following are the values that the
// 'setup' attribute can take in an offer/answer exchange:
//       Offer      Answer
//      ________________
//      active     passive / holdconn
//      passive    active / holdconn
//      actpass    active / passive / holdconn
//      holdconn   holdconn




-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int WebRtcAnswer::GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen)
{
	int iRet=-1;
    struct timeval tCreateTime;
    T_LocalCandidate tLocalCandidate;
    char strLocalFingerprint[160];
    char strCandidate[128];
    const char *strStreamType="video";
    string strSdpFmt("");
    int i=0;
    
    if (o_strSDP == NULL || NULL==i_ptVideoInfo || i_iSdpMaxLen <= 0) 
    {
		printf("GenerateLocalSDP NULL\r\n");
		return iRet;
    }
    memset(&tLocalCandidate,0,sizeof(T_LocalCandidate));
    m_Libnice.GetLocalCandidate(&tLocalCandidate);
	if(tLocalCandidate.iGatheringDoneFlag == 0)
	{
		printf("GenerateLocalSDP err\r\n");
		return iRet;
	}
    memset(&tCreateTime,0,sizeof(struct timeval));
    gettimeofday(&tCreateTime, NULL);
    memset(strLocalFingerprint,0,sizeof(strLocalFingerprint));
    m_pDtlsOnlyHandshake->GetLocalFingerprint(strLocalFingerprint,sizeof(strLocalFingerprint));
#if 0    
    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,给定了会话的发起者信息
        "s=ywf webrtc\r\n"//s=<sessionname> ;给定了Session Name
        "t=0 0\r\na=group:BUNDLE %d\r\n"//t=<start time><stop time> ;Time 与sdpMLineIndex sdpMid里的一致
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u UDP/TLS/RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=setup:active\r\n"
        "a=mid:%d\r\n"//与sdpMLineIndex sdpMid里的一致
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=ice-options:trickle\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:actpass\r\n"
        "a=connection:new\r\n"
        "a=rtpmap:%d %s/%d\r\n"
        "a=ssrc:%ld cname:ywf%s\r\n"
        "a=ssrc:%ld msid:janus janusa0\r\n"
        "a=ssrc:%ld mslabel:janus\r\n"
        "a=ssrc:%ld label:janusa0\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {//目前为1个，多个也是失败
        if(NULL!= strstr(tLocalCandidate.strCandidateData[i],"udp")&&NULL!= strstr(tLocalCandidate.strCandidateData[i],"."))
        {//后续可以优化为全部取出来放到数组中,sdp中的ip填0.0.0.0
            memset(strCandidate,0,sizeof(strCandidate));
            snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
            strSdpFmt.append(strCandidate);
            break;
        }
    }
    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,tLocalCandidate.strIP[i-1],
        i_ptVideoInfo->iID,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->ucRtpPayloadType,
        tLocalCandidate.strIP[i-1],//"0.0.0.0"还是失败，多个也是失败
        i_ptVideoInfo->iID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->ucRtpPayloadType,i_ptVideoInfo->pstrFormatName,i_ptVideoInfo->dwTimestampFrequency,
        tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec);
#endif
    strSdpFmt.assign("v=0\r\n"
        "o=- %ld%06ld %d IN IP4 %s\r\n"//o=<username><session id> <version> <network type> <address type><address> Origin ,给定了会话的发起者信息
        "s=ywf webrtc\r\n"//s=<sessionname> ;给定了Session Name
        "t=0 0\r\na=group:BUNDLE %d %d\r\n"//t=<start time><stop time> ;BUNDLE 与sdpMLineIndex sdpMid里的一致
        "a=msid-semantic: WMS ywf\r\n"
        "m=%s %u RTP/SAVPF %d\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//与sdpMLineIndex sdpMid里的一致
        "a=sendrecv\r\n"
        "a=rtcp-mux\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=ice-options:trickle\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:passive\r\n"
        "a=connection:new\r\n"
        "a=rtpmap:%d %s/%d\r\n"
        "a=ssrc:%ld cname:ywf%s\r\n"
        "a=ssrc:%ld msid:janus janusa0\r\n"
        "a=ssrc:%ld mslabel:janus\r\n"
        "a=ssrc:%ld label:janusa0\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {
        memset(strCandidate,0,sizeof(strCandidate));
        snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
        strSdpFmt.append(strCandidate);
    }
    strSdpFmt.append("m=%s %u DTLS/SCTP\r\n"
        "c=IN IP4 %s\r\n"
        "a=mid:%d\r\n"//与sdpMLineIndex sdpMid里的加1
        "a=sendrecv\r\n"
        "a=ice-ufrag:%s\r\n"
        "a=ice-pwd:%s\r\n"
        "a=fingerprint:sha-256 %s\r\n"
        "a=setup:passive\r\n");
    for(i=0;i<tLocalCandidate.iCurCandidateNum;i++)
    {
        memset(strCandidate,0,sizeof(strCandidate));
        snprintf(strCandidate,sizeof(strCandidate),"a=%s\r\n",tLocalCandidate.strCandidateData[i]);
        strSdpFmt.append(strCandidate);
    }
    
    iRet=snprintf(o_strSDP,i_iSdpMaxLen,strSdpFmt.c_str(),
        tCreateTime.tv_sec,tCreateTime.tv_usec,1,"0.0.0.0",
        i_ptVideoInfo->iID,i_ptVideoInfo->iID+1,
        strStreamType,i_ptVideoInfo->wPortNumForSDP,i_ptVideoInfo->ucRtpPayloadType,
        "0.0.0.0",//"0.0.0.0"还是失败，多个也是失败
        i_ptVideoInfo->iID,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint,
        i_ptVideoInfo->ucRtpPayloadType,i_ptVideoInfo->pstrFormatName,i_ptVideoInfo->dwTimestampFrequency,
        tCreateTime.tv_sec, strStreamType,tCreateTime.tv_sec, tCreateTime.tv_sec, tCreateTime.tv_sec,
        "application",i_ptVideoInfo->wPortNumForSDP,
        "0.0.0.0",
        i_ptVideoInfo->iID+1,
        tLocalCandidate.strUfrag, 
        tLocalCandidate.strPassword,
        strLocalFingerprint);


	return iRet;
}

