#pragma once

#include "PluginLib.h"
#include "IHook.h"

class IPlugin
{
public:
	virtual bool __cdecl SetProxyOverride(uint32_t, FARPROC, void**) = 0;
	virtual const wchar_t* __cdecl GetExecutablePath() = 0;
	virtual void __cdecl SetPluginName(const char*) = 0;
	virtual void __cdecl SetPluginVersion(uint8_t, uint8_t, uint16_t) = 0;

	virtual IHook * __cdecl CreateHookInterface() = 0;
	virtual uint32_t __cdecl GetLoaderVersion() = 0;
	//virtual IHookLogged* CreateLoggedHookInterface(Log::ILog* log) = 0;

	IScoped<IHook> GetHookInterface()
	{
		return IScoped<IHook>(this->CreateHookInterface());
	}
};