#include "DataChange.h"
#include "helpers.h"

#define CUR_HEAD_LEN	64 //头部字节增加数

static const unsigned char tab_init[256] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,
60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,
100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,
120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,
140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,
200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,
220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};

DataChange::~DataChange(void)
{
	Reset();
}

UINT DataChange::HeadLen()
{
	return CUR_HEAD_LEN;
}

std::string  DataChange::CreateHead(const std::string &key)
{
	std::string r;
	if (!key.empty()) {
		Reset();
		std::string strK=key;
		Helpers::GetBreakAway(strK, CUR_HEAD_LEN);

		m_pk = Helpers::CreateRandomString(CUR_HEAD_LEN,*(int*)strK.c_str());
		char xorR[CUR_HEAD_LEN];
		UINT index = 0;
		while (index < CUR_HEAD_LEN) {
			xorR[index] = strK.at(index) ^ m_pk.at(index);
			index++;
		}

		r.assign(xorR, CUR_HEAD_LEN);//头信息 = bk(key)^rand 

		//key 信息 = bk(rand)
		Helpers::GetBreakAway(m_pk, CUR_HEAD_LEN);
		//等效扭曲表，将输入扭曲 ，根据 m_pk ,生成扭曲表 bk(m_pk,128) tab[256]
		InitTable();
	}
	return r;
}

bool DataChange::SetHead(const std::string &key, const std::string &head)
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
		InitTable();
		return true;
	}

	return false;
}

void DataChange::InitTable()
{
	if (m_pk.size() != CUR_HEAD_LEN) return;
	const unsigned short TableLen = 256;
	unsigned short i;
	memcpy(m_tableEn, tab_init, TableLen);//表复制
	unsigned char index1, index2, t;
	std::string srand = m_pk;
	Helpers::GetBreakAway(srand, TableLen);//根据 m_pk 扩散到256，生成扭曲表 , srand 里面有重复也无所谓
	for (i = 0; i < TableLen; i++) {//输入扭曲表
		index1 = srand[i];//位置交换信息
		index2 = srand[i + 1];
		t = m_tableEn[index1];
		m_tableEn[index1] = m_tableEn[index2];
		m_tableEn[index2] = t;
	}

	for (i = 0; i < TableLen; i++) {//输出表
		m_tableDe[m_tableEn[i]] = (unsigned char)i;
	}
	//c=tableEn[I] , I=tableDe[c]
	m_chLianshiEn = m_chLianshiDe = srand.back();
}

inline char DataChange::EnChar(char in)
{
	auto rnd =  ((unsigned char)(m_pk[m_nKeyCurrentEn]^m_chLianshiEn)) % 4 + 1;//[1,4]
	for (auto i = 0; i < rnd; ++i) {
		in = m_tableEn[(unsigned char)(in)] ^ m_pk[(m_nKeyCurrentEn + i)%m_pk.size()];
	}
	in ^= m_chLianshiEn;
	m_chLianshiEn =in^ m_tableDe[(unsigned char)in];

	m_nKeyCurrentEn = (m_nKeyCurrentEn+rnd)%m_pk.size();
	return in;
}

inline char DataChange::DeChar(char in)
{
	auto t = in;
	in ^= m_chLianshiDe;	

	auto rnd =  ((unsigned char)(m_pk[m_nKeyCurrentDe]^ m_chLianshiDe)) % 4 + 1;//[1,4]
	for (auto i = 0; i < rnd; ++i) {
		in = m_tableDe[(unsigned char)(in ^ m_pk[(m_nKeyCurrentDe + rnd - 1 - i) % m_pk.size()])];
	}

	m_chLianshiDe = t^ m_tableDe[(unsigned char)t];

	m_nKeyCurrentDe = (m_nKeyCurrentDe+rnd)%m_pk.size();
	return in;
}


void DataChange::Encrypt( char *pData ,UINT nLen)
{
	if(pData==nullptr || nLen<1 ) return;
	UINT nIndex=0;
	while(nIndex<nLen)
	{
		pData[nIndex]= EnChar(pData[nIndex]);//		
		++nIndex;
	}
}

void DataChange::Decrypt( char *pData ,UINT nLen)
{
	if(pData==nullptr || nLen<1 ) return;
	UINT nIndex=0;
	while(nIndex<nLen)
	{
		pData[nIndex]= DeChar(pData[nIndex]);//
		++nIndex;
	}
}

void DataChange::Reset()
{
	m_nKeyCurrentEn =0;
	m_chLianshiEn =0;
	m_nKeyCurrentDe = 0;
	m_chLianshiDe = 0;
	m_pk.clear();
}



