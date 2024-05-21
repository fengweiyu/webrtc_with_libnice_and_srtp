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
-Description	: ������ʽ
.��				ƥ�����\r\n��֮����κε����ַ�
*				ƥ��ǰ����ӱ��ʽ����Ρ����磬zo*��ƥ�䡰z����Ҳ��ƥ�䡰zo���Լ���zoo����*�ȼ���o{0,}
				����.*��ƥ��������洢�����������
(pattern)		ƥ��ģʽ��pattern����ȡ��һƥ�䡣����ȡ��ƥ����ԴӲ�����Matches���ϵõ�
[xyz]			�ַ����ϡ�ƥ��������������һ���ַ������磬��[abc]������ƥ�䡰plain���еġ�a����
+				ƥ��ǰ����ӱ��ʽһ�λ���(���ڵ���1�Σ������磬��zo+����ƥ�䡰zo���Լ���zoo����������ƥ�䡰z����+�ȼ���{1,}��
				//���������в���+��Ĭ����һ�Σ���ֻ��ƥ�䵽һ������6
				
[A-Za-z0-9] 	26����д��ĸ��26��Сд��ĸ��0��9����
[A-Za-z0-9+/=]	26����д��ĸ��26��Сд��ĸ0��9�����Լ�+/= �����ַ�


-Input			: i_strPattern ģʽ��,i_strBuf��ƥ���ַ���,
-Output 		: o_ptMatch �洢ƥ�䴮λ�õ�����,���ڴ洢ƥ�����ڴ�ƥ�䴮�е��±귶Χ
//����0��Ԫ�����������ʽƥ������λ��,�������������������ƥ��������ߵĵ�Ԫ���δ����������ʽƥ������λ��
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/11/01	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Http::Regex(const char *i_strPattern,string *i_strBuf,smatch &o_Match)
{
    char acErrBuf[256];
    int iRet=-1;

    // ����������ʽģʽ
    regex Pattern(i_strPattern);
    // ʹ�� std::regex_search ����ƥ�������
    if (std::regex_search(*i_strBuf, o_Match, Pattern)) //ĳ��ƥ��ʧ�ܣ�std::smatch���󽫲��ᱣ����һ�ε�ƥ����
    {//o_Matchָ����Ӵ��ڴ���i_strBuf�У�������Ҫ��֤i_strBuf�������򣬼�i_strBuf�����ȱ��ͷŵ�
        // �����һ��ƥ�䵽���Ӵ�
        std::cout << "Match found: " <<o_Match.size()<< o_Match.str() << std::endl;

        // �������ƥ�䵽���Ӵ�
        for (size_t i = 0; i < o_Match.size(); ++i) 
        {
            std::string matchStr = o_Match[i].str(); // �� std::sub_match ת��Ϊ�ַ���
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


