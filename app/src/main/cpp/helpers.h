
#pragma once
#include <string>

namespace Helpers
{
	//根据内容，生成新的离散内容,新内容长度len
	std::string &GetBreakAway(std::string &strInfo,int len);
	//创建随机字符串
	std::string CreateRandomString(size_t length);//创建随机字串

	//一般用于显示转换
	std::string BasicToHex(const char *pSrc,int len);
	std::string HexToBasic(const char *pSrc,int len);
};


