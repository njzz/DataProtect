#pragma once
#include "DataChange.h"
#include "FileOpt.h"

namespace DP {
	
	//文件加密写入
	class DateSave{
	public:
		bool Open(LPCTSTR lpFileOut,const std::string &skey);
		bool Write(const char *data, size_t size);
		void Close();

	protected:
		DataChange m_dataChange;
		FileOpt m_fileOut;
	};

	//文件解密读出
	class DataLoad {
	public:
		bool Open(LPCTSTR lpFileIn, const std::string &skey);
		size_t Read(char *data, size_t size);
		void Close();
		bool IsEof() const;

	protected:
		DataChange m_dataChange;
		FileOpt m_fileIn;
	};

	class DataLoadLine:public DataLoad {
	public:
		void ReadLine(std::string &line);
		bool IsEof() const;
	protected:
		void GetLine(std::string &str, size_t start);
		size_t Read(char *data, size_t size);//复写父类，但不实现
	protected:
		std::string m_readCache;	
	};


	//创建复杂密钥 保存验证密钥到文件 返回当前复杂密钥
	std::string CreateAndSave(LPCTSTR  lpFileOut, const char *lpPass);
	//去文件验证密钥 返回复杂密钥 失败返回空
	std::string AuthInFile(LPCTSTR  lpFileKey, const char *lpPass);
	//根据pass获得复杂密钥
	std::string GetSKey(const char *lpPass);
	//验证密钥文件是否有效
	bool IsKeyFileValid(LPCTSTR  lpFileKey);

	//集成接口
	//处理文件
	bool ProcessFile(LPCTSTR  lpFileIn, LPCTSTR  lpFileOut, const std::string &skey, bool bEncrypt);
	//处理字符串(加密模式：加密后strOut为hex，解谜模式：strIn为hex)
	bool ProcessStr(const std::string &strIn, std::string &strOut, const std::string &skey, bool bEncrypt);
	//处理数据 (data 为任意二进制数据)
	bool ProcessData(const std::string &dataIn, std::string &dataOut, const std::string &skey, bool bEncrypt);
	//字符串加密并保存到文件
	bool StrEncode2File(const char * lpStrIn, size_t strLen, LPCTSTR  lpFileOut, const std::string &skey);
	//文件解密并输出到字符串
	bool FileDecode2Str(LPCTSTR  lpFileIn, std::string& strOut, const std::string &skey);

}