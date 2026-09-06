#ifndef CTR_NATIVE_PC_ACCOUNT_H
#define CTR_NATIVE_PC_ACCOUNT_H

int NativePcAccount_Init(void);
int NativePcAccount_IsAvailable(void);
const char *NativePcAccount_GetToken(void);
const char *NativePcAccount_GetPublicId(void);
const char *NativePcAccount_GetUsername(void);
const char *NativePcAccount_GetKeyPath(void);
void NativePcAccount_SetVerifiedIdentity(const char *username, const char *publicId);

#endif
