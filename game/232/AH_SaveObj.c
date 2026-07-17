#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af3a4-0x800af3e4.
void AH_SaveObj_ThDestroy(struct Thread *t)
{
	struct SaveObj *save = t->object;

	if (save->inst != NULL)
	{
		INSTANCE_Death(save->inst);
		save->inst = NULL;
	}
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af3e4-0x800af7f0.
void AH_SaveObj_ThTick(struct Thread *t)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];
	struct Instance *saveInst = t->inst;
	struct Instance *driverInst = driver->instSelf;

	// Get difference in positions between instances
	int distX = saveInst->matrix.t[0] - driverInst->matrix.t[0];
	int distY = saveInst->matrix.t[1] - driverInst->matrix.t[1];
	int distZ = saveInst->matrix.t[2] - driverInst->matrix.t[2];

	struct SaveObj *save = t->object;

	// get distance from player instance and thread object instance
	int dist = distX * distX + distY * distY + distZ * distZ;

	if ((save->flags & AH_SAVEOBJ_FLAG_INTERACTION_ACTIVE) == 0)
	{
		if ( // If game is not paused
		    ((gGT->gameMode1 & PAUSE_ALL) != 0) ||

		    // If driver is too far away from screen
		    (AH_SAVEOBJ_INTERACTION_DIST_SQ < dist))
		{
			// dont trigger menu, but play
			// sound of the load/save screen
			goto LAB_800af72c;
		}

		// If hint is not unlocked
		if (!CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_SAVE_LOAD_SCREEN))
		{
			// Aku Hint "This is the load/save screen..."
			MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_SAVE_LOAD_SCREEN, 0);
		}

		// Check if Aku Aku is giving a hint:
		// 0 - speaking, 1 - gone.
		if ((driver->speed < AH_SAVEOBJ_ENTRY_SPEED_MAX) && AH_MaskHint_boolCanSpawn())
		{
			s16 scanlineFrame = save->scanlineFrame - 1;
			save->scanlineFrame = scanlineFrame;

			// if scanline goes past the top
			if (scanlineFrame < 0)
			{
				SVec3 desiredPos;
				SVec3 desiredRot;
				struct SpawnPosRot *saveSpawn = gGT->level1->ptrSpawnType2_PosRot->posRot;

				// desired transition position (x,y,z)
				desiredPos.x = saveSpawn->pos.x + (s16)((int)saveInst->matrix.m[0][0] * AH_SAVEOBJ_CAMERA_FORWARD_OFFSET >> 7);
				desiredPos.y = saveSpawn->pos.y + (s16)((int)saveInst->matrix.m[1][0] * AH_SAVEOBJ_CAMERA_FORWARD_OFFSET >> 7);
				desiredPos.z = saveSpawn->pos.z + (s16)((int)saveInst->matrix.m[2][0] * AH_SAVEOBJ_CAMERA_FORWARD_OFFSET >> 7);

				// desired transition rotation (x,y,z)
				desiredRot.x = saveSpawn->rot.x + D232.saveObjCameraOffset.x;
				desiredRot.y = saveSpawn->rot.y + D232.saveObjCameraOffset.y;
				desiredRot.z = saveSpawn->rot.z + D232.saveObjCameraOffset.z;

				// VehBirth_NullThread is an empty function that does nothing
				driver->instSelf->thread->funcThTick = VehBirth_NullThread;

				// Set CameraDC's desired position and rotation,
				// then begin the transition by setting flag
				CAM_SetDesiredPosRot(&gGT->cameraDC[0], &desiredPos, &desiredRot);

				GAMEPAD_JogCon2(driver, 0, 0);

				save->flags |= AH_SAVEOBJ_FLAG_INTERACTION_ACTIVE;

				// backup HUD flags while driver sees Save/Load screen
				save->hudFlagBackup = gGT->hudFlags;

				// clear HUD flags
				gGT->hudFlags = 0;
			}
			goto LAB_800af72c;
		}
	}
	else
	{
		if (driver->speed < AH_SAVEOBJ_EXIT_SPEED_MAX)
		{
			// if camera is not transitioning
			if (((gGT->cameraDC->flags & CAMERA_FLAG_TRANSITION_AWAY) == 0) &&

			    ((save->flags & AH_SAVEOBJ_FLAG_HUD_RESTORED) == 0))
			{
				driverInst->thread->funcThTick = NULL;

				save->flags |= AH_SAVEOBJ_FLAG_HUD_RESTORED;
#if defined(CTR_NATIVE)
				// NOTE(aalhendi): Retail hides the HUD before queueing a hub
				// load while levelID is temporarily MAIN_MENU_LEVEL. Native can
				// keep this save object ticking during that transition, so do not
				// restore the hub HUD/map until loading is idle again.
				if (sdata->Loading.stage == LOAD_IDLE)
				{
#endif
					gGT->hudFlags = (u8)save->hudFlagBackup;
#if defined(CTR_NATIVE)
				}
#endif
			}

			// if camera is transitioning
			else
			{
				// Check the flag again
				u32 cameraFlags = gGT->cameraDC->flags;

				// if transition reached it's destination position
				if ((cameraFlags & CAMERA_FLAG_TRANSITION_HOLD) != 0)
				{
					// if it is not time to return to player
					if ((save->flags & AH_SAVEOBJ_FLAG_MENU_SHOWN) == 0)
					{
						save->flags |= AH_SAVEOBJ_FLAG_MENU_SHOWN;

						SelectProfile_GetTrackID();

						// enable menu for green load/save screen
						RECTMENU_Show(&data.menuGreenLoadSave);
					}

					// if it is time to return to player
					else
					{
						if (
						    // if you aren't already returning to player
						    ((cameraFlags & CAMERA_FLAG_TRANSITION_BACK) == 0) &&

						    // if there's no Menu active
						    (sdata->ptrActiveMenu == NULL))
						{
							// toggle flag to return, this either snaps back
							// or transitions back depending on & 0x200 (like 0x600 or 0xe00)
							gGT->cameraDC->flags = cameraFlags | CAMERA_FLAG_TRANSITION_BACK;
						}
					}
				}
			}
			goto LAB_800af72c;
		}
		save->flags = AH_SAVEOBJ_FLAG_NONE;
	}

	save->scanlineFrame = AH_SAVEOBJ_SCANLINE_START_FRAME;

LAB_800af72c:

	// SaveObj keeps the save/load instance pointer first.
	saveInst = save->inst;

	if (saveInst != NULL)
	{
		s16 animFrame = saveInst->animFrame;
		int numAnimFrames = INSTANCE_GetNumAnimFrames(saveInst, 0);

		// if animation is not finished
		if ((int)animFrame < numAnimFrames - 1)
		{
			// increment animation frame
			saveInst->animFrame += 1;
		}

		// if animation is finished,
		// reset animation, and play sound
		else
		{
			// get square root to have "true" distance
			int volumeDistance = SquareRoot0_stub(dist);

			// multiply by 1.5f
			// mul 3, divide 2
			volumeDistance = volumeDistance * 3 >> 1;

			// play sound at max volume
			u32 volume = HOWL_SFX_VOLUME_MAX;

			// if distance is large
			if (AH_SAVEOBJ_SCAN_SFX_NEAR_DIST < volumeDistance)
			{
				// Map the distance
				//	[300, 6000] (close, far)
				// to volume
				//	[0xff, 0] (loud, soft)
				volume = VehCalc_MapToRange(volumeDistance, AH_SAVEOBJ_SCAN_SFX_NEAR_DIST, AH_SAVEOBJ_SCAN_SFX_FAR_DIST, HOWL_SFX_VOLUME_MAX, 0);
			}

			// Play save/load screen sound
			// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af7a8-0x800af7c0 for save/load screen loop SFX.
			OtherFX_Play_LowLevel(AH_SAVEOBJ_SCAN_SFX_ID, 1, HowlSfx_Pack(HOWL_SFX_LR_CENTER, HOWL_SFX_DISTORTION_NONE, volume, 0));

			// reset animation
			saveInst->animFrame = 0;
		}
	}
	return;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af7f0-0x800af9f8.
void AH_SaveObj_LInB(struct Instance *savInst)
{
	struct GameTracker *gGT = sdata->gGT;

	// if this Instance's thread is not valid
	if (savInst->thread == NULL)
	{
		struct Thread *t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct SaveObj), NONE, SMALL, STATIC),

		                                        AH_SaveObj_ThTick, R232.s_saveobj, 0);

		savInst->thread = t;

		// if the thread was built properly
		if (t != NULL)
		{
			struct SaveObj *save = t->object;

			t->inst = savInst;

			t->funcThDestroy = AH_SaveObj_ThDestroy;

			// initialize object
			save->flags = AH_SAVEOBJ_FLAG_NONE;

			save->scanlineFrame = 0;

			savInst->flags |= HIDE_MODEL;

			if (gGT->level1->numSpawnType2_PosRot == 0)
			{
				save->inst = NULL;
			}
			else
			{
				struct SpawnType2 *spawn = gGT->level1->ptrSpawnType2_PosRot;
				struct SpawnPosRot *saveSpawn = spawn->posRot;
				struct Instance *inst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_SCAN], R232.s_scan, t);
				save->inst = inst;

				// NOTE(aalhendi): Native low-RAM audit candidate only. Retail
				// uses this scan instance allocation before any null fallback;
				// keep unpatched until a valid hub/save repro proves failure.
				memcpy(&inst->matrix, &savInst->matrix, sizeof(inst->matrix));

				SVec3 rot = saveSpawn->rot;

				ConvertRotToMatrix(&inst->matrix, &rot);

				inst->matrix.t[0] = saveSpawn->pos.x;
				inst->matrix.t[1] = saveSpawn->pos.y;
				inst->matrix.t[2] = saveSpawn->pos.z;

				inst->depthBiasNormal = AH_SAVEOBJ_SCAN_DEPTH_BIAS;
			}
		}
	}
	return;
}
