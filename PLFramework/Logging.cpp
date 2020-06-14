#include "pch.h"

namespace Log
{
	static std::locale loc(std::locale(), new std::codecvt_utf8<wchar_t, 0x10ffff>);

	ILog::ILog()
	{
		InitializeCriticalSection(&cs);
	}

	ILog::~ILog() noexcept
	{
		try 
		{
			Close();
		}
		catch (...) 
		{
		}
		DeleteCriticalSection(&cs);
	}

	bool ILog::Open(const TCHAR* fname)
	{
		return Open(NULL, fname);
	}

	bool ILog::Open(const TCHAR* basepath, const TCHAR* fname)
	{
		if (ofs.is_open()) {
			return false;
		}

		STD_STRING fullpath;
		if (basepath != NULL) {
			fullpath = basepath;
			if (MKDIR(fullpath.c_str()) == -1) {
				if (errno != EEXIST) {
					return false;
				}
			}
			fullpath += _T("\\");
		}

		fullpath += fname;

		ofs.open(fullpath.c_str(), std::ios_base::trunc);
		if (!ofs.is_open()) {
			return false;
		}

		ofs.imbue(loc);
		return true;
	}

	void ILog::Close()
	{
		if (ofs.is_open()) {
			ofs.close();
		}
	}

	void ILog::Message(const TCHAR* fmt, ...)
	{
		va_list args;

		va_start(args, fmt);
		_Message(fmt, args);
		va_end(args);
	}

	void ILog::_Message(const TCHAR* fmt, va_list args)
	{
		EnterCriticalSection(&cs);

		_vsnwprintf_s(fmtBuf, _TRUNCATE, fmt, args);
		Write(fmtBuf);

		LeaveCriticalSection(&cs);
	}

	void ILog::Write(const TCHAR* buf)
	{
		if (ofs.is_open()) {
			ofs << buf << std::endl;
		}
	}

	ILog gLog;
}