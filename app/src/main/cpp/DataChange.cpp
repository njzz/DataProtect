
#include "DataChange.h"
#include "helpers.h"
#include "MD5.h"

#define CUR_HEAD_LEN	64 //头部字节增加数

DataChange::~DataChange(void)
{
	Reset();
}

UINT DataChange::HeadLen()
{
	return CUR_HEAD_LEN;
}

std::string  DataChange::InitEncode(const std::string &key)
{
	std::string r;
	if (!key.empty()) {
		Reset();
		std::string strK=key;
		Helpers::GetBreakAway(strK, CUR_HEAD_LEN);

		m_pk = Helpers::CreateRandomString(CUR_HEAD_LEN);
		char xorR[CUR_HEAD_LEN];
		UINT index = 0;
		while (index < CUR_HEAD_LEN) {
			xorR[index] = strK.at(index) ^ m_pk.at(index);
			index++;
		}

		r.assign(xorR, CUR_HEAD_LEN);//头信息 = bk(key)^rand 

		//key 信息 = bk(rand)
		Helpers::GetBreakAway(m_pk, CUR_HEAD_LEN);
		m_chLianshi = m_pk[CUR_HEAD_LEN /2];
	}
	return r;
}

bool DataChange::InitDecode(const std::string &key, const std::string &head)
{
	if( !key.empty() && head.size()== CUR_HEAD_LEN)
	{
		Reset();
		std::string strK = key;
		Helpers::GetBreakAway(strK, CUR_HEAD_LEN);

		char xorR[CUR_HEAD_LEN];//rand = head ^ bk(key)
		UINT index = 0;
		while (index < CUR_HEAD_LEN) {
			xorR[index] = strK.at(index) ^ head.at(index);
			index++;
		}
		
		m_pk.assign(xorR, CUR_HEAD_LEN);

		Helpers::GetBreakAway(m_pk, CUR_HEAD_LEN);
		m_chLianshi = m_pk[CUR_HEAD_LEN / 2];
		return true;
	}

	return false;
}


void DataChange::Encode( char *pData ,UINT nLen)
{
	if(pData==nullptr || nLen<1 ) return;
	UINT nIndex=0;
	char chEncoded;
	while(nIndex<nLen)
	{
		chEncoded =(pData[nIndex]^m_pk[m_nKeyCurrent]^ m_chLianshi);
		m_chLianshi = (pData[nIndex] ^ m_pk[m_nKeyCurrent]);//下一个key为上一个原文^key位
		pData[nIndex]= chEncoded;//

		if(++m_nKeyCurrent == m_pk.length()) m_nKeyCurrent =0;
		
		++nIndex;
	}
}

void DataChange::Decode( char *pData ,UINT nLen)
{
	if(pData==nullptr || nLen<1 ) return;
	UINT nIndex=0;
	char chDecoded;
	while(nIndex<nLen)
	{
		chDecoded=(char)(pData[nIndex]^m_pk[m_nKeyCurrent]^ m_chLianshi);
		pData[nIndex]=chDecoded;//

		m_chLianshi = (char)(/*pData[nIndex]*/chDecoded^ m_pk[m_nKeyCurrent]);//下一个解码和原内容和key有关

		if(++m_nKeyCurrent ==m_pk.length()) m_nKeyCurrent =0;

		++nIndex;
	}
}

void DataChange::Reset()
{
	m_nKeyCurrent =0;
	m_chLianshi =0;
	m_pk.clear();
}

