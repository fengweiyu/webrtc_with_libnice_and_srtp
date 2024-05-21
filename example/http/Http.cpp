/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Http.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "Http.h"
#include "HttpCommon.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using std::cout;
using std::endl;

/*****************************************************************************
-Fuction		: Http
-Description	: Http
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Http :: Http()
{

}

/*****************************************************************************
-Fuction		: ~Http
-Description	: ~Http
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Http :: ~Http()
{
}

/*****************************************************************************
-Fuction		: Regex
-Description	: 正则表达式
.点				匹配除“\r\n”之外的任何单个字符
*				匹配前面的子表达式任意次。例如，zo*能匹配“z”，也能匹配“zo”以及“zoo”。*等价于o{0,}
				其中.*的匹配结果不会存储到结果数组里
(pattern)		匹配模式串pattern并获取这一匹配。所获取的匹配可以从产生的Matches集合得到
[xyz]			字符集合。匹配所包含的任意一个字符。例如，“[abc]”可以匹配“plain”中的“a”。
+				匹配前面的子表达式一次或多次(大于等于1次）。例如，“zo+”能匹配“zo”以及“zoo”，但不能匹配“z”。+等价于{1,}。
				//如下例子中不用+，默认是一次，即只能匹配到一个数字6
				
[A-Za-z0-9] 	26个大写字母、26个小写字母和0至9数字
[A-Za-z0-9+/=]	26个大写字母、26个小写字母0至9数字以及+/= 三个字符


-Input			: i_strPattern 模式串,i_strBuf待匹配字符串,
-Output 		: o_ptMatch 存储匹配串位置的数组,用于存储匹配结果在待匹配串中的下标范围
//数组0单元存放主正则表达式匹配结果的位置,即所有正则组合起来的匹配结果，后边的单元依次存放子正则表达式匹配结果的位置
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/01	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Http::Regex(const char *i_strPattern,string *i_strBuf,smatch &o_Match)
{
    char acErrBuf[256];
    int iRet=-1;

    // 定义正则表达式模式
    regex Pattern(i_strPattern);
    // 使用 std::regex_search 查找匹配的内容
    if (std::regex_search(*i_strBuf, o_Match, Pattern)) //某次匹配失败，std::smatch对象将不会保存上一次的匹配结果
    {//o_Match指向的子串内存在i_strBuf中，所以需要保证i_strBuf的作用域，即i_strBuf不能先被释放掉
        // 输出第一个匹配到的子串
        std::cout << "Match found: " <<o_Match.size()<< o_Match.str() << std::endl;

        // 输出所有匹配到的子串
        for (size_t i = 0; i < o_Match.size(); ++i) 
        {
            std::string matchStr = o_Match[i].str(); // 将 std::sub_match 转换为字符串
            std::cout << "Match " << i << ": " << matchStr << std::endl;
        }
        iRet = 0;
    } 
    else 
    {
        std::cout << "No match found." << std::endl;
    }
    
    return iRet;
}


