#pragma once

__inline uintptr_t* GetVFT(void* p) noexcept
{
	return reinterpret_cast<uintptr_t*>(
		reinterpret_cast<uintptr_t*>(p)[0]);
}

template <typename T>
__inline T GetVFTProc(void* p, uint32_t offset) noexcept
{
	return reinterpret_cast<T>(GetVFT(p)[offset]);
}

template <typename T>
__inline
bool WriteVFT(
	void* cl,
	uint32_t offset,
	void* newp,
	T* oldp) noexcept
{
	auto addr = reinterpret_cast<LPVOID>(&GetVFT(cl)[offset]);

	DWORD oldProtect;
	if (!VirtualProtect(addr, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*oldp = reinterpret_cast<T>(
		InterlockedExchangePointer(
			reinterpret_cast<volatile PVOID*>(addr), newp));

	VirtualProtect(addr, sizeof(void*), oldProtect, &oldProtect);
	return true;
}


template <typename T>
__inline
bool VFT_InterlockedExchangeIfNotEqual(volatile PVOID*addr, void* newp, T* oldp)
{
	PVOID initial;
	do
	{
		initial = *addr;
		if (initial == newp) {
			return false;
		}
	} while (InterlockedCompareExchangePointer(addr, newp, initial) != initial);

	*oldp = reinterpret_cast<T>(initial);

	return true;
}

template <typename T>
__inline
bool WriteVFT_s(
	void* cl,
	uint32_t offset,
	void* newp,
	T* oldp) noexcept
{
	auto addr = reinterpret_cast<LPVOID>(&GetVFT(cl)[offset]);

	DWORD oldProtect;
	if (!VirtualProtect(addr, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	bool r = VFT_InterlockedExchangeIfNotEqual(
		reinterpret_cast<volatile PVOID*>(addr), 
		newp, oldp);
	
	VirtualProtect(addr, sizeof(void*), oldProtect, &oldProtect);
	return r;
}