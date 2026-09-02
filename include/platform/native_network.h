#ifndef CTR_NATIVE_NETWORK_H
#define CTR_NATIVE_NETWORK_H

int NativeNetwork_Init(void);
void NativeNetwork_Shutdown(void);
void NativeNetwork_RequestInternetModeRestore(void);
void NativeNetwork_Update(void);
int NativeNetwork_IsInternetConnected(void);

#endif
