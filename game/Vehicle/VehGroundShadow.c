#include <common.h>

enum
{
	VEH_GROUND_SHADOW_MAX_DRIVERS = 8,
	VEH_GROUND_SHADOW_MAX_PLAYERS = 4,
	VEH_GROUND_SHADOW_NUM_POINTS = 9,
	VEH_GROUND_SHADOW_NUM_QUADS = 4,
	VEH_GROUND_SHADOW_TEXTURE_COUNT = 2,

	VEH_GROUND_SHADOW_STATE_CULLED = -1,
	VEH_GROUND_SHADOW_STATE_PENDING = 0,
	VEH_GROUND_SHADOW_STATE_READY = 1,

	VEH_GROUND_SHADOW_TPAGE_BLEND_MASK = 0xff9f,
	VEH_GROUND_SHADOW_TPAGE_BLEND_MODE = 0x40,

	VEH_GROUND_SHADOW_WORLD_POS_SHIFT = 8,
	VEH_GROUND_SHADOW_GROUND_Y_OFFSET = 3,
	VEH_GROUND_SHADOW_DEPTH_BIAS_BONUS = 1,
	VEH_GROUND_SHADOW_HEIGHT_BASE = 0x100,
	VEH_GROUND_SHADOW_HEIGHT_MIN = 1,
	VEH_GROUND_SHADOW_HEIGHT_MAX_EXCLUSIVE = 0x10a,
	VEH_GROUND_SHADOW_LOCAL_X_FACTOR = 0x28,
	VEH_GROUND_SHADOW_LOCAL_Z0_FACTOR = 0x29,
	VEH_GROUND_SHADOW_LOCAL_Z1_FACTOR = 0x34,
	VEH_GROUND_SHADOW_LOCAL_SCALE_SHIFT = 6,

	VEH_GROUND_SHADOW_PRIM_GUARD_WORDS = 0x140,
	VEH_GROUND_SHADOW_GTE_SCREEN_SHIFT = 15,
	VEH_GROUND_SHADOW_LARGE_GEOM_SCREEN_THRESHOLD = 0x100,
	VEH_GROUND_SHADOW_CAMERA_DELTA_SCALE = 4,
	VEH_GROUND_SHADOW_SMALL_SCREEN_MAX_EXCLUSIVE = 0x1771,
	VEH_GROUND_SHADOW_SMALL_SCREEN_MIN = -0x1770,
	VEH_GROUND_SHADOW_GROUND_DISTANCE_SHIFT = 2,
	VEH_GROUND_SHADOW_GROUND_DISTANCE_MIN = -0x34,

	VEH_GROUND_SHADOW_OT_MAX = 0x3ff,
	VEH_GROUND_SHADOW_GPU_TAG_POLY_FT4 = 0x09000000u,

	VEH_GROUND_SHADOW_COLOR_PREFIX = 0x2e000000u,
	VEH_GROUND_SHADOW_LARGE_COLOR = 0x2e1f1f1f,
	VEH_GROUND_SHADOW_FULL_INTENSITY = 0x1f,
	VEH_GROUND_SHADOW_FULL_INTENSITY_DISTANCE = 0x180,
	VEH_GROUND_SHADOW_FADE_END_DISTANCE = 0x200,
	VEH_GROUND_SHADOW_FADE_ROUNDING_BIAS = 0x7f,
	VEH_GROUND_SHADOW_FADE_SHIFT = 7,

	VEH_GROUND_SHADOW_ICON_LEFT = 0,
	VEH_GROUND_SHADOW_ICON_RIGHT = 1,
};

/// @brief Copies texture layout data from icon to arbitrary mem address. Particularly used to copy kart shadow textures to scratchpad.
/// @param dst - destination texture layout
/// @param iconIndex - icon index to take data from
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b6b8-0x8005b720.
b32 VehGroundShadow_Subset1(struct TextureLayout *pDst, int iconIndex)
{
	// get pointer to icon
	struct Icon *pIcon = sdata->gGT->ptrIcons[iconIndex];

	// validate icon pointer
	if (!pIcon)
	{
		return false;
	}

	// copy entire struct
	memcpy(pDst, &(pIcon->texLayout), sizeof(pIcon->texLayout));

	// fix blending mode
	pDst->tpage = (pDst->tpage & VEH_GROUND_SHADOW_TPAGE_BLEND_MASK) | VEH_GROUND_SHADOW_TPAGE_BLEND_MODE;

	return true;
}

struct VehGroundShadowEntry
{
	SVec3 local[3];
	s8 state;
	s8 depthBias;
	struct Driver *driver;
	struct Instance *inst;
	u8 idppFlags[VEH_GROUND_SHADOW_MAX_PLAYERS];
	SVec3 pos;
	u16 instFlags;
};

struct VehGroundShadowScratch
{
	u8 pad_000[0xa4];
	struct VehGroundShadowEntry entries[VEH_GROUND_SHADOW_MAX_DRIVERS];
	u8 pad_1e4[0x14];
	struct Driver *sentinelDriver;
	u8 pad_1fc[0x28];
	struct TextureLayout shadowTex[VEH_GROUND_SHADOW_TEXTURE_COUNT];
};

CTR_STATIC_ASSERT(sizeof(struct VehGroundShadowEntry) == 0x28);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowEntry, local) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowEntry, state) == 0x12);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowEntry, depthBias) == 0x13);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowEntry, driver) == 0x14);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowEntry, inst) == 0x18);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowEntry, idppFlags) == 0x1c);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowEntry, pos) == 0x20);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowEntry, instFlags) == 0x26);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowScratch, entries) == 0xa4);
CTR_STATIC_ASSERT(offsetof(struct VehGroundShadowScratch, sentinelDriver) == 0x1f8);
CTR_STATIC_ASSERT(CTR_OFFSET_OF_ARRAY(struct VehGroundShadowScratch, shadowTex, 0) == 0x224);
CTR_STATIC_ASSERT(CTR_OFFSET_OF_ARRAY(struct VehGroundShadowScratch, shadowTex, 1) == 0x230);

static u32 VehGroundShadow_ReadWord(const void *base, size_t offset)
{
	u32 value;
	memcpy(&value, (const char *)base + offset, sizeof(value));
	return value;
}

static u16 VehGroundShadow_ReadHalf(const void *base, size_t offset)
{
	u16 value;
	memcpy(&value, (const char *)base + offset, sizeof(value));
	return value;
}

static u32 VehGroundShadow_PackXY(s32 x, s32 y)
{
	return (u32)(u16)x | ((u32)(u16)y << 16);
}

static s32 VehGroundShadow_DiffS16(s32 lhs, s32 rhs)
{
	return (s16)((u16)lhs - (u16)rhs);
}

static struct InstDrawPerPlayer *VehGroundShadow_GetIdpp(struct Instance *inst, int playerIndex)
{
	return (struct InstDrawPerPlayer *)((char *)INST_GETIDPP(inst) + (playerIndex * sizeof(struct InstDrawPerPlayer)));
}

static void VehGroundShadow_LoadGteRotMatrix(const MATRIX *m)
{
	CTC2(VehGroundShadow_ReadWord(m, 0x00), 0);
	CTC2(VehGroundShadow_ReadWord(m, 0x04), 1);
	CTC2(VehGroundShadow_ReadWord(m, 0x08), 2);
	CTC2(VehGroundShadow_ReadWord(m, 0x0c), 3);
	CTC2(VehGroundShadow_ReadWord(m, 0x10), 4);
}

static void VehGroundShadow_LoadGteLightMatrix(const MATRIX *m)
{
	CTC2(VehGroundShadow_ReadWord(m, 0x00), 8);
	CTC2(VehGroundShadow_ReadWord(m, 0x04), 9);
	CTC2(VehGroundShadow_ReadWord(m, 0x08), 10);
	CTC2(VehGroundShadow_ReadWord(m, 0x0c), 11);
	CTC2(VehGroundShadow_ReadWord(m, 0x10), 12);
}

static void VehGroundShadow_LoadRtptVectors(SVec3 points[VEH_GROUND_SHADOW_NUM_POINTS], int start)
{
	MTC2(VehGroundShadow_PackXY(points[start + 0].x, points[start + 0].y), 0);
	MTC2((u32)(s32)points[start + 0].z, 1);
	MTC2(VehGroundShadow_PackXY(points[start + 1].x, points[start + 1].y), 2);
	MTC2((u32)(s32)points[start + 1].z, 3);
	MTC2(VehGroundShadow_PackXY(points[start + 2].x, points[start + 2].y), 4);
	MTC2((u32)(s32)points[start + 2].z, 5);
}

static void VehGroundShadow_ProjectPoints(SVec3 points[VEH_GROUND_SHADOW_NUM_POINTS], u32 sxy[VEH_GROUND_SHADOW_NUM_POINTS],
                                          s32 depth[VEH_GROUND_SHADOW_NUM_POINTS])
{
	for (int i = 0; i < VEH_GROUND_SHADOW_NUM_POINTS; i += 3)
	{
		VehGroundShadow_LoadRtptVectors(points, i);
		gte_rtpt_b();

		sxy[i + 0] = MFC2(12);
		sxy[i + 1] = MFC2(13);
		sxy[i + 2] = MFC2(14);
		depth[i + 0] = (s32)MFC2(17);
		depth[i + 1] = (s32)MFC2(18);
		depth[i + 2] = (s32)MFC2(19);
	}
}

static void VehGroundShadow_BuildEntry(struct VehGroundShadowEntry *entry, struct Driver *driver, int numPlayers)
{
	struct Instance *inst = driver->instSelf;

	entry->driver = driver;
	entry->inst = inst;
	entry->instFlags = (u16)inst->flags;

	for (int playerIndex = numPlayers - 1; playerIndex >= 0; playerIndex--)
	{
		entry->idppFlags[playerIndex] = (u8)VehGroundShadow_GetIdpp(inst, playerIndex)->instFlags;
	}

	entry->pos.x = (s16)CTR_MipsSra(driver->posCurr.x, VEH_GROUND_SHADOW_WORLD_POS_SHIFT);
	entry->pos.y = (s16)CTR_MipsAddLo(CTR_MipsSra(driver->quadBlockHeight, VEH_GROUND_SHADOW_WORLD_POS_SHIFT), VEH_GROUND_SHADOW_GROUND_Y_OFFSET);
	entry->pos.z = (s16)CTR_MipsSra(driver->posCurr.z, VEH_GROUND_SHADOW_WORLD_POS_SHIFT);
	entry->depthBias = (s8)((entry->instFlags & SPLIT_LINE) != 0 ? inst->depthBiasSecondary : inst->depthBiasNormal) + VEH_GROUND_SHADOW_DEPTH_BIAS_BONUS;
	entry->state = VEH_GROUND_SHADOW_STATE_PENDING;
}

static void VehGroundShadow_TransformLocalAxes(struct VehGroundShadowEntry *entry)
{
	struct Driver *driver = entry->driver;
	MATRIX axisMatrix;
	SVec3 local;
	int height =
	    CTR_MipsSubLo(VEH_GROUND_SHADOW_HEIGHT_BASE, CTR_MipsSra(CTR_MipsSubLo(driver->posCurr.y, driver->quadBlockHeight), VEH_GROUND_SHADOW_WORLD_POS_SHIFT));
	int localX;
	int localZ0;
	int localZ1;

	if (height < VEH_GROUND_SHADOW_HEIGHT_MIN || height >= VEH_GROUND_SHADOW_HEIGHT_MAX_EXCLUSIVE)
	{
		entry->state = VEH_GROUND_SHADOW_STATE_CULLED;
		return;
	}

	if (height > VEH_GROUND_SHADOW_HEIGHT_BASE)
	{
		height = VEH_GROUND_SHADOW_HEIGHT_BASE;
	}

	VehPhysForce_RotAxisAngle(&axisMatrix, driver->AxisAngle3_normalVec.v, driver->rotCurr.y);
	VehGroundShadow_LoadGteLightMatrix(&axisMatrix);

	localX = (height * VEH_GROUND_SHADOW_LOCAL_X_FACTOR) >> VEH_GROUND_SHADOW_LOCAL_SCALE_SHIFT;
	localZ0 = (height * VEH_GROUND_SHADOW_LOCAL_Z0_FACTOR) >> VEH_GROUND_SHADOW_LOCAL_SCALE_SHIFT;
	localZ1 = (height * VEH_GROUND_SHADOW_LOCAL_Z1_FACTOR) >> VEH_GROUND_SHADOW_LOCAL_SCALE_SHIFT;
	local.x = (s16)localX;
	local.y = 0;
	local.z = 0;

	for (int i = 0; i < 3; i++)
	{
		CTR_GteLoadSVec3V0(&local);
		gte_llv0_b();

		if (i == 0)
		{
			local.x = 0;
			local.z = (s16)localZ0;
		}
		else if (i == 1)
		{
			local.z = (s16)localZ1;
		}

		entry->local[i].x = (s16)MFC2(25);
		entry->local[i].y = (s16)MFC2(26);
		entry->local[i].z = (s16)MFC2(27);
	}

	entry->state = VEH_GROUND_SHADOW_STATE_READY;
}

static void VehGroundShadow_BuildProjectionPoints(struct VehGroundShadowEntry *entry, s32 baseX, s32 baseY, s32 baseZ,
                                                  SVec3 points[VEH_GROUND_SHADOW_NUM_POINTS])
{
	SVec3 *local = entry->local;

	points[0].x = (s16)baseX;
	points[0].y = (s16)baseY;
	points[0].z = (s16)baseZ;

	points[1].x = (s16)((baseX - local[0].x) - local[1].x);
	points[1].y = (s16)((baseY - local[0].y) - local[1].y);
	points[1].z = (s16)((baseZ - local[0].z) - local[1].z);

	points[2].x = (s16)(baseX - local[1].x);
	points[2].y = (s16)(baseY - local[1].y);
	points[2].z = (s16)(baseZ - local[1].z);

	points[3].x = (s16)((baseX + local[0].x) - local[1].x);
	points[3].y = (s16)((baseY + local[0].y) - local[1].y);
	points[3].z = (s16)((baseZ + local[0].z) - local[1].z);

	points[4].x = (s16)(baseX + local[0].x);
	points[4].y = (s16)(baseY + local[0].y);
	points[4].z = (s16)(baseZ + local[0].z);

	points[5].x = (s16)(baseX + local[2].x + local[0].x);
	points[5].y = (s16)(baseY + local[2].y + local[0].y);
	points[5].z = (s16)(baseZ + local[2].z + local[0].z);

	points[6].x = (s16)(baseX + local[2].x);
	points[6].y = (s16)(baseY + local[2].y);
	points[6].z = (s16)(baseZ + local[2].z);

	points[7].x = (s16)(baseX + local[2].x - local[0].x);
	points[7].y = (s16)(baseY + local[2].y - local[0].y);
	points[7].z = (s16)(baseZ + local[2].z - local[0].z);

	points[8].x = (s16)(baseX - local[0].x);
	points[8].y = (s16)(baseY - local[0].y);
	points[8].z = (s16)(baseZ - local[0].z);
}

static void VehGroundShadow_WriteUv(POLY_FT4 *poly, const struct TextureLayout *tex)
{
	CtrGpu_WritePackedUVWord(&poly->u0, VehGroundShadow_ReadWord(tex, offsetof(struct TextureLayout, u0)));
	CtrGpu_WritePackedUVWord(&poly->u1, VehGroundShadow_ReadWord(tex, offsetof(struct TextureLayout, u1)));
	CtrGpu_WritePackedUV(&poly->u2, VehGroundShadow_ReadHalf(tex, offsetof(struct TextureLayout, u2)));
	CtrGpu_WritePackedUV(&poly->u3, VehGroundShadow_ReadHalf(tex, offsetof(struct TextureLayout, u3)));
}

static void VehGroundShadow_EmitQuad(u32 **primCursor, u32 *otBase, const struct TextureLayout *texture, u32 color, u32 sxy[VEH_GROUND_SHADOW_NUM_POINTS],
                                     s32 depth, s8 depthBias, int quadIndex)
{
	static const u8 quadPointIndex[VEH_GROUND_SHADOW_NUM_QUADS][4] = {
	    {8, 0, 1, 2},
	    {8, 0, 7, 6},
	    {4, 0, 3, 2},
	    {4, 0, 5, 6},
	};
	POLY_FT4 *poly = (POLY_FT4 *)*primCursor;
	int depthIndex = (depth >> 8) + depthBias;
	u32 *ot;

	if (depthIndex < 0)
	{
		depthIndex = 0;
	}
	else if (depthIndex > VEH_GROUND_SHADOW_OT_MAX)
	{
		depthIndex = VEH_GROUND_SHADOW_OT_MAX;
	}

	CtrGpu_WriteColorCode(&poly->r0, color);
	CtrGpu_WritePackedXY(&poly->x0, sxy[quadPointIndex[quadIndex][0]]);
	CtrGpu_WritePackedXY(&poly->x1, sxy[quadPointIndex[quadIndex][1]]);
	CtrGpu_WritePackedXY(&poly->x2, sxy[quadPointIndex[quadIndex][2]]);
	CtrGpu_WritePackedXY(&poly->x3, sxy[quadPointIndex[quadIndex][3]]);
	VehGroundShadow_WriteUv(poly, texture);

	ot = &otBase[depthIndex];
	CtrGpu_LinkPacket24(ot, &poly->tag, poly, VEH_GROUND_SHADOW_GPU_TAG_POLY_FT4);

	*primCursor = (u32 *)(poly + 1);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b720-0x8005c120.
void VehGroundShadow_Main(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct PrimMem *primMem;
	u32 *prim;
	struct VehGroundShadowScratch *scratch = CTR_SCRATCHPAD_PTR(struct VehGroundShadowScratch, 0);
	struct TextureLayout *shadowTex0 = &scratch->shadowTex[0];
	struct TextureLayout *shadowTex1 = &scratch->shadowTex[1];
	struct VehGroundShadowEntry *entries = scratch->entries;
	int numPlayers;

	if (!VehGroundShadow_Subset1(shadowTex0, VEH_GROUND_SHADOW_ICON_LEFT))
	{
		return;
	}

	if (!VehGroundShadow_Subset1(shadowTex1, VEH_GROUND_SHADOW_ICON_RIGHT))
	{
		return;
	}

	primMem = &gGT->backBuffer->primMem;
	prim = (u32 *)primMem->cursor;
	if (prim + VEH_GROUND_SHADOW_PRIM_GUARD_WORDS >= (u32 *)primMem->guardEnd)
	{
		return;
	}

	CTC2(0, 5);
	CTC2(0, 6);
	CTC2(0, 7);

	numPlayers = gGT->numPlyrCurrGame;

	for (int driverIndex = 0; driverIndex < VEH_GROUND_SHADOW_MAX_DRIVERS; driverIndex++)
	{
		struct VehGroundShadowEntry *entry = &entries[driverIndex];
		struct Driver *driver = gGT->drivers[driverIndex];

		if (driver != NULL)
		{
			VehGroundShadow_BuildEntry(entry, driver, numPlayers);
		}
		else
		{
			entry->driver = NULL;
			entry->state = VEH_GROUND_SHADOW_STATE_PENDING;
		}
	}
	scratch->sentinelDriver = NULL;

	for (int playerIndex = numPlayers - 1; playerIndex >= 0; playerIndex--)
	{
		struct PushBuffer *pb = &gGT->pushBuffer[playerIndex];
		s32 camX = pb->matrix_Camera.t[0];
		s32 camY = pb->matrix_Camera.t[1];
		s32 camZ = pb->matrix_Camera.t[2];
		u32 *otBase = pb->ptrOT;
		int isLargeGeomScreen;

		CTC2((u32)(s32)pb->rect.w << VEH_GROUND_SHADOW_GTE_SCREEN_SHIFT, 24);
		CTC2((u32)(s32)pb->rect.h << VEH_GROUND_SHADOW_GTE_SCREEN_SHIFT, 25);
		CTC2((u32)pb->distanceToScreen_PREV, 26);
		VehGroundShadow_LoadGteRotMatrix(&pb->matrix_ViewProj);
		isLargeGeomScreen = pb->distanceToScreen_PREV > VEH_GROUND_SHADOW_LARGE_GEOM_SCREEN_THRESHOLD;

		for (int driverIndex = 0; driverIndex < VEH_GROUND_SHADOW_MAX_DRIVERS; driverIndex++)
		{
			struct VehGroundShadowEntry *entry = &entries[driverIndex];
			s32 diffX;
			s32 diffY;
			s32 diffZ;
			s32 scaledX;
			s32 scaledY;
			s32 scaledZ;
			s16 scaled[4];
			s32 groundDistance;
			u32 color;
			SVec3 points[VEH_GROUND_SHADOW_NUM_POINTS];
			u32 sxy[VEH_GROUND_SHADOW_NUM_POINTS];
			s32 depth[VEH_GROUND_SHADOW_NUM_POINTS];

			if (entry->driver == NULL)
			{
				break;
			}

			if (entry->state == VEH_GROUND_SHADOW_STATE_CULLED)
			{
				continue;
			}

			if ((entry->instFlags & HIDE_MODEL) != 0)
			{
				entry->state = VEH_GROUND_SHADOW_STATE_CULLED;
				continue;
			}

			if ((entry->idppFlags[playerIndex] & DRAW_SUCCESSFUL) == 0)
			{
				continue;
			}

			diffX = VehGroundShadow_DiffS16(entry->pos.x, camX);
			diffY = VehGroundShadow_DiffS16(entry->pos.y, camY);
			diffZ = VehGroundShadow_DiffS16(entry->pos.z, camZ);
			scaledX = (s16)(diffX * VEH_GROUND_SHADOW_CAMERA_DELTA_SCALE);
			scaledY = (s16)(diffY * VEH_GROUND_SHADOW_CAMERA_DELTA_SCALE);
			scaledZ = (s16)(diffZ * VEH_GROUND_SHADOW_CAMERA_DELTA_SCALE);

			if (!isLargeGeomScreen)
			{
				if (scaledX >= VEH_GROUND_SHADOW_SMALL_SCREEN_MAX_EXCLUSIVE || scaledY >= VEH_GROUND_SHADOW_SMALL_SCREEN_MAX_EXCLUSIVE ||
				    scaledZ >= VEH_GROUND_SHADOW_SMALL_SCREEN_MAX_EXCLUSIVE || scaledX < VEH_GROUND_SHADOW_SMALL_SCREEN_MIN ||
				    scaledY < VEH_GROUND_SHADOW_SMALL_SCREEN_MIN || scaledZ < VEH_GROUND_SHADOW_SMALL_SCREEN_MIN)
				{
					continue;
				}
			}

			scaled[0] = (s16)scaledX;
			scaled[1] = (s16)scaledY;
			scaled[2] = (s16)scaledZ;
			scaled[3] = 0;
			CTR_GteLoadS16TripletV0(scaled);
			gte_rtv0_b();
			groundDistance = (s32)MFC2(27) >> VEH_GROUND_SHADOW_GROUND_DISTANCE_SHIFT;

			if (groundDistance < VEH_GROUND_SHADOW_GROUND_DISTANCE_MIN)
			{
				continue;
			}

			if (isLargeGeomScreen)
			{
				color = VEH_GROUND_SHADOW_LARGE_COLOR;
			}
			else
			{
				s32 intensity;

				if (groundDistance < VEH_GROUND_SHADOW_FULL_INTENSITY_DISTANCE)
				{
					intensity = VEH_GROUND_SHADOW_FULL_INTENSITY;
				}
				else
				{
					int fade = (VEH_GROUND_SHADOW_FADE_END_DISTANCE - groundDistance) * VEH_GROUND_SHADOW_FULL_INTENSITY;

					if (fade < 0)
					{
						fade += VEH_GROUND_SHADOW_FADE_ROUNDING_BIAS;
					}

					intensity = fade >> VEH_GROUND_SHADOW_FADE_SHIFT;
					if (intensity < 1)
					{
						continue;
					}
				}

				color = VEH_GROUND_SHADOW_COLOR_PREFIX | (u32)intensity | ((u32)intensity << 8) | ((u32)intensity << 16);
			}

			if (entry->state == VEH_GROUND_SHADOW_STATE_PENDING)
			{
				VehGroundShadow_TransformLocalAxes(entry);
				VehGroundShadow_LoadGteRotMatrix(&pb->matrix_ViewProj);

				if (entry->state == VEH_GROUND_SHADOW_STATE_CULLED)
				{
					continue;
				}
			}

			VehGroundShadow_BuildProjectionPoints(entry, scaledX, scaledY, scaledZ, points);
			VehGroundShadow_ProjectPoints(points, sxy, depth);

			for (int quadIndex = 0; quadIndex < VEH_GROUND_SHADOW_NUM_QUADS; quadIndex++)
			{
				const struct TextureLayout *texture = (quadIndex & 1) != 0 ? shadowTex1 : shadowTex0;

				VehGroundShadow_EmitQuad(&prim, otBase, texture, color, sxy, depth[0], entry->depthBias, quadIndex);
			}
		}
	}

	primMem->cursor = prim;
}
