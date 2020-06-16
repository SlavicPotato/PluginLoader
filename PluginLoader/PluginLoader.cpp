#include "pch.h"
#include <detours\Detours.h>

using namespace Log;

static HMODULE hModuleLocal = nullptr;
static HMODULE hProxy = nullptr;

extern "C" {
#ifdef _PLUGIN_LOADER_DXGI
    volatile FARPROC procs[19];
#elif _PLUGIN_LOADER_D3D9
    volatile FARPROC procs[17];
#endif
}

#ifdef _PLUGIN_LOADER_DXGI
#define PLUGIN_NAME "dxgi"
#elif _PLUGIN_LOADER_D3D9
#define PLUGIN_NAME "d3d9"
#endif

static STD_STRING logName(
    _T(PLUGIN_NAME) _T(".log")
);

static STD_STRING pluginsPath(_T("\\DLLPlugins"));
static STD_STRING SKSEPluginsPath(_T("\\Data\\SKSE\\Plugins"));
static STD_STRING ExecutablePath;

static bool isAttached = false;
static bool pluginsLoaded = false;

static IConfig conf;
static std::vector<PluginDescriptorInt*> plugins;

static std::locale loc(std::locale(), new std::codecvt_utf8<wchar_t, 0x10ffff>);

const char* procNames[] = {
#ifdef _PLUGIN_LOADER_DXGI
    "ApplyCompatResolutionQuirking",
    "CompatString",
    "CompatValue",
    "DXGIDumpJournal",
    "PIXBeginCapture",
    "PIXEndCapture",
    "PIXGetCaptureState",
    "SetAppCompatStringPointer",
    "UpdateHMDEmulationStatus",
    "CreateDXGIFactory",
    "CreateDXGIFactory1",
    "CreateDXGIFactory2",
    "DXGID3D10CreateDevice",
    "DXGID3D10CreateLayeredDevice",
    "DXGID3D10GetLayeredDeviceSize",
    "DXGID3D10RegisterLayers",
    "DXGIDeclareAdapterRemovalSupport",
    "DXGIGetDebugInterface1",
    "DXGIReportAdapterConfiguration"
#elif _PLUGIN_LOADER_D3D9
    "Direct3DCreate9On12",
    "Direct3DCreate9On12Ex",
    "Direct3DShaderValidatorCreate9",
    "PSGPError",
    "PSGPSampleTexture",
    "D3DPERF_BeginEvent",
    "D3DPERF_EndEvent",
    "D3DPERF_GetStatus",
    "D3DPERF_QueryRepeatFrame",
    "D3DPERF_SetMarker",
    "D3DPERF_SetOptions",
    "D3DPERF_SetRegion",
    "DebugSetLevel",
    "DebugSetMute",
    "Direct3D9EnableMaximizedWindowedModeShim",
    "Direct3DCreate9",
    "Direct3DCreate9Ex"
#endif
};

constexpr size_t PN_SIZE = (sizeof(procNames) / sizeof(const char*));

bool PluginDescriptorInt::SetProxyOverride(uint32_t index, FARPROC p, void** oldp)
{
    if (index < PN_SIZE)
    {
        *oldp = InterlockedExchangePointer(
            reinterpret_cast<volatile PVOID*>(&procs[index]),
            reinterpret_cast<PVOID>(p));

        return true;
    }
    return false;
}

const wchar_t* PluginDescriptorInt::GetExecutablePath()
{
    return
#ifdef UNICODE
        ExecutablePath
#else
        StrHelpers::to_wstring(ExecutablePath)
#endif
        .c_str();
}

void PluginDescriptorInt::SetPluginName(const char* name)
{
    _snprintf_s(m_pluginName, _TRUNCATE, "%s", name);
}

void PluginDescriptorInt::SetPluginVersion(uint8_t major, uint8_t minor, uint16_t rev)
{
    m_version = MAKE_PLUGIN_VERSION(
        static_cast<uint32_t>(major),
        static_cast<uint32_t>(minor),
        static_cast<uint32_t>(rev));
}

void* PluginDescriptorInt::CreateHookInterface()
{
    return reinterpret_cast<void*>(new Hook::IHook());
}

uint32_t PluginDescriptorInt::GetLoaderVersion()
{
#ifdef _PLUGIN_LOADER_DXGI
    return Loader::Version::DXGI;
#elif _PLUGIN_LOADER_D3D9
    return Loader::Version::D3D9;
#endif
}

const char* PluginDescriptorInt::GetPluginName()
{
    return const_cast<const char*>(m_pluginName);
}

STD_STRING PluginDescriptorInt::GetPluginVersionString()
{
    STD_OSSTREAM ss;
    ss << GET_PLUGIN_VERSION_MAJOR(m_version) << _T(".")
        << GET_PLUGIN_VERSION_MINOR(m_version) << _T(".")
        << GET_PLUGIN_VERSION_REV(m_version);
    return ss.str();
}

uint32_t PluginDescriptorInt::GetPluginVersion()
{
    return m_version;
}

void PluginDescriptorInt::GetPluginVersion(uint32_t& major, uint32_t& minor, uint32_t& rev)
{
    major = GET_PLUGIN_VERSION_MAJOR(m_version);
    minor = GET_PLUGIN_VERSION_MINOR(m_version);
    rev = GET_PLUGIN_VERSION_MINOR(m_version);
}

static int LoadPlugin(const TCHAR* path, PluginDescriptorInt* pd, bool is_skse)
{
    int state = -1;
    __try
    {
        HMODULE lib = LoadLibrary(path);
        if (lib == NULL)
            return 0;

        pd->hModule = lib;

        int ok = 1;
        FARPROC funcAddr = GetProcAddress(lib, "Initialize");
        if (funcAddr != 0)
        {
            state = -2;

            if (is_skse) {
                ((SKSEPlugin_Initialize)funcAddr)();
                ok = 4;
            }
            else {
                if (((Plugin_Initialize)funcAddr)(reinterpret_cast<void*>(pd))) {
                    ok = 3;
                }
                else {
                    ok = 2;
                }
            }
        }

        plugins.push_back(pd);

        return ok;
    }
    __except (1)
    {
    }

    return state;
}

static STD_STRING GetDLLPluginsDirectory()
{
    return ExecutablePath + pluginsPath;
}

static STD_STRING GetSKSEPluginsDirectory()
{
    return ExecutablePath + SKSEPluginsPath;
}

static bool LoadPlugins(STD_STRING& dir, std::vector<STD_STRING>& list, bool is_skse)
{
    for (auto file : list)
    {
        STD_STRING fileName(dir + _T('\\') + file);

        gLog.Message(_T("Attempting to load %s"), file.c_str());

        PluginDescriptorInt* pd = new PluginDescriptorInt;

        int result = LoadPlugin(fileName.c_str(), pd, is_skse);

        switch (result)
        {
        case 4:
            gLog.Message(_T("%s loaded and Initialize() called"), file.c_str());
            break;
        case 2:
        case 3:
            gLog.Message(_T("Loaded \"%s\" version %s (Initialize() = %s)"),
                StrHelpers::StrToNative(pd->GetPluginName()).c_str(),
                pd->GetPluginVersionString().c_str(),
                (result == 2 ? _T("false") : _T("true")));
            break;
        case 1:
            gLog.Message(_T("%s loaded but has no initializer function defined"), file.c_str());
            break;
        case 0:
            gLog.Message(_T("%s: LoadLibrary failed"), file.c_str());
            break;
        case -1:
            gLog.Message(_T("%s: LoadLibrary crashed"), file.c_str());
            break;
        case -2:
            gLog.Message(_T("%s: Initialize() crashed"), file.c_str());
            break;
        }

        if (result < 1) {
            delete pd;
        }
    }

    return true;
}

static bool LoadDLLPlugins()
{
    WIN32_FIND_DATA wfd;

    std::vector<STD_STRING> filesToLoad;
    STD_STRING dir = GetDLLPluginsDirectory();
    STD_STRING search_dir = dir + _T("\\*.dll");
    HANDLE hFind = FindFirstFile(search_dir.c_str(), &wfd);

    if (hFind == INVALID_HANDLE_VALUE) {
        gLog.Message(_T("No plugins found in %s"), dir.c_str());
        return false;
    }

    gLog.Message(_T("Searching for plugins in %s"), dir.c_str());

    do
    {
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        filesToLoad.push_back(wfd.cFileName);

    } while (FindNextFile(hFind, &wfd) != 0);

    FindClose(hFind);

    return LoadPlugins(dir, filesToLoad, false);
}

static bool LoadSKSEPlugins()
{
    std::vector<STD_STRING> filesToLoad;
    WIN32_FIND_DATA wfd;
    STD_STRING dir = GetSKSEPluginsDirectory();
    STD_STRING search_dir = dir + _T("\\*.txt");
    HANDLE hFind = FindFirstFile(search_dir.c_str(), &wfd);

    if (hFind == INVALID_HANDLE_VALUE) {
        gLog.Message(_T("No plugins found in %s"), dir.c_str());
        return false;
    }

    gLog.Message(_T("Searching for compatible plugins in %s"), dir.c_str());

    do
    {
        if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            continue;

        STD_STRING fileName = wfd.cFileName;

        if (fileName.length() < 4)
            continue;

        STD_STRING ext = fileName.substr(fileName.length() - 4, 4);

        if (_wcsicmp(ext.c_str(), _T(".txt")) != 0)
            continue;

        fileName = fileName.substr(0, fileName.length() - 4);

        if (fileName.length() < 8)
            continue;

        STD_STRING end = fileName.substr(fileName.length() - 8, 8);

        if (_wcsicmp(end.c_str(), _T("_preload")) != 0)
            continue;

        fileName = fileName.substr(0, fileName.length() - 8) + _T(".dll");

        filesToLoad.push_back(fileName);

    } while (FindNextFile(hFind, &wfd));

    FindClose(hFind);

    return LoadPlugins(dir, filesToLoad, true);
}

static void UnloadPlugins()
{
    for (const auto lib : plugins) {
        FreeLibrary(lib->hModule);
        delete lib;
    }
    plugins.clear();
}

bool load_skse = false;

static void InitializePlugins()
{
    static volatile LONG isInitialized = FALSE;

    if (InterlockedCompareExchangeAcquire(&isInitialized, TRUE, FALSE)) {
        return;
    }

    LoadDLLPlugins();
    if (load_skse) {
        LoadSKSEPlugins();
    }

    FlushInstructionCache(GetCurrentProcess(), NULL, 0);

    //gLog.Close();
}

typedef PVOID(*_initterm_e_t)(PVOID, PVOID);
_initterm_e_t _initterm_e_c;


static PVOID _initterm_e_hook(PVOID arg1, PVOID arg2)
{
    InitializePlugins();
    return _initterm_e_c(arg1, arg2);
}

//typedef HRESULT(WINAPI* CreateDXGIFactory_T)(REFIID riid, _COM_Outptr_ void** ppFactory);

#ifdef _PLUGIN_LOADER_DXGI

HRESULT WINAPI CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    //gLog.Message(_T("Triggering: %s"), _T(__FUNCTION__));
    InitializePlugins();

    auto index = Misc::Underlying(Plugin::Proxy::Index::kCreateDXGIFactory);
    return reinterpret_cast<CreateDXGIFactory_T>(procs[index])(riid, ppFactory);
}

HRESULT WINAPI CreateDXGIFactory1_Hook(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    //gLog.Message(_T("Triggering: %s"), _T(__FUNCTION__));
    InitializePlugins();

    auto index = Misc::Underlying(Plugin::Proxy::Index::kCreateDXGIFactory1);
    return reinterpret_cast<CreateDXGIFactory_T>(procs[index])(riid, ppFactory);
}

HRESULT WINAPI CreateDXGIFactory2_Hook(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory)
{
    //gLog.Message(_T("Triggering: %s"), _T(__FUNCTION__));
    InitializePlugins();

    auto index = Misc::Underlying(Plugin::Proxy::Index::kCreateDXGIFactory2);
    return reinterpret_cast<CreateDXGIFactory2_T>(procs[index])(Flags, riid, ppFactory);
}


#elif _PLUGIN_LOADER_D3D9

IDirect3D9* WINAPI Direct3DCreate9_Hook(UINT SDKVersion)
{
    InitializePlugins();

    auto index = Misc::Underlying(Plugin::ProxyD3D9::Index::kDirect3DCreate9);
    return reinterpret_cast<Direct3DCreate9_T>(procs[index])(SDKVersion);
}

HRESULT WINAPI Direct3DCreate9Ex_Hook(UINT SDKVersion, IDirect3D9Ex** d)
{
    InitializePlugins();

    auto index = Misc::Underlying(Plugin::ProxyD3D9::Index::kDirect3DCreate9Ex);
    return reinterpret_cast<Direct3DCreate9Ex_T>(procs[index])(SDKVersion, d);
}

#endif

static LoadLibraryA_T LoadLibraryA_O;
static LoadLibraryW_T LoadLibraryW_O;
static LoadLibraryExA_T LoadLibraryExA_O;
static LoadLibraryExW_T LoadLibraryExW_O;

static HMODULE
WINAPI
LoadLibraryW_Hook(
    _In_ LPCWSTR lpLibFileName
)
{
    InitializePlugins();
    return LoadLibraryW_O(lpLibFileName);
}

static HMODULE
WINAPI
LoadLibraryA_Hook(
    _In_ LPCSTR lpLibFileName
)
{
    InitializePlugins();
    return LoadLibraryA_O(lpLibFileName);
}

static HMODULE
WINAPI
LoadLibraryExA_Hook(
    _In_ LPCSTR lpLibFileName,
    _Reserved_ HANDLE hFile,
    _In_ DWORD dwFlags
)
{
    InitializePlugins();
    return LoadLibraryExA_O(lpLibFileName, hFile, dwFlags);
}

static HMODULE
WINAPI
LoadLibraryExW_Hook(
    _In_ LPCWSTR lpLibFileName,
    _Reserved_ HANDLE hFile,
    _In_ DWORD dwFlags
)
{
    InitializePlugins();
    return LoadLibraryExW_O(lpLibFileName, hFile, dwFlags);
}


bool Initialize(HMODULE hModule)
{
    bool r = Attach(hModule);
    if (!r) {
        gLog.Close();
    }
    return r;
}

template <typename T>
static int InstallIATHook(T& o, HMODULE h, const char* l, const char* i, void* t)
{
    auto orig = reinterpret_cast<T>(
        Detours::IATHook(reinterpret_cast<uintptr_t>(h),
            l, i, reinterpret_cast<uintptr_t>(t)));

    if (orig == nullptr) {
        gLog.Message(_T("Could not install IAT hook for import %s from %s"),
            StrHelpers::StrToNative(i).c_str(), StrHelpers::StrToNative(l).c_str());
        return 0;
    }

    o = orig;
    return 1;
}

static void IncModuleRefCount(HMODULE hModule)
{
    TCHAR moduleName[MAX_PATH];
    GetModuleFileName(hModule, moduleName, MAX_PATH);
    hModuleLocal = LoadLibrary(moduleName);
}

bool Attach(HMODULE hModule)
{
    isAttached = true;

    Memory::MemZeroVolatile(procs, PN_SIZE);

    // ensure we remain loaded
    IncModuleRefCount(hModule);

    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    TCHAR exeFolder[MAX_PATH];
    _tcscpy_s(exeFolder, exePath);
    PathRemoveFileSpec(exeFolder);

    ExecutablePath = exeFolder;

    gLog.Open((ExecutablePath + (_T("\\") + logName)).c_str());

    /*Hook::IHook h;
    Hook::IHook::IATModuleList l;
    h.GetIATModules((uintptr_t)GetModuleHandle(NULL), l);
    for (const auto& i : l) {
        log << ">>> " << i.c_str() << std::endl;
    }*/

    LoadLibrary((ExecutablePath + (_T("\\") PLUGIN_NAME ".dll")).c_str());

    gLog.Message(_T("Generic plugin loader - " _T(PLUGIN_NAME) ".dll"));

    TCHAR dllPath[MAX_PATH];
    if (!GetSystemDirectory(dllPath, MAX_PATH)) {
        gLog.Message(_T("Couldn't get system dir"));
        return false;
    }

    if (_tcscat_s(dllPath, MAX_PATH, _T("\\" _T(PLUGIN_NAME) ".dll")) != 0) {
        gLog.Message(_T("Couldn't build dll path"));
        return false;
    }

    hModule = LoadLibrary(dllPath);
    if (hModule == NULL) {
        gLog.Message(_T("Couldn't load %s"), dllPath);
        return false;
    }

    for (uint32_t i = 0; i < PN_SIZE; i++) {
        if (procNames[i] != nullptr) {
            procs[i] = GetProcAddress(hModule, procNames[i]);
        }
        else {
            procs[i] = nullptr;
        }
    }

    /*ExecutablePath = exePath;
    auto pos = ExecutablePath.rfind(_T('\\'));
    if (pos != STD_STRING::npos) {
        ExecutablePath = ExecutablePath.substr(0, pos);
    }*/

    gLog.Message(_T("Loaded into %s"), PathFindFileName(exePath));

    //STD_STRING proxyDll = ExecutablePath + _T("\\DLLProxy\\dxgi.dll");

    TCHAR proxyDll[MAX_PATH];
    _tcscpy_s(proxyDll, exeFolder);

    if (conf.Load(StrHelpers::ToString(ExecutablePath) + "\\" PLUGIN_NAME ".ini") != 0) {
        gLog.Message(_T("Couldn't load config file"));
    }

    if (_tcscat_s(proxyDll, MAX_PATH, _T("\\DLLProxy\\" _T(PLUGIN_NAME) ".dll")) == 0) {
        DWORD dwAttrib = GetFileAttributes(proxyDll);
        if (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
        {
            hProxy = LoadLibrary(proxyDll);
            if (hProxy != NULL) {
                gLog.Message(_T("Proxying to: %s"), proxyDll);

                for (uint32_t i = 0; i < PN_SIZE; i++) {
                    if (procNames[i] != nullptr) {
                        FARPROC p = GetProcAddress(hProxy, procNames[i]);
                        if (p != NULL) {
                            procs[i] = p;
                        }
                    }
                }
            }
            else {
                gLog.Message(_T("Failed loading proxy library: %s"), proxyDll);
            }
        }
    }
    else {
        gLog.Message(_T("Couldn't build proxy dll path"));
    }

    int32_t delayed = conf.Get("Main", "DelayedLoad", 1);
    load_skse = conf.Get("Main", "LoadSKSEPlugins", false);

    if (delayed == 3) {
        gLog.Message(_T("WARNING: immediate load could cause deadlocks or crashes"));
        InitializePlugins();
    }
    else if (delayed == 2)
    {
        _initterm_e_c = reinterpret_cast<_initterm_e_t>(
            Detours::IATHook(
                reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr)),
                "api-ms-win-crt-runtime-l1-1-0.dll", "_initterm_e",
                reinterpret_cast<uintptr_t>(_initterm_e_hook)));

        if (_initterm_e_c == 0) {
            gLog.Message(_T("Hooking _initterm_e failed, plugins won't be loaded"));
        }
        else {
            gLog.Message(_T("Delaying plugin load until _initterm_e is called"));
        }
    }
    else if (delayed == 1)
    {
        auto lib = StrHelpers::StrToNative(conf.Get("Main", "IATHookModule", ""));
        HMODULE handle;
        if (lib == _T("")) {
            handle = GetModuleHandle(nullptr);
        }
        else {
            handle = GetModuleHandle(lib.c_str());
        }

        if (handle != nullptr)
        {
            int r = InstallIATHook(LoadLibraryA_O, handle, "kernel32.dll", "LoadLibraryA", LoadLibraryA_Hook);
            r += InstallIATHook(LoadLibraryW_O, handle, "kernel32.dll", "LoadLibraryW", LoadLibraryW_Hook);
            r += InstallIATHook(LoadLibraryExA_O, handle, "kernel32.dll", "LoadLibraryExA", LoadLibraryExA_Hook);
            r += InstallIATHook(LoadLibraryExW_O, handle, "kernel32.dll", "LoadLibraryExW", LoadLibraryExW_Hook);

            if (r == 0) {
                gLog.Message(_T("No IAT hooks were installed, loading delayed until CreateDXGIFactory is called"));
            }
        }
        else {
            gLog.Message(_T("Couldn't get handle to %s"), lib.c_str());
        }
    }
    else {
        gLog.Message(_T("Plugins will be loaded once CreateDXGIFactory is called"));
    }

    //FlushInstructionCache(GetCurrentProcess(), NULL, 0);

    return true;
}

void Detach()
{
    UnloadPlugins();

    if (isAttached) {
        if (hProxy != nullptr) {
            FreeLibrary(hProxy);
        }
        if (hModuleLocal != nullptr) {
            FreeLibrary(hModuleLocal);
        }
        isAttached = false;
    }
}
