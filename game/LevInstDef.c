#include <common.h>

#if defined(CTR_NATIVE)
#define NATIVE_SUPER_TURBO_VISUAL_QUAD_LIMIT 8192

static const struct QuadBlock *s_nativeSuperTurboQuadBase;
static int s_nativeSuperTurboQuadCount;
static u32 s_nativeSuperTurboVisualBits[(NATIVE_SUPER_TURBO_VISUAL_QUAD_LIMIT + 31) / 32];

static int LevInstDef_AbsInt(int value)
{
	return value < 0 ? -value : value;
}

static b32 LevInstDef_QuadHasTexture(const struct QuadBlock *quad)
{
	return quad->ptr_texture_low != NULL || quad->ptr_texture_mid[0] != NULL || quad->ptr_texture_mid[1] != NULL ||
	       quad->ptr_texture_mid[2] != NULL || quad->ptr_texture_mid[3] != NULL;
}

static b32 LevInstDef_QuadSharesSuperTurboTexture(const struct QuadBlock *quad, const struct QuadBlock *seed)
{
	if ((quad == NULL) || (seed == NULL))
	{
		return false;
	}

	int sharedTextures = ((quad->ptr_texture_low != NULL) && (quad->ptr_texture_low == seed->ptr_texture_low)) ? 1 : 0;
	for (int quadFace = 0; quadFace < 4; quadFace++)
	{
		void *quadTexture = quad->ptr_texture_mid[quadFace];
		if (quadTexture == NULL)
		{
			continue;
		}

		for (int seedFace = 0; seedFace < 4; seedFace++)
		{
			if (quadTexture == seed->ptr_texture_mid[seedFace])
			{
				sharedTextures++;
				break;
			}
		}
	}

	return sharedTextures >= 2;
}

static b32 LevInstDef_QuadHasSameTextureSignature(const struct QuadBlock *quad, const struct QuadBlock *seed)
{
	if ((quad == NULL) || (seed == NULL) || (quad->ptr_texture_low != seed->ptr_texture_low))
	{
		return false;
	}

	for (int face = 0; face < 4; face++)
	{
		if (quad->ptr_texture_mid[face] != seed->ptr_texture_mid[face])
		{
			return false;
		}
	}

	return true;
}

static int LevInstDef_CountTextureSignatureUses(const struct QuadBlock *quadBlocks, int numQuadBlocks, const struct QuadBlock *seed)
{
	int useCount = 0;
	for (int quadIndex = 0; quadIndex < numQuadBlocks; quadIndex++)
	{
		const struct QuadBlock *quad = &quadBlocks[quadIndex];
		if (((quad->quadFlags & QUADBLOCK_FLAG_TRIGGER) == 0) && LevInstDef_QuadHasSameTextureSignature(quad, seed))
		{
			useCount++;
		}
	}

	return useCount;
}

static void LevInstDef_TintSuperTurboGlow(struct mesh_info *mesh, const struct QuadBlock *trigger)
{
	if ((mesh == NULL) || (mesh->ptrVertexArray == NULL) || (trigger == NULL))
	{
		return;
	}

	const int expandXZ = 0x300;
	const int expandY = 0x280;
	const int minX = trigger->bbox.min.x - expandXZ;
	const int maxX = trigger->bbox.max.x + expandXZ;
	const int minY = trigger->bbox.min.y - expandY;
	const int maxY = trigger->bbox.max.y + expandY;
	const int minZ = trigger->bbox.min.z - expandXZ;
	const int maxZ = trigger->bbox.max.z + expandXZ;

	for (int vertexIndex = 0; vertexIndex < mesh->numVertex; vertexIndex++)
	{
		struct LevVertex *vertex = &mesh->ptrVertexArray[vertexIndex];
		if ((vertex->pos.x < minX) || (vertex->pos.x > maxX) || (vertex->pos.y < minY) || (vertex->pos.y > maxY) ||
		    (vertex->pos.z < minZ) || (vertex->pos.z > maxZ))
		{
			continue;
		}

		const u8 r = vertex->color_hi[0];
		const u8 g = vertex->color_hi[1];
		const u8 b = vertex->color_hi[2];
		if ((g < 56) || (g < r + 20) || (g < b + 12))
		{
			continue;
		}

		const u8 peak = g;
		vertex->color_hi[0] = (u8)((r * 2 + peak) / 3);
		vertex->color_hi[1] = peak;
		vertex->color_hi[2] = peak > b ? peak : b;
	}

}

static void LevInstDef_MarkSuperTurboVisualQuad(int quadIndex)
{
	if ((quadIndex < 0) || (quadIndex >= NATIVE_SUPER_TURBO_VISUAL_QUAD_LIMIT))
	{
		return;
	}

	s_nativeSuperTurboVisualBits[quadIndex >> 5] |= 1u << (quadIndex & 31);
}

b32 LevInstDef_IsSuperTurboVisualQuad(const struct QuadBlock *quad)
{
	if ((quad == NULL) || (s_nativeSuperTurboQuadBase == NULL))
	{
		return false;
	}

	const ptrdiff_t quadIndex = quad - s_nativeSuperTurboQuadBase;
	if ((quadIndex < 0) || (quadIndex >= s_nativeSuperTurboQuadCount) || (quadIndex >= NATIVE_SUPER_TURBO_VISUAL_QUAD_LIMIT))
	{
		return false;
	}

	return (s_nativeSuperTurboVisualBits[quadIndex >> 5] & (1u << (quadIndex & 31))) != 0;
}

static void LevInstDef_FindSuperTurboVisualQuads(struct mesh_info *mesh)
{
	const struct QuadBlock *quadBlocks = mesh->ptrQuadBlockArray;
	const int numQuadBlocks = mesh->numQuadBlock;
	SDL_memset(s_nativeSuperTurboVisualBits, 0, sizeof(s_nativeSuperTurboVisualBits));
	s_nativeSuperTurboQuadBase = quadBlocks;
	s_nativeSuperTurboQuadCount = numQuadBlocks;

	for (int triggerIndex = 0; triggerIndex < numQuadBlocks; triggerIndex++)
	{
		const struct QuadBlock *trigger = &quadBlocks[triggerIndex];
		if (((trigger->quadFlags & QUADBLOCK_FLAG_TRIGGER) == 0) || ((trigger->terrain_type & COLL_STEP_TRIGGER_SUPER_TURBO_PAD) == 0))
		{
			continue;
		}

		const int triggerSpanX = trigger->bbox.max.x - trigger->bbox.min.x;
		const int triggerSpanZ = trigger->bbox.max.z - trigger->bbox.min.z;
		const s64 triggerArea = (s64)(triggerSpanX > 0 ? triggerSpanX : 1) * (s64)(triggerSpanZ > 0 ? triggerSpanZ : 1);
		int bestIndex = -1;
		int bestScore = -0x7fffffff;
		int bestTriggerOverlap = 0;
		int bestCandidateOverlap = 0;
		int bestYDelta = 0;
		for (int candidateIndex = 0; candidateIndex < numQuadBlocks; candidateIndex++)
		{
			const struct QuadBlock *candidate = &quadBlocks[candidateIndex];
			if (((candidate->quadFlags & QUADBLOCK_FLAG_TRIGGER) != 0) || !LevInstDef_QuadHasTexture(candidate))
			{
				continue;
			}

			const int spanX = candidate->bbox.max.x - candidate->bbox.min.x;
			const int spanY = candidate->bbox.max.y - candidate->bbox.min.y;
			const int spanZ = candidate->bbox.max.z - candidate->bbox.min.z;
			if ((spanX <= 0) || (spanZ <= 0))
			{
				continue;
			}

			// Reject thin walls/rails that merely intersect the trigger volume. Turbo-pad
			// surfaces and their overlay layers are broad in X/Z compared with their Y span.
			const int minHorizontalSpan = spanX < spanZ ? spanX : spanZ;
			if ((minHorizontalSpan < 96) || ((spanY > 0) && (spanY * 2 > minHorizontalSpan * 3 + 64)))
			{
				continue;
			}

			const int overlapMinX = candidate->bbox.min.x > trigger->bbox.min.x ? candidate->bbox.min.x : trigger->bbox.min.x;
			const int overlapMaxX = candidate->bbox.max.x < trigger->bbox.max.x ? candidate->bbox.max.x : trigger->bbox.max.x;
			const int overlapMinZ = candidate->bbox.min.z > trigger->bbox.min.z ? candidate->bbox.min.z : trigger->bbox.min.z;
			const int overlapMaxZ = candidate->bbox.max.z < trigger->bbox.max.z ? candidate->bbox.max.z : trigger->bbox.max.z;
			const int overlapSpanX = overlapMaxX - overlapMinX;
			const int overlapSpanZ = overlapMaxZ - overlapMinZ;
			if ((overlapSpanX <= 0) || (overlapSpanZ <= 0))
			{
				continue;
			}

			const s64 overlapArea = (s64)overlapSpanX * (s64)overlapSpanZ;
			const s64 candidateArea = (s64)spanX * (s64)spanZ;
			const int triggerOverlap = (int)((overlapArea * 1000) / triggerArea);
			const int candidateOverlap = (int)((overlapArea * 1000) / candidateArea);
			const int triggerCenterY = trigger->bbox.min.y + trigger->bbox.max.y;
			const int candidateCenterY = candidate->bbox.min.y + candidate->bbox.max.y;
			const int yDelta = LevInstDef_AbsInt(candidateCenterY - triggerCenterY) / 2;
			const b32 exactFootprint = (candidate->bbox.min.x == trigger->bbox.min.x) && (candidate->bbox.max.x == trigger->bbox.max.x) &&
			                           (candidate->bbox.min.z == trigger->bbox.min.z) && (candidate->bbox.max.z == trigger->bbox.max.z);

			if (yDelta <= 256 && (triggerOverlap >= 120 || candidateOverlap >= 500))
			{
				const int sizeDelta = LevInstDef_AbsInt(spanX - triggerSpanX) + LevInstDef_AbsInt(spanZ - triggerSpanZ);
				int score = triggerOverlap * 6 + candidateOverlap * 3 - yDelta * 3 - sizeDelta / 8;
				if (exactFootprint && yDelta <= 96)
				{
					score += 100000;
				}

				if (score > bestScore)
				{
					bestScore = score;
					bestIndex = candidateIndex;
					bestTriggerOverlap = triggerOverlap;
					bestCandidateOverlap = candidateOverlap;
					bestYDelta = yDelta;
				}
			}
		}

			if (bestIndex >= 0)
			{
				const struct QuadBlock *candidate = &quadBlocks[bestIndex];
				if ((sdata != NULL) && (sdata->gGT != NULL) && (sdata->gGT->levelID == CORTEX_CASTLE))
				{
					const int textureUseCount = LevInstDef_CountTextureSignatureUses(quadBlocks, numQuadBlocks, candidate);
					// Cortex Castle intentionally tags a few ordinary wooden floor tiles as USF.
					if (textureUseCount >= 8)
					{
						continue;
					}
				}

				LevInstDef_MarkSuperTurboVisualQuad(bestIndex);

				int partCount = 0;
				for (int partIndex = 0; partIndex < numQuadBlocks && partCount < 6; partIndex++)
				{
					if (partIndex == bestIndex)
					{
						continue;
					}

					const struct QuadBlock *part = &quadBlocks[partIndex];
					if (((part->quadFlags & QUADBLOCK_FLAG_TRIGGER) != 0) ||
					    !LevInstDef_QuadSharesSuperTurboTexture(part, candidate))
					{
						continue;
					}

					const int overlapMinX = part->bbox.min.x > trigger->bbox.min.x ? part->bbox.min.x : trigger->bbox.min.x;
					const int overlapMaxX = part->bbox.max.x < trigger->bbox.max.x ? part->bbox.max.x : trigger->bbox.max.x;
					const int overlapMinZ = part->bbox.min.z > trigger->bbox.min.z ? part->bbox.min.z : trigger->bbox.min.z;
					const int overlapMaxZ = part->bbox.max.z < trigger->bbox.max.z ? part->bbox.max.z : trigger->bbox.max.z;
					const int overlapSpanX = overlapMaxX - overlapMinX;
					const int overlapSpanZ = overlapMaxZ - overlapMinZ;
					if ((overlapSpanX <= 0) || (overlapSpanZ <= 0))
					{
						continue;
					}

					const int spanX = part->bbox.max.x - part->bbox.min.x;
					const int spanZ = part->bbox.max.z - part->bbox.min.z;
					if ((spanX <= 0) || (spanZ <= 0))
					{
						continue;
					}

					const s64 overlapArea = (s64)overlapSpanX * (s64)overlapSpanZ;
					const s64 partArea = (s64)spanX * (s64)spanZ;
					const int triggerOverlap = (int)((overlapArea * 1000) / triggerArea);
					const int partOverlap = (int)((overlapArea * 1000) / partArea);
					const int triggerCenterY = trigger->bbox.min.y + trigger->bbox.max.y;
					const int partCenterY = part->bbox.min.y + part->bbox.max.y;
					const int yDelta = LevInstDef_AbsInt(partCenterY - triggerCenterY) / 2;

					if ((yDelta > 192) || ((triggerOverlap < 80) && (partOverlap < 500)))
					{
						continue;
					}

					LevInstDef_MarkSuperTurboVisualQuad(partIndex);
					partCount++;
				}
			}


			if ((sdata != NULL) && (sdata->gGT != NULL) &&
			    ((sdata->gGT->levelID == OXIDE_STATION) || (sdata->gGT->levelID == HOT_AIR_SKYWAY)))
			{
				LevInstDef_TintSuperTurboGlow(mesh, trigger);
			}
		}
}
#endif


void LevInstDef_UnPack(struct mesh_info *ptr_mesh_info)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003116c-0x80031268.
	int i;
	int numQuadBlock;
	struct QuadBlock *ptrQuadBlockArray;
	struct QuadBlock *qbCurr;
	struct InstDef **visInstSrc;
	struct Level *level1;

	numQuadBlock = ptr_mesh_info->numQuadBlock;
	ptrQuadBlockArray = ptr_mesh_info->ptrQuadBlockArray;

#if defined(CTR_NATIVE)
	LevInstDef_FindSuperTurboVisualQuads(ptr_mesh_info);
#endif

	// loop through all quadblocks
	for (i = 0; i < numQuadBlock; i++)
	{
		qbCurr = &ptrQuadBlockArray[i];

		if ((qbCurr->pvs != 0) && (qbCurr->pvs->visInstSrc != 0))
		{
			// loop through all instance pointers visible on quadblock
			for (visInstSrc = (struct InstDef **)qbCurr->pvs->visInstSrc; visInstSrc[0] != NULL; visInstSrc++)
			{
				//ND BUG: This operation is not idempotent. The outer for loop means we will do this operation multiple times
				//on the same pointer, so we keep switching it from an InstDef pointer to an Instance pointer and back again.
				//This is not a problem in the original game because LEVs were designed with this in mind (odd numbers of
				//quadblocks), but we need to keep this in mind. The easiest solution I can think of is to keep track of which
				//InstDefs have been unpacked and only unpack them once, but that requires a lot of extra bookkeeping and wouldn't.
				//be "vanilla".
				visInstSrc[0] = (struct InstDef *)visInstSrc[0]->ptrInstance;
			}
		}
	}

	level1 = sdata->gGT->level1;

	if (level1->ptrInstDefPtrArray != 0)
	{
		// loop through all instDef pointers in the LEV
		for (visInstSrc = level1->ptrInstDefPtrArray; visInstSrc[0] != 0; visInstSrc++)
		{
			visInstSrc[0] = (struct InstDef *)visInstSrc[0]->ptrInstance;
		}
	}
}


void LevInstDef_RePack(struct mesh_info *ptr_mesh_info, b32 boolAdvHub)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80031268-0x800313c8.
	int i;
	int numQuadBlock;
	struct QuadBlock *ptrQuadBlockArray;
	struct QuadBlock *qbCurr;
	struct Instance **visInstSrc;
	struct Level *level1;
	struct Thread *th;

	numQuadBlock = ptr_mesh_info->numQuadBlock;
	ptrQuadBlockArray = ptr_mesh_info->ptrQuadBlockArray;

	// loop through all quadblocks
	for (i = 0; i < numQuadBlock; i++)
	{
		qbCurr = &ptrQuadBlockArray[i];

		if ((qbCurr->pvs != 0) && (qbCurr->pvs->visInstSrc != 0))
		{
			// loop through all instance pointers visible on quadblock
			for (visInstSrc = qbCurr->pvs->visInstSrc; visInstSrc[0] != NULL; visInstSrc++)
			{
				visInstSrc[0] = (struct Instance *)visInstSrc[0]->instDef; // maybe `visInstSrc[0]->instDef->ptrInstance`?
			}
		}
	}

	level1 = sdata->gGT->level1;

	if (level1->ptrInstDefPtrArray != 0)
	{
		// loop through all instDef pointers in the LEV
		for (visInstSrc = (struct Instance **)level1->ptrInstDefPtrArray; visInstSrc[0] != NULL; visInstSrc++)
		{
			struct Instance *inst = visInstSrc[0];
			struct InstDef *instDef = inst->instDef;

			// if on adv hub
			if (boolAdvHub != 0)
			{
				th = inst->thread;
				if (th != 0)
				{
					th->flags |= THREAD_FLAG_DEAD;
				}

				// erase instance in pool
				LIST_AddFront(&sdata->gGT->JitPools.instance.free, (struct Item *)inst);
			}

			// go back to instDef
			visInstSrc[0] = (struct Instance *)instDef;
		}
	}

	PROC_CheckAllForDead();
}
