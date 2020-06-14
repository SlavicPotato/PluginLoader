#include "pch.h"

namespace Debug
{
	void AssertionFailed(const TCHAR* file, unsigned long line, const TCHAR* desc)
	{
		MESSAGE(_T("Assertion failed in %s (%d): %s"), file, line, desc);
		__debugbreak();
	}

	void WaitForDebugger(void)
	{
		while (!IsDebuggerPresent())
		{
			Sleep(10);
		}

		Sleep(1000 * 2);
	}
}