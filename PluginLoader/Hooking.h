#pragma once

#include <vector>
#include <string>

namespace Hook
{
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
		: private Common::ICommon
	{
	public:
		typedef std::vector<std::string> IATModuleList;

		IHook();

		virtual void __cdecl Register(PVOID* target, PVOID detour);
		virtual void __cdecl Clear() noexcept;
		virtual int __cdecl Install();
		virtual LONG __cdecl GetLastError() const noexcept;
		virtual HookDescriptor* __cdecl GetLastDescriptor() const noexcept;
		virtual uintptr_t __cdecl IATHook(uintptr_t Module, const char* ImportModule, const char* API, uintptr_t Detour) noexcept;
		virtual uintptr_t __cdecl IATDelayedHook(uintptr_t Module, const char* ImportModule, const char* API, uintptr_t Detour) noexcept;
		virtual bool __cdecl GetIATModules(uintptr_t Module, IATModuleList& out);

	private:
		LONG m_lasterr;
		HookDescriptor* m_lasthookdesc;
		std::vector<HookDescriptor> hooks;
	};


}