#ifndef CTR_NATIVE_HTTP_WIN32_H
#define CTR_NATIVE_HTTP_WIN32_H

#include <stddef.h>

typedef struct NativeWinHttpResponse
{
    unsigned char *data;
    size_t size;
    long status;
    char ghostSha256[65];
} NativeWinHttpResponse;

int NativeWinHttp_Request(const char *method, const char *url, const char *bearerToken,
                          const char *contentType, const void *requestData, size_t requestSize,
                          size_t maxResponseSize, NativeWinHttpResponse *response);
void NativeWinHttp_FreeResponse(NativeWinHttpResponse *response);
int NativeWinHttp_Sha256(const void *data, size_t size, unsigned char digest[32]);

#endif
