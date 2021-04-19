#pragma once
#include <string>
//����Ҫ���㷨�ǿ��Թ����ģ����ܸ������ģ�ԭ�ģ��㷨��ϣ������(��Ч)��Կ����ܵ�ǰ����


//�㷨���������ݳ���(ÿ�μ������ݣ���Կ����ͬ)
//������Կֻ��������������Կ��
//���������Կ��������Ť����Ť�����룬��ֹ ԭ�ģ����ģ��㷨��Ϲ���
//���ɶ����������  �������^��Կ��ɢ  ��Ϊ��ʼ���������������ݿ�ʼ
// key = ��ɢ������ݣ���һ����ʽ�ֽ�Ϊ len/2
//���ս��Ϊ src^key^��ʽ�ֽ�
#include "pubheader.h"
class DataChange
{
public:
	//��ȡͷ���ֽ���
	static UINT HeadLen();
	//��ʼ��
	//����ͷ��Ϣ
	std::string CreateHead(const std::string &key);
	//����ͷ��Ϣ�������ǽ��յģ�����ĵ�
	bool SetHead(const std::string &key, const std::string &head);

	//���ܽ���
	void Encrypt(char *pData,UINT nlen);
	void Decrypt(char *pData,UINT nlen);//
	DataChange(void) = default;
	~DataChange(void);

protected:
	void Reset();//����
	void InitTable();//��ʼ����
	inline char EnChar(char in);
	inline char DeChar(char in);

protected:
	std::string m_pk;//ԭʼ��Чkey
	unsigned int  m_nKeyCurrentEn;//��ǰ����N��key
	char m_chLianshiEn;//��ǰ��ʽ�ֽ�

	unsigned int  m_nKeyCurrentDe;//��ǰ����N��key
	char m_chLianshiDe;//��ǰ��ʽ�ֽ�

	unsigned  char m_tableEn[256];//��ǰ����Ť����
	unsigned  char m_tableDe[256];//��ǰ����ָ���

};

