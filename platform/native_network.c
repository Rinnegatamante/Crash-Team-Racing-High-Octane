#include <common.h>
#include "platform/native_network.h"
#include "platform/native_leaderboard.h"
#include "platform/native_log.h"

#if defined(__vita__)
#include <vitasdk.h>
#endif

#if defined(__vita__)
enum
{
    NATIVE_NETWORK_MEMORY_SIZE = 1024 * 1024,
};

static void *s_nativeNetworkMemory;
static b32 s_nativeNetworkInitialized;
static b32 s_nativeNetworkNetCtlInitialized;
static b32 s_nativeNetworkModuleLoadedByUs;
static b32 s_nativeNetworkRestoreRequested;
static b32 s_nativeNetworkRestoreProbeActive;
static u64 s_nativeNetworkRestoreNextProbeTimeUs;
#endif

#if defined(__vita__)
static int NativeNetwork_StartStack(void)
{
    if (sceSysmoduleIsLoaded(SCE_SYSMODULE_NET) != SCE_SYSMODULE_LOADED)
    {
        int result = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
        if (result < 0)
        {
            Platform_Log("[CTR Network] failed to load SceNet: 0x%08x\n", result);
            return 0;
        }
        s_nativeNetworkModuleLoadedByUs = true;
    }

    if (s_nativeNetworkMemory == NULL)
    {
        s_nativeNetworkMemory = malloc(NATIVE_NETWORK_MEMORY_SIZE);
        if (s_nativeNetworkMemory == NULL)
        {
            Platform_Log("[CTR Network] failed to allocate SceNet memory\n");
            return 0;
        }
    }

    if (!s_nativeNetworkInitialized)
    {
        SceNetInitParam netInitParam;
        memset(&netInitParam, 0, sizeof(netInitParam));
        netInitParam.memory = s_nativeNetworkMemory;
        netInitParam.size = NATIVE_NETWORK_MEMORY_SIZE;

        int result = sceNetInit(&netInitParam);
        if (result < 0)
        {
            Platform_Log("[CTR Network] sceNetInit failed: 0x%08x\n", result);
            return 0;
        }
        s_nativeNetworkInitialized = true;
    }

    if (!s_nativeNetworkNetCtlInitialized)
    {
        int result = sceNetCtlInit();
        if (result < 0)
        {
            Platform_Log("[CTR Network] sceNetCtlInit failed: 0x%08x\n", result);
            return 0;
        }
        s_nativeNetworkNetCtlInitialized = true;
    }

    return 1;
}

#endif

int NativeNetwork_Init(void)
{
#if defined(__vita__)
    if (s_nativeNetworkInitialized && s_nativeNetworkNetCtlInitialized)
    {
        return 1;
    }

    if (!NativeNetwork_StartStack())
    {
        NativeNetwork_Shutdown();
        return 0;
    }
    return 1;
#else
    return 0;
#endif
}

void NativeNetwork_Shutdown(void)
{
#if defined(__vita__)
    s_nativeNetworkRestoreRequested = false;
    s_nativeNetworkRestoreProbeActive = false;
    s_nativeNetworkRestoreNextProbeTimeUs = 0;

    if (s_nativeNetworkNetCtlInitialized)
    {
        sceNetCtlTerm();
        s_nativeNetworkNetCtlInitialized = false;
    }

    if (s_nativeNetworkInitialized)
    {
        sceNetTerm();
        s_nativeNetworkInitialized = false;
    }

    free(s_nativeNetworkMemory);
    s_nativeNetworkMemory = NULL;

    if (s_nativeNetworkModuleLoadedByUs)
    {
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        s_nativeNetworkModuleLoadedByUs = false;
    }
#endif
}

void NativeNetwork_RequestInternetModeRestore(void)
{
#if defined(__vita__)
    if (!s_nativeNetworkInitialized || !s_nativeNetworkNetCtlInitialized)
    {
        return;
    }

    s_nativeNetworkRestoreRequested = true;
    s_nativeNetworkRestoreProbeActive = false;
    s_nativeNetworkRestoreNextProbeTimeUs = 0;
#endif
}

void NativeNetwork_Update(void)
{
#if defined(__vita__)
    if (!s_nativeNetworkRestoreRequested)
    {
        return;
    }

    int state = SCE_NETCTL_STATE_DISCONNECTED;
    int result = sceNetCtlInetGetState(&state);
    if ((result >= 0) && (state == SCE_NETCTL_STATE_CONNECTED))
    {
        s_nativeNetworkRestoreRequested = false;
        s_nativeNetworkRestoreProbeActive = false;
        s_nativeNetworkRestoreNextProbeTimeUs = 0;
            return;
    }

    u64 now = sceKernelGetProcessTimeWide();
    if (s_nativeNetworkRestoreProbeActive)
    {
        if (!NativeLeaderboard_IsNetworkIdle())
        {
            return;
        }
        s_nativeNetworkRestoreProbeActive = false;
        s_nativeNetworkRestoreNextProbeTimeUs = now + 2000000u;
    }

    if ((now < s_nativeNetworkRestoreNextProbeTimeUs) || !NativeLeaderboard_IsNetworkIdle())
    {
        return;
    }

    if (NativeLeaderboard_RequestNetworkRestoreProbe())
    {
        s_nativeNetworkRestoreProbeActive = true;
    }
#endif
}

int NativeNetwork_IsInternetConnected(void)
{
#if defined(__vita__)
    int state = SCE_NETCTL_STATE_DISCONNECTED;

    if (!s_nativeNetworkNetCtlInitialized)
    {
        return 0;
    }

    if (sceNetCtlInetGetState(&state) < 0)
    {
        return 0;
    }

    return state == SCE_NETCTL_STATE_CONNECTED;
#else
    return 0;
#endif
}