#pragma once

namespace Memory
{
	__inline void SafeWrite(uintptr_t addr, const void* data, size_t len)
	{
		DWORD oldProtect;
		VirtualProtect(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(reinterpret_cast<void*>(addr), data, len);
		VirtualProtect(reinterpret_cast<void*>(addr), len, oldProtect, &oldProtect);
	}

	__inline void SafeMemset(uintptr_t addr, int val, size_t len)
	{
		DWORD oldProtect;
		VirtualProtect(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &oldProtect);
		memset(reinterpret_cast<void*>(addr), val, len);
		VirtualProtect(reinterpret_cast<void*>(addr), len, oldProtect, &oldProtect);
	}

	__inline void SafeWrite(uintptr_t addr, void* val)
	{
		DWORD oldProtect;
		VirtualProtect(reinterpret_cast<void*>(addr), sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
		InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(addr), val);
		VirtualProtect(reinterpret_cast<void*>(addr), sizeof(uintptr_t), oldProtect, &oldProtect);
	}

	__inline void SafeWrite(uintptr_t addr, uintptr_t val)
	{
		SafeWrite(addr, reinterpret_cast<void*>(val));
	}

	template <typename T>
	__inline void SafeWrite(uintptr_t addr, T val)
	{
		DWORD oldProtect;
		VirtualProtect(reinterpret_cast<void*>(addr), sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
		InterlockedExchange(reinterpret_cast<volatile T*>(addr), val);
		VirtualProtect(reinterpret_cast<void*>(addr), sizeof(uintptr_t), oldProtect, &oldProtect);
	}

	template <typename T>
	void MemZeroVolatile(volatile T* s, size_t n)
	{
		volatile T* p = s;
		while (n-- > 0) {
			*p++ = 0;
		}
	}

}