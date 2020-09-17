#pragma once

#include "pubheader.h"
#include <fstream> 
class FileOpt
{
public:
	bool Open(LPCTSTR lpFile,bool isRead);
	bool IsOpend() const;
	void Close();
	bool Write(const void *data,UINT len,bool bFlush=false);
	bool IsEof() const;
	UINT Read(void *dataout,UINT len);
	FileOpt(void);
	~FileOpt(void);

protected:
	std::fstream m_file;
};

