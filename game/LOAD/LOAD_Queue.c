#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032d30-0x80032d8c.
void LOAD_AppendQueue(struct BigHeader *bigfile, int type, int fileIndex, void *destinationPtr, void (*callback)(struct LoadQueueSlot *))
{
	if (sdata->queueLength >= LOAD_QUEUE_SLOT_COUNT)
	{
		return;
	}

	struct LoadQueueSlot *lqs = &sdata->queueSlots[(s32)sdata->queueLength];
	lqs->ptrBigfileCdPos_UNUSED = bigfile;
	lqs->flags = 0;
	lqs->type_UNUSED = type;
	lqs->subfileIndex = fileIndex;
	lqs->ptrDestination = destinationPtr;
	lqs->size_UNUSED = 0;
	lqs->callbackFuncPtr = callback;

	sdata->queueLength++;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032d8c-0x80032dc0.
void LOAD_CDRequestCallback(struct LoadQueueSlot *lqs)
{
	if (lqs->callbackFuncPtr != NULL)
	{
		lqs->callbackFuncPtr(lqs);
	}

	sdata->queueReady = 1;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 PS1 path 0x80032dc0-0x80032ffc.
void LOAD_NextQueuedFile()
{
	struct LoadQueueSlot *curr = &data.currSlot;

	if ((sdata->queueReady != 0) && (sdata->XA_State == 0) && (sdata->queueLength != 0))
	{
		sdata->queueReady = 0;

		if (sdata->queueRetry != 0)
		{
			sdata->queueRetry = 0;
		}
		else
		{
			*curr = sdata->queueSlots[0];

			for (int i = LOAD_QUEUE_FIRST_PENDING_SLOT; i < sdata->queueLength; i++)
			{
				sdata->queueSlots[i - 1] = sdata->queueSlots[i];
			}
		}

		switch (curr->type_UNUSED)
		{
		case LT_RAW:
			curr->ptrDestination = LOAD_ReadFile_ex(curr->ptrBigfileCdPos_UNUSED, LT_SETADDR, curr->subfileIndex, curr->ptrDestination, &curr->size_UNUSED,
			                                        LOAD_CDRequestCallback);
			break;

		case LT_DRAM:
			curr->ptrDestination =
			    LOAD_DramFile(curr->ptrBigfileCdPos_UNUSED, curr->subfileIndex, curr->ptrDestination, &curr->size_UNUSED, (int)(intptr_t)curr->callbackFuncPtr);
			break;

		case LT_VRAM:
			curr->ptrDestination =
			    LOAD_VramFile(curr->ptrBigfileCdPos_UNUSED, curr->subfileIndex, curr->ptrDestination, &curr->size_UNUSED, (int)(intptr_t)curr->callbackFuncPtr);
			break;
		}

		sdata->queueLength--;
	}

	if (sdata->frameFinishedVRAM != 0)
	{
		if ((u32)(sdata->gGT->frameTimer_VsyncCallback - sdata->frameFinishedVRAM) >= LOAD_QUEUE_VRAM_CALLBACK_DELAY_FRAMES)
		{
			if (curr->callbackFuncPtr != NULL)
			{
				curr->callbackFuncPtr(curr);
			}

			sdata->frameFinishedVRAM = 0;

#if defined(CTR_NATIVE)
			// NOTE(aalhendi): CTR_NATIVE marks Mempack allocations with a host-only
			// flag while the retail path uses LT_SETADDR for the same ownership.
			if ((curr->flags & (LT_SETADDR | LT_MEMPACK)) != 0)
#else
			if ((curr->flags & LT_SETADDR) != 0)
#endif
			{
				MEMPACK_PopState();
			}

			sdata->queueReady = 1;
		}
	}
}
