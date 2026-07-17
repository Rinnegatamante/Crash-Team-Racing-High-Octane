#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3fe4-0x800b4014.
void MM_Scrapbook_Init(void)
{
	D230.scrapbookState = SCRAP_INIT;

	// change checkered flag
	RaceFlag_SetDrawOrder(1);

	// clear gamepad input (for menus)
	RECTMENU_ClearInput();
}

#ifdef CTR_NATIVE
#include <platform.h>
#include <platform/native_audio.h>
#include <platform/native_renderer.h>
#include <platform/native_str.h>

#define SCRAPBOOK_NATIVE_XA_PATH       "TEST.STR"
#define SCRAPBOOK_NATIVE_XA_CHANNEL    1
#define SCRAPBOOK_NATIVE_FRAME_Y_PAD   4
#define SCRAPBOOK_NATIVE_DISPLAY_WIDTH SCREEN_WIDTH

global_variable s32 s_scrapbookNativeNextVBlank;

static void MM_Scrapbook_GetNativeSource(s16 *srcX, s16 *srcY, s16 *displayY)
{
	struct GameTracker *gGT = sdata->gGT;
	DRAWENV *drawEnv = &gGT->db[1 - gGT->swapchainIndex].drawEnv;

	// NOTE(aalhendi): Retail decodes Scrapbook into the inactive draw page.
	// Native then presents that VRAM display page directly instead of drawing
	// the movie back through a texture primitive.
	*srcX = drawEnv->ofs[0];
	*displayY = drawEnv->ofs[1];
	*srcY = *displayY + SCRAPBOOK_NATIVE_FRAME_Y_PAD;
}
#endif

#ifndef CTR_NATIVE
CTR_GCC_OPTIMIZE_O0 int ScrapBookPlayMovie_DecodeFrame()
{
	struct GameTracker *gGT = sdata->gGT;
	DRAWENV *ptrDrawEnv = &gGT->db[1 - gGT->swapchainIndex].drawEnv;

	return MM_Video_DecodeFrame(ptrDrawEnv->ofs[0], ptrDrawEnv->ofs[1] + 4) == 0;
}
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800b4014-0x800b42b0 PSX path.
void MM_Scrapbook_PlayMovie(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;

	// book state (0,1,2,3,4)
	switch (D230.scrapbookState)
	{
	// Init State,
	// alter checkered flag
	case SCRAP_INIT:
		if (RaceFlag_IsFullyOnScreen())
		{
			// checkered flag, begin transition off-screen
			RaceFlag_BeginTransition(2);
		}

		// go to Load State
		D230.scrapbookState = SCRAP_LOAD;
		menu->state &= ~NEEDS_TO_CLOSE;
		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b4070-0x800b408c for scrapbook audio state handoff.
		Audio_SetState_Safe(1);
		break;

	// find the TEST.STR file
	case SCRAP_LOAD:

		// if not fully off screen
		if (!RaceFlag_IsFullyOffScreen())
		{
			// quit, dont start video yet
			return;
		}

		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b40a8-0x800b40b4 for scrapbook CD stream mode.
		CDSYS_SetMode_StreamData();

#ifdef CTR_NATIVE
		if (NativeSTR_StartScrapbook() != 0)
		{
			// NOTE(aalhendi): Native video decoding skips interleaved XA records;
			// play the Scrapbook CD-XA channel from the same raw STR file.
			if (NativeAudio_PlayXAFile(SCRAPBOOK_NATIVE_XA_PATH, SCRAPBOOK_NATIVE_XA_CHANNEL, sdata->vol_Music << 7, sdata->vol_Music << 7) == 0)
			{
				NativeSTR_Stop();
				goto GO_BACK;
			}
			s_scrapbookNativeNextVBlank = Platform_GetVBlankCount() + SCRAPBOOK_FRAME_VBLANKS;
			D230.scrapbookState = SCRAP_PLAY;
			return;
		}
#else
		// \TEST.STR;1
		// if file was found
		if (CdSearchFile(&cdlFile, R230.s_teststr1) != 0)
		{
			SpuSetCommonCDVolume(sdata->vol_Music << 7, sdata->vol_Music << 7);

			// Alloc memory to store Scrapbook
			MM_Video_AllocMem(SCRAPBOOK_VIDEO_WIDTH, SCRAPBOOK_VIDEO_HEIGHT, MM_VIDEO_FLAG_HAS_XA_AUDIO | MM_VIDEO_FLAG_SCRAPBOOK,
			                  MM_VIDEO_DEFAULT_RING_SECTORS, 1);

			cdPos = CdPosToInt(&cdlFile.pos);

			// CD position of video, and stream frame count
			MM_Video_StartStream(cdPos, SCRAPBOOK_STREAM_FRAMES);

			// start playing movie
			D230.scrapbookState = SCRAP_PLAY;

			return;
		}
#endif

		goto GO_BACK;

	// Actually play the movie
	case SCRAP_PLAY:
	{
		int getButtonPress;

#ifndef CTR_NATIVE
		// infinite loop (cause this is scrapbook),
		// keep doing DecodeFrame and VSync until done
		while (ScrapBookPlayMovie_DecodeFrame())
		{
			VSync(0);
		}

		// If you press Start, Cross, Circle, Triangle, or Square
		getButtonPress = (sdata->buttonTapPerPlayer[0] & SCRAPBOOK_SKIP_INPUT);

		if ((MM_Video_CheckIfFinished(0) == 1) || (getButtonPress != 0))
#else
		getButtonPress = (sdata->buttonTapPerPlayer[0] & SCRAPBOOK_SKIP_INPUT);
		s32 nativeUploaded = 0;
		s16 nativeSrcX;
		s16 nativeSrcY;
		s16 nativeDisplayY;

		MM_Scrapbook_GetNativeSource(&nativeSrcX, &nativeSrcY, &nativeDisplayY);
		if (getButtonPress == 0)
		{
			NativeRenderer_ClearVRAM(nativeSrcX, nativeDisplayY, SCRAPBOOK_NATIVE_DISPLAY_WIDTH, SCREEN_HEIGHT, 0, 0, 0);
			nativeUploaded = NativeSTR_UploadNextFrame(nativeSrcX, nativeSrcY);
		}

		if ((getButtonPress != 0) || (nativeUploaded == 0))
#endif
		{
			if (getButtonPress != 0)
			{
				RaceFlag_SetFullyOnScreen();
			}

			D230.scrapbookState = SCRAP_STOP;
		}
#ifdef CTR_NATIVE
		else
		{
			Platform_PinVRAMDisplayRect(nativeSrcX, nativeDisplayY, SCRAPBOOK_NATIVE_DISPLAY_WIDTH, SCREEN_HEIGHT, 1);
		}
#endif

#ifdef CTR_NATIVE
		if ((getButtonPress == 0) && (nativeUploaded != 0))
		{
			// NOTE(aalhendi): Native decodes this frame on the CPU. Count any
			// elapsed decode vblanks toward the retail 15fps cadence instead of
			// adding them on top of a fresh VSync(4).
			Platform_WaitUntilVBlank(s_scrapbookNativeNextVBlank);
			s_scrapbookNativeNextVBlank += SCRAPBOOK_FRAME_VBLANKS;
		}
		else
		{
			VSync(SCRAPBOOK_FRAME_VBLANKS);
		}
#else
		VSync(SCRAPBOOK_FRAME_VBLANKS);
#endif
		break;
	}

	// return disc to normal,
	// return checkered flag to normal
	case SCRAP_STOP:
#ifndef CTR_NATIVE
		SpuSetCommonCDVolume(0, 0);

		MM_Video_StopStream();

		MM_Video_ClearMem();
#else
		NativeAudio_StopXA();
		NativeSTR_Stop();
#endif

		if (RaceFlag_IsFullyOffScreen())
		{
			RaceFlag_BeginTransition(1);
		}
	GO_BACK:

		D230.scrapbookState = SCRAP_EXIT;
		break;

	case SCRAP_EXIT:
		if (RaceFlag_IsFullyOnScreen())
		{
			s16 lev;

			// change checkered flag back
			RaceFlag_SetDrawOrder(0);

			if ((gGT->gameMode1 & ADVENTURE_MODE) == 0)
			{
				lev = MAIN_MENU_LEVEL;

				MM_JumpTo_Title_Returning();

				// return to main menu (adv, tt, arcade, vs, battle)
				sdata->mainMenuState = MAIN_MENU_TITLE;
			}
			else
			{
				lev = GEM_STONE_VALLEY;
			}

			MainRaceTrack_RequestLoad(lev);

			RECTMENU_Hide(menu);
		}
		break;
	default:
		return;
	}
}
