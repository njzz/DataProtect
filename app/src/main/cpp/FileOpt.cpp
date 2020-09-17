
#include "FileOpt.h"


FileOpt::FileOpt(void)
{
}


FileOpt::~FileOpt(void)
{
	Close();
}

bool FileOpt::Open( LPCTSTR lpFile,bool isRead )
{
	Close();
	if (isRead) {
		m_file.open(lpFile, std::fstream::binary | std::fstream::in);
	}
	else {
		m_file.open(lpFile, std::fstream::binary | std::fstream::out);
	}
	return m_file.is_open();
}

bool FileOpt::IsOpend() const
{
	return m_file.is_open();
}

bool FileOpt::Write(const void *data,UINT len ,bool bFlush)
{
	bool br=false;
	if(len && data && m_file.is_open())
	{
		m_file.write((const char *)data, len);
		br = m_file.good();
		if(bFlush)
			m_file.flush();
	}
	return br;
}

bool FileOpt::IsEof() const
{
	return !m_file.is_open() || m_file.eof();
}

void FileOpt::Close()
{
	if(m_file.is_open())
		m_file.close();
}

UINT FileOpt::Read( void *dataout,UINT len )
{
	if(len && dataout && m_file.is_open())
	{
		m_file.read((char*)dataout, len);
		return m_file.gcount();
	}
	return 0;
}
