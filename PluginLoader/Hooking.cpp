#include "pch.h"

#include "detours.h"
#include "detours/Detours.h"
#include "VFT.h"

namespace Hook
{
	IHook::IHook()
		: m_lasterr(0), m_lasthookdesc(nullptr)
	{
	}

	void __cdecl IHook::Register(PVOID* target, PVOID detour)
	{
		hooks.push_back(HookDescriptor(target, detour));
	}

	void __cdecl IHook::Clear() noexcept
	{
		hooks.clear();
	}

	int __cdecl IHook::Install()
	{
		if (!hooks.size()) {
			return -1;
		}

		m_lasterr = DetourTransactionBegin();
		if (m_lasterr != NO_ERROR) {
			return -2;
		}

		int r = 0;

		m_lasterr = DetourUpdateThread(GetCurrentThread());
		if (m_lasterr != NO_ERROR) {
			r = -3;
			goto abort_detours;
		}

		for (const auto& hd : hooks)
		{
			m_lasthookdesc = const_cast<HookDescriptor*>(&hd);

			PVOID target, detour;
			PDETOUR_TRAMPOLINE trampoline;
			m_lasterr = DetourAttachEx(hd.target, hd.detour, &trampoline, &target, &detour);
			if (m_lasterr != NO_ERROR) {
				r = -4;
				goto abort_detours;
			}
		}

		m_lasterr = DetourTransactionCommit();
		if (m_lasterr != NO_ERROR) {
			r = -5;
			goto abort_detours;
		}

		hooks.clear();

		return r;

	abort_detours:
		DetourTransactionAbort();
		hooks.clear();
		return r;
	}

	uintptr_t __cdecl IHook::IATHook(uintptr_t Module, const char* ImportModule, const char* API, uintptr_t Detour) noexcept
	{
		return Detours::IATHook(Module, ImportModule, API, Detour);
	}

	uintptr_t __cdecl IHook::IATDelayedHook(uintptr_t Module, const char* ImportModule, const char* API, uintptr_t Detour) noexcept
	{
		return Detours::IATDelayedHook(Module, ImportModule, API, Detour);
	}

	bool __cdecl IHook::GetIATModules(uintptr_t Module, IATModuleList& out)
	{
		auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(Module);
		auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(Module + dosHeader->e_lfanew);

		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return false;

		if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
			return false;

		if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
			return false;

		auto dataDirectory = ntHeaders->OptionalHeader.DataDirectory;
		auto sectionRVA = dataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		auto sectionSize = dataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

		if (sectionRVA == 0 || sectionSize == 0)
			return false;

		auto importDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(Module + sectionRVA);

		for (size_t i = 0; importDescriptor[i].Name != 0; i++)
		{
			auto dllName = reinterpret_cast<PSTR>(Module + importDescriptor[i].Name);
			out.push_back(dllName);
		}

		return true;
	}

	LONG __cdecl IHook::GetLastError() const noexcept
	{
		return m_lasterr;
	}

	HookDescriptor* __cdecl IHook::GetLastDescriptor() const noexcept
	{
		return m_lasthookdesc;
	}


}