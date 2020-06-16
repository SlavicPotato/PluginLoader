#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (!Initialize(hModule)) {
            return FALSE;
        }
        break;
    case DLL_PROCESS_DETACH:
        Detach();
        break;
    }
    return TRUE;
}

