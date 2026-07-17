#include <common.h>

enum
{
	UI_LAP_TIME_LAPS_PER_PLAYER = 7,
	UI_LAP_TIME_TICKS_PER_SECOND = 0x3c0,
	UI_LAP_TIME_TICKS_PER_TEN_SECONDS = 0x2580,
	UI_LAP_TIME_TICKS_PER_MINUTE = 0xe100,
	UI_LAP_TIME_MAX_MINUTES = 9,
	UI_LAP_TIME_SECONDS_TENS_MOD = 6,
	UI_LAP_TIME_DECIMAL_BASE = 10,
	UI_LAP_TIME_CENTISECOND_SCALE = 100,

	UI_REWARD_WUMPA_SHINE_CENTER = 0x80,
	UI_REWARD_WUMPA_SHINE_SHIFT = 4,
	UI_REWARD_PICKUP_ROT_SLOW = 0x40,
	UI_REWARD_PICKUP_ROT_FAST = 0x80,
	UI_REWARD_HUD_VISIBLE_WORD_MASK = 0xff0100,
	UI_REWARD_HUD_VISIBLE_WORD_VALUE = 0x100,
	UI_REWARD_FADE_VISIBLE_MIN = 0xfff,
	UI_REWARD_CTR_LETTER_BASE_SCALE = 0x800,
	UI_REWARD_CTR_LETTER_NEGATIVE_SCALE_BIAS = 0x401,
	UI_REWARD_CTR_LETTER_ROT_SHIFT = 10,
	UI_REWARD_CTR_LETTER_ROT_STEP = 0x200,
};

static const u32 UI_REWARD_PICKUP_COLOR = 0xffff0000u;

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004c55c-0x8004c718.
void UI_SaveLapTime(int numLaps, int lapTime, s16 driverID)
{
	int playerLapIndex = ((int)driverID * UI_LAP_TIME_LAPS_PER_PLAYER) + numLaps;
	int numMinutes = lapTime / UI_LAP_TIME_TICKS_PER_MINUTE;

	sdata->LapTimes.p1_Min1s[playerLapIndex] = numMinutes;

	if (UI_LAP_TIME_MAX_MINUTES < numMinutes)
	{
		sdata->LapTimes.p1_Min1s[playerLapIndex] = UI_LAP_TIME_MAX_MINUTES;
		sdata->LapTimes.p1_Sec10s[playerLapIndex] = UI_LAP_TIME_SECONDS_TENS_MOD - 1;
		sdata->LapTimes.p1_Sec1s[playerLapIndex] = UI_LAP_TIME_DECIMAL_BASE - 1;
		sdata->LapTimes.p1_Ms10s[playerLapIndex] = UI_LAP_TIME_DECIMAL_BASE - 1;
		sdata->LapTimes.p1_Ms1s[playerLapIndex] = UI_LAP_TIME_DECIMAL_BASE - 1;
		return;
	}

	sdata->LapTimes.p1_Sec10s[playerLapIndex] = (lapTime / UI_LAP_TIME_TICKS_PER_TEN_SECONDS) % UI_LAP_TIME_SECONDS_TENS_MOD;
	sdata->LapTimes.p1_Sec1s[playerLapIndex] = (lapTime / UI_LAP_TIME_TICKS_PER_SECOND) % UI_LAP_TIME_DECIMAL_BASE;
	sdata->LapTimes.p1_Ms10s[playerLapIndex] = (CTR_MipsMulLo(lapTime, UI_LAP_TIME_DECIMAL_BASE) / UI_LAP_TIME_TICKS_PER_SECOND) % UI_LAP_TIME_DECIMAL_BASE;
	sdata->LapTimes.p1_Ms1s[playerLapIndex] = (CTR_MipsMulLo(lapTime, UI_LAP_TIME_CENTISECOND_SCALE) / UI_LAP_TIME_TICKS_PER_SECOND) % UI_LAP_TIME_DECIMAL_BASE;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004c718-0x8004c850.
void UI_ThTick_CountPickup(struct Thread *bucket)
{
	struct GameTracker *gGT = sdata->gGT;
	struct UiElement3D *obj = bucket->object;
	struct Instance *inst = bucket->inst;
	b32 isTimeCrate = inst->model->id == STATIC_TIME_CRATE_01;

	inst->colorRGBA = UI_REWARD_PICKUP_COLOR;

	if ((gGT->numPlyrCurrGame == 1) && !isTimeCrate)
	{
		inst->alphaScale = (gGT->drivers[0]->numWumpas < DRIVER_WUMPA_JUICED_COUNT)
		                       ? 0
		                       : ((s16)sdata->wumpaShineResult - UI_REWARD_WUMPA_SHINE_CENTER) << UI_REWARD_WUMPA_SHINE_SHIFT;
	}

	obj->rot.y += isTimeCrate ? UI_REWARD_PICKUP_ROT_SLOW : UI_REWARD_PICKUP_ROT_FAST;

	MATRIX *mat = &inst->matrix;

	ConvertRotToMatrix(mat, &obj->rot);

	MatrixRotate(mat, &obj->m, mat);

	u32 drawOtagState = CTR_ReadU32LE(&gGT->bool_DrawOTag_InProgress);
	if ((drawOtagState & UI_REWARD_HUD_VISIBLE_WORD_MASK) == UI_REWARD_HUD_VISIBLE_WORD_VALUE)
	{
		inst->flags &= ~HIDE_MODEL;
	}
	else
	{
		inst->flags |= HIDE_MODEL;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004c850-0x8004c914.
void UI_ThTick_Reward(struct Thread *bucket)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Instance *inst = bucket->inst;
	struct UiElement3D *obj = bucket->object;

	obj->rot.y += UI_REWARD_PICKUP_ROT_SLOW;

	Vector_SpecLightSpin2D(inst, &obj->rot, &obj->lightDir);

	MATRIX *mat = &inst->matrix;

	ConvertRotToMatrix(mat, &obj->rot);

	MatrixRotate(mat, &obj->m, mat);

	u32 drawOtagState = CTR_ReadU32LE(&gGT->bool_DrawOTag_InProgress);
	if (((drawOtagState & UI_REWARD_HUD_VISIBLE_WORD_MASK) == UI_REWARD_HUD_VISIBLE_WORD_VALUE) &&
	    (UI_REWARD_FADE_VISIBLE_MIN < gGT->pushBuffer_UI.fadeFromBlack_currentValue))
	{
		inst->flags &= ~HIDE_MODEL;
	}
	else
	{
		inst->flags |= HIDE_MODEL;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004c914-0x8004ca04.
void UI_ThTick_CtrLetters(struct Thread *bucket)
{
	struct Instance *inst = bucket->inst;
	struct UiElement3D *obj = bucket->object;

	obj->rot.y += UI_REWARD_PICKUP_ROT_SLOW;

	Vector_SpecLightSpin2D(inst, &obj->rot, &obj->lightDir);

	if (((sdata->gGT->gameMode1 & END_OF_RACE) != 0) && RaceFlag_IsTransitioning())
	{
		inst->scale.x = 0;
		inst->scale.y = 0;
		inst->scale.z = 0;
	}

	SVec3 rot;
	if (inst->scale.x == UI_REWARD_CTR_LETTER_BASE_SCALE)
	{
		rot.y = 0;
	}
	else
	{
		int endOfRaceTransition = (int)inst->scale.x - UI_REWARD_CTR_LETTER_BASE_SCALE;
		if (endOfRaceTransition < 0)
		{
			endOfRaceTransition = (int)inst->scale.x - UI_REWARD_CTR_LETTER_NEGATIVE_SCALE_BIAS;
		}
		rot.y = ((s16)(endOfRaceTransition >> UI_REWARD_CTR_LETTER_ROT_SHIFT) + 1) * UI_REWARD_CTR_LETTER_ROT_STEP;
	}
	rot.x = 0;
	rot.z = 0;

	ConvertRotToMatrix(&inst->matrix, &rot);

	MatrixRotate(&inst->matrix, &obj->m, &inst->matrix);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004ca04-0x8004caa8.
void UI_ThTick_big1(struct Thread *bucket)
{
	struct UiElement3D *obj = bucket->object;
	struct Instance *inst = bucket->inst;

	s16 scale = obj->scale;
	CTR_WriteU32LE(&inst->matrix.m[0][0], scale);
	CTR_WriteU32LE(&inst->matrix.m[0][2], 0);
	CTR_WriteU32LE(&inst->matrix.m[1][1], scale);
	CTR_WriteU32LE(&inst->matrix.m[2][0], 0);
	inst->matrix.m[2][2] = scale;

	MatrixRotate(&inst->matrix, &obj->m, &inst->matrix);

	u32 drawOtagState = CTR_ReadU32LE(&sdata->gGT->bool_DrawOTag_InProgress);
	if ((drawOtagState & UI_REWARD_HUD_VISIBLE_WORD_MASK) == UI_REWARD_HUD_VISIBLE_WORD_VALUE)
	{
		inst->flags &= ~HIDE_MODEL;
	}
	else
	{
		inst->flags |= HIDE_MODEL;
	}
}
