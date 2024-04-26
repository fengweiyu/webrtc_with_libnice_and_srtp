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
#include <string>


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
int Http::Regex(const char *i_strPattern,char *i_strBuf,regmatch_t *o_ptMatch)
{
    char acErrBuf[256];
    int iRet=-1;
    regex_t tReg;    //����һ������ʵ��
    //const size_t dwMatch = 6;    //����ƥ�������������       //��ʾ������ƥ��


    //REG_ICASE ƥ����ĸʱ���Դ�Сд��
    iRet =regcomp(&tReg, i_strPattern, REG_EXTENDED);    //��������ģʽ��
    if(iRet != 0) 
    {
        regerror(iRet, &tReg, acErrBuf, sizeof(acErrBuf));
        HTTP_LOGE("Regex Error:\r\n");
    }
    else
    {
        iRet = regexec(&tReg, i_strBuf, HTTP_MAX_MATCH_NUM, o_ptMatch, 0); //ƥ����
        if (iRet == REG_NOMATCH)
        { //���ûƥ����
            HTTP_LOGE("Regex No Match!\r\n");
        }
        else if (iRet == REG_NOERROR)
        { //���ƥ������
            HTTP_LOGD("Match\r\n");
            int i=0,j=0;
			for(j=0;j<HTTP_MAX_MATCH_NUM;j++)
			{
				for (i= o_ptMatch[j].rm_so; i < o_ptMatch[j].rm_eo; i++)
				{ //�������ƥ�䷶Χ���ַ���
					printf("%c", i_strBuf[i]);
				}
				printf("\n");
			}
        }
        else
        {
            HTTP_LOGE("Regex Unknow err:\r\n");
        }
        regfree(&tReg);  //�ͷ�������ʽ
    }
    
    return iRet;
}


