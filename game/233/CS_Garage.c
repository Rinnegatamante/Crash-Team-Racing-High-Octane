#include <common.h>

enum GarageInputMask
{
	GARAGE_INPUT_HORIZONTAL = BTN_LEFT | BTN_RIGHT,
	GARAGE_INPUT_MENU = BTN_TRIANGLE | BTN_CROSS_one | BTN_CIRCLE | BTN_SQUARE_one,
	GARAGE_INPUT_CONFIRM = BTN_CROSS_one | BTN_CIRCLE,
	GARAGE_INPUT_BACK = BTN_TRIANGLE | BTN_SQUARE_one,
};

enum
{
	GARAGE_STAT_BAR_RATE = 3,
	GARAGE_STAT_BAR_SEGMENT_COUNT = 6,
	GARAGE_STAT_BAR_SEGMENT_WIDTH = 13,
	GARAGE_STAT_BAR_ROW_STEP = 15,
	GARAGE_STAT_BAR_POLY_G4_CODE = 0x38,
	GARAGE_STAT_BAR_POLY_G4_COLOR_CODE = GARAGE_STAT_BAR_POLY_G4_CODE << 24,
	GARAGE_STAT_NAME_SPEED_Y = 30,
	GARAGE_STAT_NAME_ACCEL_Y = 0x2d,
	GARAGE_STAT_NAME_TURN_Y = 60,
	GARAGE_CLASS_NAME_Y = 15,
	GARAGE_STAT_BAR_START_Y = 33,
	GARAGE_STAT_BAR_END_Y = 40,
	GARAGE_STAT_BAR_SHADOW_Y = 34,
	GARAGE_STAT_BAR_HEIGHT = 7,
	GARAGE_STAT_BAR_SHADOW_HEIGHT = 5,
	GARAGE_STAT_BOX_Y = 11,
	GARAGE_STAT_BOX_HEIGHT = 68,
	GARAGE_STAT_BOX_PADDING_X = 6,
	GARAGE_OSK_DELAY_FRAMES = 60,
	GARAGE_CAMERA_FRAME_STEP = 30,
	GARAGE_CAMERA_WRAP_RIGHT_FRAME = 240,
	GARAGE_CAMERA_WRAP_LEFT_FRAME = 210,
	GARAGE_CHARACTER_NAME_X = 0x100,
	GARAGE_CHARACTER_NAME_Y = 0xb4,
	GARAGE_CHARACTER_ARROW_Y = 187,
	GARAGE_CHARACTER_ARROW_LEFT_BASE_X = 236,
	GARAGE_CHARACTER_ARROW_RIGHT_BASE_X = 274,
	GARAGE_CHARACTER_ARROW_ICON_INDEX = 0x38,
	GARAGE_CHARACTER_ARROW_SCALE = 0x1000,
	GARAGE_CHARACTER_ARROW_ROT_LEFT = 0x800,
};

CTR_STATIC_ASSERT(GARAGE_STAT_BAR_POLY_G4_COLOR_CODE == 0x38000000);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b7784-0x800b7834
void CS_Garage_ZoomOut(s16 zoomState)
{
	if (zoomState != 0)
	{
		// number of frames to zoom in, or out,
		// when selecting or cancelling OSK
		gGarage.numFramesCurr_ZoomIn = gGarage.numFramesMax_Zoom;
		gGarage.numFramesCurr_ZoomOut = gGarage.numFramesMax_Zoom;
	}
	else
	{
		gGarage.numFramesCurr_ZoomIn = 0;
		gGarage.numFramesCurr_ZoomOut = 0;
	}

	gGarage.numFramesCurr_GarageMove = 0;
	gGarage.boolSelected = 0;
	gGarage.delayOneSecond = 0;

	sdata->gGT->gameMode2 &= ~(GARAGE_OSK);

	// if just entered garage
	if (zoomState == 0)
	{
		Garage_Init();
		Garage_Enter(sdata->advCharSelectIndex_curr);

		Audio_SetState_Safe(AUDIO_GARAGE);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b7834-0x800b854c
void CS_Garage_MenuProc(struct RectMenu *menu)
{
	(void)menu;
	s16 garageFrames;
	s16 *statBarLength;
	u16 classNamePosX;
	u32 statNamePosX;
	u32 statBarPosX;

	u32 currSelectIndex = sdata->advCharSelectIndex_curr;
	struct GameTracker *gGT = sdata->gGT;
	struct PrimMem *primMem = &gGT->backBuffer->primMem;
	s16 currCharacterID = gGarage.garageCharacterIDs[currSelectIndex];
	struct MetaDataCHAR *MDC = &data.MetaDataCharacters[currCharacterID];
	int nameIndex = MDC->name_LNG_long;
	RECT r;
	Color white = MakeColor(0xFF, 0xFF, 0xFF);
	Color black = MakeColor(0, 0, 0);

	// CameraDC, freecam mode
	gGT->cameraDC[0].cameraMode = CAMERA_MODE_FREECAM;


	// subtract transition timer by one frame
	garageFrames = gGarage.numFramesCurr_GarageMove - 1;

	// if mid-transition, skip some code
	if (gGarage.numFramesCurr_GarageMove != 0)
	{
		goto update_garage_camera;
	}

	// At this point, there must not be a transition
	// between drivers, so start drawing the UI

	// count frames in garage?
	gGarage.unusedFrameCount++;

	// animate growth of all three stat bars
	for (int i = 0; i < 3; i++)
	{
		statBarLength = &gGarage.statBarLengths[i];
		s16 stat = gGarage.statBarTargetLengths[MDC->engineID * 3 + i];

		if (*statBarLength < stat)
		{
			*statBarLength = *statBarLength + GARAGE_STAT_BAR_RATE;
		}
		if (stat < *statBarLength)
		{
			*statBarLength = stat;
		}
	}

	if ((nameIndex == LNG_TINY_TIGER) ||

	    (statNamePosX = 383,

	     nameIndex == LNG_PURA_FULL))
	{
		statNamePosX = 129;
		classNamePosX = 128;
		statBarPosX = 139;
	}
	else
	{
		classNamePosX = 384;
		statBarPosX = 393;
	}

	DecalFont_DrawLine(sdata->lngStrings[LNG_SPEED], statNamePosX, GARAGE_STAT_NAME_SPEED_Y, FONT_BIG, JUSTIFY_RIGHT | ORANGE_RED);
	DecalFont_DrawLine(sdata->lngStrings[LNG_ACCEL], statNamePosX, GARAGE_STAT_NAME_ACCEL_Y, FONT_BIG, JUSTIFY_RIGHT | LIME_GREEN);
	DecalFont_DrawLine(sdata->lngStrings[LNG_TURN], statNamePosX, GARAGE_STAT_NAME_TURN_Y, FONT_BIG, JUSTIFY_RIGHT | BLUE);

	int engineID = MDC->engineID;

	// 0x248 - Beginner
	// EngineID == 3
	int classStringIndex = 0;

	// 0x24A - Advanced
	if (engineID == SPEED)
	{
		classStringIndex = 2;
	}

	// 0x249 - Intermediate
	if (engineID < SPEED)
	{
		classStringIndex = 1;
	}

	// 7 pixels tall
	u16 statBarStart_Y = GARAGE_STAT_BAR_START_Y;
	u16 statBarEnd_Y = GARAGE_STAT_BAR_END_Y;

	u16 statBarShadows_Y = GARAGE_STAT_BAR_SHADOW_Y;

	// Draw class name
	DecalFont_DrawLine(sdata->lngStrings[gGarage.classStringIDs[classStringIndex]], classNamePosX, GARAGE_CLASS_NAME_Y, FONT_BIG, (JUSTIFY_CENTER | ORANGE));

	// bar length (animated)

	for (int i = 0; i < 3; i++)
	{
		statBarLength = &gGarage.statBarLengths[i];

		// bar outline
		r.x = statBarPosX;
		r.y = statBarStart_Y;
		r.w = *statBarLength;
		r.h = GARAGE_STAT_BAR_HEIGHT;

		// outline color white at 0x800b7780
		CTR_Box_DrawWireBox(&r, &white, gGT->pushBuffer_UI.ptrOT, primMem);

		// bar shadows
		r.x = statBarPosX + 1;
		r.y = statBarShadows_Y;
		r.w = *statBarLength - 2;
		r.h = GARAGE_STAT_BAR_SHADOW_HEIGHT;

		// outline color black (shadows)
		CTR_Box_DrawWireBox(&r, &black, gGT->pushBuffer_UI.ptrOT, primMem);

		int segmentLen = GARAGE_STAT_BAR_SEGMENT_WIDTH;
		int segmentStart = 0;
		int segmentEnd = segmentLen;

		for (int segmentIndex = 0; segmentIndex < GARAGE_STAT_BAR_SEGMENT_COUNT; segmentIndex++)
		{
			// color data of bars (blue green yellow red)
			u32 *barColor = &gGarage.statBarSegmentColors[segmentIndex];
			s16 currSegmentLen = (s16)segmentLen;

			if (*statBarLength <= segmentEnd)
			{
				currSegmentLen = *statBarLength - segmentStart;
			}

			if ((int)currSegmentLen << 0x10 < 0)
			{
				currSegmentLen = 0;
			}

			if (segmentStart + currSegmentLen <= *statBarLength)
			{
				// primMem curr
				POLY_G4 *p = primMem->cursor;

				// quit if prim mem runs out
				if (primMem->end < (void *)p)
				{
					return;
				}

				primMem->cursor = p + 1;

				// color data
				CtrGpu_WriteColorCode(&p->r0, barColor[0] | GARAGE_STAT_BAR_POLY_G4_COLOR_CODE);
				CtrGpu_WriteColorCode(&p->r1, barColor[1] | GARAGE_STAT_BAR_POLY_G4_COLOR_CODE);
				CtrGpu_WriteColorCode(&p->r2, barColor[0] | GARAGE_STAT_BAR_POLY_G4_COLOR_CODE);
				CtrGpu_WriteColorCode(&p->r3, barColor[1] | GARAGE_STAT_BAR_POLY_G4_COLOR_CODE);

				s16 segmentX = statBarPosX + segmentStart;

				// top left
				p->x0 = segmentX;
				p->y0 = statBarStart_Y;

				// top right
				p->x1 = segmentX + currSegmentLen;
				p->y1 = statBarStart_Y;

				// bottom left
				p->x2 = segmentX;
				p->y2 = statBarEnd_Y;

				// bottom right
				p->x3 = segmentX + currSegmentLen;
				p->y3 = statBarEnd_Y;

				addPolyG4(gGT->pushBuffer_UI.ptrOT, p);
			}

			segmentStart += segmentLen;
			segmentEnd += segmentLen;
		}

		statBarStart_Y += GARAGE_STAT_BAR_ROW_STEP;
		statBarEnd_Y += GARAGE_STAT_BAR_ROW_STEP;
		statBarShadows_Y += GARAGE_STAT_BAR_ROW_STEP;
	}

	s16 classMaxLen = DecalFont_GetLineWidth(sdata->lngStrings[LNG_INTERMEDIATE], FONT_BIG);

	// Stats box
	r.x = (classNamePosX - (classMaxLen >> 1)) - GARAGE_STAT_BOX_PADDING_X;
	r.y = GARAGE_STAT_BOX_Y;
	r.w = classMaxLen + GARAGE_STAT_BOX_PADDING_X * 2;
	r.h = GARAGE_STAT_BOX_HEIGHT;

	// Draw 2D Menu rectangle background
	RECTMENU_DrawInnerRect(&r, 4, gGT->backBuffer->otMem.uiOT);

	char *name = sdata->lngStrings[nameIndex];

	// Draw character name
	DecalFont_DrawLine(name, GARAGE_CHARACTER_NAME_X, GARAGE_CHARACTER_NAME_Y, FONT_BIG, 0xffff8000);

	int arrowColor = ORANGE;

	// blink arrows
	if ((sdata->frameCounter & 4) == 0)
	{
		arrowColor = RED;
	}

	// Color data
	u32 *arrowColors = data.ptrColor[(s32)arrowColor];

	int nameLen = DecalFont_GetLineWidth(name, FONT_BIG) >> 1;

	int arrowPos[2] = {GARAGE_CHARACTER_ARROW_LEFT_BASE_X - nameLen, nameLen + GARAGE_CHARACTER_ARROW_RIGHT_BASE_X};
	int arrowRot[2] = {GARAGE_CHARACTER_ARROW_ROT_LEFT, 0};

	struct Icon **iconPtrArray = ICONGROUP_GETICONS(gGT->iconGroup[4]);

	for (int i = 0; i < 2; i++)
	{
		DecalHUD_Arrow2D(iconPtrArray[GARAGE_CHARACTER_ARROW_ICON_INDEX], arrowPos[i], GARAGE_CHARACTER_ARROW_Y,

		                 primMem, gGT->pushBuffer_UI.ptrOT,

		                 arrowColors[0], arrowColors[1], arrowColors[2], arrowColors[3],

		                 0, GARAGE_CHARACTER_ARROW_SCALE, arrowRot[i]);
	}

	garageFrames = gGarage.numFramesCurr_GarageMove;

	if (((gGT->renderFlags & RENDER_FLAG_CHECKERED_FLAG) != 0) ||
	    (((sdata->AnyPlayerTap & GARAGE_INPUT_MENU) == 0) && ((sdata->AnyPlayerHold & GARAGE_INPUT_HORIZONTAL) == 0)))
	{
		goto update_garage_camera;
	}

	// If you dont press D-pad
	if ((sdata->AnyPlayerHold & GARAGE_INPUT_HORIZONTAL) == 0)
	{
		// If you do not press Cross or Circle
		if ((sdata->AnyPlayerTap & GARAGE_INPUT_CONFIRM) == 0)
		{
			// If you press Triangle or Square
			if ((sdata->AnyPlayerTap & GARAGE_INPUT_BACK) != 0)
			{
				// Play Sound
				OtherFX_Play(2, 1);

				garageFrames = gGarage.numFramesCurr_ZoomIn;
				if (gGarage.boolSelected == 1)
				{
					gGarage.boolSelected = 0;
					gGT->gameMode2 &= ~GARAGE_OSK;

					if (garageFrames < gGarage.numFramesMax_Zoom)
					{
						gGarage.numFramesCurr_ZoomOut = gGarage.numFramesMax_Zoom - garageFrames;
					}
				}
				else
				{
					// return to main menu
					sdata->mainMenuState = MAIN_MENU_TITLE;

					Garage_Leave();

					// load main menu LEV
					MainRaceTrack_RequestLoad(MAIN_MENU_LEVEL);
				}
			}
		}

		// If you press Cross or Circle
		else
		{
			// "Have you selected character?"
			// If true, it will show an animation, and then show the
			// OSK (keyboard) screen. If set to 0 after in that screen,
			// the screen does not disappear

			// if false
			if (gGarage.boolSelected == 0)
			{
				// make it true
				gGarage.boolSelected = 1;
			}

			// if true
			else
			{
				// if pressed X twice quickly
				if (gGarage.boolSelected == 1)
				{
					// set desiredMenu to OSK (on-screen keyboard)
					sdata->ptrDesiredMenu = &data.menuSubmitName;

					data.characterIDs[0] = gGarage.garageCharacterIDs[currSelectIndex];
					sdata->advProgress.characterID = data.characterIDs[0];

					SubmitName_RestoreName(0);
					OtherFX_Play(1, 1);
				}
			}
		}
	}

	// if using D-pad
	else
	{
		// erase animated bars
		for (int i = 2; i > -1; i--)
		{
			statBarLength = &gGarage.statBarLengths[i];
			*statBarLength = 0;
		}
		// Play Sound
		OtherFX_Play(0, 1);

		// If you dont press Left
		if ((sdata->AnyPlayerHold & BTN_LEFT) == 0)
		{
			// If you dont press Right
			if ((sdata->AnyPlayerHold & BTN_RIGHT) != 0)
			{
				currSelectIndex++;
				goto LAB_800b8084;
			}
		}

		// If you press Left
		else
		{
			currSelectIndex--;

		LAB_800b8084:

			// previous equals current
			sdata->advCharSelectIndex_prev = sdata->advCharSelectIndex_curr;

			// clamp 0-7
			currSelectIndex &= 7;
			sdata->advCharSelectIndex_curr = currSelectIndex;

			Garage_MoveLR(currSelectIndex);
		}

		// reset frame counter to max number of frames
		gGarage.numFramesCurr_GarageMove = gGarage.numFramesMax_GarageMove;

		if (gGarage.numFramesCurr_ZoomIn < gGarage.numFramesMax_Zoom)
		{
			gGarage.numFramesCurr_ZoomOut = gGarage.numFramesMax_Zoom - gGarage.numFramesCurr_ZoomIn;
		}

		gGarage.boolSelected = 0;
		gGT->gameMode2 &= ~GARAGE_OSK;
	}

	// clear gamepad input (for menus)
	RECTMENU_ClearInput();

	garageFrames = gGarage.numFramesCurr_GarageMove;
update_garage_camera:
	gGarage.numFramesCurr_GarageMove = garageFrames;

	// if frames remaing for zoom camera
	if (0 < gGarage.numFramesCurr_ZoomIn)
	{
		// decrease zoom frame timer
		gGarage.numFramesCurr_ZoomIn--;
	}

	// if pressed X once, and waited for countdown clock
	if ((gGarage.boolSelected == 1) && (gGarage.numFramesCurr_ZoomIn == 0))
	{
		if (
		    // frames remaining for animation
		    (GARAGE_OSK_DELAY_FRAMES <= gGarage.delayOneSecond) || ((gGT->gameMode2 & GARAGE_OSK) != 0))
		{
			// set desiredMenu to OSK (on-screen keyboard)
			sdata->ptrDesiredMenu = &data.menuSubmitName;

			data.characterIDs[0] = gGarage.garageCharacterIDs[currSelectIndex];
			sdata->advProgress.characterID = data.characterIDs[0];

			SubmitName_RestoreName(0);
			OtherFX_Play(1, 1);
		}
		else
		{
			gGarage.delayOneSecond++;
		}
	}

#ifdef CTR_NATIVE
	if (sdata->ptrDesiredMenu == &data.menuSubmitName)
	{
		// NOTE(aalhendi): PC-only keyboard shim; retail gamepad flow above stays unchanged.
		// flush async key state buffer. If not, tapping Enter "before" picking a garage character,
		//  then picking character, will immediately warp you to the adv hub, with no time to type the name
		NikoGetEnterKey();
	}
#endif

	if (gGarage.boolSelected == 0)
	{
		gGarage.numFramesCurr_ZoomIn = gGarage.numFramesMax_Zoom;
	}

	if (gGarage.numFramesCurr_ZoomOut != 0)
	{
		gGarage.numFramesCurr_ZoomOut--;
	}

	u32 prevSelectIndex = sdata->advCharSelectIndex_prev;

	// Pura->Crash
	if ((currSelectIndex == 0) && (prevSelectIndex == 7))
	{
		garageFrames = GARAGE_CAMERA_WRAP_RIGHT_FRAME - gGarage.numFramesCurr_GarageMove;
	}
	// Crash->Pura
	else if ((currSelectIndex == 7) && (prevSelectIndex == 0))
	{
		garageFrames = gGarage.numFramesCurr_GarageMove + GARAGE_CAMERA_WRAP_LEFT_FRAME;
	}
	// Move Right
	else if (prevSelectIndex < currSelectIndex)
	{
		garageFrames = currSelectIndex * GARAGE_CAMERA_FRAME_STEP - gGarage.numFramesCurr_GarageMove;
	}
	// Move Left
	else
	{
		garageFrames = currSelectIndex * GARAGE_CAMERA_FRAME_STEP + gGarage.numFramesCurr_GarageMove;
	}

	// animation frame index,
	// pointer to position,
	// pointer to rotation

	s16 pathFlags;
	SVec3 camPos;
	SVec3 camRot;
	CAM_Path_Move((int)garageFrames, camPos.v, camRot.v, &pathFlags);

	// set position and rotation to pushBuffer
	gGT->pushBuffer[0].pos = camPos;

	gGT->pushBuffer[0].rot = camRot;

	int zoom = gGarage.numFramesCurr_ZoomOut;
	if (zoom == 0)
	{
		zoom = (gGarage.numFramesMax_Zoom - gGarage.numFramesCurr_ZoomIn) * (gGarage.fovMax - gGarage.fovMin);
	}
	else
	{
		zoom = zoom * (gGarage.fovMax - gGarage.fovMin);
	}

	zoom = gGarage.fovMin + zoom / gGarage.numFramesMax_Zoom;

	gGT->pushBuffer[0].distanceToScreen_CURR = zoom;
	gGT->pushBuffer[0].distanceToScreen_PREV = zoom;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b854c-0x800b8558
struct RectMenu *CS_Garage_GetMenuPtr(void)
{
	return &gGarage.menuGarage;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b8558-0x800b8598
void CS_Garage_Init(void)
{
	// go to 3D character selection
	sdata->ptrActiveMenu = &gGarage.menuGarage;

	gGarage.menuGarage.state &= ~(ONLY_DRAW_TITLE);

	// 0 = just entered garage
	CS_Garage_ZoomOut(0);
}
