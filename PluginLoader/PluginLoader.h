#pragma once

bool Attach(HMODULE hModule);
bool Initialize(HMODULE hModule);
void Detach();

typedef HRESULT(WINAPI* CreateDXGIFactory_T)(REFIID riid, _COM_Outptr_ void** ppFactory);
typedef HRESULT(WINAPI* CreateDXGIFactory2_T)(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);


#ifdef _PLUGIN_LOADER_DXGI

HRESULT WINAPI CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory);
HRESULT WINAPI CreateDXGIFactory1_Hook(REFIID riid, _COM_Outptr_ void** ppFactory);
HRESULT WINAPI CreateDXGIFactory2_Hook(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);

#elif _PLUGIN_LOADER_D3D9

#include <d3d9.h>

typedef IDirect3D9* (WINAPI* Direct3DCreate9_T)(UINT SDKVersion);
typedef HRESULT (WINAPI* Direct3DCreate9Ex_T)(UINT SDKVersion, IDirect3D9Ex**);

IDirect3D9* WINAPI Direct3DCreate9_Hook(UINT SDKVersion);
HRESULT WINAPI Direct3DCreate9Ex_Hook(UINT SDKVersion, IDirect3D9Ex**);

#endif

typedef HMODULE
(WINAPI* LoadLibraryA_T)(
	_In_ LPCSTR lpLibFileName);

typedef HMODULE
(WINAPI* LoadLibraryW_T)(
	_In_ LPCWSTR lpLibFileName);

typedef HMODULE
(WINAPI* LoadLibraryExA_T)(
	_In_ LPCSTR lpLibFileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags);

typedef HMODULE
(WINAPI* LoadLibraryExW_T)(
	_In_ LPCWSTR lpLibFileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags);

#include "PluginLib.h"

class PluginDescriptorInt
{
public:
	PluginDescriptorInt()
		:m_version(0)
	{
		memset(m_pluginName, 0x0, sizeof(m_pluginName));
	}

	virtual bool __cdecl SetProxyOverride(uint32_t, FARPROC, void**);
	virtual const wchar_t* __cdecl GetExecutablePath();
	virtual void __cdecl SetPluginName(const char*);
	virtual void __cdecl SetPluginVersion(uint8_t, uint8_t, uint16_t);
	virtual void* __cdecl CreateHookInterface();
	virtual uint32_t __cdecl GetLoaderVersion();

	const char* GetPluginName();
	STD_STRING GetPluginVersionString();
	uint32_t GetPluginVersion();
	void GetPluginVersion(uint32_t& major, uint32_t& minor, uint32_t& rev);

	HMODULE hModule;
private:
	char m_pluginName[64];
	uint32_t m_version;

};

typedef bool(__cdecl* Plugin_Initialize)(void*);
typedef void(__cdecl* SKSEPlugin_Initialize)();
