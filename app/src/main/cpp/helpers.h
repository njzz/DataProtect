
#pragma once
#include <string>

namespace Helpers
{
	//�������ݣ������µ���ɢ����,�����ݳ���len
	std::string &GetBreakAway(std::string &strInfo,int len);
	//��������ַ��� , rd �����Լ��ṩһ�����������
	std::string CreateRandomString(size_t length, int rd = 0);//��������ִ�

	//һ��������ʾת��
	std::string BasicToHex(const char *pSrc,size_t len);
	std::string HexToBasic(const char *pSrc,size_t len);
};


