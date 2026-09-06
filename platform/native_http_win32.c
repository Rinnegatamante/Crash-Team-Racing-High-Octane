#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "platform/native_http_win32.h"

static int NativeWinHttp_Utf8ToWide(const char *src, wchar_t *dst, int dstCount)
{
    if ((src == NULL) || (dst == NULL) || (dstCount <= 0)) return 0;
    return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, -1, dst, dstCount) > 0;
}

static int NativeWinHttp_Append(NativeWinHttpResponse *response, const void *data, size_t size, size_t maxSize)
{
    if (size == 0) return 1;
    if ((response->size + size) > maxSize) return 0;
    unsigned char *newData = (unsigned char *)realloc(response->data, response->size + size + 1);
    if (newData == NULL) return 0;
    response->data = newData;
    memcpy(response->data + response->size, data, size);
    response->size += size;
    response->data[response->size] = 0;
    return 1;
}

int NativeWinHttp_Request(const char *methodUtf8, const char *url, const char *bearerToken,
                          const char *contentType, const void *requestData, size_t requestSize,
                          size_t maxResponseSize, NativeWinHttpResponse *response)
{
    if ((methodUtf8 == NULL) || (url == NULL) || (response == NULL) || (requestSize > 0xffffffffu)) return 0;
    memset(response, 0, sizeof(*response));

    wchar_t wideMethod[16], wideUrl[1024];
    if (!NativeWinHttp_Utf8ToWide(methodUtf8, wideMethod, 16) || !NativeWinHttp_Utf8ToWide(url, wideUrl, 1024)) return 0;

    URL_COMPONENTS parts;
    memset(&parts, 0, sizeof(parts));
    parts.dwStructSize = sizeof(parts);
    parts.dwHostNameLength = (DWORD)-1;
    parts.dwUrlPathLength = (DWORD)-1;
    parts.dwExtraInfoLength = (DWORD)-1;
    if (!WinHttpCrackUrl(wideUrl, 0, 0, &parts)) return 0;

    wchar_t host[256], path[1024];
    if ((parts.dwHostNameLength >= 256) || ((parts.dwUrlPathLength + parts.dwExtraInfoLength) >= 1024)) return 0;
    memcpy(host, parts.lpszHostName, parts.dwHostNameLength * sizeof(wchar_t));
    host[parts.dwHostNameLength] = L'\0';
    DWORD pathLength = 0;
    if (parts.dwUrlPathLength)
    {
        memcpy(path + pathLength, parts.lpszUrlPath, parts.dwUrlPathLength * sizeof(wchar_t));
        pathLength += parts.dwUrlPathLength;
    }
    if (parts.dwExtraInfoLength)
    {
        memcpy(path + pathLength, parts.lpszExtraInfo, parts.dwExtraInfoLength * sizeof(wchar_t));
        pathLength += parts.dwExtraInfoLength;
    }
    if (pathLength == 0) path[pathLength++] = L'/';
    path[pathLength] = L'\0';

    HINTERNET session = WinHttpOpen(L"CTR High Octane/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (session == NULL) return 0;
    WinHttpSetTimeouts(session, 8000, 8000, 20000, 20000);
    HINTERNET connection = WinHttpConnect(session, host, parts.nPort, 0);
    if (connection == NULL) { WinHttpCloseHandle(session); return 0; }
    DWORD flags = (parts.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET request = WinHttpOpenRequest(connection, wideMethod, path, NULL, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (request == NULL) { WinHttpCloseHandle(connection); WinHttpCloseHandle(session); return 0; }

    wchar_t headers[1024];
    headers[0] = L'\0';
    if (bearerToken != NULL)
    {
        wchar_t token[128];
        if (!NativeWinHttp_Utf8ToWide(bearerToken, token, 128)) goto fail;
        _snwprintf_s(headers, 1024, _TRUNCATE, L"Authorization: Bearer %ls\r\nX-CTR-Key: %ls\r\n", token, token);
    }
    if (contentType != NULL)
    {
        wchar_t type[256];
        if (!NativeWinHttp_Utf8ToWide(contentType, type, 256)) goto fail;
        size_t used = wcslen(headers);
        _snwprintf_s(headers + used, 1024 - used, _TRUNCATE, L"Content-Type: %ls\r\n", type);
    }

    if (!WinHttpSendRequest(request,
                            headers[0] ? headers : WINHTTP_NO_ADDITIONAL_HEADERS,
                            headers[0] ? (DWORD)-1L : 0,
                            requestData ? (LPVOID)requestData : WINHTTP_NO_REQUEST_DATA,
                            (DWORD)requestSize, (DWORD)requestSize, 0)) goto fail;
    if (!WinHttpReceiveResponse(request, NULL)) goto fail;

    DWORD status = 0, statusSize = sizeof(status);
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX)) goto fail;
    response->status = (long)status;

    wchar_t hash[80];
    DWORD hashSize = sizeof(hash);
    if (WinHttpQueryHeaders(request, WINHTTP_QUERY_CUSTOM, L"X-Ghost-SHA256", hash, &hashSize, WINHTTP_NO_HEADER_INDEX))
    {
        char ascii[80];
        if (WideCharToMultiByte(CP_UTF8, 0, hash, -1, ascii, sizeof(ascii), NULL, NULL) > 0)
        {
            int i = 0;
            for (; (i < 64) && isxdigit((unsigned char)ascii[i]); i++)
                response->ghostSha256[i] = (char)toupper((unsigned char)ascii[i]);
            response->ghostSha256[i] = '\0';
        }
    }

    for (;;)
    {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request, &available)) goto fail;
        if (available == 0) break;
        unsigned char chunk[8192];
        while (available)
        {
            DWORD read = 0;
            DWORD toRead = available > sizeof(chunk) ? sizeof(chunk) : available;
            if (!WinHttpReadData(request, chunk, toRead, &read)) goto fail;
            if (read == 0) break;
            if (!NativeWinHttp_Append(response, chunk, read, maxResponseSize)) goto fail;
            available -= read;
        }
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    return (status >= 200) && (status < 300);

fail:
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    return 0;
}

void NativeWinHttp_FreeResponse(NativeWinHttpResponse *response)
{
    if (response == NULL) return;
    free(response->data);
    memset(response, 0, sizeof(*response));
}

int NativeWinHttp_Sha256(const void *data, size_t size, unsigned char digest[32])
{
    BCRYPT_ALG_HANDLE algorithm = NULL;
    BCRYPT_HASH_HANDLE hash = NULL;
    DWORD objectSize = 0, resultSize = 0;
    unsigned char *object = NULL;
    int ok = 0;
    if ((digest == NULL) || (size > 0xffffffffu)) return 0;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0) < 0) goto cleanup;
    if (BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, (PUCHAR)&objectSize, sizeof(objectSize), &resultSize, 0) < 0) goto cleanup;
    object = (unsigned char *)malloc(objectSize);
    if (object == NULL) goto cleanup;
    if (BCryptCreateHash(algorithm, &hash, object, objectSize, NULL, 0, 0) < 0) goto cleanup;
    if (size && BCryptHashData(hash, (PUCHAR)data, (ULONG)size, 0) < 0) goto cleanup;
    if (BCryptFinishHash(hash, digest, 32, 0) < 0) goto cleanup;
    ok = 1;
cleanup:
    if (hash) BCryptDestroyHash(hash);
    if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
    free(object);
    return ok;
}
#endif
