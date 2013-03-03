#pragma once
#define DEBUGFLAG
#ifdef DEBUGFLAG
#define DXLOG(a) logger.Log(a)
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#define _LOGGING

class CLog
{
private:
	ofstream m_logfile;
	bool m_bLogOpen;
	char* m_pFileName;

public:

	CLog()
	{
		m_bLogOpen = false;
	}

	~CLog(void)
	{
		closeLogFile();
	}
	
	void openLogFile(char* filename)
	{
	#ifdef _LOGGING
		if(!m_bLogOpen)
		{
			m_pFileName = filename;
			m_logfile.open(m_pFileName, std::ios_base::app);
			m_bLogOpen = true;
		}
	#endif
	}

	void closeLogFile()
	{
	#ifdef _LOGGING
		if(m_bLogOpen)
		{
			m_logfile.close();
			m_bLogOpen = false;
		}
	#endif
	}

	void Log(char* msg)
	{
	#ifdef _LOGGING
		openLogFile(m_pFileName);
		m_logfile << msg  << endl;
		m_logfile.flush();
	#endif
	}


};
#else
#define DXLOG(a) 
#endif

