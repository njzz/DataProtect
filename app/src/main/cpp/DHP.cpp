#include "DHP.h"
#include "FileOpt.h"
#include "DataChange.h"
#include "helpers.h"

#define AUTH_OFFSET_BYTE	8

namespace DP {
	//验证密钥长度
	static inline UINT GetAuthKeyLen() { //56
		return DataChange::HeadLen() - AUTH_OFFSET_BYTE;
	}
	//生成的复杂密钥长度，和DataChange的HeadLen一致
	static inline UINT GetLongKeyLen() { // 64
		return DataChange::HeadLen();
	}
	//验证保存长度
	static inline UINT GetAuthKeySaveLen() { //112
		return GetAuthKeyLen() * 2;
	}
	//根据key 获得一个复杂密钥
	static inline std::string GetLongKey(const char * lpPass) {
		std::string skey(lpPass);
		return std::move(Helpers::GetBreakAway(skey, GetLongKeyLen()));
	}
	//根据key 获得一个保存key，用来验证key (HEX)
	static inline std::string GetAuthKeySave(const char * lpPass) {
		std::string keyAuth(lpPass);
		Helpers::GetBreakAway(keyAuth, GetAuthKeyLen());
		return  std::move(Helpers::BasicToHex(keyAuth.c_str(), keyAuth.length()));
	}
	//创建验证密钥保存到文件并返回头部密文
	std::string CreateAndSave(LPCTSTR  lpFileOut, const char *lpPass)
	{
		//输入一个验证key ,将该key扩展到HeadLen()/2字节(k1,验证),扩展到HeadLen()字节(k2,解密加密key)
		std::string k2;
		if (lpFileOut != nullptr && lpPass != nullptr && strlen(lpPass) > 3) {

			FileOpt fileOut;
			if (fileOut.Open(lpFileOut, false)) {
				std::string strSave = GetAuthKeySave(lpPass);

				if (strSave.length() == GetAuthKeySaveLen() && fileOut.Write(strSave.c_str(), strSave.length())) {
					k2 = GetLongKey(lpPass);//解密				
				}
				fileOut.Close();
			}
		}
		return k2;
	}
	//从文件获取验证密钥
	static std::string GetSaveKey(LPCTSTR  lpFileKey) {
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
	//验证文件密钥并返回头部密文(验证失败返回空)
	std::string AuthInFile(LPCTSTR lpFileKey, const char * lpPass)
	{
		auto keyInfo = GetSaveKey(lpFileKey);
		if (keyInfo.size() == GetAuthKeySaveLen()) {
			std::string k1 = GetAuthKeySave(lpPass);

			if (k1 == keyInfo) {
				return GetLongKey(lpPass);//
			}
		}
		return std::string();
	}

	std::string GetSKey(const char * lpPass)
	{
		return GetLongKey(lpPass);//
	}

	bool IsKeyFileValid(LPCTSTR  lpFileKey)
	{
		return GetSaveKey(lpFileKey).size() == GetAuthKeySaveLen();
	}

	bool ProcessFile(LPCTSTR  lpFileIn, LPCTSTR  lpFileOut, const std::string &key, bool bEncode)
	{
		bool bSuccessed = false;
		if (lpFileIn != lpFileOut && lpFileOut != nullptr)
		{
			FileOpt fileIn, fileOut;
			if (fileIn.Open(lpFileIn, true) && fileOut.Open(lpFileOut, false))
			{
				if (key.size() == GetLongKeyLen())
				{
					constexpr int nReadCount = 32 * 1024;
					UINT nRead = 0;
					char bufIn[nReadCount];

					DataChange dataChange;
					if (bEncode) {
						auto head = dataChange.CreateHead(key);//set key
						bSuccessed = fileOut.Write(head.c_str(), head.length());//写入头信息
					}
					else {
						nRead = fileIn.Read(bufIn, GetLongKeyLen());
						bSuccessed = (nRead == GetLongKeyLen()) && (dataChange.SetHead(key, std::string(bufIn, nRead)));
					}

					while (bSuccessed)
					{
						nRead = fileIn.Read(bufIn, nReadCount);
						if (nRead > 0)
						{
							if (bEncode)
								dataChange.Encode(bufIn, nRead);
							else
								dataChange.Decode(bufIn, nRead);
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

	bool ProcessStr(const std::string & strIn, std::string & strOut, const std::string & key, bool bEncode)
	{
		bool bSuccessed = false;
		const auto lkeylen = GetLongKeyLen();
		if (!strIn.empty() && key.size() == lkeylen)
		{
			std::string decode;
			size_t dataSize = 0;
			const char *pData=nullptr;

			DataChange dataChange;
			if (bEncode) {//加密
				auto head = dataChange.CreateHead(key);//set key
				strOut.reserve((strIn.size() + lkeylen) * 2 + 1);//reserver
				strOut = Helpers::BasicToHex(head.c_str(), head.length());//写入头信息
				bSuccessed = true;

				dataSize = strIn.size();
				pData = strIn.c_str();
			}
			else if (strIn.size() > lkeylen * 2) {//解密
				decode = Helpers::HexToBasic(strIn.c_str(), strIn.length());//西安从hex转换回来
				bSuccessed = (decode.size() > lkeylen) && dataChange.SetHead(key, decode.substr(0, lkeylen));

				dataSize = decode.size() - lkeylen;
				pData = decode.c_str() + lkeylen;
			}

			if (bSuccessed)
			{
				char *buf = new char[dataSize];
				if (buf) {
					memcpy(buf, pData, dataSize);

					if (bEncode) {
						dataChange.Encode(buf, dataSize);
						strOut.append(Helpers::BasicToHex(buf, dataSize));
					}
					else {
						dataChange.Decode(buf, dataSize);
						strOut.assign(buf, dataSize);
					}
				}

				bSuccessed = buf != nullptr;
			}
		}

		return bSuccessed;
	}

	bool StrEncode2File(const char * lpStrIn, size_t strLen, LPCTSTR  lpFileOut, const std::string &key)
	{
		DateSave ds;
		if (ds.Open(lpFileOut, key)) {
			return ds.Write(lpStrIn, strLen);
		}
		return false;
	}

	bool FileDecode2Str(LPCTSTR  lpFileIn, std::string& strOut, const std::string &key)
	{
		DataLoad dl;
		strOut.clear();
		if (dl.Open(lpFileIn, key)) {
			constexpr  size_t  load = 32 * 1024;
			char buf[load];
			size_t l = load;
			while (l == load) {
				 l = dl.Read(buf, load);
				 strOut.append(buf, l);
			}

		}
		return !strOut.empty();
	}

	bool DateSave::Open(LPCTSTR lpFile, const std::string &skey)
	{
		bool bSuccessed = false;
		if (skey.size() == GetLongKeyLen() && lpFile != nullptr)	{
			if (m_fileOut.Open(lpFile, false))
			{
				auto head = m_dataChange.CreateHead(skey);//set key
				bSuccessed = m_fileOut.Write(head.c_str(), head.length());//写入头信息
			}
		}

		return bSuccessed;
	}

	bool DateSave::Write(const char * data, size_t size)
	{
		constexpr size_t once = 32 * 1024;
		size_t wtd=0,curSize=0;//已写，当前
		char dataSet[once];//写入缓冲
		if (!m_fileOut.IsOpend()) return false;

		while (wtd < size) {
			if (size - wtd > once) {//不能一次处理完
				curSize = once;
			}
			else {//能处理完
				curSize = size - wtd;				
			}

			memcpy(dataSet, data + wtd, curSize);//数据复制
			wtd += curSize;
			m_dataChange.Encode(dataSet, (UINT)curSize);//加密
			if (!m_fileOut.Write(dataSet, curSize))//写入
				return false;
		}
		return true;
	}

	void DateSave::Close()
	{
		m_fileOut.Close();
	}

	bool DataLoad::Open(LPCTSTR  lpFileIn, const std::string & skey)
	{
		bool bSuccessed = false;
		if (lpFileIn != nullptr && skey.size() == GetLongKeyLen())
		{
			if (m_fileIn.Open(lpFileIn, true))
			{
				constexpr int nReadCount = 1024;
				UINT nRead = 0;
				char bufIn[nReadCount];

				nRead = m_fileIn.Read(bufIn, GetLongKeyLen());
				bSuccessed = (nRead == GetLongKeyLen()) && (m_dataChange.SetHead(skey, std::string(bufIn, nRead)));
			}
		}
		return bSuccessed;
	}

	size_t DataLoad::Read(char * data, size_t size)
	{
		size_t rdd = 0;
		if (m_fileIn.IsOpend()) {
			rdd = m_fileIn.Read(data, size);
			if (rdd > 0) {
				m_dataChange.Decode(data, rdd);
			}
		}
		return rdd;
	}

	void DataLoad::Close()
	{
		m_fileIn.Close();
	}

	void DateLoadLine::ReadLine(std::string & line)
	{
		size_t start = 0;//开始位置
		constexpr size_t szRead = 4096;
		char buf[szRead];
		line.clear();
		do {
			GetLine(line, start);//查找
			if (!line.empty()) break;//找到内容
			if (m_fileIn.IsEof()) {//如果已经结束，最后交换
				if (m_readCache.size())
					line = std::move(m_readCache);//最后的内容
				break;
			}
			else {//读取
				start = m_readCache.size();//新查找位置
				auto r = DataLoad::Read(buf, szRead);//读取新内容并追加
				if (r > 0) {
					m_readCache.append(buf, r);//内容追加
				}
			}
		} while (1);
	}

	void DateLoadLine::GetLine(std::string & str, size_t start)
	{
		auto sz = m_readCache.size();
		if (sz > start) {
			auto i = m_readCache.find_first_of('\n', start);
			if (i != std::string::npos) {
				if(i>0)
					str = m_readCache.substr(0, i);
				if (i == sz - 1)
					m_readCache.clear();
				else
					m_readCache.assign(m_readCache.c_str() + i + 1, sz - i - 1);//跳过 \n
			}
		}
	}

}