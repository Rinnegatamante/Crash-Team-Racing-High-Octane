#include <common.h>

#ifndef CTR_NATIVE

void MM_Video_DecDCToutCallbackFunc(void)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b5a64-0x800b5b7c.
#ifndef CTR_NATIVE
// part of PSYQ BSS
#define StCdIntrFlag *(u32 *)0x8009ebf8

	if (((V230.flags & MM_VIDEO_FLAG_RGB24) != 0) && (StCdIntrFlag != 0))
	{
		StCdInterrupt();

		StCdIntrFlag = 0;
	}
#else
	// NOTE(aalhendi): Native PsyCross does not map PSYQ BSS at 0x8009ebf8.
#endif

	uint32_t *ot = BreakDraw();

	LoadImage(&V230.slice, V230.out_Buf[V230.dctOutBufferIndex]);

	/* update slice (rectangular strip) area to next one on the right */
	V230.slice.x += V230.slice.w;
	V230.dctOutBufferIndex ^= 1;

	if (V230.sliceIndex == V230.finalSliceIndex)
	{
		V230.dctOutputDone = 1;
	}
	else
	{
		V230.sliceIndex++;
		DecDCTout(V230.out_Buf[V230.dctOutBufferIndex], V230.dctOutSliceSize);
	}

	if (ot != 0)
	{
		DrawOTag(ot);
	}
}

void MM_Video_KickCD(CdlLOC *location)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b5b7c-0x800b5c8c.
	int result;
	int cdlMode;
	u8 mode[4];

	if ((location != NULL) && (V230.ptrCdLoc != &V230.cdLocation1))
	{
		V230.cdKickState = 0;
		V230.ptrCdLoc = location;
	}

	switch (V230.cdKickState)
	{
	case 0:
		result = CdControl(CdlSetloc, (u8 *)V230.ptrCdLoc, 0);
		if (result == 0)
			return;

		V230.cdKickState = 1;

		// do NOT break,
		// original code never quit here

	case 1:
		mode[0] = CdlModeSpeed;

		result = CdControl(CdlSetmode, mode, 0);
		if (result == 0)
			return;

		V230.cdKickState = 2;
		break;

	case 2:
		V230.cdKickState = 3;
		break;

	case 3:
		cdlMode = CdlModeStream2 | CdlModeSpeed;

		// scrapbook
		// if video contains audio
		if ((V230.flags & MM_VIDEO_FLAG_HAS_XA_AUDIO) != 0)
		{
			cdlMode = CdlModeStream2 | CdlModeSpeed | CdlModeRT;
		}

		V230.cdKickState = 0;

		result = CdRead2(cdlMode);
		if (result == 0)
			return;

		V230.ptrCdLoc = 0;
		break;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800b5c8c-0x800b615c.
void MM_Video_VLC_Decode(void)
{
	s16 oldDecodeState;
	int backloc;
	int result;
	uint32_t size;
	s16 freeSectors;
	s16 overSectors;
	uint32_t *sectorData;
	StHEADER *sectorHeader[2];
	int waitTime;
	CdlLOC *sectorLoc;

	waitTime = MM_VIDEO_VLC_WAIT_FRAMES;

	// free sectors and over sectors
	StRingStatus(&freeSectors, &overSectors);

	backloc = StGetBackloc(&V230.cdLocation2);

	oldDecodeState = V230.decodeState;
	if ((V230.decodeState == 1) && ((V230.ringSectorCount - (V230.ringSectorCount >> 2)) <= freeSectors))
	{
		V230.stallRecoveryFrames++;

		if (MM_VIDEO_STALL_RECOVERY_FRAMES < V230.stallRecoveryFrames)
		{
			V230.stallRecoveryFrames = 0;
			StClearRing();
			V230.endOfStream = 0;
			V230.sectorFrameCount = 0;
			V230.lastSectorFrameCount = 0;
			V230.lastBackloc = 0;
			V230.loopStartBackloc = MM_VIDEO_BACKLOC_NONE;
			V230.loopEndBackloc = MM_VIDEO_BACKLOC_NONE;
			V230.loopWrapPending = 0;
			V230.stalledBacklocFrames = 0;
			V230.stallRecoveryFrames = 0;
			V230.unused_0x38 = MM_VIDEO_BACKLOC_NONE;
			V230.decodeState = oldDecodeState;

			MM_Video_KickCD(&V230.cdLocation1);
		}

		V230.drawNextFrame = 0;
		return;
	}

	V230.stallRecoveryFrames = 0;

	// Scrapbook
	if (((V230.flags & MM_VIDEO_FLAG_SCRAPBOOK) == 0) && (freeSectors < (V230.ringSectorCount >> 4)))
	{
		MM_Video_KickCD(&V230.cdLocation2);
	}

	if (backloc == V230.lastBackloc)
	{
		V230.stalledBacklocFrames++;
		if (MM_VIDEO_STALLED_BACKLOC_FRAMES < V230.stalledBacklocFrames)
		{
			V230.stalledBacklocFrames = 0;
			V230.drawNextFrame = 0;
			StClearRing();
			V230.lastSectorFrameCount = 0;
			V230.loopStartBackloc = MM_VIDEO_BACKLOC_NONE;
			V230.loopEndBackloc = MM_VIDEO_BACKLOC_NONE;
			V230.loopWrapPending = 0;
			V230.stallRecoveryFrames = 0;
			V230.lastBackloc = 0;
			V230.stalledBacklocFrames = 0;
			V230.unused_0x38 = MM_VIDEO_BACKLOC_NONE;

			MM_Video_KickCD(&V230.cdLocation3);
		}
	}
	else
	{
		V230.stalledBacklocFrames = 0;
	}

	V230.decodeState = 0;

	// if reached end of video,
	// choose to loop or not loop
	if ((V230.loopStartBackloc < 0) &&

	    // length of video
	    ((V230.streamFrameCount <= backloc ||

	      (backloc < V230.lastBackloc))))
	{
		// scrapbook not track select,
		// if video is not looping
		if ((V230.flags & MM_VIDEO_FLAG_LOOP) == 0)
		{
			do
			{
				result = CdControl(CdlPause, 0, 0);
			} while (result == 0);
			// end of scrapbook
			V230.endOfStream = 1;
		}

		// track select, not scrapbook,
		// if video is looping
		else
		{
			V230.unused_0x38 = MM_VIDEO_BACKLOC_NONE;
			if (V230.loopEndBackloc < 1)
			{
				MM_Video_KickCD(&V230.cdLocation1);

				if (backloc == V230.streamFrameCount)
				{
					V230.loopEndBackloc = CdPosToInt(&V230.cdLocation2);
				}
				else
				{
					V230.loopStartBackloc = CdPosToInt(&V230.cdLocation2);
					V230.loopStartBackloc--;
					V230.loopWrapPending = 0;
				}
			}
			else
			{
				if (backloc != V230.streamFrameCount)
				{
					result = CdPosToInt(&V230.cdLocation2);
					if (V230.loopEndBackloc < result)
					{
						V230.loopStartBackloc = CdPosToInt(&V230.cdLocation2);
						V230.loopStartBackloc = V230.loopStartBackloc + -1;
						V230.loopWrapPending = 0;

						MM_Video_KickCD(&V230.cdLocation1);
					}
					V230.loopEndBackloc = MM_VIDEO_BACKLOC_NONE;
				}
			}
		}
	}

	V230.lastSectorFrameCount = V230.sectorFrameCount;
	V230.lastBackloc = backloc;

LAB_800b5fec:

	// retrieve data with timeout (10 frames)
	do
	{
		result = StGetNext(&sectorData, sectorHeader);
		if (result == 0)
		{
			V230.sectorFrameCount = sectorHeader[0]->frameCount;

			if (V230.sectorFrameCount == V230.lastSectorFrameCount)
			{
				StFreeRing(sectorData);
				goto LAB_800b5fec;
			}

			if (0 < V230.loopStartBackloc)
			{
				sectorLoc = &sectorHeader[0]->loc;
				result = CdPosToInt(sectorLoc);

				waitTime = MM_VIDEO_VLC_WAIT_FRAMES;

				if (V230.loopStartBackloc <= result)
				{
					V230.loopWrapPending = 1;
					StFreeRing(sectorData);
					goto LAB_800b5fec;
				}
				if (V230.loopWrapPending == 1)
				{
					V230.loopStartBackloc = MM_VIDEO_BACKLOC_NONE;
					V230.loopWrapPending = 0;
					V230.lastBackloc = backloc;
				}
			}

			size = DecDCTBufSize(sectorData);

			if (size <= V230.vlcBufferSize)
			{
				sectorLoc = &sectorHeader[0]->loc;
				V230.cdLocation3 = *sectorLoc;

				// VLC Decode
				// last parameter is "VLC Table"
				DecDCTvlc2(sectorData, V230.in_Buf[V230.vlcBufferIndex], sdata->ptrVlcTable);

				// ready to draw next frame
				V230.drawNextFrame = 1;

				StFreeRing(sectorData);
				return;
			}
			V230.drawNextFrame = 0;
			StFreeRing(sectorData);
			return;
		}
		waitTime--;
	} while (waitTime != 0);

	V230.drawNextFrame = 0;
}

void MM_Video_StartStream(int cdStartSector, int streamFrameCount)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b615c-0x800b6260.
	V230.cdRetryState = 0;
	V230.endOfStream = 0;
	V230.decodeState = 1;

	V230.dctOutputDone = 0;

	V230.sectorFrameCount = 0;
	V230.lastSectorFrameCount = 0;
	V230.lastBackloc = 0;
	V230.loopStartBackloc = MM_VIDEO_BACKLOC_NONE;
	V230.loopEndBackloc = MM_VIDEO_BACKLOC_NONE;
	V230.loopWrapPending = 0;
	V230.stalledBacklocFrames = 0;
	V230.stallRecoveryFrames = 0;
	V230.unused_0x38 = MM_VIDEO_BACKLOC_NONE;
	V230.drawNextFrame = 0;

	V230.streamFrameCount = streamFrameCount;

	// start streaming video
	CdIntToPos(cdStartSector, &V230.cdLocation1);

	// next parameter (0) = START_FRAME
	StSetStream((V230.flags & MM_VIDEO_FLAG_RGB24), 0, MM_VIDEO_STREAM_END_FRAME_NONE, 0, 0);

	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b6210-0x800b621c for video CD stream mode.
	CDSYS_SetMode_StreamData();

	// 800b6814 = Ring_Buf (mempack)
	StSetRing(V230.out_Buf[2], V230.ringSectorCount);

	StClearRing();

	V230.cdKickState = 0;

	V230.ptrCdLoc = &V230.cdLocation1;
}

void MM_Video_StopStream(void)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b6260-0x800b62d8.
	int cdReady = CdDiskReady(1);
	if (cdReady == CdlComplete)
	{
		do
		{
			cdReady = CdControl(CdlPause, 0, 0);

		} while (cdReady == 0);
	}

	StClearRing();

	StSetMask(1, 0, 0);

	CdDataCallback(0);

	CdReadyCallback(0);

	// Discontinue current decoding,
	// does not affect internal states (libref)
	DecDCTReset(1);

	V230.drawNextFrame = 0;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b62d8-0x800b64d4.
void MM_Video_AllocMem(u32 width, u16 height, u32 flags, int ringSectorCount, int vlcBufferShift)
{
	MEMPACK_PushState();

	// just in case
	width &= 0xffff;
	height &= 0xffff;

	V230.ringSectorCount = ringSectorCount;

	if (ringSectorCount < 1)
	{
		V230.ringSectorCount = MM_VIDEO_DEFAULT_RING_SECTORS;
	}

	b32 isRGB24 = (flags & MM_VIDEO_FLAG_RGB24);

	int bytesPerPixel = (isRGB24) ? 3 : 2;

	V230.dctMode = (u16)isRGB24;

	u32 paddedHeight = (((height - 1) >> 4) + 1) * 0x10;
	V230.finalSliceIndex = (((width - 1) >> 4) + 1U) - 1;
	V230.dctOutBufferIndex = 0;
	V230.vlcBufferIndex = 0;
	V230.dctOutSliceSize = (int)(bytesPerPixel * 8 * paddedHeight) >> 1;
	V230.vlcBufferSize = (int)(((width * bytesPerPixel) >> 1) * paddedHeight) >> (vlcBufferShift + 1U);
	V230.flags = flags;

	V230.out_Buf[0] = MEMPACK_AllocMem(V230.dctOutSliceSize << 3); //, OVR_230.s_SliceBuf);
	V230.out_Buf[1] = (uint32_t *)(((int)V230.out_Buf[0]) + V230.dctOutSliceSize * 4);

	V230.in_Buf[0] = MEMPACK_AllocMem(V230.vlcBufferSize << 3); //, OVR_230.s_VlcBuf);
	V230.in_Buf[1] = (uint32_t *)(((int)V230.in_Buf[0]) + V230.vlcBufferSize * 4);

	V230.out_Buf[2] = MEMPACK_AllocMem(V230.ringSectorCount << LOAD_CD_DATA_SECTOR_SHIFT); //, OVR_230.s_RingBuf);

	V230.slice.x = 0;
	V230.slice.y = 0;
	V230.slice.w = (s16)(bytesPerPixel << 3);
	V230.slice.h = height;

	// reinitialize everything
	DecDCTReset(0);

	DecDCTvlcSize2(V230.vlcBufferSize);
	EnterCriticalSection();
	DecDCToutCallback(&MM_Video_DecDCToutCallbackFunc);
	ExitCriticalSection();
}
#endif


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b64d4-0x800b64f4.
void MM_Video_ClearMem(void)
{
	MEMPACK_PopState();
}

#ifndef CTR_NATIVE

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b64f4-0x800b6674.
b32 MM_Video_DecodeFrame(s16 offsetX, s16 offsetY)
{
	int cdReady = CdDiskReady(1);
	b32 canDrawFrame;

	if (V230.cdRetryState == 1)
	{
		if (cdReady == 2)
		{
			V230.cdRetryState = 0;
			V230.drawNextFrame = 0;
			MM_Video_KickCD(&V230.cdLocation3);
			return 0;
		}
	}
	else
	{
		if (cdReady == CdlStatShellOpen)
		{
			V230.decodeState = 1;
			V230.stallRecoveryFrames = 0;
			V230.cdRetryState = 1;
			V230.lastBackloc = V230.sectorFrameCount - 1;
			StClearRing();
		}
	}
	if (V230.cdRetryState == 1)
	{
		V230.drawNextFrame = 0;
		canDrawFrame = false;
	}
	else
	{
		if (V230.ptrCdLoc != 0)
		{
			MM_Video_KickCD(0);
		}

		MM_Video_VLC_Decode();

		// if value is zero, return zero,
		// not ready to draw
		canDrawFrame = V230.drawNextFrame != 0;

		if (V230.drawNextFrame == 1)
		{
			V230.sliceIndex = 0;

			V230.slice.x = offsetX;
			V230.slice.y = offsetY;

			// start decoding video
			DecDCTin(V230.in_Buf[V230.vlcBufferIndex], V230.dctMode);

			V230.vlcBufferIndex ^= 1;

			// get result of decoding
			DecDCTout(V230.out_Buf[V230.dctOutBufferIndex], V230.dctOutSliceSize);

			// return 1, ready to draw
			canDrawFrame = V230.drawNextFrame != 0;
		}
	}
	return canDrawFrame;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 PSX path 0x800b6674-0x800b67ac.
b32 MM_Video_CheckIfFinished(b32 pollCdReady)
{
	b32 isFinished;
	int cdReady;
	int timeoutFrames = MM_VIDEO_DCT_OUTPUT_TIMEOUT_POLLS;
	int cdPollTimer = MM_VIDEO_CD_READY_POLL_INTERVAL;

	b32 cdReadError = false;

	if (V230.drawNextFrame == 0)
	{
		isFinished = 0;
	}
	else
	{
		do
		{
			if ((pollCdReady == 1) && (cdPollTimer--, cdPollTimer == 0))
			{
				cdReady = CdDiskReady(1);

				if (cdReady == CdlStatShellOpen)
				{
					cdReadError = true;
					V230.dctOutputDone = 1;
				}
				else
				{
					cdPollTimer = MM_VIDEO_CD_READY_POLL_INTERVAL;
				}
			}

			timeoutFrames--;

			if (timeoutFrames == 0)
			{
				V230.dctOutputDone = 1;
			}

		} while (!V230.dctOutputDone);

		do
		{
			cdReady = IsIdleGPU(MM_VIDEO_GPU_IDLE_TIMEOUT);

		} while (cdReady != 0);

		V230.dctOutputDone = 0;

		V230.drawNextFrame = 0;

		if ((!cdReadError) && (V230.sliceIndex != V230.finalSliceIndex))
		{
			// Discontinue current decoding,
			// does not affect internal states (libref)
			DecDCTReset(1);
		}

		// end of scrapbook
		isFinished = V230.endOfStream != 0;
	}
	return isFinished;
}
#endif
