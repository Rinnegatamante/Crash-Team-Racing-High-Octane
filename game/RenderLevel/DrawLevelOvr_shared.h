#ifndef DRAW_LEVEL_OVR_SHARED_H
#define DRAW_LEVEL_OVR_SHARED_H

// NOTE(aalhendi): Native compiles overlays 226-229 into one unity unit. Retail
// carries similar copied draw-level code per overlay; these declarations expose
// the canonical 226 C helpers to the 227/228/229 overlay slices.

// Bucket metadata and scratch views.
typedef int (*DrawLevelOvrClipConsumer)(struct PushBuffer *pb, struct PrimMem *primMem, u8 *clipCursor, int playerIndex);

extern const struct DrawLevelOvr1PBucket sDrawLevelOvr1PBuckets[];
struct DrawLevelOvr1PStableScratch *DrawLevelOvr1P_Scratch(void);
struct MainRenderLevelGeometryScratch *DrawLevelOvr1P_RenderScratch(void);

// Shared overlay scratch controls.
void DrawLevelOvr1P_SetMosaicReloadSpanOverride(u32 reloadSpan);
void Ovr226_800a0dc4_ClearProjectedScratch(void);
u8 *DrawLevelOvr1P_GetClipRecordCursor(void);
void DrawLevelOvr1P_SetClipRecordCursor(u8 *cursor);
void DrawLevelOvr1P_SetClipRecordStart(u8 *start);
void DrawLevelOvr1P_SetViewportScratchContext(struct PushBuffer *pb, const int *visFaceList, u8 *clipStart, u8 *clipCursor,
                                              struct QuadBlock **renderedOverflowBase);
void DrawLevelOvr1P_SetPrimReserveBias(u32 bias);
void DrawLevelOvr1P_SetListHandlersSeedRenderedCursor(int enabled);
void DrawLevelOvr_ClearRenderedOverflowBase(int playerIndex);

// Shared dispatch helpers.
int DrawLevelOvr1P_ConsumeClipRecords(struct PushBuffer *pb, struct PrimMem *primMem);
int DrawLevelOvr_ConsumeClipRecordsForViewport(struct PushBuffer *pb, struct PrimMem *primMem, u8 *clipCursor, int playerIndex,
                                               DrawLevelOvrClipConsumer consume);
void *DrawLevelOvr1P_GetRenderListBucketValue(struct DrawLevelOvr1PRenderList *renderList, const struct DrawLevelOvr1PBucket *bucket);
int DrawLevelOvr1P_DrawBspListQuadBlocks(struct VisMemBspListNode *slot, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                         const int *visFaceList, int role);
int DrawLevelOvr1P_DrawRenderedQuadBlocks(struct QuadBlock **renderedList, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem, int role);

// Canonical 226 helper bodies reused by 227/228/229.
int Ovr226_800a0ef4_DrawFullDynamicBspList(struct VisMemBspListNode *slot, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                           const int *visFaceList);
int Ovr226_800a1e30_DrawWaterBspList(struct VisMemBspListNode *slot, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                     const int *visFaceList);
void DrawLevelOvr1P_SetSplitGroundThresholdScratch(void);
int DrawLevelOvr1P_DrawSplitGroundListABspList(struct VisMemBspListNode *slot, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                               const int *visFaceList);
int Ovr226_800a2904_DrawWaterRenderedListWithDefaultHandler(struct QuadBlock **renderedList, struct PushBuffer *pb, struct mesh_info *mesh,
                                                            struct PrimMem *primMem, u32 defaultHandlerAddress);

#endif
