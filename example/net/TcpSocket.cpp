/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	TcpSocket.cpp
* Description		: 	TcpSocket  server operation center
TcpServerEpoll 后续改名TcpServer
TcpServer后续改名TcpServerSelect
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/tcp.h> 
#include <sys/epoll.h>

#include "TcpSocket.h"
#include "NetAdapter.h"




using std::cout;
using std::endl;
using std::string;


/*****************************************************************************
-Fuction		: TcpServer
-Description	: TcpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpServer::TcpServer()
{
    m_iServerSocketFd = -1;
}

/*****************************************************************************
-Fuction		: ~TcpServer
-Description	: ~TcpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpServer::~TcpServer()
{
	if(m_iServerSocketFd >= 0)
	{
        close(m_iServerSocketFd);  
	}
}

/*****************************************************************************
-Fuction		: Init
-Description	: Init
-Input			: 
-Output 		: 
-Return 		: 失败返回-1，成功返回0
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServer::Init(string *i_strIP,unsigned short i_wPort)
{
	int iRet=-1;
	int iSocketFd=-1;
	unsigned short wPort=i_wPort;
	struct sockaddr_in tServerAddr;

	if(m_iServerSocketFd !=-1)
	{
	    iRet=0;
	}
	else
	{
        iSocketFd=socket(AF_INET,SOCK_STREAM,0);
        if(iSocketFd<0)
        {
            perror(NULL);
            TCP_LOGE("TcpServer Init err");
        }
        else
        {
            // Set Sockfd NONBLOCK //暂时使用阻塞形式的
            //iSocketStatus=fcntl(iSocketFd, F_GETFL, 0);
            //fcntl(iSocketFd, F_SETFL, iSocketStatus | O_NONBLOCK);    
            int tmp = 1;
            if (setsockopt(iSocketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp)) < 0) 
            {
                TCP_LOGE("TcpServer setsockopt err");
                close(iSocketFd);
                iSocketFd=-1;
                return iRet;
            }
            int chOpt = 1;
            if (setsockopt(iSocketFd, IPPROTO_TCP, TCP_NODELAY, (char *)&chOpt, sizeof(chOpt)) < 0) 
            {
                perror(NULL);
                TCP_LOGE("TcpServer setsockopt TCP_NODELAY err");
                close(iSocketFd);
                iSocketFd=-1;
                return iRet;
            }
            int flags = fcntl(iSocketFd, F_GETFL, 0);//设置非阻塞
            if (flags < 0) 
            {
                TCP_LOGE("Error getting socket flags");
                close(iSocketFd);
                iSocketFd=-1;
                return iRet;
            }
            if (fcntl(iSocketFd, F_SETFL, flags | O_NONBLOCK) < 0) //设置非阻塞
            {
                TCP_LOGE("Error setting socket to non-blocking mode");
                close(iSocketFd);
                iSocketFd=-1;
                return iRet;
            }
            
            // Connect to server
            //this->GetIpAndPort(i_URL,&IP,&wPort);
            bzero(&tServerAddr, sizeof(tServerAddr));
            tServerAddr.sin_family = AF_INET;
            tServerAddr.sin_port = htons(wPort);//一般是554
            if(i_strIP == NULL)
            {
                tServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);//也可以使用htonl(INADDR_ANY),表示使用本机的所有IP
            }
            else
            {
                tServerAddr.sin_addr.s_addr = inet_addr(i_strIP->c_str());//也可以使用htonl(INADDR_ANY),表示使用本机的所有IP
            }
            if(bind(iSocketFd,(struct sockaddr*)&tServerAddr,sizeof(tServerAddr))<0)
            {
                perror(NULL);
                TCP_LOGE("TcpServer bind err");
                close(iSocketFd);
                iSocketFd=-1;
            }
            else
            {
                //当前服务器ip和端口号最大允许连接的客户端个数为100
                if(listen(iSocketFd,100)<0) //等待连接个数,也就是允许连接的客户端个数100
                {
                    perror(NULL);
                    TCP_LOGE("TcpServer listen err");
                    close(iSocketFd);
                    iSocketFd=-1;
                }
                else
                {
                    m_iServerSocketFd=iSocketFd;
                    iRet=0;
                }
            }
	    }
    }
	return iRet;
}

/*****************************************************************************
-Fuction		: Accept
-Description	: 非阻塞
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServer::Accept()
{
    int iClientSocketFd=-1;
    struct sockaddr_un tClientAddr; 
    socklen_t iLen=0;


	if(m_iServerSocketFd < 0)
	{
        TCP_LOGE("(m_iServerSocketFd < 0\r\n");  
        return -1;
	}
    
    //have connect request use accept  
    iLen=sizeof(tClientAddr);  
    iClientSocketFd=accept(m_iServerSocketFd,(struct sockaddr*)&tClientAddr,&iLen);  //这里会等待客户端连接
    if(iClientSocketFd<0)  
    {  
        //TCP_LOGE("cannot accept client connect request");  //非阻塞在没有连接的情况下，就会返回负数
        //close(m_iServerSocketFd);  
        //unlink(UNIX_DOMAIN);  //错误： ‘UNIX_DOMAIN’在此作用域中尚未声明
    } 
	else
	{
	}
	return iClientSocketFd;
}

/*****************************************************************************
-Fuction		: Send
-Description	: 阻塞的操作形式
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServer::Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd)
{
	int iRet=-1;
	if(i_acSendBuf==NULL ||i_iSendLen<=0)
	{
        TCP_LOGE("Send err");
	}
	else
	{
        iRet=send(i_iClientSocketFd,i_acSendBuf,i_iSendLen,0);
        if(iRet<0)
        {
            close(i_iClientSocketFd);
        }
        else
        {
            iRet=0;
        }
        string strSend(i_acSendBuf);
        TCP_LOGD("Send : %d\r\n",i_iSendLen);
	}

	return iRet;
}

/*****************************************************************************
-Fuction		: Recv
-Description	: 阻塞的操作形式
-Input			: 
-Output 		: o_piRecvLen 返回小于要读的数据，则表示读完了，默认超时1s
-Return 		: 
iRet=-1;表示套接字出错，
iRet = 0; 表示没有出错，包括超时
o_piRecvLen 表示接收到的长度
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServer::Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd,milliseconds *i_pTime)
{
    int iRecvLen=-1;
    int iRet=-1;
    fd_set tReadFds;
    timeval tTimeValue;
    char *pcRecvBuf=o_acRecvBuf;
    int iLeftRecvLen=i_iRecvBufMaxLen;

    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen ||i_iRecvBufMaxLen <= 0)
    {
        TCP_LOGE("TcpServer::Recv NULL");
        return iRet;
    }   
    memset(o_acRecvBuf,0,i_iRecvBufMaxLen);;
    while(iLeftRecvLen > 0)
    {
        FD_ZERO(&tReadFds); //清空描述符集合    
        FD_SET(i_iClientSocketFd, &tReadFds); //设置描述符集合
        tTimeValue.tv_sec      = 1;//超时时间，超时返回错误
        tTimeValue.tv_usec     = 0;
        if(NULL != i_pTime)
        {
            // 获取毫秒时间间隔的值
            long long millisecondsValue = i_pTime->count();
            tTimeValue.tv_sec      = millisecondsValue/1000;//超时时间，超时返回错误
            tTimeValue.tv_usec     = millisecondsValue%1000*1000;
        }
        iRet = select(i_iClientSocketFd + 1, &tReadFds, NULL, NULL, &tTimeValue);//调用select（）监控函数//NULL 一直等到有变化
        if(iRet<0)  
        {
            TCP_LOGE("select Recv err\n");  
            close(i_iClientSocketFd);
            iRet=-1;
            break;
        }
        else if(0 == iRet)
        {
            //perror("select Recv timeout\r\n");
            iRet = 0;
            break;
        }
        else
        {
        }
        if (FD_ISSET(i_iClientSocketFd, &tReadFds))   //测试fd1是否可读  
        {
            iRecvLen=recv(i_iClientSocketFd,pcRecvBuf,iLeftRecvLen,0);  
            if(iRecvLen<=0)
            {
                if(errno != EINTR)
                {
                    TCP_LOGE("errno Recv err%d\r\n",iRecvLen); 
                    perror("errno"); 
                    iRet=-1;
                    break;
                }
            }
            else
            {
                iLeftRecvLen = iLeftRecvLen-iRecvLen;
                pcRecvBuf += iRecvLen;
                iRet = 0;
            }
        }
        else
        {
            TCP_LOGE("errno FD_ISSET err"); 
            iRet=-1;
        	break;
        }
    }
    if(iLeftRecvLen < i_iRecvBufMaxLen)
    {
        string strRecv(o_acRecvBuf);
        *o_piRecvLen = i_iRecvBufMaxLen - iLeftRecvLen;
        TCP_LOGD("SvcRecv :%d\r\n",*o_piRecvLen);
    }
    else
    {
        //TCP_LOGE("Recv err:"<<iRecvAllLen);
    }
    return iRet;
}


/*****************************************************************************
-Fuction		: Close
-Description	: Close
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void TcpServer::Close(int i_iClientSocketFd)
{
	if(i_iClientSocketFd!=-1)
	{
		close(i_iClientSocketFd);
	}
	else
	{
		TCP_LOGE("Close err:%d",i_iClientSocketFd);
	}
}

/*****************************************************************************
-Fuction		: TcpClient
-Description	: TcpClient
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpClient ::TcpClient()
{
    m_iClientSocketFd = -1;
}

/*****************************************************************************
-Fuction		: ~TcpSocket
-Description	: ~TcpSocket
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpClient ::~TcpClient()
{
    Close();
}

/*****************************************************************************
-Fuction		: Init
-Description	: Init
-Input			: 
-Output 		: 
-Return 		: 失败返回-1，成功返回0
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpClient::Init(string *i_strIP,unsigned short i_wPort)
{
	int iRet=-1;
	int iSocketFd=-1;
	struct sockaddr_in tServerAddr;

	
	if(i_strIP==NULL)
	{
		perror(NULL);
		TCP_LOGE("TcpSocketInit NULL");
	}

	iSocketFd=socket(AF_INET,SOCK_STREAM,0);
	if(iSocketFd<0)
	{
		perror(NULL);
		TCP_LOGE("TcpSocketInit err");
	}
	else
	{
		// Set Sockfd NONBLOCK //暂时使用阻塞形式的
		//iSocketStatus=fcntl(iSocketFd, F_GETFL, 0);
		//fcntl(iSocketFd, F_SETFL, iSocketStatus | O_NONBLOCK);	
        int tmp = 1;
        if (setsockopt(iSocketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp)) < 0) 
        {
            TCP_LOGE("TcpClient setsockopt err");
            close(iSocketFd);
            iSocketFd=-1;
            return iRet;
        }
		// Connect to server
		bzero(&tServerAddr, sizeof(tServerAddr));
		tServerAddr.sin_family = AF_INET;
		tServerAddr.sin_port = htons(i_wPort);
		tServerAddr.sin_addr.s_addr = inet_addr(i_strIP->c_str());
		if(connect(iSocketFd, (struct sockaddr *)&tServerAddr, sizeof(tServerAddr)) < 0 && errno != EINPROGRESS) 
		{
			perror(NULL);
			TCP_LOGE("TcpSocket connect err");
			close(iSocketFd);
			iSocketFd=-1;
		}
		else
		{
			//test
			m_iClientSocketFd=iSocketFd;
			iRet=0;
		}
	}
	return iRet;


}

/*****************************************************************************
-Fuction		: Send
-Description	: 阻塞的操作形式
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpClient::Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd)
{
	int iRet=-1;
	int iClientSocketFd=-1;

	
	if(i_acSendBuf==NULL ||i_iSendLen<=0)
	{
        TCP_LOGE("Send err");
        return -1;
	}
    if(i_iClientSocketFd<=0)
    {
        iClientSocketFd=m_iClientSocketFd;
    }
    else
    {
        iClientSocketFd=i_iClientSocketFd;
    }
    iRet=send(iClientSocketFd,i_acSendBuf,i_iSendLen,0);
    if(iRet<0)
    {
        close(iClientSocketFd);
    }
    else
    {
        iRet=0;
    }
    string strSend(i_acSendBuf);
    TCP_LOGD("Send :%d\r\n",i_iSendLen);
	
	return iRet;
}

/*****************************************************************************
-Fuction		: Recv
-Description	: 阻塞的操作形式
-Input			: 
-Output 		: o_piRecvLen 返回小于要读的数据，则表示读完了，默认超时1s
-Return 		: 
iRet=-1;表示套接字出错，
iRet = 0; 表示没有出错，包括超时
o_piRecvLen 表示接收到的长度
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpClient::Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd,milliseconds *i_pTime)
{
    int iRecvLen=-1;
    int iRet=-1;
    fd_set tReadFds;
    timeval tTimeValue;
    char *pcRecvBuf=o_acRecvBuf;
    int iLeftRecvLen=i_iRecvBufMaxLen;
	int iClientSocketFd=-1;

    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen ||i_iRecvBufMaxLen <= 0)
    {
        TCP_LOGE("TcpClient::Recv NULL");
        return iRet;
    } 
    if(i_iClientSocketFd<=0)
    {
        iClientSocketFd=m_iClientSocketFd;
    }
    else
    {
        iClientSocketFd=i_iClientSocketFd;
    }
    while(iLeftRecvLen > 0)
    {
        FD_ZERO(&tReadFds); //清空描述符集合    
        FD_SET(iClientSocketFd, &tReadFds); //设置描述符集合
        tTimeValue.tv_sec  =1;//超时时间，超时返回错误
        tTimeValue.tv_usec = 0;
        if(NULL != i_pTime)
        {
            // 获取毫秒时间间隔的值
            long long millisecondsValue = i_pTime->count();
            tTimeValue.tv_sec      = millisecondsValue/1000;//超时时间，超时返回错误
            tTimeValue.tv_usec     = millisecondsValue%1000*1000;
        }
        iRet = select(iClientSocketFd + 1, &tReadFds, NULL, NULL, &tTimeValue);//调用select（）监控函数//NULL 一直等到有变化
        if(iRet<0)  
        {
            TCP_LOGE("select Recv err\n");  
            close(iClientSocketFd);
            iRet=-1;
            break;
        }
        else if(0 == iRet)
        {
            //perror("select Recv timeout\r\n");
            iRet = 0;
            break;
        }
        else
        {
        }
        if (FD_ISSET(iClientSocketFd, &tReadFds))   //测试fd1是否可读  
        {
            iRecvLen=recv(i_iClientSocketFd,pcRecvBuf,iLeftRecvLen,0);  
            if(iRecvLen<=0)
            {
                if(errno != EINTR)
                {
                    iRet=-1;
                    break;
                }
            }
            else
            {
                iLeftRecvLen = iLeftRecvLen-iRecvLen;
                pcRecvBuf += iRecvLen;
                iRet = 0;
            }
        }
        else
        {
            iRet=-1;
        	break;
        }
    }
    if(iLeftRecvLen < i_iRecvBufMaxLen)
    {
        string strRecv(o_acRecvBuf);
        *o_piRecvLen = i_iRecvBufMaxLen - iLeftRecvLen;
        TCP_LOGD("Recv :%d\r\n",*o_piRecvLen);
    }
    else
    {
        //TCP_LOGE("Recv err:"<<iRecvAllLen);
    }
    return iRet;
}


/*****************************************************************************
-Fuction		: Close
-Description	: Close
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void TcpClient::Close(int i_iClientSocketFd)
{
	int iClientSocketFd=-1;

    if(i_iClientSocketFd<=0)
    {
        iClientSocketFd=m_iClientSocketFd;
    }
    else
    {
        iClientSocketFd=i_iClientSocketFd;
    }
	if(iClientSocketFd!=-1)
	{
		close(iClientSocketFd);
		m_iClientSocketFd=-1;
	}
	else
	{
		TCP_LOGE("Close err:%d",iClientSocketFd);
	}
}

/*****************************************************************************
-Fuction        : GetClientSocket
-Description    : GetClientSocket
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TcpClient::GetClientSocket()
{
    return m_iClientSocketFd;
}

/*****************************************************************************
-Fuction		: TcpServer
-Description	: TcpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpServerEpoll::TcpServerEpoll()
{
    m_iServerSocketFd = -1;
    m_iServerEpollFd = -1;
    m_iClientEpollFd = -1;
    m_iMaxListenSocket = 1000;
}

/*****************************************************************************
-Fuction		: ~TcpServer
-Description	: ~TcpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpServerEpoll::~TcpServerEpoll()
{
    CloseServer();
}

/*****************************************************************************
-Fuction		: Init
-Description	: Init
-Input			: 
-Output 		: 
-Return 		: 失败返回-1，成功返回0
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServerEpoll::Init(unsigned short i_wPort,char * i_strIP)
{
	int iRet=-1;
	int iSocketFd=-1;
	unsigned short wPort=i_wPort;
	struct sockaddr_in tServerAddr;

	if(m_iServerSocketFd < 0)
	{
        iSocketFd=socket(AF_INET,SOCK_STREAM,0);
        if(iSocketFd<0)
        {
            perror(NULL);
            TCP_LOGE("TcpSocketInit err");
        }
        else
        {
            // Set Sockfd NONBLOCK //暂时使用阻塞形式的
            //iSocketStatus=fcntl(iSocketFd, F_GETFL, 0);
            //fcntl(iSocketFd, F_SETFL, iSocketStatus | O_NONBLOCK);    
            int tmp = 1;
            if (setsockopt(iSocketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp)) < 0) 
            {
                TCP_LOGE("TcpSocket setsockopt err");
                close(iSocketFd);
                iSocketFd=-1;
                return iRet;
            }
            int chOpt = 1;
            if (setsockopt(iSocketFd, IPPROTO_TCP, TCP_NODELAY, (char *)&chOpt, sizeof(chOpt)) < 0) 
            {
                perror(NULL);
                TCP_LOGE("TcpSocket setsockopt TCP_NODELAY err");
                close(iSocketFd);
                iSocketFd=-1;
                return iRet;
            }
            // Connect to server
            //this->GetIpAndPort(i_URL,&IP,&wPort);
            bzero(&tServerAddr, sizeof(tServerAddr));
            tServerAddr.sin_family = AF_INET;
            tServerAddr.sin_port = htons(wPort);//一般是554
            if(NULL == i_strIP)
                tServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);//也可以使用htonl(INADDR_ANY),表示使用本机的所有IP
            else
                tServerAddr.sin_addr.s_addr = inet_addr(i_strIP);//也可以使用htonl(INADDR_ANY),表示使用本机的所有IP
            if(bind(iSocketFd,(struct sockaddr*)&tServerAddr,sizeof(tServerAddr))<0)
            {
                perror(NULL);
                TCP_LOGE("TcpSocket bind err");
                close(iSocketFd);
                iSocketFd=-1;
            }
            else
            {
                //当前服务器ip和端口号最大允许连接的客户端个数为1000,一般1024
                m_iMaxListenSocket = 1000;//内核默认的SOMAXCONN 是128，一般不够用
                if(listen(iSocketFd,m_iMaxListenSocket)<0) 
                {
                    perror(NULL);
                    TCP_LOGE("TcpSocket listen err\r\n");
                    close(iSocketFd);
                    iSocketFd=-1;
                }
                else
                {
                    m_iServerSocketFd=iSocketFd;
                    iRet=0;
                }
            }
	    }
    }
    if(m_iServerSocketFd >= 0 && m_iServerEpollFd < 0)
    {
        struct epoll_event ev;
        m_iServerEpollFd = epoll_create(m_iMaxListenSocket);//size用来告诉内核这个监听的数目一共有多大，新版内核一般是大于0就行
        ev.events = EPOLLIN|EPOLLET;//采用ET模式只接收了一部分数据就再也得不到通知了
        ev.data.fd = m_iServerSocketFd;
        iRet = epoll_ctl(m_iServerEpollFd, EPOLL_CTL_ADD, m_iServerSocketFd, &ev);
        if(iRet < 0)
        {
            TCP_LOGE("epoll_ctl err \r\n");
            CloseServer();
            iRet = -1;
        }
        else
        {
            iRet = 0;
        }
    }
	return iRet;
}

/*****************************************************************************
-Fuction		: Accept
-Description	: 非阻塞的操作形式
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServerEpoll::Accept()
{
    int iClientSocketFd=-1;
    struct sockaddr_un tClientAddr; 
    socklen_t iLen=0;
    struct epoll_event ev;
	struct epoll_event events[64];
    int iEvents = 0;
    int i = 0;
    int iRet = -1;
    
    if(m_iServerSocketFd < 0 || m_iServerEpollFd < 0)
    {
        TCP_LOGE("Accept err no init\r\n");
        return iRet;
    }
    
    iEvents = epoll_wait(m_iServerEpollFd, events, sizeof(events)/sizeof(struct epoll_event), 0);
    for(i = 0; i < iEvents; i++)
    {
        if(m_iServerSocketFd == events[i].data.fd)
        {
            if(events[i].events & EPOLLIN)//可读
            {
                iLen=sizeof(tClientAddr); //have connect request use accept  
                iClientSocketFd=accept(m_iServerSocketFd,(struct sockaddr*)&tClientAddr,&iLen);  //这里会等待客户端连接
                if(iClientSocketFd<0)  
                {  
                    TCP_LOGE("cannot accept client connect request");  
                } 
                else
                {
                    if(m_iClientEpollFd < 0)
                    {
                        m_iClientEpollFd = epoll_create(m_iMaxListenSocket);//size用来告诉内核这个监听的数目一共有多大，新版内核一般是大于0就行
                    }
                    ev.events = EPOLLIN;//LT模式是只要有数据没有处理就会一直通知下去的.
                    ev.data.fd = iClientSocketFd;
                    iRet = epoll_ctl(m_iClientEpollFd, EPOLL_CTL_ADD, iClientSocketFd, &ev);
                    if(iRet < 0)
                    {
                        TCP_LOGE("epoll_ctl EPOLL_CTL_ADD err \r\n");
                        close(iClientSocketFd);
                        iRet = -1;
                    }
                    else
                    {
                        iRet = iClientSocketFd;
                    }
                }
                break;
            }
        }
    }
	return iRet;
}

/*****************************************************************************
-Fuction		: Send
-Description	: 阻塞的操作形式
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServerEpoll::Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd)
{
	int iRet=-1;
	if(i_acSendBuf==NULL ||i_iSendLen<=0)
	{
        TCP_LOGE("Send err");
	}
	else
	{
        iRet=send(i_iClientSocketFd,i_acSendBuf,i_iSendLen,0);
	}

	return iRet;
}

/*****************************************************************************
-Fuction		: Recv
-Description	: 非阻塞的操作形式
-Input			: i_iTimeoutMs 默认5ms
-Output 		: o_piRecvLen 返回小于要读的数据，则表示读完了，
-Return 		: 
iRet=-1;表示套接字出错，
iRet = 0; 表示没有出错，包括超时
o_piRecvLen 表示接收到的长度
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServerEpoll::Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd,int i_iTimeoutMs)
{
    int iRecvLen=-1;
    int iRet=-1;
    struct epoll_event ev;
	struct epoll_event events[64];
    int iEvents = 0;
    int i = 0;
    char *pcRecvBuf=o_acRecvBuf;
    int iLeftRecvLen=i_iRecvBufMaxLen;

    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen ||i_iRecvBufMaxLen <= 0)
    {
        TCP_LOGE("TcpServer::Recv NULL");
        return iRet;
    }   
    memset(o_acRecvBuf,0,i_iRecvBufMaxLen);;
    while(iLeftRecvLen > 0)
    {//m_iEpollFd同时被accept和recv关注，如果这里一直循环到accept的事件则可能无法退出循环 
        iEvents = epoll_wait(m_iClientEpollFd, events, sizeof(events)/sizeof(struct epoll_event), i_iTimeoutMs);//故超时时间最好大于0以便让出cpu(后续优化)
        if(iEvents < 0)//超时时间0，-1阻塞单位ms 
        {
            TCP_LOGE("epoll_wait Recv err\n");  
            iRet=-1;
            break;
        }
        else if(0 == iEvents)//如果自己的事件，没有accept等其他事件，则这里会退出，则超时时间可填0
        {
            iRet = 0;//perror("epoll_wait Recv timeout\r\n");
            break;
        }
        for(i = 0; i < iEvents; i++)
        {
            if(i_iClientSocketFd == events[i].data.fd && events[i].events & EPOLLIN)//可读
            {
                iRecvLen=recv(i_iClientSocketFd,pcRecvBuf,iLeftRecvLen,0);  
                if(iRecvLen<=0)
                {
                    if(errno != EINTR)
                    {
                        TCP_LOGE("errno Recv err%d\r\n",iRecvLen); 
                        iRet=-1;
                        return iRet;
                    }
                }
                else
                {
                    iLeftRecvLen = iLeftRecvLen-iRecvLen;
                    pcRecvBuf += iRecvLen;
                    iRet = 0;
                }
                break;
            }
        }
    }
    if(iLeftRecvLen < i_iRecvBufMaxLen)
    {
        *o_piRecvLen = i_iRecvBufMaxLen - iLeftRecvLen;
    }
    return iRet;
}


/*****************************************************************************
-Fuction		: Close
-Description	: Close
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void TcpServerEpoll::CloseClient(int i_iClientSocketFd)
{
	if(m_iClientEpollFd != -1 && i_iClientSocketFd >= 0)
	{
        epoll_ctl(m_iClientEpollFd, EPOLL_CTL_DEL, i_iClientSocketFd,NULL);
	}
	if(i_iClientSocketFd >= 0)
	{
        close(i_iClientSocketFd);
	}
}


/*****************************************************************************
-Fuction		: CloseServer
-Description	: Close
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void TcpServerEpoll::CloseServer()
{
	if(m_iServerEpollFd != -1 && m_iServerSocketFd != -1)
	{
        epoll_ctl(m_iServerEpollFd, EPOLL_CTL_DEL, m_iServerSocketFd,NULL);
	}
	if(m_iClientEpollFd != -1)
	{
        close(m_iClientEpollFd);
        m_iClientEpollFd = -1;
	}
	if(m_iServerEpollFd != -1)
	{
        close(m_iServerEpollFd);
        m_iServerEpollFd = -1;
	}
	if(m_iServerSocketFd != -1)
	{
        close(m_iServerSocketFd);
        m_iServerSocketFd = -1;
	}
}
