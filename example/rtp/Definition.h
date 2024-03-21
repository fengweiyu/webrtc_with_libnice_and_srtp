/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	Definition.h
* Description       : 	Definition operation center
* Created           : 	2017.11.21.
* Author            : 	Yu Weifeng
* Function List     : 	
* Last Modified     : 	
* History           : 	
******************************************************************************/
#ifndef DEFINITION_H
#define DEFINITION_H

#define TRUE        0
#define FALSE       -1


#if 0
#define  MH_LOGW(...)     logi(MediaHandle) << lformat(MediaHandle,__VA_ARGS__) << lend 
#define  MH_LOGE(...)     loge(MediaHandle) << lformat(MediaHandle,__VA_ARGS__) << lend
#define  MH_LOGD(...)     logd(MediaHandle) << lformat(MediaHandle,__VA_ARGS__) << lend
#define  MH_LOGI(...)     logi(MediaHandle) << lformat(MediaHandle,__VA_ARGS__) << lend
#else
#define  MH_LOGW(...)     printf(__VA_ARGS__)
#define  MH_LOGE(...)     printf(__VA_ARGS__)
#define  MH_LOGD(...)     printf(__VA_ARGS__)
#define  MH_LOGI(...)     printf(__VA_ARGS__)
#endif

#endif //DEFINITION_H
