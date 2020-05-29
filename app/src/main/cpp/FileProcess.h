#pragma once
#include "pubheader.h"
class FileProcess
{
public:
	//∑µªÿ√‹‘ø
	static std::string CreateKey(LPCTSTR lpFileOut,const char *lpPass);
	static std::string GetKey(LPCTSTR lpFileKey, const char *lpPass);
	static bool IsKeyFileValid(LPCTSTR lpFileKey);
	static bool ProcessFile(LPCTSTR lpFileIn,LPCTSTR lpFileOut,const std::string &key,bool bEncode);
	static bool StrEncode2File(LPCTSTR lpStrIn,LPCTSTR lpFileOut,const std::string &key);
	static bool FileDecode2Str(LPCTSTR lpFileIn,std::string& strOut,const std::string &key);

};

