// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Minhook.h>
#include <thread>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    return TRUE;
}

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)
const char* message = "Created by Jan4V, half-life:alyx only, not other source 2 games";

typedef void(__fastcall* source2)(HINSTANCE, HINSTANCE, LPWSTR, long long, const char*, const char*);
typedef char(__fastcall* vrinit)(char*);
typedef __int64(__fastcall* dedicatedinit)(__int64, __int64, __int64);
typedef __int64(__fastcall* cl_cvar)(__int64, __int64);

LPVOID origVRInit, origDedicatedServer, origConvarSetup;


char __fastcall VRInitHook(char* settings)
{
    settings[299] = 0; //VRMode = false;

    return ((vrinit)origVRInit)(settings);
}
__int64 __fastcall DedicatedServerHook(__int64 rcx, __int64 rdx, __int64 r8)
{
    *(DWORD*)(rdx + 100) = 64; //MaxMaxPlayers = 64;
    *(char*)(rdx + 88) = 1; //SteamMode = true;
    
    return ((dedicatedinit)origDedicatedServer)(rcx, rdx, r8);
}

__int64 __fastcall ConvarSetupHook(__int64 a1, __int64 a2)
{
    *(uint64_t*)(a2 + 40) &= ~3; //Flags &= ~3; // removes hidden and dev flags

    return ((cl_cvar)origConvarSetup)(a1, a2);
}

EXTERN_DLL_EXPORT void __fastcall Source2Main(HINSTANCE a, HINSTANCE b, LPWSTR c, long long d, const char* e, const char* f)
{
    auto lib2 = LoadLibraryExA("vstdlib.dll", NULL, 8);
    MH_Initialize();
    //0x9AF0
    MH_CreateHook((LPVOID)(((unsigned long long)lib2) + 0x9AB1 + strlen(message)), &ConvarSetupHook, &origConvarSetup);
    MH_EnableHook((LPVOID)(((unsigned long long)lib2) + 0x9AB1 + strlen(message)));

    auto lib = LoadLibraryExA("engine2.dll", NULL, 8);
    source2 init = (source2)GetProcAddress(lib, "Source2Main");

    char* line = GetCommandLineA();

    if (!strstr(line, "-dedicated"))
    {
        if (!strstr(line, "-vr"))
        {
            //0x103760
            MH_CreateHook((LPVOID)(((unsigned long long)lib) + 0x103721 + strlen(message)), &VRInitHook, &origVRInit);
            MH_EnableHook((LPVOID)(((unsigned long long)lib) + 0x103721 + strlen(message)));
        }
    }
    else
    {
        //0x636D0
        MH_CreateHook((LPVOID)(((unsigned long long)lib) + 0x63691 + strlen(message)), &DedicatedServerHook, &origDedicatedServer);
        MH_EnableHook((LPVOID)(((unsigned long long)lib) + 0x63691 + strlen(message)));
    }
    
    auto modParam = strstr(line, "-game ");
    if (modParam != 0)
    {
        modParam += 6;

        auto len = strchr(modParam, ' ');

        if (len == 0)
            len = modParam + strlen(modParam) + 1;

        char* modname = new char[(int)(len - modParam) + 1];

        memcpy(modname, modParam, (int)(len - modParam));
        modname[(int)(len - modParam)] = '\0';

        init(a, b, c, d, e, modname);
    }
    else
        init(a, b, c, d, e, f);

}