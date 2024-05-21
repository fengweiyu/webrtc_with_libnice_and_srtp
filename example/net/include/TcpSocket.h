/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	TcpSocket.h
* Description		: 	TcpSocket operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H


#include <map>
#include <stdio.h>
#include <string>
#include <chrono> //C++11引入的时间库,timeval linux和win定义(头文件)冲突，使用chrono替代

using std::map;
using std::string;
// 定义一个时间间隔类型，表示秒和微秒
using TimeVal = std::chrono::duration<long, std::micro>;
// 创建一个时间间隔对象
//     TimeVal timeInterval = TimeVal(1); // 1微秒
// 获取时间间隔的秒数和微秒数
//   long seconds = std::chrono::duration_cast<std::chrono::seconds>(timeInterval).count();
//  long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(timeInterval).count() % 1000000;
using std::chrono::milliseconds;


#define TCP_BODY_MTU    1448 //1518-4-14-20-32,1460MTU (1514-54 以太网帧最大1514-mac头14-ip头20-tcp头20)


/*****************************************************************************
-Class			: TcpSocket
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class TcpSocket
{
public:
	virtual int Init(string * i_strIP,unsigned short i_wPort)=0;//由于string是深拷贝,不是默认的只拷贝值的那种浅拷贝,所以可以不用指针,可以直接用或用引用均可
	virtual int Send(char * i_acSendBuf,int i_iSendLen,int i_iSocketFd)=0;
	virtual int Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd,milliseconds *i_pTime=NULL)=0;
	virtual void Close(int i_iSocketFd)=0;	
};


/*****************************************************************************
-Class			: TcpServer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class TcpServer : public TcpSocket
{
public:
	TcpServer();
	virtual ~TcpServer();
	int Init(string *i_strIP,unsigned short i_wPort);	
    int Accept();
	int Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd);
	int Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd,milliseconds *i_pTime=NULL);
	void Close(int i_iClientSocketFd);
	
private:
	int  m_iServerSocketFd;
};


/*****************************************************************************
-Class			: TcpClient
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class TcpClient : public TcpSocket
{
public:
	TcpClient();
	virtual ~TcpClient();
	int Init(string *i_strIP,unsigned short i_wPort);
	int Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd=0);
	int Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd=0,milliseconds *i_pTime=NULL);
	void Close(int i_iClientSocketFd=0);
    int GetClientSocket();
private:
	int  m_iClientSocketFd;
};

/*****************************************************************************
-Class			: TcpServer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class TcpServerEpoll
{
public:
	TcpServerEpoll();
	virtual ~TcpServerEpoll();
	int Init(unsigned short i_wPort,char * i_strIP = NULL);	
    int Accept();
	int Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd);
	int Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd,int i_iTimeoutMs = 5);
	void CloseClient(int i_iClientSocketFd);
	void CloseServer();
	
private:
	int  m_iServerSocketFd;
	int  m_iServerEpollFd;
	int  m_iClientEpollFd;
	int  m_iMaxListenSocket;
};

#endif
