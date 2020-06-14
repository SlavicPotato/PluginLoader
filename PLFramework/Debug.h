#pragma once

namespace Debug 
{
	void AssertionFailed(const TCHAR* file, unsigned long line, const TCHAR* desc);

	void WaitForDebugger(void);

}

#define ASSERT(x) { if (!(x)) { Debug::AssertionFailed(_T(__FILE__), __LINE__, _T(#x)); } }
