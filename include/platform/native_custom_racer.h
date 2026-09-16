#ifndef PLATFORM_NATIVE_CUSTOM_RACER_H
#define PLATFORM_NATIVE_CUSTOM_RACER_H

#define NATIVE_CUSTOM_RACER_MAX 64
#define NATIVE_CUSTOM_RACER_MAX_VOICE_FILES 32
#define NATIVE_CUSTOM_RACER_SAMPLED_VOICE_COUNT 2

enum NativeCustomRacerAsset
{
	NATIVE_CUSTOM_RACER_ASSET_MODEL_HI = 0,
	NATIVE_CUSTOM_RACER_ASSET_SHARED_VRM,
	NATIVE_CUSTOM_RACER_ASSET_PORTRAIT_VRM,
	NATIVE_CUSTOM_RACER_ASSET_VOICE_XNF,
	NATIVE_CUSTOM_RACER_ASSET_COUNT,
};

struct BigHeader;
struct ChannelAttr;
struct LoadQueueSlot;
struct Model;
struct Icon;

int NativeCustomRacer_Scan(void);
int NativeCustomRacer_GetCount(void);
const char *NativeCustomRacer_GetName(int index);
int NativeCustomRacer_GetTemplateCharacterID(int index);
int NativeCustomRacer_GetEngineClass(int index);
int NativeCustomRacer_IsRosterEnabled(void);
int NativeCustomRacer_DisablesRecords(void);
struct Model *NativeCustomRacer_GetPreviewModel(int index);
u32 NativeCustomRacer_GetPortraitTexture(int index, const struct Icon *templateIcon, int *width, int *height);
u32 NativeCustomRacer_GetRetailPortraitTexture(int templateCharacterID, const struct Icon *templateIcon, int *width, int *height);
int NativeCustomRacer_GetVoiceTrack(int categoryID, int xaID, int *channelFilter, int *numSectors,
                                    const char **packagePath, u64 *assetOffset, u32 *assetSize);
void NativeCustomRacer_SetActiveVoiceCharacter(int characterID);
void NativeCustomRacer_SetActiveVoiceDriver(int driverID);
int NativeCustomRacer_PlayActiveSampledVoice(int voiceType, int characterID);
int NativeCustomRacer_PlayDriverSampledVoice(int driverID, int voiceType, int characterID, int *soundIDCount);
int NativeCustomRacer_InitSampledVoiceChannelAttr(int racerIndex, int soundID, struct ChannelAttr *attr,
                                                  int vol, int LR, int distort);
int NativeCustomRacer_UpdateSampledVoiceVolume(int racerIndex, int soundID, struct ChannelAttr *attr, int vol, int LR);
void NativeCustomRacer_LoadSelectedSamplesToSpu(void);

int NativeCustomRacer_QueueSelectedModel(int playerIndex, void **destination);
struct Model *NativeCustomRacer_GetLoadedPlayerModel(int playerIndex);
int NativeCustomRacer_QueueDriverModel(int driverIndex, void **destination);
int NativeCustomRacer_LoadDriverModelNow(int driverIndex, void **destination);
struct Model *NativeCustomRacer_GetLoadedDriverModel(int driverIndex);
int NativeCustomRacer_LoadPodiumModelNow(int podiumRank, int danceModelID, void **destination);
void NativeCustomRacer_QueueSharedVramForSelections(struct BigHeader *retailBigfile);
void NativeCustomRacer_ApplyDriverVramPatches(void);
void NativeCustomRacer_ApplyPodiumVramPatches(void);
void NativeCustomRacer_CaptureRetailSharedVram(const void *fileData, u32 fileSize);
int NativeCustomRacer_IsBigHeader(const struct BigHeader *bigfile);
int NativeCustomRacer_LoadQueueSlot(struct LoadQueueSlot *slot);
void NativeCustomRacer_FinishQueueSlot(struct LoadQueueSlot *slot);

void NativeCustomRacer_ClearPlayerSelections(void);
void NativeCustomRacer_SetPlayerSelection(int playerIndex, int racerIndex);
int NativeCustomRacer_GetPlayerSelection(int playerIndex);
void NativeCustomRacer_ClearDriverSelections(void);
void NativeCustomRacer_SetDriverSelection(int driverIndex, int racerIndex);
int NativeCustomRacer_GetDriverSelection(int driverIndex);
void NativeCustomRacer_ClearPodiumSelections(void);
void NativeCustomRacer_SetPodiumSelection(int podiumRank, int racerIndex);
int NativeCustomRacer_GetPodiumSelection(int podiumRank);

#endif
