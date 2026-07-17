#include <common.h>

enum AHDoorConstants
{
	AH_DOOR_ACCESS_FLAG = 2,
	AH_DOOR_INTERACTION_FORWARD_OFFSET = 0x300,
	AH_DOOR_NEAR_DIST_SQ = 0x90000,
	AH_DOOR_NEAR_DIST_MAX_SQ = AH_DOOR_NEAR_DIST_SQ - 1,
	AH_DOOR_CAMERA_HOLD_FRAMES = CTR_SECONDS_TO_FRAMES(2),
	AH_DOOR_KEY_SPIN_FRAMES = CTR_SECONDS_TO_FRAMES(4),
	AH_DOOR_KEY_LIGHT_X = -0xc98,
	AH_DOOR_KEY_LIGHT_Y = 0x99f,
	AH_DOOR_KEY_LIGHT_Z = 0x232,
	AH_DOOR_KEY_TARGET_SCALE = 0xa00,
	AH_DOOR_KEY_SCALE_STEP = 0x40,
	AH_DOOR_KEY_ROT_STEP = 0x40,
	AH_DOOR_KEY_RAISE_HEIGHT = 0xa0,
	AH_DOOR_KEY_RAISE_STEP = 4,
	AH_DOOR_KEY_ORBIT_STEP = 0x10,
	AH_DOOR_KEY_SHRINK_FRAME_COUNT = 0xb,
	AH_DOOR_KEY_FLOAT_RADIUS_SHIFT = 5,
	AH_DOOR_KEY_FLOAT_SFX_FRAME_0 = 0x0a,
	AH_DOOR_KEY_FLOAT_SFX_FRAME_1 = 0x0f,
	AH_DOOR_KEY_FLOAT_SFX_FRAME_2 = 0x14,
	AH_DOOR_KEY_FLOAT_SFX_FRAME_3 = 0x19,
	AH_DOOR_KEY_FLOAT_SFX_ID = 0x67,
	AH_DOOR_UNLOCK_SFX_FRAME = 0x50,
	AH_DOOR_UNLOCK_SFX_ID = 0x93,
	AH_DOOR_CREAK_SFX_ID = 0x94,
	AH_DOOR_OPEN_ROTATION = 0x400,
	AH_DOOR_OPEN_ROTATION_STEP = 0x10,
	AH_DOOR_PAIR_OFFSET = 0x600,
	AH_DOOR_CAMERA_FORWARD_OFFSET = 0x312,
	AH_DOOR_CAMERA_SIDE_OFFSET = 0x600,
	AH_DOOR_CAMERA_HEIGHT_OFFSET = 0x17a,
	AH_DOOR_CAMERA_PITCH_OFFSET = 0x800,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af9f8-0x800afa60.
void AH_Door_ThDestroy(struct Thread *t)
{
	int i;
	struct WoodDoor *woodDoor = t->object;

	if (woodDoor->otherDoor != NULL)
	{
		INSTANCE_Death(woodDoor->otherDoor);
		woodDoor->otherDoor = NULL;
	}

	for (i = 0; i < AH_WOOD_DOOR_KEY_COUNT; i++)
	{
		INSTANCE_Death(woodDoor->keyInst[i]);
		woodDoor->keyInst[i] = NULL;
	}
	return;
}

static b32 AH_Door_IsOpenByRewards(s16 levelID, AdventureHubDoorID doorID)
{
	if ((levelID == N_SANITY_BEACH) && (doorID == AH_DOOR_BEACH_TO_GLACIER_PARK))
	{
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_BEACH_TO_GLACIER_PARK_MASK) != 0;
	}

	if ((levelID == N_SANITY_BEACH) && (doorID == AH_DOOR_BEACH_TO_GEMSTONE_VALLEY))
	{
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_BEACH_TO_GEMSTONE_VALLEY_MASK) != 0;
	}

	if (levelID == GEM_STONE_VALLEY)
	{
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_GEMSTONE_VALLEY_TO_CUPS_MASK) != 0;
	}

	if (levelID == THE_LOST_RUINS)
	{
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_LOST_RUINS_TO_GLACIER_PARK_MASK) != 0;
	}

	if (levelID == GLACIER_PARK)
	{
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_GLACIER_PARK_TO_CITADEL_CITY_MASK) != 0;
	}

	return false;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 232 0x800afa60-0x800b072c.
void AH_Door_ThTick(struct Thread *t)
{
	b32 doorIsOpen;
	AdventureHubDoorID doorID;
	s16 lev;
	s16 numKeys;
	u16 hintId;
	u32 chkRewards;
	int i;
	int ratio;
	int distX;
	int distY;
	int distZ;
	int dist;
	SVec3 desiredPos;
	SVec3 desiredRot;
	SVec3 keyLightDir;
	const s16 *scaler;

	struct GameTracker *gGT = sdata->gGT;
	struct WoodDoor *door = t->object;
	struct Instance *doorInst = t->inst;
	struct Instance *keyInst;
	struct Driver *driver = gGT->drivers[0];
	struct Instance *driverInst;
	struct CameraDC *cDC = gGT->cameraDC;

	doorID = door->doorID;

	// Assume door is closed
	doorIsOpen = false;

	lev = gGT->levelID;

	// NOTE(aalhendi): Retail derives open state from adventure rewards here.
	doorIsOpen = AH_Door_IsOpenByRewards(lev, doorID);

	// Cosine(angle)
	ratio = MATH_Cos((int)doorInst->instDef->rot.y);

	// X distance of player and door
	distX = doorInst->matrix.t[0] + (ratio * AH_DOOR_INTERACTION_FORWARD_OFFSET >> 0xc) - driver->instSelf->matrix.t[0];

	// Y distance of player and door
	distY = doorInst->matrix.t[1] - driver->instSelf->matrix.t[1];

	// Sine(angle)
	ratio = MATH_Sin((int)doorInst->instDef->rot.y);

	// Z distance of player and door
	distZ = doorInst->matrix.t[2] + (ratio * AH_DOOR_INTERACTION_FORWARD_OFFSET >> 0xc) - driver->instSelf->matrix.t[2];

	// distance from player and door
	dist = distX * distX + distY * distY + distZ * distZ;

	// If player is close to a door
	if (dist < AH_DOOR_NEAR_DIST_SQ)
	{
		// if door is open
		if (doorIsOpen)
		{
			// enable access through a door (disable collision)
			sdata->doorAccessFlags |= AH_DOOR_ACCESS_FLAG;
		}

		// if door is locked
		else
		{
			// remove access (enable collision)
			sdata->doorAccessFlags &= ~AH_DOOR_ACCESS_FLAG;
		}
	}

	// If this is N Sane Beach
	if (lev == N_SANITY_BEACH)
	{
		// if this is beach -> gemstone,
		// must have 1 key
		numKeys = 1;

		// If this is door from beach -> glacier
		if (doorID == AH_DOOR_BEACH_TO_GLACIER_PARK)
		{
			// must have 2 keys
			numKeys = 2;
		}
	}
	// If not N Sane Beach
	else
	{
		// get number of keys for whichever door is on the hub
		numKeys = D232.keysNeededByHub[(lev + -0x19)];
	}

	// if in a state where you're seeing the boss key open an adv door,
	// or some other kind of cutscene where you can't move
	if ((gGT->gameMode2 & 4) != 0)
	{
		return;
	}

	// If door is open
	if (doorIsOpen)
	{
		// camera transition, watch door open
		if ((cDC->flags & CAMERA_FLAG_TRANSITION_AWAY) != 0)
		{
			// quit, come back when camera transition ends
			return;
		}

		// check if hint is unlocked
		chkRewards = CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_NEW_WORLD_GREETING);

		// "Congrats on opening this new area..."
		hintId = ADV_MASK_HINT_ID_NEW_WORLD_GREETING;

	joined_r0x800b06ec:

		// if hint is not unlocked
		if (chkRewards == 0)
		{
			MainFrame_RequestMaskHint(hintId, 0);
		}
		return;
	}

	// == if door is closed ==

	if (
	    // if player is far from the door
	    (AH_DOOR_NEAR_DIST_MAX_SQ < dist) &&
	    // flags
	    ((door->camFlags & WdCam_CutscenePlaying) == 0))
	{
		return;
	}

	// if player has less than that amount
	if (gGT->currAdvProfile.numKeys < numKeys)
	{
		// if one key is required
		if (numKeys == 1)
		{
			// check if hint is unlocked
			chkRewards = CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_MUST_HAVE_ONE_BOSS_KEY);

			// Aku Hint "You must have a boss key"
			hintId = ADV_MASK_HINT_ID_MUST_HAVE_ONE_BOSS_KEY;
		}

		// not one...
		else
		{
			// if not 2...
			if (numKeys != 2)
			{
				// no hint for that
				return;
			}

			// if 2 keys are needed to enter

			// check if hint is unlocked
			chkRewards = CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_MUST_HAVE_TWO_BOSS_KEYS);

			// Aku Hint "You must have two boss keys"
			hintId = ADV_MASK_HINT_ID_MUST_HAVE_TWO_BOSS_KEYS;
		}

		// request hint and quit
		goto joined_r0x800b06ec;
	}

	// == door is closed, ready to unlock ==

	if ((door->camFlags & WdCam_FlyingOut) != 0)
	{
		door->camTimer_unused = AH_DOOR_CAMERA_HOLD_FRAMES;

		if (((cDC->flags & CAMERA_FLAG_TRANSITION_AWAY) == 0) && ((door->camFlags & WdCam_FlyingIn) == 0))
		{
			driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;
			door->camFlags |= WdCam_FlyingIn;
		}
		else if (((cDC->flags & CAMERA_FLAG_TRANSITION_HOLD) != 0) && ((door->camFlags & WdCam_FullyOut) == 0))
		{
			door->camFlags |= WdCam_FullyOut;
		}
	}
	else
	{
		if (door->camTimer_unused != 0)
		{
			door->camTimer_unused--;
			return;
		}

		// If the game is paused
		if ((gGT->gameMode1 & PAUSE_ALL) != 0)
		{
			return;
		}

		// If you are here, game must not be paused

		driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_FreezeEndEvent_Init;

		door->camFlags |= WdCam_CutscenePlaying;

		// if timer is less than four full seconds
		if (door->frameCount_doorOpenAnim < AH_DOOR_KEY_SPIN_FRAMES)
		{
			if (driver->speedApprox < 0x80)
			{
				keyLightDir.x = AH_DOOR_KEY_LIGHT_X;
				keyLightDir.y = AH_DOOR_KEY_LIGHT_Y;
				keyLightDir.z = AH_DOOR_KEY_LIGHT_Z;

				// if keys are not spawned, create them
				if (door->keyInst[0] == NULL)
				{
					// if number of keys is more than zero
					if (numKeys > 0)
					{
						// spawn instances for every key you have,
						// this makes them spin in a circle before
						// seeing the doors animate open
						for (i = 0; i < numKeys; i++)
						{
							// name = "key"
							keyInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_KEY], R232.s_key, t);

							// Set Key Color
							keyInst->colorRGBA = INST_COLOR_KEY;

							keyInst->flags |= USE_SPECULAR_LIGHT;
							door->frameCount_unused++;

							driverInst = driver->instSelf;
							keyInst->matrix = driverInst->matrix;

							// set scale to zero
							keyInst->scale.x = 0;
							keyInst->scale.y = 0;
							keyInst->scale.z = 0;

							door->keyInst[i] = keyInst;
						}
					}

					// Make a backup of HUD variable
					door->hudFlags = (u8)gGT->hudFlags;

					// Disable HUD
					gGT->hudFlags = 0;

					// For watching hub doors animate open
				}

				// if more than zero
				if (0 < numKeys)
				{
					// loop through all keys
					for (i = 0; i < numKeys; i++)
					{
						keyInst = door->keyInst[i];
						if (keyInst != NULL)
						{
							// if key is still growing to full display scale
							if (keyInst->scale.x < AH_DOOR_KEY_TARGET_SCALE)
							{
								// increase scale on X, Y, Z
								keyInst->scale.x += AH_DOOR_KEY_SCALE_STEP;
								keyInst->scale.y += AH_DOOR_KEY_SCALE_STEP;
								keyInst->scale.z += AH_DOOR_KEY_SCALE_STEP;
							}

							// if key posY is below its hover height over the player
							if (keyInst->matrix.t[1] < (driver->instSelf->matrix.t[1] + AH_DOOR_KEY_RAISE_HEIGHT))
							{
								// increase key posY
								keyInst->matrix.t[1] += AH_DOOR_KEY_RAISE_STEP;
							}

							if (1 < numKeys)
							{
								int keyOrbitOffset = i * (FP_ONE / numKeys);
								int keyOrbitRadius = (int)keyInst->scale.x;

								if (keyOrbitRadius < 0)
								{
									keyOrbitRadius += (1 << AH_DOOR_KEY_FLOAT_RADIUS_SHIFT) - 1;
								}

								keyOrbitRadius >>= AH_DOOR_KEY_FLOAT_RADIUS_SHIFT;

								ratio = MATH_Sin(door->keyOrbit + keyOrbitOffset);

								keyInst->matrix.t[0] = driver->instSelf->matrix.t[0] + (keyOrbitRadius * ratio >> 0xc);

								ratio = MATH_Cos(door->keyOrbit + keyOrbitOffset);

								keyInst->matrix.t[2] = driver->instSelf->matrix.t[2] + (keyOrbitRadius * ratio >> 0xc);
							}

							Vector_SpecLightSpin3D(keyInst, &door->keyRot, &keyLightDir);

							// convert 3 rotation shorts into rotation matrix
							ConvertRotToMatrix(&keyInst->matrix, &door->keyRot);
						}
						door->keyInst[i] = keyInst;
					}
				}

				door->keyRot.x = 0;
				door->keyRot.y += AH_DOOR_KEY_ROT_STEP;
				door->keyRot.z = 0;

				door->keyOrbit += AH_DOOR_KEY_ORBIT_STEP;

				door->frameCount_doorOpenAnim++;

				// Sound effects when keys float in air

				switch (door->frameCount_doorOpenAnim)
				{
				case AH_DOOR_KEY_FLOAT_SFX_FRAME_0:
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0208-0x800b0218 for first floating-key SFX.
					OtherFX_Play_LowLevel(AH_DOOR_KEY_FLOAT_SFX_ID, 1, 0xff7680);
					break;
				case AH_DOOR_KEY_FLOAT_SFX_FRAME_1:
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b022c-0x800b023c for second floating-key SFX.
					OtherFX_Play_LowLevel(AH_DOOR_KEY_FLOAT_SFX_ID, 1, 0xeb8080);
					break;
				case AH_DOOR_KEY_FLOAT_SFX_FRAME_2:
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0250-0x800b0260 for third floating-key SFX.
					OtherFX_Play_LowLevel(AH_DOOR_KEY_FLOAT_SFX_ID, 1, 0xd78a80);
					break;
				case AH_DOOR_KEY_FLOAT_SFX_FRAME_3:
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0274-0x800b0284 for fourth floating-key SFX.
					OtherFX_Play_LowLevel(AH_DOOR_KEY_FLOAT_SFX_ID, 1, 0xc39480);
					break;
				case AH_DOOR_UNLOCK_SFX_FRAME:
					// unlock door sound
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0298-0x800b02ac for door unlock SFX.
					OtherFX_Play(AH_DOOR_UNLOCK_SFX_ID, 1);
					break;
				case AH_DOOR_KEY_SPIN_FRAMES:
					// on last frame, doors creak open
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b02a0-0x800b02ac for door creak SFX.
					OtherFX_Play(AH_DOOR_CREAK_SFX_ID, 1);
					break;

				default:
					break;
				}
			}
			return;
		}

		// == After 4 seconds ==

		ratio = MATH_Cos((int)doorInst->instDef->rot.y);

		i = MATH_Cos((int)doorInst->instDef->rot.y + AH_DOOR_OPEN_ROTATION);

		// desired posX for transition
		desiredPos.x = doorInst->matrix.t[0] + (s16)(ratio * AH_DOOR_CAMERA_FORWARD_OFFSET >> 0xc) + (s16)(i * AH_DOOR_CAMERA_SIDE_OFFSET >> 0xc);
		// desired posY for transition
		desiredPos.y = doorInst->matrix.t[1] + AH_DOOR_CAMERA_HEIGHT_OFFSET;

		ratio = MATH_Sin((int)doorInst->instDef->rot.y);

		i = MATH_Sin((int)doorInst->instDef->rot.y + AH_DOOR_OPEN_ROTATION);

		// desired posZ for transition
		desiredPos.z = doorInst->matrix.t[2] + (s16)(ratio * AH_DOOR_CAMERA_FORWARD_OFFSET >> 0xc) + (s16)(i * AH_DOOR_CAMERA_SIDE_OFFSET >> 0xc);

		// desired rotation for transition
		desiredRot.x = doorInst->instDef->rot.x + AH_DOOR_CAMERA_PITCH_OFFSET;
		desiredRot.y = doorInst->instDef->rot.y;
		desiredRot.z = doorInst->instDef->rot.z;

		// set desired position and rotation for CamerDC transition
		CAM_SetDesiredPosRot(&gGT->cameraDC[0], &desiredPos, &desiredRot);

		GAMEPAD_JogCon2(driver, 0, 0);

		// start camera out transition (in "else" below)
		door->camFlags |= WdCam_FlyingOut;

		return;
	}

	// == door is opening ==

	if (door->doorRot.y < AH_DOOR_OPEN_ROTATION)
	{
		door->doorRot.y += AH_DOOR_OPEN_ROTATION_STEP;

		// right-hand door rot[x,y,z]
		desiredRot.x = door->doorRot.x;
		desiredRot.y = doorInst->instDef->rot.y - door->doorRot.y;
		desiredRot.z = door->doorRot.z;

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&door->otherDoor->matrix, &desiredRot);

		// left-hand door rot[x,y,z]
		desiredRot.y = doorInst->instDef->rot.y + door->doorRot.y;

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&doorInst->matrix, &desiredRot);

		// if less than 11 frames have passed,
		// decrease key scale, then quit function
		if (door->keyShrinkFrame < AH_DOOR_KEY_SHRINK_FRAME_COUNT)
		{
			scaler = R232.doorKeyShrinkScale;

			// loop through door key slots
			for (i = 0; i < AH_WOOD_DOOR_KEY_COUNT; i++)
			{
				keyInst = door->keyInst[i];
				// if instance exists
				if (keyInst != NULL)
				{
					// decrease scale of key
					keyInst->scale.x = scaler[door->keyShrinkFrame];
					keyInst->scale.y = scaler[door->keyShrinkFrame];
					keyInst->scale.z = scaler[door->keyShrinkFrame];
				}
			}

			door->keyShrinkFrame++;

			return;
		}

		// loop through door key slots
		for (i = 0; i < AH_WOOD_DOOR_KEY_COUNT; i++)
		{
			INSTANCE_Death(door->keyInst[i]);
			door->keyInst[i] = NULL;
		}

		return;
	}

	// == Door is fully open ==

	if (
	    // if this is N Sane Beach
	    ((lev == N_SANITY_BEACH) &&

	     // if this is door #4 (beach -> glacier)
	     (doorID == AH_DOOR_BEACH_TO_GLACIER_PARK)) ||

	    // if this is lost ruins (ruins -> glacier)
	    (lev == THE_LOST_RUINS))
	{
		// open all doors to glacier
		sdata->advProgress.storyFlags |= ADV_REWARD_DOORS_TO_GLACIER_PARK_MASK;
	}

	else if (
	    // if this is N Sane Beach
	    (lev == N_SANITY_BEACH) &&

	    // Door #5 (beach -> gemstone)
	    (doorID == AH_DOOR_BEACH_TO_GEMSTONE_VALLEY))
	{
		// record that door is open
		sdata->advProgress.storyFlags |= ADV_REWARD_DOOR_BEACH_TO_GEMSTONE_VALLEY_MASK;
	}

	// Gemstone valley (cup door)
	else if (lev == GEM_STONE_VALLEY)
	{
		// record that door is open
		sdata->advProgress.storyFlags |= ADV_REWARD_DOOR_GEMSTONE_VALLEY_TO_CUPS_MASK;
	}

	// Glacier Park (glacier -> citadel)
	else
	{
		// record that door is open
		sdata->advProgress.storyFlags |= ADV_REWARD_DOOR_GLACIER_PARK_TO_CITADEL_CITY_MASK;
	}

	cDC->flags |= CAMERA_FLAG_TRANSITION_BACK;

	driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;

	// cutscene over
	door->camFlags = (door->camFlags & ~WdCam_CutscenePlaying) | WdCam_FlyingIn;

	// bring HUD back
	gGT->hudFlags = (u8)door->hudFlags;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b072c-0x800b0b98.
void AH_Door_LInB(struct Instance *inst)
{
	int levelID;
	int ratio;
	SVec3 leftRot;
	SVec3 rightRot;

	struct GameTracker *gGT;
	struct Thread *t;
	struct Instance *otherDoorInst;
	struct Model *m;
	struct ModelHeader *headers;
	struct WoodDoor *woodDoor;
	struct Instance **instPtrArr;

	gGT = sdata->gGT;
	levelID = gGT->levelID;

	// If this Instance already has a thread
	if (inst->thread != NULL)
	{
		return;
	}

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct WoodDoor), NONE, SMALL, STATIC),
	                         AH_Door_ThTick, // behavior
	                         R232.s_door,    // debug name
	                         0               // thread relative
	);

	inst->thread = t;

	// if the thread failed to build
	if (t == NULL)
	{
		return;
	}

	woodDoor = t->object;

	t->inst = inst;

	t->funcThDestroy = AH_Door_ThDestroy;

	// this instance is always the left-hand door,
	// and every left-hand door has one key hole
	inst->flags |= SPLIT_SPECIAL;

	// Key slots are next to each other.
	instPtrArr = &woodDoor->keyInst[0];
	for (int i = 0; i < AH_WOOD_DOOR_KEY_COUNT; i++)
	{
		instPtrArr[i] = NULL;
	}

	woodDoor->frameCount_unused = 0;
	woodDoor->camFlags = WdCam_None;
	woodDoor->camTimer_unused = 0;
	woodDoor->frameCount_doorOpenAnim = 0;
	woodDoor->keyShrinkFrame = 0;

	woodDoor->doorRot.x = 0;
	woodDoor->doorRot.y = 0;
	woodDoor->doorRot.z = 0;
	woodDoor->doorID = 0;

	for (int i = 5; inst->name[i] != '\0'; i++)
	{
		woodDoor->doorID = woodDoor->doorID * 10 + inst->name[i] - '0';
	}

	// Level ID is Glacier Park
	if (levelID == GLACIER_PARK)
	{
		// door with two key holes
		m = gGT->modelPtr[STATIC_DOOR3];
	}

	// Level ID is not Glacier Park
	else if (
	    // Level ID is N Sanity Beach
	    (levelID == N_SANITY_BEACH) &&

	    // beach -> gemstone door
	    (woodDoor->doorID == AH_DOOR_BEACH_TO_GEMSTONE_VALLEY))
	{
		// door with no key holes
		m = gGT->modelPtr[STATIC_DOOR2];
	}

	// if not that door
	else
	{
		// door with one key hole
		m = gGT->modelPtr[STATIC_DOOR];
	}

	// DAT_800abaa4
	// "door"

	// INSTANCE_Birth3D -- ptrModel, name, thread
	otherDoorInst = INSTANCE_Birth3D(m, R232.s_door, t);

	// spawn instance of right-hand door,
	// which is not in LEV file, only built in thread
	woodDoor->otherDoor = otherDoorInst;

	// NOTE(aalhendi): Native low-RAM audit candidate only. Retail uses the
	// generated right-door instance before any null fallback; keep unpatched
	// until a valid hub/door repro proves the allocation can fail.

	otherDoorInst->flags |= (SPLIT_SPECIAL | REVERSE_CULL_DIRECTION);

	// copy full matrix (position and rotation)
	// from left-hand door to right-hand door
	otherDoorInst->matrix = inst->matrix;

	// set scaleX to -0x1000
	otherDoorInst->scale.x = -0x1000;

	ratio = MATH_Cos((int)inst->instDef->rot.y);

	otherDoorInst->matrix.t[0] += (ratio * AH_DOOR_PAIR_OFFSET >> 0xc);

	otherDoorInst->matrix.t[1] = inst->matrix.t[1];

	ratio = MATH_Sin((int)inst->instDef->rot.y);

	otherDoorInst->matrix.t[2] += (ratio * AH_DOOR_PAIR_OFFSET >> 0xc);

	// both doors always face camera
	headers = inst->model->headers;

	headers->flags |= 2;

	headers = otherDoorInst->model->headers;

	headers->flags |= 2;

	if (
	    // Level ID is N Sanity Beach, check door to Glacier Park
	    (levelID == N_SANITY_BEACH && woodDoor->doorID == AH_DOOR_BEACH_TO_GLACIER_PARK &&
	     ((sdata->advProgress.storyFlags & ADV_REWARD_DOOR_BEACH_TO_GLACIER_PARK_MASK) != 0)) ||

	    // Level ID is N Sanity Beach, check door to Gemstone Valley
	    (levelID == N_SANITY_BEACH && woodDoor->doorID == AH_DOOR_BEACH_TO_GEMSTONE_VALLEY &&
	     ((sdata->advProgress.storyFlags & ADV_REWARD_DOOR_BEACH_TO_GEMSTONE_VALLEY_MASK) != 0)) ||

	    // Level ID is Gemstone Valley, check door to Cup room
	    (levelID == GEM_STONE_VALLEY && ((sdata->advProgress.storyFlags & ADV_REWARD_DOOR_GEMSTONE_VALLEY_TO_CUPS_MASK) != 0)) ||

	    // Level ID is Lost Ruins, check door to Glacier Park
	    (levelID == THE_LOST_RUINS && ((sdata->advProgress.storyFlags & ADV_REWARD_DOOR_LOST_RUINS_TO_GLACIER_PARK_MASK) != 0)) ||

	    // Level ID is Glacier Park, check door to Citadel City
	    ((levelID == GLACIER_PARK) && ((sdata->advProgress.storyFlags & ADV_REWARD_DOOR_GLACIER_PARK_TO_CITADEL_CITY_MASK) != 0)))
	{
		// rotation = 90 degrees
		woodDoor->doorRot.y = AH_DOOR_OPEN_ROTATION;

		leftRot.x = woodDoor->doorRot.x;
		leftRot.y = inst->instDef->rot.y + woodDoor->doorRot.y;
		leftRot.z = woodDoor->doorRot.z;

		rightRot.x = woodDoor->doorRot.x;
		rightRot.y = inst->instDef->rot.y - woodDoor->doorRot.y;
		rightRot.z = woodDoor->doorRot.z;

		// make matrices for both doors rotated open

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&inst->matrix, &leftRot);
		ConvertRotToMatrix(&otherDoorInst->matrix, &rightRot);
	}
	return;
}
