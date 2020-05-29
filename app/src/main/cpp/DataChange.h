#pragma once
#include <string>
//加密要求，算法是可以公开的，不能根据密文，原文，算法组合，推算出密钥或(密钥最终加密串)

//第一种算法，不会增加数据长度(加密相同数据为相同结果)[必须要有Key2，因为知道密钥和密钥md5区别不大]
//根据key的长度，计算出等效的md5，作为key1 :比如key 为 15 字节，那么key1为16字节 ，key1是变长，根据key 16字节对齐
//根据key1的md5，计算出等效等长key2，第一个链式字节为 key2/2
//最终结果为 src^key1^key2^链式字节
//不安全！ 因为 key1^key2^链式字节，是固定密钥推算出来的固定内容，可以通过攻击计算出来，而解开其它加密文件(虽然得不到密钥)

//第二种算法，增加数据长度(每次加密数据，密钥都不同)
//生成定长随机数据  随机数据^密钥离散  作为初始定长数据置于数据开始
// key = 离散随机数据，第一个链式字节为 len/2
//最终结果为 src^key^链式字节
#include "pubheader.h"
class DataChange
{
public:
	//获取头部字节数
	static UINT HeadLen();
	//返回头信息
	std::string InitEncode(const std::string &key);
	//输入 info , key
	bool InitDecode(const std::string &key, const std::string &head);
	void Encode(char *pData,UINT nlen);
	void Decode(char *pData,UINT nlen);//
	DataChange(void) = default;
	~DataChange(void);

protected:
	void Reset();

protected:
	std::string m_pk;//原始等效key
	unsigned int  m_nKeyCurrent;//当前到第N个key
	char m_chLianshi;//当前链式字节

};

