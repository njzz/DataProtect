#pragma once
#include "DataChange.h"
#include "FileOpt.h"

namespace DP {
	
	//�ļ�����д��
	class DateSave{
	public:
		bool Open(LPCTSTR lpFileOut,const std::string &skey);
		bool Write(const char *data, size_t size);
		void Close();

	protected:
		DataChange m_dataChange;
		FileOpt m_fileOut;
	};

	//�ļ����ܶ���
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
		size_t Read(char *data, size_t size);//��д���࣬����ʵ��
	protected:
		std::string m_readCache;	
	};


	//����������Կ ������֤��Կ���ļ� ���ص�ǰ������Կ
	std::string CreateAndSave(LPCTSTR  lpFileOut, const char *lpPass);
	//ȥ�ļ���֤��Կ ���ظ�����Կ ʧ�ܷ��ؿ�
	std::string AuthInFile(LPCTSTR  lpFileKey, const char *lpPass);
	//����pass��ø�����Կ
	std::string GetSKey(const char *lpPass);
	//��֤��Կ�ļ��Ƿ���Ч
	bool IsKeyFileValid(LPCTSTR  lpFileKey);

	//���ɽӿ�
	//�����ļ�
	bool ProcessFile(LPCTSTR  lpFileIn, LPCTSTR  lpFileOut, const std::string &skey, bool bEncrypt);
	//�����ַ���(����ģʽ�����ܺ�strOutΪhex������ģʽ��strInΪhex)
	bool ProcessStr(const std::string &strIn, std::string &strOut, const std::string &skey, bool bEncrypt);
	//�������� (data Ϊ�������������)
	bool ProcessData(const std::string &dataIn, std::string &dataOut, const std::string &skey, bool bEncrypt);
	//�ַ������ܲ����浽�ļ�
	bool StrEncode2File(const char * lpStrIn, size_t strLen, LPCTSTR  lpFileOut, const std::string &skey);
	//�ļ����ܲ�������ַ���
	bool FileDecode2Str(LPCTSTR  lpFileIn, std::string& strOut, const std::string &skey);

}