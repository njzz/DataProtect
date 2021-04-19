#pragma once
#include <string>
//加密要求，算法是可以公开的，不能根据密文，原文，算法组合，推算出(等效)密钥或解密当前数据


//算法，增加数据长度(每次加密数据，密钥都不同)
//输入密钥只是用来获得随机密钥的
//根据随机密钥生成输入扭曲表，扭曲输入，防止 原文，密文，算法组合攻击
//生成定长随机数据  随机数据^密钥离散  作为初始定长数据置于数据开始
// key = 离散随机数据，第一个链式字节为 len/2
//最终结果为 src^key^链式字节
#include "pubheader.h"
class DataChange
{
public:
	//获取头部字节数
	static UINT HeadLen();
	//初始化
	//创建头信息
	std::string CreateHead(const std::string &key);
	//设置头信息，可以是接收的，保存的等
	bool SetHead(const std::string &key, const std::string &head);

	//加密解密
	void Encrypt(char *pData,UINT nlen);
	void Decrypt(char *pData,UINT nlen);//
	DataChange(void) = default;
	~DataChange(void);

protected:
	void Reset();//重置
	void InitTable();//初始化表
	inline char EnChar(char in);
	inline char DeChar(char in);

protected:
	std::string m_pk;//原始等效key
	unsigned int  m_nKeyCurrentEn;//当前到第N个key
	char m_chLianshiEn;//当前链式字节

	unsigned int  m_nKeyCurrentDe;//当前到第N个key
	char m_chLianshiDe;//当前链式字节

	unsigned  char m_tableEn[256];//当前输入扭曲表
	unsigned  char m_tableDe[256];//当前输出恢复表

};

