#ifndef CTR_NATIVE_USER_ID_H
#define CTR_NATIVE_USER_ID_H

enum
{
    NATIVE_USER_ID_HASH_SIZE = 32,
};

const char *NativeUserId_GetDisplayString(void);
int NativeUserId_CopyHash(unsigned char outHash[NATIVE_USER_ID_HASH_SIZE]);

#endif
