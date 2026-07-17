#include <common.h>

enum UIRankConstants
{
	UI_RANK_DRIVER_COUNT = 8,
	UI_RANK_VISIBLE_ARCADE_COUNT = 4,
	UI_RANK_VISIBLE_BOSS_COUNT = 2,
	UI_RANK_DRIVER_COUNT_PLUS_ONE = 9,
	UI_RANK_TEXT_X = 0x34,
	UI_RANK_TEXT_START_Y = 0x38,
	UI_RANK_TEXT_SLOT_HEIGHT = 0x1b,
	UI_RANK_ICON_X = 0x14,
	UI_RANK_ICON_BASE_Y = 0x39,
	UI_RANK_ICON_OFFSCREEN_X = -100,
	UI_RANK_ICON_SCALE = FP(1),
	UI_RANK_TRANSITION_FRAMES = 5,
	UI_RANK_DAMAGE_TIMER_FRAMES = 30,
	UI_RANK_DAMAGE_COLOR_STEP = 4,
	UI_RANK_DAMAGE_COLOR_NEUTRAL = 0x808080,
	UI_RANK_DAMAGE_COLOR_MAX_CHANNEL = 0xff,
	UI_RANK_TRACK_DISTANCE_SCALE = 8,
	UI_RANK_TRACK_SCREEN_DIVISOR = 0x1d1,
	UI_RANK_TRACK_SMOOTH_DIVISOR = 0xe,
	UI_RANK_TRACK_SNAP_THRESHOLD = 400,
	UI_RANK_TRACK_ICON_POS_X_OFFSET = 5,
	UI_RANK_TRACK_ICON_POS_Y = 0x66,
	UI_RANK_TRACK_ICON_SCALE = 0x9d8,
	UI_RANK_TRACK_WARPBALL_ICON = 0xe,
	UI_RANK_TRACK_WARPBALL_SCALE = 0x8aa,
	UI_RANK_TRACK_WARPBALL_PROGRESS_SHIFT = 0xc,
	UI_RANK_TRACK_CHECKPOINT_COUNT_LIMIT = 0xff,
};

CTR_STATIC_ASSERT(UI_RANK_DRIVER_COUNT == 8);
CTR_STATIC_ASSERT(UI_RANK_VISIBLE_ARCADE_COUNT == 4);
CTR_STATIC_ASSERT(UI_RANK_VISIBLE_BOSS_COUNT == 2);
CTR_STATIC_ASSERT(UI_RANK_DRIVER_COUNT_PLUS_ONE == 9);
CTR_STATIC_ASSERT(UI_RANK_TEXT_X == 0x34);
CTR_STATIC_ASSERT(UI_RANK_TEXT_START_Y == 0x38);
CTR_STATIC_ASSERT(UI_RANK_TEXT_SLOT_HEIGHT == 0x1b);
CTR_STATIC_ASSERT(UI_RANK_ICON_X == 0x14);
CTR_STATIC_ASSERT(UI_RANK_ICON_BASE_Y == 0x39);
CTR_STATIC_ASSERT(UI_RANK_ICON_OFFSCREEN_X == -100);
CTR_STATIC_ASSERT(UI_RANK_ICON_SCALE == 0x1000);
CTR_STATIC_ASSERT(UI_RANK_TRANSITION_FRAMES == 5);
CTR_STATIC_ASSERT(UI_RANK_DAMAGE_TIMER_FRAMES == 30);
CTR_STATIC_ASSERT(UI_RANK_DAMAGE_COLOR_STEP == 4);
CTR_STATIC_ASSERT(UI_RANK_DAMAGE_COLOR_NEUTRAL == 0x808080);
CTR_STATIC_ASSERT(UI_RANK_DAMAGE_COLOR_MAX_CHANNEL == 0xff);
CTR_STATIC_ASSERT(UI_RANK_TRACK_DISTANCE_SCALE == 8);
CTR_STATIC_ASSERT(UI_RANK_TRACK_SCREEN_DIVISOR == 0x1d1);
CTR_STATIC_ASSERT(UI_RANK_TRACK_SMOOTH_DIVISOR == 0xe);
CTR_STATIC_ASSERT(UI_RANK_TRACK_SNAP_THRESHOLD == 400);
CTR_STATIC_ASSERT(UI_RANK_TRACK_ICON_POS_X_OFFSET == 5);
CTR_STATIC_ASSERT(UI_RANK_TRACK_ICON_POS_Y == 0x66);
CTR_STATIC_ASSERT(UI_RANK_TRACK_ICON_SCALE == 0x9d8);
CTR_STATIC_ASSERT(UI_RANK_TRACK_WARPBALL_ICON == 0xe);
CTR_STATIC_ASSERT(UI_RANK_TRACK_WARPBALL_SCALE == 0x8aa);
CTR_STATIC_ASSERT(UI_RANK_TRACK_WARPBALL_PROGRESS_SHIFT == 0xc);
CTR_STATIC_ASSERT(UI_RANK_TRACK_CHECKPOINT_COUNT_LIMIT == 0xff);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800524c4-0x80052f98.
// UI_Rank_GetDamageColor factors the duplicated inline color-timer logic.

// Draw player icons on left side of screen
// in Arcade mode and Boss mode, and draws
// icons in multiplayer on the midY axis (and warpball)

internal int UI_Rank_GetDamageColor(int driverIndex)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *d = gGT->drivers[driverIndex];

	int damageTimer = d->damageColorTimer;

	// make icon white
	int color = UI_RANK_DAMAGE_COLOR_NEUTRAL;

	// -30 to -1
	if (damageTimer < 0)
	{
		color = 0;
		int strength = (damageTimer + UI_RANK_DAMAGE_TIMER_FRAMES) * UI_RANK_DAMAGE_COLOR_STEP;
		color += strength;
		color += (UI_RANK_DAMAGE_COLOR_MAX_CHANNEL - strength) * 0x100;
		color += strength * 0x10000;

		// one frame closer to zero
		d->damageColorTimer += 1;
	}

	// 30 to 1
	if (damageTimer > 0)
	{
		color = 0;
		int strength = (UI_RANK_DAMAGE_TIMER_FRAMES - damageTimer) * UI_RANK_DAMAGE_COLOR_STEP;
		color += (UI_RANK_DAMAGE_COLOR_MAX_CHANNEL - strength);
		color += strength * 0x100;
		color += strength * 0x10000;

		// one frame closer to zero
		d->damageColorTimer -= 1;
	}

	return color;
}

void UI_DrawRankedDrivers(void)
{
	int driverIndex;
	u32 damageColor;

	struct GameTracker *gGT = sdata->gGT;
	int numPlyr = gGT->numPlyrCurrGame;

	if (numPlyr == 1)
	{
		// Number of racers that have finished race
		s16 numRacersFinished = 0;

		// incremented when looping through player structures

		for (driverIndex = 0; driverIndex < UI_RANK_DRIVER_COUNT; driverIndex++)
		{
			if (data.rankIconsTransitionTimer[driverIndex] == 0)
			{
				// player structure + 0x482 is your rank in the race
				// 0 = 1st place, 1 = 2nd place, 2 = 3rd place, etc
#ifdef CTR_NATIVE
				// NOTE(aalhendi): Boss races only allocate P1 plus the boss.
				// Retail reads driverRank before its later null check; if the
				// slot is empty, PS1 low RAM is still readable, but native
				// must skip it before dereferencing a null driver pointer.
				if (gGT->drivers[driverIndex] != NULL)
				{
					data.rankIconsDesired[driverIndex] = gGT->drivers[driverIndex]->driverRank;
				}
#else
				data.rankIconsDesired[driverIndex] = gGT->drivers[driverIndex]->driverRank;
#endif
			}

			// if player structure pointer is not nullptr
			if ((gGT->drivers[driverIndex] != 0) &&

			    ((gGT->drivers[driverIndex]->actionsFlagSet & ACTION_RACE_FINISHED) != 0))
			{
				// count how many racers have finished
				numRacersFinished++;
			}
		}

		// Default for Arcade: Show 4 racers
		int visibleRankCount = UI_RANK_VISIBLE_ARCADE_COUNT;

		if (IS_BOSS_RACE(gGT->gameMode1))
		{
			// Show 2 racers
			visibleRankCount = UI_RANK_VISIBLE_BOSS_COUNT;
		}

		// height to draw rank (this bitshifts later)
		int rankTextY = UI_RANK_TEXT_START_Y << 16;

		for (int rankLineIndex = 0; rankLineIndex < visibleRankCount; rankLineIndex++)
		{
			// make the text white by default
			s16 txtColor = WHITE;

			// if racer has finished the race
			if (rankLineIndex < numRacersFinished)
			{
				// make the text red
				txtColor = RED;
			}

			// draw rank number: '1', '2', '3', '4'
			sdata->s_spacebar[0] = (char)rankLineIndex + '1';
			DecalFont_DrawLine(&sdata->s_spacebar[0], UI_RANK_TEXT_X, rankTextY >> 0x10, FONT_SMALL, txtColor);

			// add to Y, which mekes it lower on screen
			rankTextY = rankTextY + (UI_RANK_TEXT_SLOT_HEIGHT << 16);
		}

		for (driverIndex = 0; driverIndex < UI_RANK_DRIVER_COUNT; driverIndex++)
		{
			s16 *curr = &data.rankIconsCurr[driverIndex];
			s16 *des = &data.rankIconsDesired[driverIndex];

			if (
			    // if player structure pointer is not nullptr
			    (gGT->drivers[driverIndex] != 0) &&

			    // if you haven't gotten to the last driver
			    ((*des + 1) < UI_RANK_DRIVER_COUNT_PLUS_ONE))
			{
				damageColor = UI_Rank_GetDamageColor(driverIndex);
				s16 *transitionTimer = &data.rankIconsTransitionTimer[driverIndex];

				// placeholder
				Point pos;
				pos.x = UI_RANK_ICON_OFFSCREEN_X;

				// icon not transitioning
				if (*transitionTimer == 0)
				{
					// get absolute pos-rank of driver
					int desiredRank = *des;

					// if current == desired
					if (desiredRank == *curr)
					{
						// if top positions

						if (desiredRank < UI_RANK_VISIBLE_ARCADE_COUNT)
						{
							pos.x = UI_RANK_ICON_X;
							pos.y = desiredRank * UI_RANK_TEXT_SLOT_HEIGHT + UI_RANK_ICON_BASE_Y;
						}
						else
						{
							continue;
						}
					}
				}

				SVec2 iconPos = {.x = pos.x, .y = pos.y};
				s16 iconScale = UI_RANK_ICON_SCALE;

				int isTransitioning = (pos.x == UI_RANK_ICON_OFFSCREEN_X);

				// === Icon Transitioning ===
				if (isTransitioning)
				{
					UI_Lerp2D_Angular(&iconPos, *curr, *des, *transitionTimer);
					pos.x = iconPos.x;
					pos.y = iconPos.y;
				}

				UI_DrawDriverIcon(

				    gGT->ptrIcons[data.MetaDataCharacters[data.characterIDs[driverIndex]].iconID],

				    pos.x, pos.y, &gGT->backBuffer->primMem,

				    gGT->pushBuffer_UI.ptrOT,

				    1, iconScale, damageColor);

				if (isTransitioning)
				{
					transitionTimer[0]++;

					if (*transitionTimer >= UI_RANK_TRANSITION_FRAMES)
					{
						*transitionTimer = 0;
						*curr = *des;
					}
				}
			}
		}
	}

	// if this is multiplayer
	else
	{
		s16 *trackIconX = &data.rankIconsTransitionTimer[0];
		int totalNumDrivers = numPlyr + gGT->numBotsNextGame;

		for (driverIndex = 0; driverIndex < totalNumDrivers; driverIndex++)
		{
			damageColor = UI_Rank_GetDamageColor(driverIndex);

			u16 currentTrackX = *trackIconX;
			u16 targetTrackX;

			if (
			    // if racer is in first lap and
			    (gGT->drivers[driverIndex]->lapIndex == 0) &&

			    // racer crossed the startline backwards
			    // this is when race starts and you're behind the finish line
			    ((gGT->drivers[driverIndex]->actionsFlagSet & ACTION_BEHIND_START_LINE) != 0))
			{
			TrackIconAtStart:
				// icon posX is zero,
				// dont go to end of lap on the graph
				targetTrackX = 0;
			}
			else
			{
				// length of track
				int trackLength = gGT->level1->ptr_restart_points[0].distToFinish * UI_RANK_TRACK_DISTANCE_SCALE;
				int driverProgress = trackLength - gGT->drivers[driverIndex]->distanceToFinish_curr;
				int trackScreenUnit = trackLength / UI_RANK_TRACK_SCREEN_DIVISOR;

				// divide distanceToFinish by screen width
				targetTrackX = (u16)(driverProgress / trackScreenUnit);

				if ((s16)targetTrackX < 0)
				{
					goto TrackIconAtStart;
				}
			}

			// posX
			int targetTrackXInt = targetTrackX;
			int delta = targetTrackXInt - (s16)currentTrackX;

			if (delta < 0)
			{
				delta = -delta;
			}
			u32 step = delta / UI_RANK_TRACK_SMOOTH_DIVISOR;
			if ((step & 0xffff) == 0)
			{
				step = 1;
			}
			int nextTrackXInt = currentTrackX + step;
			u16 nextTrackX = (u16)nextTrackXInt;
			b32 passedTarget;
			if ((s16)currentTrackX < targetTrackXInt)
			{
				passedTarget = targetTrackXInt < (s16)nextTrackXInt;
			}
			else
			{
				nextTrackXInt = currentTrackX - step;
				nextTrackX = (u16)nextTrackXInt;
				passedTarget = (s16)nextTrackXInt < targetTrackXInt;
			}
			if (passedTarget)
			{
				nextTrackX = targetTrackX;
			}
			int snapDelta = (int)targetTrackX - (int)nextTrackX;
			if (snapDelta < 0)
			{
				snapDelta = -snapDelta;
			}

			if (UI_RANK_TRACK_SNAP_THRESHOLD < snapDelta)
			{
				nextTrackX = targetTrackX;
			}

			int posX = nextTrackX + UI_RANK_TRACK_ICON_POS_X_OFFSET;
			int posY = UI_RANK_TRACK_ICON_POS_Y;

			DecalHUD_DrawPolyGT4(gGT->ptrIcons[data.MetaDataCharacters[data.characterIDs[driverIndex]].iconID], posX, posY,

			                     // pointer to PrimMem struct
			                     &gGT->backBuffer->primMem,

			                     // pointer to OT memory
			                     gGT->pushBuffer_UI.ptrOT,

			                     // color data
			                     damageColor, damageColor, damageColor, damageColor,

			                     TRANS_50_DECAL, UI_RANK_TRACK_ICON_SCALE);

			*trackIconX = nextTrackX;
			trackIconX = trackIconX + 1;
		}

		for (struct Thread *warpballThread = gGT->threadBuckets[TRACKING].thread; warpballThread != 0; warpballThread = warpballThread->siblingThread)
		{
			// Get Instance from Thread
			struct Instance *warpballInst = warpballThread->inst;

			// if not warpball, skip
			if (warpballInst->model->id != DYNAMIC_WARPBALL)
			{
				continue;
			}

			// pointer to path data
			struct CheckpointNode *cn = gGT->level1->ptr_restart_points;

			struct TrackerWeapon *tw = warpballInst->thread->object;

			int checkpointIndex = ((intptr_t)tw->ptrNodeCurr - (intptr_t)cn) / (s32)sizeof(struct CheckpointNode);

			if (((u32)(gGT->level1->cnt_restart_points - 1) >= UI_RANK_TRACK_CHECKPOINT_COUNT_LIMIT) || (checkpointIndex < 0))
			{
				continue;
			}

			const s32 *warpballPos = &warpballInst->matrix.t[0];

			struct CheckpointNode *cn0 = &cn[checkpointIndex];
			struct CheckpointNode *cn1 = &cn[cn0->nextIndex_forward];
			struct CheckpointNode *cn2 = &cn[cn1->nextIndex_forward];

			SVec3 trackDir = {
			    .x = cn1->pos.x - cn2->pos.x,
			    .y = cn1->pos.y - cn2->pos.y,
			    .z = cn1->pos.z - cn2->pos.z,
			};
			MATH_VectorNormalize(&trackDir);

			SVec3 warpDelta = {
			    .x = warpballPos[0] - cn1->pos.x,
			    .y = warpballPos[1] - cn1->pos.y,
			    .z = warpballPos[2] - cn1->pos.z,
			};

			CTR_GteLoadRotRow0SVec3(&trackDir);
			CTR_GteLoadSVec3V0(&warpDelta);
			gte_mvmva(0, 0, 0, 3, 0);
			int projectedDistance = CTR_GteReadMAC1();

			int trackProgress = cn1->distToFinish * UI_RANK_TRACK_DISTANCE_SCALE + (projectedDistance >> UI_RANK_TRACK_WARPBALL_PROGRESS_SHIFT);
			int trackLength = gGT->level1->ptr_restart_points[0].distToFinish * UI_RANK_TRACK_DISTANCE_SCALE;
			int wrappedProgress = trackProgress % trackLength;
			if (wrappedProgress == 0)
			{
				continue;
			}

#if 0
			if (trackLength == 0) trap(0x1c00);
			if ((trackLength == -1) && (trackProgress == -0x80000000)) trap(0x1800);
#endif

			int remainingProgress = trackLength - wrappedProgress;
			int trackScreenUnit = trackLength / UI_RANK_TRACK_SCREEN_DIVISOR;

#if 0
			if (trackScreenUnit == 0) trap(0x1c00);
			if ((trackScreenUnit == -1) && (remainingProgress == -0x80000000)) trap(0x1800);
#endif

			int posX = (remainingProgress / trackScreenUnit) + UI_RANK_TRACK_ICON_POS_X_OFFSET;
			int posY = UI_RANK_TRACK_ICON_POS_Y;

			DecalHUD_DrawWeapon(
			    // warpball icon
			    gGT->ptrIcons[UI_RANK_TRACK_WARPBALL_ICON], posX, posY,

			    // pointer to PrimMem struct
			    &gGT->backBuffer->primMem,

			    // pointer to OT memory
			    gGT->pushBuffer_UI.ptrOT,

			    TRANS_50_DECAL, UI_RANK_TRACK_WARPBALL_SCALE, 1);
		}
	}
}
