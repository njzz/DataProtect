#include "DHP.h"
#include "FileOpt.h"
#include "DataChange.h"
#include "helpers.h"

#define AUTH_OFFSET_BYTE	8

namespace DP {
	//��֤��Կ����
	static inline UINT GetAuthKeyLen() { //56
		return DataChange::HeadLen() - AUTH_OFFSET_BYTE;
	}
	//���ɵĸ�����Կ���ȣ���DataChange��HeadLenһ��
	static inline UINT GetLongKeyLen() { // 64
		return DataChange::HeadLen();
	}
	//��֤���泤��
	static inline UINT GetAuthKeySaveLen() { //112
		return GetAuthKeyLen() * 2;
	}
	//����key ���һ��������Կ
	static inline std::string GetLongKey(const char * lpPass) {
		std::string skey(lpPass);
		return std::move(Helpers::GetBreakAway(skey, GetLongKeyLen()));
	}
	//����key ���һ������key��������֤key (HEX)
	static inline std::string GetAuthKeySave(const char * lpPass) {
		std::string keyAuth(lpPass);
		Helpers::GetBreakAway(keyAuth, GetAuthKeyLen());
		return  std::move(Helpers::BasicToHex(keyAuth.c_str(), keyAuth.length()));
	}
	//������֤��Կ���浽�ļ�������ͷ������
	std::string CreateAndSave(LPCTSTR  lpFileOut, const char *lpPass)
	{
		//����һ����֤key ,����key��չ��HeadLen()/2�ֽ�(k1,��֤),��չ��HeadLen()�ֽ�(k2,���ܼ���key)
		std::string k2;
		if (lpFileOut != nullptr && lpPass != nullptr && strlen(lpPass) > 3) {

			FileOpt fileOut;
			if (fileOut.Open(lpFileOut, false)) {
				std::string strSave = GetAuthKeySave(lpPass);

				if (strSave.length() == GetAuthKeySaveLen() && fileOut.Write(strSave.c_str(), strSave.length())) {
					k2 = GetLongKey(lpPass);//����				
				}
				fileOut.Close();
			}
		}
		return k2;
	}
	//���ļ���ȡ��֤��Կ
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
	//��֤�ļ���Կ������ͷ������(��֤ʧ�ܷ��ؿ�)
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

	bool ProcessFile(LPCTSTR  lpFileIn, LPCTSTR  lpFileOut, const std::string &key, bool bEncrypt)
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
					if (bEncrypt) {
						auto head = dataChange.CreateHead(key);//set key
						bSuccessed = fileOut.Write(head.c_str(), head.length());//д��ͷ��Ϣ
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
							if (bEncrypt)
								dataChange.Encrypt(bufIn, nRead);
							else
								dataChange.Decrypt(bufIn, nRead);
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

	bool ProcessStr(const std::string & strIn, std::string & strOut, const std::string & key, bool bEncrypt)
	{
		bool rt = false;
		if (bEncrypt) {
			rt = ProcessData(strIn, strOut, key, bEncrypt);
			if(rt){
				strOut = Helpers::BasicToHex(strOut.c_str(), strOut.size());//����Ϊhex
			}
		}
		else {
			rt = ProcessData(Helpers::HexToBasic(strIn.c_str(),strIn.size()), strOut, key, bEncrypt);//
		}

		return rt;
	}
	bool ProcessData(const std::string & dataIn, std::string & dataOut, const std::string & key, bool bEncrypt)
	{
		bool bSuccessed = false;
		const auto lkeylen = GetLongKeyLen();
		if (!dataIn.empty() && key.size() == lkeylen)
		{
			dataOut.clear();
			DataChange dataChange;
			if (bEncrypt) {//����
				bSuccessed = true;
				auto head = dataChange.CreateHead(key);//set key
				dataOut.reserve(dataIn.size() + head.size() + 1);//Ԥ����
				dataOut.append(head);//д��ͷ��Ϣ
				dataOut.append(dataIn);//���ݸ���
				dataChange.Encrypt(((char *)dataOut.c_str())+head.size(), (UINT)dataOut.size()-head.size());
			}
			else if (dataIn.size() > lkeylen) {//����
				bSuccessed = dataChange.SetHead(key, dataIn.substr(0, lkeylen));
				if (bSuccessed) {
					dataOut.assign(dataIn.c_str() + lkeylen, dataIn.size() - lkeylen);
					dataChange.Decrypt((char *)dataOut.c_str(), (UINT)dataOut.size());
				}
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
				bSuccessed = m_fileOut.Write(head.c_str(), head.length());//д��ͷ��Ϣ
			}
		}

		return bSuccessed;
	}

	bool DateSave::Write(const char * data, size_t size)
	{
		constexpr size_t once = 32 * 1024;
		size_t wtd=0,curSize=0;//��д����ǰ
		char dataSet[once];//д�뻺��
		if (!m_fileOut.IsOpend()) return false;

		while (wtd < size) {
			if (size - wtd > once) {//����һ�δ�����
				curSize = once;
			}
			else {//�ܴ�����
				curSize = size - wtd;				
			}

			memcpy(dataSet, data + wtd, curSize);//���ݸ���
			wtd += curSize;
			m_dataChange.Encrypt(dataSet, (UINT)curSize);//����
			if (!m_fileOut.Write(dataSet, curSize))//д��
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

	bool DataLoad::IsEof() const{
		return m_fileIn.IsEof();
	}

	size_t DataLoad::Read(char * data, size_t size)
	{
		size_t rdd = 0;
		if (m_fileIn.IsOpend()) {
			rdd = m_fileIn.Read(data, size);
			if (rdd > 0) {
				m_dataChange.Decrypt(data, rdd);
			}
		}
		return rdd;
	}

	void DataLoad::Close()
	{
		m_fileIn.Close();
	}

	void DataLoadLine::ReadLine(std::string & line)
	{
		size_t start = 0;//��ʼλ��
		constexpr size_t szRead = 4096;
		char buf[szRead];
		line.clear();
		do {
			GetLine(line, start);//����
			if (!line.empty()) break;//�ҵ�����
			if (m_fileIn.IsEof()) {//����Ѿ���������󽻻�
				if (m_readCache.size())
					line = std::move(m_readCache);//��������
				break;
			}
			else {//��ȡ
				start = m_readCache.size();//�²���λ��
				auto r = DataLoad::Read(buf, szRead);//��ȡ�����ݲ�׷��
				if (r > 0) {
					m_readCache.append(buf, r);//����׷��
				}
			}
		} while (1);
	}

	bool DataLoadLine::IsEof() const
	{
		return m_readCache.empty() && DataLoad::IsEof();
	}

	void DataLoadLine::GetLine(std::string & str, size_t start)
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
					m_readCache.assign(m_readCache.c_str() + i + 1, sz - i - 1);//���� \n
			}
		}
	}

}