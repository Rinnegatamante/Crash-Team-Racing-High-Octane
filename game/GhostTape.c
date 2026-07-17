#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80027df4-0x80027e90.
void GhostTape_Start(void)
{
	struct GhostHeader *gh;
	struct Driver *d;
	struct GameTracker *gGT = sdata->gGT;

	d = gGT->drivers[0];

	// v1 - PizzaHut (June), Spyro2 (July)
	// v4 - Aug5, Aug14, Sep3, Retail

	gh = sdata->GhostRecording.ptrGhost;
	gh->version = GHOST_TAPE_VERSION_RETAIL;
	gh->levelID = gGT->levelID;
	gh->characterID = data.characterIDs[d->driverID];

	sdata->GhostRecording.VelX = 0;
	sdata->GhostRecording.VelY = 0;
	sdata->GhostRecording.VelZ = 0;

	sdata->GhostRecording.timeElapsedInRace = 0;
	sdata->boolGhostTooBigToSave = 0;
	sdata->ghostOverflowTextTimer = 0;
	sdata->boolCanSaveGhost = 1;

	sdata->GhostRecording.ptrCurrOffset = sdata->GhostRecording.ptrStartOffset;

	sdata->GhostRecording.countEightFrames = 0;
	sdata->GhostRecording.countSixteenFrames = 0;
	sdata->GhostRecording.timeOfLast80buffer = 0;
	sdata->GhostRecording.boostCooldown1E = 0;

	sdata->GhostRecording.animFrame = -1;
	sdata->GhostRecording.animIndex = -1;
	sdata->GhostRecording.instanceFlags = 0;

	return;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80027e90-0x80027f20
void GhostTape_End(void)
{
	struct Driver *d;
	struct GhostHeader *gh;
	struct GameTracker *gGT = sdata->gGT;

	// quit, if ghost cant be saved
	if (sdata->boolCanSaveGhost == 0)
	{
		return;
	}

	// dont save ghost twice
	sdata->boolCanSaveGhost = 0;

	// Write the last chunk of ghost data
	GhostTape_WriteMoves(1);

	d = gGT->drivers[0];
	gh = sdata->GhostRecording.ptrGhost;

	gh->ySpeed = d->ySpeed;
	gh->speedApprox = d->speedApprox;
	gh->timeElapsedInRace = d->timeElapsedInRace;
	gh->size = (u32)sdata->GhostRecording.ptrCurrOffset - (u32)sdata->GhostRecording.ptrStartOffset;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80027f20-0x8002838c.
void GhostTape_WriteMoves(s16 raceFinished)
{
	struct GameTracker *gGT = sdata->gGT;
	u32 gameMode = gGT->gameMode1;

	if (raceFinished == 0)
	{
		// if you can not save ghost
		if (sdata->boolCanSaveGhost == 0)
		{
			return;
		}

		// if paused or [race ended while not yet in end-of-race menu]???
		if ((gameMode & GAME_MODE_GHOST_RECORD_BLOCK_MASK) != 0)
		{
			return;
		}

		// if traffic lights are not done counting down
		if (0 < gGT->trafficLightsTimer)
		{
			return;
		}

		// If you're in End-Of-Race menu
		if ((gameMode & END_OF_RACE) != 0)
		{
			GhostTape_End();
			return;
		}
	}
	if (sdata->GhostRecording.boostCooldown1E != 0)
	{
		sdata->GhostRecording.boostCooldown1E--;
	}

	if (

	    // If race is just finished
	    (raceFinished != 0) ||

	    // This is true every 8 frames
	    ((sdata->GhostRecording.countEightFrames & GHOST_RECORD_INTERVAL_MASK_8) == 0))
	{
		struct Driver *driver = gGT->threadBuckets[0].thread->object;

		// player instance
		struct Instance *inst = driver->instSelf;

		// compress position (x, y, z) with bitshifting
		int posX = inst->matrix.t[0] >> GHOST_RECORD_POSITION_SHIFT;
		int posY = inst->matrix.t[1] >> GHOST_RECORD_POSITION_SHIFT;
		int posZ = inst->matrix.t[2] >> GHOST_RECORD_POSITION_SHIFT;

		// get change in position (x, y, z)
		sdata->GhostRecording.VelX = (s16)posX - sdata->GhostRecording.VelX;
		sdata->GhostRecording.VelY = (s16)posY - sdata->GhostRecording.VelY;
		sdata->GhostRecording.VelZ = (s16)posZ - sdata->GhostRecording.VelZ;

		// Time elapsed since last 0x80 buffer
		int timeSincePositionPacket = sdata->GhostRecording.timeElapsedInRace - sdata->GhostRecording.timeOfLast80buffer;

		// get pointer to current recording char in buffer
		char *writeCursor = sdata->GhostRecording.ptrCurrOffset;

		if (
		    // if animation frame changed
		    (sdata->GhostRecording.animFrame != inst->animFrame) ||

		    // if animation changed
		    (sdata->GhostRecording.animIndex != inst->animIndex))
		{
			sdata->GhostRecording.animFrame = inst->animFrame;
			sdata->GhostRecording.animIndex = inst->animIndex;

			writeCursor[0] = GHOST_OP_ANIMATION;
			writeCursor[1] = inst->animIndex;
			writeCursor[2] = inst->animFrame;
			writeCursor += GHOST_SIZE_ANIMATION;
		}

		// If there is a change in instance flags,
		// determine if driver is split by water or mud
		if ((inst->flags & GHOST_RECORD_INSTANCE_SPLIT_FLAG) != (sdata->GhostRecording.instanceFlags & GHOST_RECORD_INSTANCE_SPLIT_FLAG))
		{
			// Record the instance flags
			// determine if driver is split by water or mud

			writeCursor[0] = GHOST_OP_INSTANCE;
			writeCursor[1] = (char)(inst->flags >> GHOST_RECORD_INSTANCE_SPLIT_SHIFT) & 1;
			writeCursor += GHOST_SIZE_INSTANCE;
		}

		// If velocity is small enough for a compressed 5-char message
		if (
		    // If the race is not over
		    (raceFinished == 0) &&

		    // false once every 32 counts, despite the retail field name
		    ((sdata->GhostRecording.countSixteenFrames & GHOST_RECORD_INTERVAL_MASK_32) != 0) &&

		    // If velX is small enough for one char
		    (sdata->GhostRecording.VelX < GHOST_RECORD_VELOCITY_MAX) && (GHOST_RECORD_VELOCITY_MIN_EXCLUSIVE < sdata->GhostRecording.VelX) &&

		    // If velY is small enough for one char
		    (sdata->GhostRecording.VelY < GHOST_RECORD_VELOCITY_MAX) && (GHOST_RECORD_VELOCITY_MIN_EXCLUSIVE < sdata->GhostRecording.VelY) &&

		    // If velZ is small enough for one char
		    (sdata->GhostRecording.VelZ < GHOST_RECORD_VELOCITY_MAX) && (GHOST_RECORD_VELOCITY_MIN_EXCLUSIVE < sdata->GhostRecording.VelZ) &&

		    // if not a lot of time has passed
		    // since the last 0x80 buffer
		    (timeSincePositionPacket < GHOST_RECORD_TIME_DELTA_MAX_EXCLUSIVE))
		{
			// If there is no change in position
			if (((sdata->GhostRecording.VelX == 0) && (sdata->GhostRecording.VelY == 0)) && (sdata->GhostRecording.VelZ == 0))
			{
				// Record that you are doing nothing
				writeCursor[0] = GHOST_OP_IDLE;
				writeCursor += GHOST_SIZE_IDLE;
			}

			// If you are moving
			else
			{
				// dont write opcode,
				// "no opcode" means "assume velocity"

				// Write velX to buffer
				writeCursor[0] = (char)sdata->GhostRecording.VelX;
				writeCursor[1] = (char)sdata->GhostRecording.VelY;
				writeCursor[2] = (char)sdata->GhostRecording.VelZ;
				writeCursor[3] = (char)(driver->rotCurr.y >> GHOST_RECORD_ROTATION_SHIFT);
				writeCursor[4] = (char)(driver->rotCurr.z >> GHOST_RECORD_ROTATION_SHIFT);
				writeCursor += GHOST_SIZE_VELOCITY;
			}
		}

		// If velocity is too large,
		// If the race just ended
		// If you're in a forced full-position interval
		// write a longer message
		else
		{
			// 0x80-style chunks are 11 chars long (including 0x80)

			// Write to ghost recording buffer
			writeCursor[0] = GHOST_OP_POSITION;

			// flipping endians

			// Write 2-char X position
			writeCursor[1] = (char)(posX >> 8);
			writeCursor[2] = (char)posX;

			// Write 2-char Y position
			writeCursor[3] = (char)(posY >> 8);
			writeCursor[4] = (char)posY;

			// Write 2-char Z position
			writeCursor[5] = (char)(posZ >> 8);
			writeCursor[6] = (char)posZ;

			// Write 2-char ???
			// related to time
			writeCursor[7] = (char)(timeSincePositionPacket >> 8);
			writeCursor[8] = (char)timeSincePositionPacket;

			// Write 2-char rotation
			writeCursor[9] = (char)(driver->rotCurr.y >> GHOST_RECORD_ROTATION_SHIFT);
			writeCursor[10] = (char)(driver->rotCurr.z >> GHOST_RECORD_ROTATION_SHIFT);

			writeCursor += GHOST_SIZE_POSITION;

			// Time of last 0x80 buffer
			sdata->GhostRecording.timeOfLast80buffer = sdata->GhostRecording.timeElapsedInRace;
		}

		// Make a copy of instance flags
		sdata->GhostRecording.instanceFlags = inst->flags;

		if (
		    // if offset of ghost-recording buffer exceeds
		    // the maximum size of a ghost that can be recorded
		    // (if you're one frame away from max capacity)
		    ((u32)sdata->GhostRecording.ptrEndOffset < (u32)writeCursor + GHOST_RECORD_BUFFER_END_GUARD) &&

		    (sdata->boolCanSaveGhost = 0,

		     // If you're not in End-Of-Race menu
		     // (if you were, you'd be just in time to save the ghost)
		     (gameMode & END_OF_RACE) == 0))
		{
			sdata->boolGhostTooBigToSave = 1;

			// set ghostOverflowTextTimer
			// to 180 frames (6 seconds 30fps)
			sdata->ghostOverflowTextTimer = GHOST_RECORD_OVERFLOW_TEXT_FRAMES;
		}

		// Increment frame counter
		sdata->GhostRecording.countSixteenFrames++;

		// Save this frame's X, Y, Z positions,
		// so that they can be used next frame to
		// calculate velocity
		sdata->GhostRecording.VelX = (s16)posX;
		sdata->GhostRecording.VelY = (s16)posY;
		sdata->GhostRecording.VelZ = (s16)posZ;

		// save incremeneted pointer
		sdata->GhostRecording.ptrCurrOffset = writeCursor;
	}

	// Increment frame counter
	sdata->GhostRecording.countEightFrames++;

	// Increment race timer by elapsed milliseconds per frame, ~32
	sdata->GhostRecording.timeElapsedInRace += gGT->elapsedTimeMS;
	return;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002838c-0x80028410.
void GhostTape_WriteBoosts(int addReserve, u8 type, int speedCap)
{
	char *writeCursor;

	// quit, if ghost cant be saved
	if (sdata->boolCanSaveGhost == 0)
	{
		return;
	}

	writeCursor = sdata->GhostRecording.ptrCurrOffset;

	if ((type & TURBO_PAD) != 0)
	{
		if (sdata->GhostRecording.boostCooldown1E != 0)
		{
			return;
		}
		sdata->GhostRecording.boostCooldown1E = GHOST_RECORD_BOOST_COOLDOWN_FRAMES;
	}

	// 0x82-style chunks are 6 bytes long (including 0x82)

	// Write to recording buffer
	writeCursor[0] = GHOST_OP_BOOST;

	// big endian reserve
	writeCursor[1] = (char)((u32)addReserve >> 8);
	writeCursor[2] = (char)addReserve;

	// char, add type (increment or set)
	writeCursor[3] = type;

	// big endian speedCcap
	writeCursor[4] = (char)((u32)speedCap >> 8);
	writeCursor[5] = (char)speedCap;

	sdata->GhostRecording.ptrCurrOffset += GHOST_SIZE_BOOST;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028410-0x8002843c.
void GhostTape_Destroy()
{
	if (sdata->ptrGhostTapePlaying != 0)
	{
		MEMPACK_ClearHighMem();
		sdata->ptrGhostTapePlaying = 0;
	}
}
