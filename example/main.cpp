/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       main.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/

#include "conductor.h"
#include "peer_connection_client.h"

#include "webrtc/base/ssladapter.h"
#include "webrtc/base/thread.h"

/*****************************************************************************
-Fuction        : PrintUsage
-Description    : PrintUsage
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static void PrintUsage(char *i_strProcName)
{
    printf("Usage: %s ip port\r\n",i_strProcName);
    printf("egg: %s 192.168.0.119 8888\r\n",i_strProcName);
}

int main(int argc, char* argv[]) 
{
    string strSeverIp;
    int dwPort;
    if(argc !=3)
    {
        PrintUsage(argv[0]);
    }
    else
    {
        if(strcmp(argv[1],"client")==0)
        {

        }
        else if(strcmp(argv[1],"server")==0)
        {
 
        }
        else
        {
            PrintUsage(argv[0]);
        }
    }


    rtc::InitializeSSL();

    PeerConnectionClient client;

    Conductor * pConductor = new Conductor(&client);


    pConductor->StartLogin(strSeverIp, dwPort);


    while(1)
    {
        sleep(1);
    }
        
    rtc::CleanupSSL();
    return 0;
}
