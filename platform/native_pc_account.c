#include <common.h>
#include "platform/native_pc_account.h"

#if defined(_WIN32) && !defined(__vita__)
#include "platform/native_log.h"
#include <SDL3/SDL.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define NATIVE_PC_ACCOUNT_KEY_HEADER "CTR-HIGH-OCTANE-PC-KEY-V1"

typedef struct NativePcAccountState
{
    b32 initialized;
    b32 available;
    b32 verified;
    char token[96];
    char publicId[24];
    char username[18];
    char keyPath[512];
} NativePcAccountState;

static NativePcAccountState s_nativePcAccount;

static char *NativePcAccount_Trim(char *text)
{
    while ((*text != '\0') && isspace((unsigned char)*text)) text++;
    char *end = text + strlen(text);
    while ((end > text) && isspace((unsigned char)end[-1])) *--end = '\0';
    return text;
}

static b32 NativePcAccount_IsTokenValid(const char *token)
{
    if ((token == NULL) || (strlen(token) != 43)) return false;
    for (const char *p = token; *p != '\0'; p++)
    {
        if (!(isalnum((unsigned char)*p) || (*p == '-') || (*p == '_'))) return false;
    }
    return true;
}

int NativePcAccount_Init(void)
{
    if (s_nativePcAccount.initialized) return s_nativePcAccount.available != 0;
    memset(&s_nativePcAccount, 0, sizeof(s_nativePcAccount));
    s_nativePcAccount.initialized = true;

    const char *basePath = SDL_GetBasePath();
    if (basePath == NULL) basePath = ".\\";
    int pathLength = snprintf(s_nativePcAccount.keyPath, sizeof(s_nativePcAccount.keyPath), "%shighoctane.key", basePath);
    if ((pathLength <= 0) || ((size_t)pathLength >= sizeof(s_nativePcAccount.keyPath))) return 0;

    FILE *file = fopen(s_nativePcAccount.keyPath, "rb");
    if (file == NULL) return 0;

    char contents[1024];
    size_t bytesRead = fread(contents, 1, sizeof(contents) - 1, file);
    fclose(file);
    contents[bytesRead] = '\0';

    char *context = NULL;
    char *line = strtok_s(contents, "\r\n", &context);
    if ((line == NULL) || (strcmp(NativePcAccount_Trim(line), NATIVE_PC_ACCOUNT_KEY_HEADER) != 0))
    {
        Platform_LogWarn("[CTR Account] Invalid highoctane.key header\n");
        return 0;
    }

    while ((line = strtok_s(NULL, "\r\n", &context)) != NULL)
    {
        line = NativePcAccount_Trim(line);
        char *equals = strchr(line, '=');
        if (equals == NULL) continue;
        *equals++ = '\0';
        char *key = NativePcAccount_Trim(line);
        char *value = NativePcAccount_Trim(equals);
        if (strcmp(key, "token") == 0)
            snprintf(s_nativePcAccount.token, sizeof(s_nativePcAccount.token), "%s", value);
    }

    if (!NativePcAccount_IsTokenValid(s_nativePcAccount.token))
    {
        memset(s_nativePcAccount.token, 0, sizeof(s_nativePcAccount.token));
        Platform_LogWarn("[CTR Account] highoctane.key is malformed\n");
        return 0;
    }

    s_nativePcAccount.available = true;
    return 1;
}

int NativePcAccount_IsAvailable(void) { NativePcAccount_Init(); return s_nativePcAccount.available != 0; }
const char *NativePcAccount_GetToken(void) { return NativePcAccount_IsAvailable() ? s_nativePcAccount.token : NULL; }
const char *NativePcAccount_GetPublicId(void) { return (NativePcAccount_IsAvailable() && s_nativePcAccount.verified) ? s_nativePcAccount.publicId : NULL; }
const char *NativePcAccount_GetUsername(void) { return (NativePcAccount_IsAvailable() && s_nativePcAccount.verified) ? s_nativePcAccount.username : NULL; }
const char *NativePcAccount_GetKeyPath(void) { NativePcAccount_Init(); return s_nativePcAccount.keyPath; }

void NativePcAccount_SetVerifiedIdentity(const char *username, const char *publicId)
{
    if (!NativePcAccount_IsAvailable() || (username == NULL) || (publicId == NULL)) return;
    size_t userLen = strlen(username);
    if ((userLen < 1) || (userLen > 16) || (strlen(publicId) != 19) || (memcmp(publicId, "PC-", 3) != 0)) return;
    for (int i = 3; i < 19; i++) if (!isxdigit((unsigned char)publicId[i])) return;
    snprintf(s_nativePcAccount.username, sizeof(s_nativePcAccount.username), "%s", username);
    snprintf(s_nativePcAccount.publicId, sizeof(s_nativePcAccount.publicId), "%s", publicId);
    for (int i = 3; i < 19; i++) s_nativePcAccount.publicId[i] = (char)toupper((unsigned char)s_nativePcAccount.publicId[i]);
    s_nativePcAccount.verified = true;
}

#else
int NativePcAccount_Init(void) { return 0; }
int NativePcAccount_IsAvailable(void) { return 0; }
const char *NativePcAccount_GetToken(void) { return NULL; }
const char *NativePcAccount_GetPublicId(void) { return NULL; }
const char *NativePcAccount_GetUsername(void) { return NULL; }
const char *NativePcAccount_GetKeyPath(void) { return NULL; }
void NativePcAccount_SetVerifiedIdentity(const char *username, const char *publicId) { (void)username; (void)publicId; }
#endif
