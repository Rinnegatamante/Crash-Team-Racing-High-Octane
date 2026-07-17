#include <common.h>

enum
{
	VEH_GROUND_SKIDS_ICON_TIREMARK = 0x2f,
	VEH_GROUND_SKIDS_ALT_TPAGE_FLAG = 0x1,
	VEH_GROUND_SKIDS_TPAGE_BLEND_ALT = 0x00600000,
	VEH_GROUND_SKIDS_TPAGE_BLEND_NORMAL = 0x00400000,
	VEH_GROUND_SKIDS_GPU_TAG_POLY_GT4 = 0x0c000000u,
	VEH_GROUND_SKIDS_OT_DEPTH_SHIFT = 6,
	VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT = 2,
	VEH_GROUND_SKIDS_CULL_ABS_MAX = 0x1771,
	VEH_GROUND_SKIDS_DEPTH_SHIFT = 2,
	VEH_GROUND_SKIDS_FULL_INTENSITY_DEPTH = 0x180,
	VEH_GROUND_SKIDS_FULL_INTENSITY = 0x7f,
	VEH_GROUND_SKIDS_LZCR_SHIFT_BASE = 0x1a,
	VEH_GROUND_SKIDS_MIN_INTENSITY = 0x10,
	VEH_GROUND_SKIDS_COLOR_PREFIX = 0x3e000000u,
	VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH = 0x20,
	VEH_GROUND_SKIDS_COLOR_FADE_SHIFT = 1,
};

static const u32 VEH_GROUND_SKIDS_TPAGE_BLEND_MASK = 0xff9fffff;
static const u32 VEH_GROUND_SKIDS_COLOR_SENTINEL = 0xffffffffu;

CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_ICON_TIREMARK == 0x2f);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_ALT_TPAGE_FLAG == 0x1);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_TPAGE_BLEND_ALT == 0x00600000);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_TPAGE_BLEND_NORMAL == 0x00400000);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_GPU_TAG_POLY_GT4 == 0x0c000000u);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_OT_DEPTH_SHIFT == 6);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT == 2);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_CULL_ABS_MAX == 0x1771);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_DEPTH_SHIFT == 2);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_FULL_INTENSITY_DEPTH == 0x180);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_FULL_INTENSITY == 0x7f);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_LZCR_SHIFT_BASE == 0x1a);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_MIN_INTENSITY == 0x10);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_COLOR_PREFIX == 0x3e000000u);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH == 0x20);
CTR_STATIC_ASSERT(VEH_GROUND_SKIDS_COLOR_FADE_SHIFT == 1);

static u32 VehGroundSkids_ReadTexWord(const struct TextureLayout *layout, size_t offset)
{
	u32 word;
	memcpy(&word, (const u8 *)layout + offset, sizeof(word));
	return word;
}

static u16 VehGroundSkids_ReadTexHalf(const struct TextureLayout *layout, size_t offset)
{
	u16 half;
	memcpy(&half, (const u8 *)layout + offset, sizeof(half));
	return half;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005c120-0x8005c278.
void VehGroundSkids_Subset1(u32 *currXY, u32 *prevXY, int depth, struct VehGroundSkidsScratch *scratch)
{
	struct GameTracker *gGT = sdata->gGT;
	struct DB *backBuffer = gGT->backBuffer;
	POLY_GT4 *poly = backBuffer->primMem.cursor;
	POLY_GT4 *nextPrim = poly + 1;

	if ((char *)backBuffer->primMem.guardEnd < (char *)nextPrim)
	{
		return;
	}

	backBuffer->primMem.cursor = nextPrim;

	CtrGpu_WriteColorCode(&poly->r0, scratch->colorNear);
	CtrGpu_WriteColorCode(&poly->r1, scratch->colorNear);
	CtrGpu_WriteColorCode(&poly->r2, scratch->colorFar);
	CtrGpu_WriteColorCode(&poly->r3, scratch->colorFar);

	CtrGpu_WritePackedXY(&poly->x0, currXY[0]);
	CtrGpu_WritePackedXY(&poly->x1, currXY[1]);
	CtrGpu_WritePackedXY(&poly->x2, prevXY[0]);
	CtrGpu_WritePackedXY(&poly->x3, prevXY[1]);

	struct Icon *icon = gGT->ptrIcons[VEH_GROUND_SKIDS_ICON_TIREMARK];
	CtrGpu_WritePackedUVWord(&poly->u0, VehGroundSkids_ReadTexWord(&icon->texLayout, offsetof(struct TextureLayout, u0)));

	u32 tpage = VehGroundSkids_ReadTexWord(&icon->texLayout, offsetof(struct TextureLayout, u1));
	if ((scratch->segmentFlags & VEH_GROUND_SKIDS_ALT_TPAGE_FLAG) != 0)
	{
		tpage = (tpage & VEH_GROUND_SKIDS_TPAGE_BLEND_MASK) | VEH_GROUND_SKIDS_TPAGE_BLEND_ALT;
	}
	else
	{
		tpage = (tpage & VEH_GROUND_SKIDS_TPAGE_BLEND_MASK) | VEH_GROUND_SKIDS_TPAGE_BLEND_NORMAL;
	}
	CtrGpu_WritePackedUVWord(&poly->u1, tpage);

	CtrGpu_WritePackedUV(&poly->u2, VehGroundSkids_ReadTexHalf(&icon->texLayout, offsetof(struct TextureLayout, u2)));
	CtrGpu_WritePackedUV(&poly->u3, VehGroundSkids_ReadTexHalf(&icon->texLayout, offsetof(struct TextureLayout, u3)));

	struct PushBuffer *pb = scratch->pushBuffer;
	u32 *ot = pb->ptrOT + ((s32)depth >> VEH_GROUND_SKIDS_OT_DEPTH_SHIFT);
	CtrGpu_LinkPacket24(ot, &poly->tag, poly, VEH_GROUND_SKIDS_GPU_TAG_POLY_GT4);
}

static s16 VehGroundSkids_ScaleRelative(u16 value, u16 origin)
{
	// NOTE(aalhendi): Retail uses lhu/subu/sll/sh, so preserve unsigned halfword wraparound.
	return (s16)(u16)(((u32)value - (u32)origin) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005c278-0x8005c354.
void VehGroundSkids_Subset2(struct VehGroundSkidsScratch *scratch, const SVECTOR *v1, const SVECTOR *v2, const SVECTOR *v3)
{
	u16 originX = (u16)scratch->origin.x;
	u16 originY = (u16)scratch->origin.y;
	u16 originZ = (u16)scratch->origin.z;

	scratch->projected[0].vx = VehGroundSkids_ScaleRelative((u16)v1->vx, originX);
	scratch->projected[0].vy = VehGroundSkids_ScaleRelative((u16)v1->vy, originY);
	scratch->projected[0].vz = VehGroundSkids_ScaleRelative((u16)v1->vz, originZ);

	scratch->projected[1].vx = VehGroundSkids_ScaleRelative((u16)v2->vx, originX);
	scratch->projected[1].vy = VehGroundSkids_ScaleRelative((u16)v2->vy, originY);
	scratch->projected[1].vz = VehGroundSkids_ScaleRelative((u16)v2->vz, originZ);

	scratch->projected[2].vx = VehGroundSkids_ScaleRelative((u16)v3->vx, originX);
	scratch->projected[2].vy = VehGroundSkids_ScaleRelative((u16)v3->vy, originY);
	scratch->projected[2].vz = VehGroundSkids_ScaleRelative((u16)v3->vz, originZ);
}

static u32 VehGroundSkids_ColorWord(int value)
{
	return VEH_GROUND_SKIDS_COLOR_PREFIX | ((u32)value << 16) | ((u32)value << 8) | (u32)value;
}

static s32 VehGroundSkids_Abs(s32 value)
{
	return value < 0 ? -value : value;
}

static int VehGroundSkids_InitPoint(SVECTOR *scratch, const SVECTOR *point, const s32 *origin)
{
	// NOTE(aalhendi): Retail uses lh/lw/subu/sll here; preserve unsigned 32-bit wraparound.
	s32 x = (s32)(((u32)(s32)point->vx - (u32)origin[0]) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	if (VehGroundSkids_Abs(x) >= VEH_GROUND_SKIDS_CULL_ABS_MAX)
	{
		return 0;
	}

	s32 y = (s32)(((u32)(s32)point->vy - (u32)origin[1]) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	if (VehGroundSkids_Abs(y) >= VEH_GROUND_SKIDS_CULL_ABS_MAX)
	{
		return 0;
	}

	s32 z = (s32)(((u32)(s32)point->vz - (u32)origin[2]) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	if (VehGroundSkids_Abs(z) >= VEH_GROUND_SKIDS_CULL_ABS_MAX)
	{
		return 0;
	}

	scratch[0].vx = x;
	scratch[0].vy = y;
	scratch[0].vz = z;
	return 1;
}

static int VehGroundSkids_IntensityFromDepth(int depth)
{
	depth >>= VEH_GROUND_SKIDS_DEPTH_SHIFT;
	if (depth < VEH_GROUND_SKIDS_FULL_INTENSITY_DEPTH)
	{
		return VEH_GROUND_SKIDS_FULL_INTENSITY;
	}

	MTC2(depth - VEH_GROUND_SKIDS_FULL_INTENSITY_DEPTH, 30);
	int shift = VEH_GROUND_SKIDS_LZCR_SHIFT_BASE - MFC2(31);
	if (shift < 0)
	{
		shift = 0;
	}

	int intensity = VEH_GROUND_SKIDS_FULL_INTENSITY >> (shift & 0x1f);
	if (intensity < VEH_GROUND_SKIDS_MIN_INTENSITY)
	{
		return -1;
	}

	return intensity;
}

force_inline void VehGroundSkids_ProjectTriplet(struct VehGroundSkidsScratch *scratch, const SVECTOR *frame, u32 *sxy, s32 *depth)
{
	VehGroundSkids_Subset2(scratch, &frame[0], &frame[1], &frame[2]);
	CTR_GteLoadSV3(&scratch->projected[0], &scratch->projected[1], &scratch->projected[2]);
	gte_rtpt();
	CTR_GteStoreSXY3(&sxy[0], &sxy[1], &sxy[2]);
	gte_stsz3(&depth[0], &depth[1], &depth[2]);
}

static void VehGroundSkids_ProjectFrame(struct VehGroundSkidsScratch *scratch, const SVECTOR *frame, u32 *sxy, s32 *depth)
{
	VehGroundSkids_ProjectTriplet(scratch, &frame[0], &sxy[0], &depth[0]);
	VehGroundSkids_ProjectTriplet(scratch, &frame[3], &sxy[3], &depth[3]);

	VehGroundSkids_Subset2(scratch, &frame[6], &frame[7], &frame[0]);
	CTR_GteLoadSV3(&scratch->projected[0], &scratch->projected[1], &scratch->projected[2]);
	gte_rtpt();
	CTR_GteStoreSXY3(&sxy[6], &sxy[7], &sxy[8]);
	gte_stsz3(&depth[6], &depth[7], &depth[8]);
}

static void VehGroundSkids_TryEmitSegment(struct VehGroundSkidsScratch *scratch, u32 *currXY, u32 *prevXY, s32 *currDepth, s32 *prevDepth, u32 flags,
                                          u32 prevFlags, int bit, const union VehEmitterSkidmark *mark, int pointIndex)
{
	if ((flags & prevFlags & bit) == 0)
	{
		return;
	}
	if (currDepth[pointIndex] <= VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH || currDepth[pointIndex + 1] <= VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH)
	{
		return;
	}
	if (prevDepth[pointIndex] <= VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH || prevDepth[pointIndex + 1] <= VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH)
	{
		return;
	}

	scratch->segmentFlagsLow = mark->flags;
	int depth = (currDepth[pointIndex] >> VEH_GROUND_SKIDS_DEPTH_SHIFT) + (mark->color << VEH_GROUND_SKIDS_OT_DEPTH_SHIFT);
	VehGroundSkids_Subset1(&currXY[pointIndex], &prevXY[pointIndex], depth, scratch);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005c354-0x8005ca24.
void VehGroundSkids_Main(struct Thread *thread, struct PushBuffer *pb)
{
	gte_SetGeomOffset(pb->rect.w >> 1, pb->rect.h >> 1);
	gte_SetGeomScreen(pb->distanceToScreen_PREV);

	struct VehGroundSkidsScratch *scratch = CTR_SCRATCHPAD_PTR(struct VehGroundSkidsScratch, 0x0);

	scratch->pushBuffer = pb;
	scratch->origin.x = 0;
	scratch->origin.y = 0;
	scratch->origin.z = 0;

	gte_SetRotMatrix(&pb->matrix_ViewProj);
	gte_SetTransVector(&scratch->origin);

	scratch->origin.x = pb->matrix_Camera.t[0];
	scratch->origin.y = pb->matrix_Camera.t[1];
	scratch->origin.z = pb->matrix_Camera.t[2];

	while (thread != NULL)
	{
		struct Driver *d = thread->object;
		u32 flags = d->skidmarkEnableFlags;

		if (flags > DRIVER_SKIDMARK_CURRENT_FRAME_MASK)
		{
			int frameIndex = ((u8)d->skidmarkFrameIndex - 1) & DRIVER_SKIDMARK_FRAME_INDEX_MASK;
			union VehEmitterSkidmark *frame = d->skidmarks[frameIndex];
			SVECTOR *framePoints = &frame[0].edge[0];

			if (VehGroundSkids_InitPoint(scratch->projected, &framePoints[0], scratch->origin.v))
			{
				CTR_GteLoadSV0(&scratch->projected[0]);
				gte_rtv0();

				int intensity = VehGroundSkids_IntensityFromDepth(MFC2_S(27));
				if (intensity >= 0)
				{
					u32 *currXY = scratch->currXY;
					u32 *prevXY = scratch->prevXY;
					s32 *currDepth = scratch->currDepth;
					s32 *prevDepth = scratch->prevDepth;

					scratch->colorNear = VehGroundSkids_ColorWord(intensity);
					scratch->colorFar = VEH_GROUND_SKIDS_COLOR_SENTINEL;

					u32 prevFlags = 0;
					while (flags != 0)
					{
						u32 currFlags = flags;

						if ((currFlags & DRIVER_SKIDMARK_CURRENT_FRAME_MASK) != 0)
						{
							frame = d->skidmarks[frameIndex];
							framePoints = &frame[0].edge[0];
							VehGroundSkids_ProjectFrame(scratch, framePoints, currXY, currDepth);

							VehGroundSkids_TryEmitSegment(scratch, currXY, prevXY, currDepth, prevDepth, currFlags, prevFlags, DRIVER_SKIDMARK_BACK_LEFT,
							                              &frame[0], 0);
							VehGroundSkids_TryEmitSegment(scratch, currXY, prevXY, currDepth, prevDepth, currFlags, prevFlags, DRIVER_SKIDMARK_BACK_RIGHT,
							                              &frame[1], 2);
							VehGroundSkids_TryEmitSegment(scratch, currXY, prevXY, currDepth, prevDepth, currFlags, prevFlags, DRIVER_SKIDMARK_FRONT_LEFT,
							                              &frame[2], 4);
							VehGroundSkids_TryEmitSegment(scratch, currXY, prevXY, currDepth, prevDepth, currFlags, prevFlags, DRIVER_SKIDMARK_FRONT_RIGHT,
							                              &frame[3], 6);
						}

						u32 *tmpXY = currXY;
						currXY = prevXY;
						prevXY = tmpXY;

						s32 *tmpDepth = currDepth;
						currDepth = prevDepth;
						prevDepth = tmpDepth;

						frameIndex = (frameIndex + 1) & DRIVER_SKIDMARK_FRAME_INDEX_MASK;

						if (scratch->colorFar == VEH_GROUND_SKIDS_COLOR_SENTINEL)
						{
							scratch->colorFar = scratch->colorNear;
							prevFlags = DRIVER_SKIDMARK_CURRENT_FRAME_MASK;
						}
						else
						{
							prevFlags = currFlags;
							flags = currFlags >> DRIVER_SKIDMARK_HISTORY_SHIFT;

							int faded = (scratch->colorNear & 0xff) >> VEH_GROUND_SKIDS_COLOR_FADE_SHIFT;
							scratch->colorFar = scratch->colorNear;
							scratch->colorNear = VehGroundSkids_ColorWord(faded);
						}

						if (scratch->colorFar == 0)
						{
							break;
						}
					}
				}
			}
		}

		thread = thread->siblingThread;
	}
}
