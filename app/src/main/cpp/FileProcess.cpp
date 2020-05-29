
#include "FileProcess.h"
#include "FileOpt.h"
#include "DataChange.h"
#include "helpers.h"

#define AUTH_OFFSET_BYTE	8

static inline UINT GetAuthKeyLen() { //56
	return DataChange::HeadLen() - AUTH_OFFSET_BYTE;
}

static inline UINT GetSKeyLen() { // 64
	return DataChange::HeadLen();
}

static inline UINT GetAuthKeySaveLen() { //112
	return GetAuthKeyLen() * 2;
}

static inline std::string GetSKey(LPCTSTR lpKey) {
	std::string skey(lpKey);
	return std::move(Helpers::GetBreakAway(skey, GetSKeyLen()));
}

static inline std::string GetAuthKeySave(LPCTSTR lpKey) {
	std::string keyAuth(lpKey);
	Helpers::GetBreakAway(keyAuth, GetAuthKeyLen());
	return  std::move(Helpers::BasicToHex(keyAuth.c_str(), keyAuth.length()));
}

std::string FileProcess::CreateKey(LPCTSTR lpFileOut, const char *lpPass)
{
	//输入一个验证key ,将该key扩展到HeadLen()/2字节(k1,验证),扩展到HeadLen()字节(k2,解密加密key)
	std::string k2;
	if (lpFileOut!=nullptr && lpPass != nullptr && strlen(lpPass) > 3) {
		
		FileOpt fileOut;
		if ( fileOut.Open(lpFileOut, false)) {
			std::string strSave = GetAuthKeySave(lpPass);

			if (strSave.length()== GetAuthKeySaveLen() && fileOut.Write(strSave.c_str(), strSave.length())) {
				k2 = GetSKey(lpPass);//解密				
			}
			fileOut.Close();
		}
	}
	return k2;
}

static std::string GetSaveKey(LPCTSTR lpFileKey) {
	FileOpt fileOut;
	if (fileOut.Open(lpFileKey, true)) {
		constexpr int optCount = 2048;
		char keyBuff[optCount];
		auto readed = fileOut.Read(keyBuff, optCount);
		if (readed == GetAuthKeySaveLen()) {//hex *2
			keyBuff[readed] = 0;
			return std::string(keyBuff, readed);
		}
	}
	return std::string();
}

std::string FileProcess::GetKey(LPCTSTR lpFileKey, const char * lpPass)
{
	auto keyInfo = GetSaveKey(lpFileKey);
	if (keyInfo.size() == GetAuthKeySaveLen()) {
		std::string k1=GetAuthKeySave(lpPass);

		if (k1 == keyInfo) {
			return GetSKey(lpPass);//
		}
	}
	return std::string();
}

bool FileProcess::IsKeyFileValid(LPCTSTR lpFileKey)
{
	return GetSaveKey(lpFileKey).size()==GetAuthKeySaveLen();
}


bool FileProcess::ProcessFile(LPCTSTR lpFileIn,LPCTSTR lpFileOut, const std::string &key,bool bEncode)
{
	bool bSuccessed=false;
	if( lpFileIn!= lpFileOut && lpFileOut!=nullptr)
	{
		FileOpt fileIn,fileOut;
		if( fileIn.Open(lpFileIn,true) && fileOut.Open(lpFileOut,false) )
		{
			if(key.size()== GetSKeyLen())
			{
				constexpr int nReadCount = 32 * 1024;
                UINT nRead = 0;
				char bufIn[nReadCount];

				DataChange dataChange;
				if (bEncode) {
					auto head = dataChange.InitEncode(key);//set key
					bSuccessed = fileOut.Write(head.c_str(), head.length());//写入头信息
				}
				else {
					nRead = fileIn.Read(bufIn, GetSKeyLen());
					bSuccessed = (nRead == GetSKeyLen()) && (dataChange.InitDecode(key, std::string(bufIn, nRead)));
				}
				
				while(bSuccessed)
				{
					nRead=fileIn.Read(bufIn,nReadCount);
					if(nRead>0)
					{
						if(bEncode)
							dataChange.Encode(bufIn,nRead);	
						else 
							dataChange.Decode(bufIn,nRead);
						if (!fileOut.Write(bufIn, nRead)) {
							bSuccessed = false;
							break;
						}
					}

					if (fileIn.IsEof())
						break;
				}
			}
		}
	}

	return bSuccessed;
}

bool FileProcess::StrEncode2File(LPCTSTR lpStrIn,LPCTSTR lpFileOut, const std::string &key)
{
	bool bSuccessed=false;
	if( lpStrIn!= nullptr && lpFileOut!=nullptr)
	{
		FileOpt fileOut;
		if( fileOut.Open(lpFileOut,false) )
		{
			if(key.size()== GetSKeyLen())
			{
				const size_t nLen = strlen(lpStrIn);
				char *pNewSet = new char[nLen];
				if(pNewSet!= nullptr) {
					memcpy(pNewSet, lpStrIn, nLen);

					DataChange dataChange;
					auto head = dataChange.InitEncode(key);//set key
					bSuccessed = fileOut.Write(head.c_str(), head.length());//写入头信息
					if(bSuccessed) {
						dataChange.Encode(pNewSet, nLen);
						bSuccessed = fileOut.Write(pNewSet, nLen);
					}
					delete [] pNewSet;
				}
			}
		}
	}

	return bSuccessed;
}

bool FileProcess::FileDecode2Str(LPCTSTR lpFileIn,std::string& strOut, const std::string &key)
{
	bool bSuccessed=false;
	strOut.clear();
	if( lpFileIn!= nullptr && key.size()== GetSKeyLen())
	{
		FileOpt fileIn;
		if( fileIn.Open(lpFileIn,true)  )
		{
				constexpr int nReadCount = 32 * 1024;
				UINT nRead = 0;
				char bufIn[nReadCount];

				DataChange dataChange;
				nRead = fileIn.Read(bufIn, GetSKeyLen());
				bSuccessed = (nRead == GetSKeyLen()) && (dataChange.InitDecode(key, std::string(bufIn, nRead)));

				if(bSuccessed) {
					while (1) {
						nRead = fileIn.Read(bufIn, nReadCount);
						if (nRead > 0) {
							dataChange.Decode(bufIn, nRead);
							strOut.append(bufIn, nRead);
						}

						if (fileIn.IsEof())
							break;
					}
				}
		}
	}

	return bSuccessed;
}
