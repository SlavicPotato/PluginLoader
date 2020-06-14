#pragma once

#include "VFT.h"

class HookDescriptor
{
public:
	HookDescriptor(PVOID* target, PVOID detour) :
		target(target), detour(detour)
	{
	}

	PVOID* target;
	PVOID detour;
};

class IHook
	: public ICommon
{
	friend class IScoped<IHook>;

public:
	typedef std::vector<std::string> IATModuleList;

	virtual void __cdecl Register(PVOID* target, PVOID detour) = 0;
	virtual void __cdecl Clear() noexcept = 0;
	virtual int __cdecl Install() = 0;
	virtual LONG __cdecl GetLastError() const noexcept = 0;
	virtual HookDescriptor* __cdecl GetLastDescriptor() const noexcept = 0;
	virtual uintptr_t __cdecl IATHook(uintptr_t Module, const char* ImportModule, const char* API, uintptr_t Detour) noexcept = 0;
	virtual uintptr_t __cdecl IATDelayedHook(uintptr_t Module, const char* ImportModule, const char* API, uintptr_t Detour) noexcept = 0;
	virtual bool __cdecl GetIATModules(uintptr_t Module, IATModuleList& out) = 0;

	template <typename T>
	static bool DetourVTable(void* cl, uint32_t offset, void* newp, T* oldp) noexcept
	{
		return WriteVFT_s(cl, offset, newp, oldp);
	}
};
/*
class VFTAddressHolder
{
	typedef std::unordered_map<void*, void*> VFTAddressHolder_T;
public:

	template <typename T>
	T Get(void* i)
	{
		return reinterpret_cast<T>(m_data[i]);
	}

	VFTAddressHolder_T & operator*()
	{
		return m_data;
	}
private:
	std::unordered_map<void*, void*> m_data;
};*/

class IHookLogged
{
public:
	IHookLogged(IHook* ihook, Log::ILog* log)
		: m_hook(ihook), m_log(log)
	{
	}

	IHook* operator->() const
	{
		return m_hook;
	}

	operator IHook* ()
	{
		return m_hook;
	}

	int InstallHooks()
	{
		int r = m_hook->Install();

		switch (r)
		{
		/*case -1:
			m_log->Message(_T("%s: No hooks were registered"), _T(__FUNCTION__));
			break;*/
		case -2:
			m_log->Message(_T("%s: DetourTransactionBegin failed (%lu)"),
				_T(__FUNCTION__), GetLastError());
			break;
		case -3:
			m_log->Message(_T("%s: DetourUpdateThread failed (%lu)"),
				_T(__FUNCTION__), GetLastError());
			break;
		case -4: {
			auto hd = m_hook->GetLastDescriptor();
			m_log->Message(_T("%s: DetourAttach failed (%lu) (%p -> %p)"),
				_T(__FUNCTION__), GetLastError(), *hd->target, hd->detour);
			break;
		}
		case -5:
			m_log->Message(_T("%s: DetourTransactionCommit failed (%lu)"),
				_T(__FUNCTION__), GetLastError());
			break;
		}

		return r;
	}

private:
	Log::ILog* m_log;
	IHook* m_hook;
};
