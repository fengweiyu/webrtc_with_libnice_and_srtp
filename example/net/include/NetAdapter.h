/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	Netdapter.h
* Description       : 	模块(对外)的依赖，比如一些系统相关的 定义 ，日志函数等，暂时先放到对外的include里
* Created           : 	2017.11.21.
* Author            : 	Yu Weifeng
* Function List     : 	
* Last Modified     : 	
* History           : 	
******************************************************************************/
#ifndef NET_ADAPTER_H
#define NET_ADAPTER_H

#ifndef TRUE
#define TRUE        0
#endif

#ifndef FALSE
#define FALSE       -1
#endif


#define  TCP_LOGW(...)     printf(__VA_ARGS__)
#define  TCP_LOGE(...)     printf(__VA_ARGS__)
#define  TCP_LOGD(...)     printf(__VA_ARGS__)
#define  TCP_LOGI(...)     printf(__VA_ARGS__)


#endif //NET_ADAPTER_H
