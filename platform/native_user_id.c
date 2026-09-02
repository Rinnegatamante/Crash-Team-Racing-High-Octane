#include <common.h>
#include "platform/native_user_id.h"

#if defined(__vita__)
#include <openssl/sha.h>
#include <psp2/kernel/openpsid.h>
#endif

enum
{
    NATIVE_USER_ID_DISPLAY_BYTES = 8,
    NATIVE_USER_ID_DISPLAY_LENGTH = NATIVE_USER_ID_DISPLAY_BYTES * 2,
};

static b32 s_nativeUserIdInitialized;
static b32 s_nativeUserIdAvailable;
static u8 s_nativeUserIdHash[NATIVE_USER_ID_HASH_SIZE];
static char s_nativeUserIdDisplay[NATIVE_USER_ID_DISPLAY_LENGTH + 1];

static void NativeUserId_Initialize(void)
{
    if (s_nativeUserIdInitialized)
    {
        return;
    }

    s_nativeUserIdInitialized = true;

#if defined(__vita__)
    static const char domain[] = "CTR-NATIVE-USER-ID-V1";
    static const char hex[] = "0123456789ABCDEF";
    SceKernelOpenPsId openPsId;
    u8 hashInput[(sizeof(domain) - 1) + sizeof(openPsId.id)];

    if (sceKernelGetOpenPsId(&openPsId) < 0)
    {
        return;
    }

    memcpy(hashInput, domain, sizeof(domain) - 1);
    memcpy(hashInput + sizeof(domain) - 1, openPsId.id, sizeof(openPsId.id));

    if (SHA256(hashInput, sizeof(hashInput), s_nativeUserIdHash) == NULL)
    {
        memset(&openPsId, 0, sizeof(openPsId));
        memset(hashInput, 0, sizeof(hashInput));
        return;
    }

    memset(&openPsId, 0, sizeof(openPsId));
    memset(hashInput, 0, sizeof(hashInput));

    for (int i = 0; i < NATIVE_USER_ID_DISPLAY_BYTES; i++)
    {
        s_nativeUserIdDisplay[i * 2 + 0] = hex[s_nativeUserIdHash[i] >> 4];
        s_nativeUserIdDisplay[i * 2 + 1] = hex[s_nativeUserIdHash[i] & 0xf];
    }
    s_nativeUserIdDisplay[NATIVE_USER_ID_DISPLAY_LENGTH] = '\0';
    s_nativeUserIdAvailable = true;
#endif
}

const char *NativeUserId_GetDisplayString(void)
{
    NativeUserId_Initialize();
    return s_nativeUserIdAvailable ? s_nativeUserIdDisplay : NULL;
}

int NativeUserId_CopyHash(unsigned char outHash[NATIVE_USER_ID_HASH_SIZE])
{
    if (outHash == NULL)
    {
        return 0;
    }

    NativeUserId_Initialize();
    if (!s_nativeUserIdAvailable)
    {
        return 0;
    }

    memcpy(outHash, s_nativeUserIdHash, NATIVE_USER_ID_HASH_SIZE);
    return 1;
}
