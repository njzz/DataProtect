
#pragma once
#include <string>

namespace Helpers
{
	//根据内容，生成新的离散内容,新内容长度len
	std::string &GetBreakAway(std::string &strInfo,int len);
	//创建随机字符串 , rd 可以自己提供一次随机数种子
	std::string CreateRandomString(size_t length, int rd = 0);//创建随机字串

	//一般用于显示转换
	std::string BasicToHex(const char *pSrc,size_t len);
	std::string HexToBasic(const char *pSrc,size_t len);
};


