#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abcac-0x800ac178.
void MM_Title_MenuUpdate(void)
{
	struct GameTracker *gGT = sdata->gGT;
	u16 seenDemo;
	s16 cutsceneLev;

	// 0 - watching Crash + C-T-R letters animation
	// 1 - in the main menu
	// 2 - leaving main menu
	// 3 - coming back to main menu after exiting another menu

	// If main menu is in focus
	if (D230.titleMenuState == TITLE_MENU_STATE_IN_MENU)
	{
		// no transitioning action is needed,
		// skip to end of function
		goto END_FUNCTION;
	}

	// If you aren't in main menu

	// if not transitioning out
	if (D230.titleMenuState < TITLE_MENU_STATE_EXITING)
	{
		// If your state is less than 2, and
		// not 1, then it must be 0 by default

		// If not transitioning in
		if (D230.titleMenuState != TITLE_MENU_STATE_INTRO)
		{
			// error, just skip everything
			goto END_FUNCTION;
		}

		// assume main menu state = 0,
		// if you are transitioning in

		// if not done watching C-T-R letters
		if (D230.titleIntroFrame < TITLE_INTRO_MENU_READY_FRAME)
		{
			D230.titleMenuTransitionFrame = D230.titleMenuTransitionDurationFrames;

			// end function
			goto END_FUNCTION;
		}

		D230.menuMainMenu.state &= ~(DISABLE_INPUT_ALLOW_FUNCPTRS);
		D230.menuMainMenu.state |= EXECUTE_FUNCPTR;

		MM_TransitionInOut(D230.transitionMeta_Menu, D230.titleMenuTransitionFrame, D230.titleMenuTransitionStep);

		// If the animation ends
		if (D230.titleMenuTransitionFrame == 0)
		{
			// you are now in main menu
			D230.titleMenuState = TITLE_MENU_STATE_IN_MENU;

			// no further transitioning is needed,
			// skip to end of function
			goto END_FUNCTION;
		}

	LAB_800ac004:

		// decrease amount of time remaining in animation
		D230.titleMenuTransitionFrame -= 1;
		goto END_FUNCTION;
	}

	// If not transitioning out
	if (D230.titleMenuState != TITLE_MENU_STATE_EXITING)
	{
		// if you are not returning from another menu,
		// so either in main menu or watching C-T-R trophy animation
		if (D230.titleMenuState != TITLE_MENU_STATE_RETURNING)
		{
			// no further action is needed
			goto END_FUNCTION;
		}

		// assume D230.titleMenuState = TITLE_MENU_STATE_RETURNING
		// if you are returning from another menu
		MM_TransitionInOut(D230.transitionMeta_Menu, D230.titleMenuTransitionFrame, D230.titleMenuTransitionStep);

		// If "fade-in" animation from other menu is done
		if (D230.titleMenuTransitionFrame == 0)
		{
			// you are now in main menu
			D230.titleMenuState = TITLE_MENU_STATE_IN_MENU;

			// end the function
			goto END_FUNCTION;
		}

		// If you're transitioning from another menu,
		// and the animation is not done, loop back and
		// check again if the transition is done
		goto LAB_800ac004;
	}

	// assume D230.titleMenuState = TITLE_MENU_STATE_EXITING
	// If you are transitioning out

	MM_TransitionInOut(D230.transitionMeta_Menu, D230.titleMenuTransitionFrame, D230.titleMenuTransitionStep);

	// Increment frame timer, increase time left in "fade-in"
	// animation, which plays it in reverse, as "fade-out"
	D230.titleMenuTransitionFrame += 1;

	// If the "fade-out" animation is not over, skip "switch" statemenet
	if (D230.titleMenuTransitionFrame <= D230.titleMenuTransitionDurationFrames)
	{
		goto END_FUNCTION;
	}

	// If you are transitioning out of the menu,
	// and if the "fade-out" animation is done,
	// time to figure out where you're going next
	MM_Title_CameraReset();

	switch (D230.desiredMenuIndex)
	{
	// adventure character selection
	case MM_EXIT_ROUTE_ADV_NEW:

		MM_Title_KillThread();
		GAMEPROG_NewProfile_InsideAdv(&sdata->advProgress);

		sdata->advProfileIndex = 0xffff;

		// go to adventure character select screen
		sdata->mainMenuState = MAIN_MENU_ADVENTURE;

		MainRaceTrack_RequestLoad(ADVENTURE_GARAGE);
		break;

	// adventure save/load
	case MM_EXIT_ROUTE_ADV_LOAD:

		// Go to save/load
		sdata->ptrDesiredMenu = &data.menuFourAdvProfiles;

		SelectProfile_ToggleMode(SELECT_PROFILE_SCREEN_ADV_LOAD);
		break;

	// regular character selection screen
	case MM_EXIT_ROUTE_CHARACTER_SELECT:

		MM_Title_KillThread();

		// return to character selection
		sdata->ptrDesiredMenu = &D230.menuCharacterSelect;

		MM_Characters_RestoreIDs();
		break;

	// high score menu
	case MM_EXIT_ROUTE_HIGH_SCORE:

		MM_HighScore_Init();

		// Go to high score menu
		sdata->ptrDesiredMenu = &D230.menuHighScores;
		break;

	// demo mode
	case MM_EXIT_ROUTE_DEMO:

		MM_Title_KillThread();

		gGT->gameMode1 &= ~(BATTLE_MODE | ADVENTURE_MODE | TIME_TRIAL | ADVENTURE_ARENA | ARCADE_MODE | ADVENTURE_CUP);
		gGT->gameMode2 &= ~(CUP_ANY_KIND);

		// enable Arcade Mode
		gGT->gameMode1 |= ARCADE_MODE;

		// If you have not viewed Oxide cutscene yet
		if (gGT->boolSeenOxideIntro == 0)
		{
			gGT->boolSeenOxideIntro = 1;
			cutsceneLev = INTRO_RACE_TODAY;
		}

		// If you've already seen Oxide Cutscene
		else
		{
			// enable Demo Mode
			gGT->boolDemoMode = 1;

			// set number of players to 1
			gGT->numPlyrNextGame = 1;

			gGT->demoCountdownTimer = TITLE_DEMO_RACE_FRAMES;

			// number of times you've seen Demo Mode,
			seenDemo = sdata->demoModeIndex;

			for (s32 demoDriverIndex = 0; demoDriverIndex < TITLE_DEMO_DRIVER_COUNT; demoDriverIndex++)
			{
				data.characterIDs[demoDriverIndex] = (seenDemo + demoDriverIndex) & TITLE_DEMO_INDEX_MASK;
			}

			// get trackID from demo mode index,
			// in order of Single Race track selection
			cutsceneLev = D230.arcadeTracks[seenDemo & TITLE_DEMO_INDEX_MASK].levID;

			// increment counter
			sdata->demoModeIndex = seenDemo + 1;
		}
		goto LAB_800abfc0;

	// scrapbook
	case MM_EXIT_ROUTE_SCRAPBOOK:

		MM_Title_KillThread();

		// go to scrapbook
		sdata->mainMenuState = MAIN_MENU_SCRAPBOOK;

		cutsceneLev = SCRAPBOOK;
	LAB_800abfc0:

		// Load level
		MainRaceTrack_RequestLoad(cutsceneLev);

		// make main menu disappear
		RECTMENU_Hide(&D230.menuMainMenu);
	}

END_FUNCTION:

	// if you're entering menu for first time in
	// Crash + C-T-R animation cutscene
	if (D230.titleMenuState == TITLE_MENU_STATE_INTRO)
	{
		D230.titleCameraPos = D230.titleBaseCameraPos;
	}
	else
	{
		D230.titleCameraPos.x = D230.titleBaseCameraPos.x + D230.titleCameraXYTransition.currX;
		D230.titleCameraPos.y = D230.titleBaseCameraPos.y + D230.titleCameraXYTransition.currY;
		D230.titleCameraPos.z = D230.titleBaseCameraPos.z + D230.titleCameraZTransition.currX;
	}

	D230.menuMainMenu.posX_curr = D230.titleMainMenuPos.x + D230.titleMainMenuTransition.currX;
	D230.menuMainMenu.posY_curr = D230.titleMainMenuPos.y + D230.titleMainMenuTransition.currY;
	D230.menuAdventure.posX_curr = D230.titleAdventureMenuPos.x + D230.titleAdventureTransition.currX;
	D230.menuAdventure.posY_curr = D230.titleAdventureMenuPos.y + D230.titleAdventureTransition.currY;
	D230.menuRaceType.posX_curr = D230.titleRaceTypeMenuPos.x + D230.titleRaceTypeTransition.currX;
	D230.menuRaceType.posY_curr = D230.titleRaceTypeMenuPos.y + D230.titleRaceTypeTransition.currY;
	D230.menuPlayers1P2P.posX_curr = D230.titlePlayersMenuPos.x + D230.titlePlayersTransition.currX;
	D230.menuPlayers1P2P.posY_curr = D230.titlePlayersMenuPos.y + D230.titlePlayersTransition.currY;
	D230.menuPlayers2P3P4P.posX_curr = D230.titlePlayersMenuPos.x + D230.titlePlayersTransition.currX;
	D230.menuPlayers2P3P4P.posY_curr = D230.titlePlayersMenuPos.y + D230.titlePlayersTransition.currY;
	D230.menuDifficulty.posX_curr = D230.titleDifficultyMenuPos.x + D230.titleDifficultyTransition.currX;
	D230.menuDifficulty.posY_curr = D230.titleDifficultyMenuPos.y + D230.titleDifficultyTransition.currY;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ac94c-0x800ac9fc.
void MM_Title_KillThread(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Title *title = D230.titleObj;

	if (                     // if "title" object exists
	    (title != NULL) && ( // if you are in main menu
	                           (gGT->gameMode1 & MAIN_MENU) != 0))
	{
		// destroy title instances
		for (s32 instanceIndex = 0; instanceIndex < TITLE_INSTANCE_COUNT; instanceIndex++)
		{
			INSTANCE_Death(title->i[instanceIndex]);
		}

		title->t->flags |= THREAD_FLAG_DEAD;
		D230.titleObj = NULL;

		// CameraDC, it must be zero to follow you
		gGT->cameraDC[0].transitionTo.rot.x = 0;
		gGT->pushBuffer[0].distanceToScreen_CURR = TITLE_DEFAULT_DISTANCE_TO_SCREEN;
	}
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ac178-0x800ac1f0.
void MM_Title_SetTrophyDPP(void)
{
	struct Title *title = D230.titleObj;

	if (title == NULL)
	{
		return;
	}

	struct InstDrawPerPlayer *idpp1 = INST_GETIDPP(title->i[1]); // "title"
	struct InstDrawPerPlayer *idpp2 = INST_GETIDPP(title->i[2]); // another "title"

	u32 secondaryFlags = idpp2->instFlags;
	if ((secondaryFlags & PUSHBUFFER_EXISTS) != 0)
	{
		return;
	}

	secondaryFlags |= ~DRAW_SUCCESSFUL;
	idpp1->instFlags &= secondaryFlags;

	int otRangeNormal = idpp2->otRangeNormal;
	int otRangeSecondary = idpp2->otRangeSecondary;
	int depthOffset = CTR_ReadU32LE(&idpp2->depthOffset[0]);

	idpp1->otRangeNormal = otRangeNormal;
	idpp1->otRangeSecondary = otRangeSecondary;
	CTR_WriteU32LE(&idpp1->depthOffset[0], depthOffset);
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ac1f0-0x800ac350.
void MM_Title_CameraMove(struct Title *title, s32 frameIndex)
{
	// after frame 0xe6, make the intro models transition from the center
	// of the screen, to the left of the screen, over the course of 15 frames
	s32 result = RaceFlag_MoveModels(D230.titleIntroFrame - TITLE_INTRO_MENU_READY_FRAME, TITLE_CAMERA_MOVE_FRAMES);

	struct GameTracker *gGT = sdata->gGT;

	const struct TitleCameraPathFrame *cameraFrame = &D230.titleIntroCameraPath[frameIndex];

	for (s32 axisIndex = 0; axisIndex < 3; axisIndex++)
	{
		// position XYZ
		gGT->pushBuffer[0].pos.v[axisIndex] =
		    title->cameraPosOffset.v[axisIndex] + cameraFrame->pos.v[axisIndex] + (s16)((D230.titleCameraPos.v[axisIndex] * result) >> 0xc);

		// rotation XYZ
		gGT->pushBuffer[0].rot.v[axisIndex] = cameraFrame->rot.v[axisIndex] + (s16)((D230.titleCameraRot.v[axisIndex] * result) >> 0xc);
	}
}

static void MM_Title_RotMatrixMul(MATRIX *matrix, const SVec3 *input, VECTOR *mac)
{
	gte_SetRotMatrix(matrix);
	CTR_GteLoadSVec3V0(input);
	gte_rtv0();
	CTR_GteStoreMAC(&mac->vx);
}

static void MM_Title_UpdateTrophySpecLight(struct Instance *titleInst)
{
	struct GameTracker *gGT = sdata->gGT;
	struct PushBuffer *pb = &gGT->pushBuffer[0];
	struct InstDrawPerPlayer *idpp = INST_GETIDPP(titleInst);
	MATRIX matrix;
	SVec3 rot;
	SVec3 light;
	SVec3 view;
	VECTOR lightMac;
	VECTOR viewMac;

	rot.x = -pb->rot.x;
	rot.y = -pb->rot.y;
	rot.z = -pb->rot.z;
	ConvertRotToMatrix_Transpose(&matrix, &rot);

	light.x = 0;
	light.y = TITLE_SPEC_LIGHT_Y;
	light.z = 0;
	MM_Title_RotMatrixMul(&matrix, &light, &lightMac);

	titleInst->specLightX = (s8)lightMac.vx;
	titleInst->reflectionRGBA = (u32)lightMac.vz;

	view.x = titleInst->matrix.t[0] - pb->pos.x;
	view.y = titleInst->matrix.t[1] - pb->pos.y;
	view.z = titleInst->matrix.t[2] - pb->pos.z;
	MATH_VectorNormalize(&view);
	MM_Title_RotMatrixMul(&matrix, &view, &viewMac);

	idpp[0].halfVector.x = (s16)((u16)lightMac.vx + (u16)viewMac.vx);
	idpp[0].halfVector.y = (s16)((u16)lightMac.vy + (u16)viewMac.vy);
	idpp[0].halfVector.z = (s16)((u16)lightMac.vz + (u16)viewMac.vz);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800ac350-0x800ac6dc.
void MM_Title_ThTick(struct Thread *title)
{
	// frame counters
	s32 timer = D230.titleIntroFrame;

	// If you press Cross, Circle, Triangle, or Square
	if ((sdata->buttonTapPerPlayer[0] & TITLE_INTRO_SKIP_INPUT) != 0)
	{
		// clear gamepad input (for menus)
		RECTMENU_ClearInput();

		// set frame to 1000, skip the animation
		D230.titleIntroFrame = TITLE_INTRO_SKIP_FRAME;
	}

	// cap at 230
	if (timer > TITLE_INTRO_MENU_READY_FRAME)
	{
		timer = TITLE_INTRO_MENU_READY_FRAME;
	}

	// play queued title sounds
	for (s32 soundIndex = 0; soundIndex < TITLE_SOUND_COUNT; soundIndex++)
	{
		if (D230.titleSounds[soundIndex].frameToPlay == timer)
		{
			// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac3e8-0x800ac400 for title queued SFX.
			OtherFX_Play(D230.titleSounds[soundIndex].soundID, 1);
		}
	}

	// copy pointer to title object
	struct Title *ptrTitle = (struct Title *)title->object;

	// loop through title instances
	for (s32 instanceIndex = 0; instanceIndex < TITLE_INSTANCE_COUNT; instanceIndex++)
	{
		// current instance
		struct Instance *titleInst = ptrTitle->i[instanceIndex];

		titleInst->flags &= ~HIDE_MODEL;

		// the frame of title screen that each instance should start animation
		s16 animFram = D230.titleInstances[instanceIndex].animStartFrame;

		// set all instances to first animation
		titleInst->animIndex = 0;

		// set animation frame, based on what frame each instance should start
		titleInst->animFrame = (timer - animFram);

		// if instance has not started animation
		if (((timer - animFram) * 0x10000) < 0)
		{
			// keep instance 2 visible before its animation starts
			if (instanceIndex != 2)
			{
				titleInst->flags |= HIDE_MODEL;
			}

			// set animFrame to zero
			titleInst->animFrame = 0;
		}

		if ((D230.titleInstances[instanceIndex].isTrophy) != 0)
		{
			// if frame is anywhere in the two seconds
			// that the trophy is in the air
			if ((u32)(timer - TITLE_TROPHY_HIDE_START_FRAME) < TITLE_TROPHY_HIDE_FRAMES)
			{
				titleInst->flags |= HIDE_MODEL;
			}

			// otherwise
			else if (TITLE_TROPHY_ANIM_START_FRAME <= timer)
			{
				// play frame index, based on total animation frame
				titleInst->animFrame = timer - TITLE_TROPHY_ANIM_START_FRAME;

				// set animation to 1
				titleInst->animIndex = 1;
			}

			MM_Title_UpdateTrophySpecLight(titleInst);
		}
	}

	MM_Title_CameraMove(ptrTitle, timer);

	// increment frame counter
	timer = D230.titleIntroFrame + 1;

	if (TITLE_INTRO_END_FRAME < D230.titleIntroFrame)
	{
		// animation is over
		D230.menuMainMenu.state &= ~(DISABLE_INPUT_ALLOW_FUNCPTRS);
		D230.menuMainMenu.state |= EXECUTE_FUNCPTR;

		// dont increment index
		timer = D230.titleIntroFrame;
	}

	// write to index
	D230.titleIntroFrame = timer;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac6dc-0x800ac92c.
void MM_Title_Init(void)
{
	struct GameTracker *gGT = sdata->gGT;

	if (
	    // if "title" object is nullptr
	    (D230.titleObj == NULL) &&

	    // if you are in main menu
	    ((gGT->gameMode1 & MAIN_MENU) != 0) &&

	    // You're not in transition between menus
	    (D230.titleMenuState != TITLE_MENU_STATE_EXITING) &&

	    // model ptr (Title blue Ring)
	    (gGT->modelPtr[STATIC_RINGTOP] != 0) &&

	    // IntroCam ptr exists
	    (gGT->level1->ptrSpawnType1->count > 2))
	{
		// freecam mode
		gGT->cameraDC[0].cameraMode = CAMERA_MODE_FREECAM;

		gGT->pushBuffer[0].distanceToScreen_CURR = TITLE_INTRO_DISTANCE_TO_SCREEN;

		void **pointers = ST1_GETPOINTERS(gGT->level1->ptrSpawnType1);

		// pointer to Intro Cam, to view Crash holding Trophy in main menu
		D230.titleIntroCameraPath = pointers[ST1_CAMERA_PATH];

		struct Thread *t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Title), NONE, MEDIUM, OTHER), MM_Title_ThTick, 0, 0);

		struct Title *title = t->object;

		D230.titleObj = title;

		memset(title, 0, sizeof(*title));

		title->t = t;

		// create title instances
		for (s32 instanceIndex = 0; instanceIndex < TITLE_INSTANCE_COUNT; instanceIndex++)
		{
			struct Instance *inst = INSTANCE_Birth3D(gGT->modelPtr[D230.titleInstances[instanceIndex].modelID], 0, t);

			// store instance
			title->i[instanceIndex] = inst;

			if (D230.titleInstances[instanceIndex].isTrophy)
			{
				inst->flags |= VISIBLE_DURING_GAMEPLAY;
			}

			CTR_WriteU32LE(&inst->matrix.m[0][0], TITLE_MATRIX_SCALE);
			CTR_WriteU32LE(&inst->matrix.m[0][2], 0);
			CTR_WriteU32LE(&inst->matrix.m[1][1], TITLE_MATRIX_SCALE);
			CTR_WriteU32LE(&inst->matrix.m[2][0], 0);
			inst->matrix.m[2][2] = TITLE_MATRIX_SCALE;

			inst->matrix.t[0] = 0;
			inst->matrix.t[1] = 0;
			inst->matrix.t[2] = 0;

			inst->flags |= HIDE_MODEL;

			struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);
			for (s32 playerIndex = 1; playerIndex < gGT->numPlyrCurrGame; playerIndex++)
			{
				idpp[playerIndex].pushBuffer = 0;
			}
		}

		MM_Title_CameraMove(title, 0);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac92c-0x800ac94c.
void MM_Title_CameraReset(void)
{
	struct Title *title = D230.titleObj;

	if (title == NULL)
	{
		return;
	}

	title->cameraPosOffset.x = TITLE_CAMERA_RESET_X;
}
