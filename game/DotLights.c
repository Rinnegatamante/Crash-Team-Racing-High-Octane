#include <common.h>

enum
{
	DOT_LIGHT_COUNT = 4,
	DOT_LIGHT_GREEN_INDEX = 3,

	DOT_LIGHT_SCALE_1P = FP(1.0),
	DOT_LIGHT_SCALE_2P = 0xaaa,
	DOT_LIGHT_SCALE_3P4P = FP(0.5),

	DOT_LIGHT_STATE_OFF = 0,
	DOT_LIGHT_STATE_ON = 1,
	DOT_LIGHT_GREEN_ICON_OFFSET = 2,

	DOT_LIGHT_TIMER_SHOW_MIN = -0x3bf,
	DOT_LIGHT_TIMER_GREEN = 0,
	DOT_LIGHT_TIMER_RED3 = 0x3c0,
	DOT_LIGHT_TIMER_RED2 = 0x780,
	DOT_LIGHT_TIMER_RED1 = 0xb40,
	DOT_LIGHT_TIMER_HIDE = 0xf00,

	DOT_LIGHT_TWEEN_RECIPROCAL_960 = -0x77777777,
	DOT_LIGHT_TWEEN_SHIFT = 9,

	DOT_LIGHT_FX_COUNTDOWN = 0x45,
	DOT_LIGHT_FX_GREEN = 0x46,
	DOT_LIGHT_FX_VOL = 0,
};

CTR_STATIC_ASSERT(DOT_LIGHT_TIMER_SHOW_MIN == -0x3bf);
CTR_STATIC_ASSERT(DOT_LIGHT_TIMER_GREEN == 0);
CTR_STATIC_ASSERT(DOT_LIGHT_TIMER_RED3 == 0x3c0);
CTR_STATIC_ASSERT(DOT_LIGHT_TIMER_RED2 == 0x780);
CTR_STATIC_ASSERT(DOT_LIGHT_TIMER_RED1 == 0xb40);
CTR_STATIC_ASSERT(DOT_LIGHT_TIMER_HIDE == 0xf00);
CTR_STATIC_ASSERT(DOT_LIGHT_TWEEN_RECIPROCAL_960 == -0x77777777);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002406c-0x800242b8.
void DotLights_Video(struct GameTracker *gGT, s32 red1, s32 red2, s32 red3, s32 green, s32 posY)
{
	if (gGT->numPlyrCurrGame == 0)
	{
		return;
	}

	s32 iconState[DOT_LIGHT_COUNT];
	iconState[0] = red1;
	iconState[1] = red2;
	iconState[2] = red3;
	iconState[DOT_LIGHT_GREEN_INDEX] = green;

	for (s32 playerIndex = 0; playerIndex < gGT->numPlyrCurrGame; playerIndex++)
	{
		struct PushBuffer *pb = &gGT->pushBuffer[playerIndex];

		s32 scale = DOT_LIGHT_SCALE_3P4P;
		if (gGT->numPlyrCurrGame == 1)
		{
			scale = DOT_LIGHT_SCALE_1P;
		}
		else if (gGT->numPlyrCurrGame == 2)
		{
			scale = DOT_LIGHT_SCALE_2P;
		}

		struct Icon *icon = gGT->trafficLightIcon[0];
		s32 sizeX = FP_Mult(icon->texLayout.u1 - icon->texLayout.u0, scale);

		s32 newPosX = (pb->rect.w - (sizeX * DOT_LIGHT_COUNT)) / 2;
		s32 newPosY = FP_Mult(pb->rect.h / 3, posY) - FP_Mult(icon->texLayout.v2 - icon->texLayout.v0, scale);

		for (s32 lightIndex = 0; lightIndex < DOT_LIGHT_COUNT; lightIndex++)
		{
			s32 iconIndex = iconState[lightIndex] + (DOT_LIGHT_GREEN_ICON_OFFSET * (lightIndex == DOT_LIGHT_GREEN_INDEX));
			DecalHUD_DrawPolyFT4(gGT->trafficLightIcon[iconIndex], newPosX + (sizeX * lightIndex), newPosY, &gGT->backBuffer->primMem, pb->ptrOT, 0, scale);
		}
	}
}


static s32 DotLights_TweenPos(s32 value)
{
	s32 quotient = (s32)((s64)((s64)value * DOT_LIGHT_TWEEN_RECIPROCAL_960) >> 0x20);
	return ((quotient + value) >> DOT_LIGHT_TWEEN_SHIFT) - (value >> 0x1f);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800242b8-0x80024464.
void DotLights_AudioAndVideo(struct GameTracker *gGT)
{
	s32 timer = gGT->trafficLightsTimer;
	s32 red1;
	s32 red2;
	s32 red3;
	s32 green;
	s32 posY;

	if (timer >= DOT_LIGHT_TIMER_SHOW_MIN)
	{
		if (timer < DOT_LIGHT_TIMER_GREEN + 1)
		{
			if (sdata->trafficLightsTimer_prevFrame > DOT_LIGHT_TIMER_GREEN)
			{
				OtherFX_Play(DOT_LIGHT_FX_GREEN, DOT_LIGHT_FX_VOL);
			}

			red1 = DOT_LIGHT_STATE_ON;
			red2 = DOT_LIGHT_STATE_ON;
			red3 = DOT_LIGHT_STATE_ON;
			green = DOT_LIGHT_STATE_ON;
			posY = DotLights_TweenPos((gGT->trafficLightsTimer + DOT_LIGHT_TIMER_RED3) * FP(1.0));
		}
		else if (timer < DOT_LIGHT_TIMER_RED3 + 1)
		{
			if (sdata->trafficLightsTimer_prevFrame > DOT_LIGHT_TIMER_RED3)
			{
				OtherFX_Play(DOT_LIGHT_FX_COUNTDOWN, DOT_LIGHT_FX_VOL);
			}

			red1 = DOT_LIGHT_STATE_ON;
			red2 = DOT_LIGHT_STATE_ON;
			red3 = DOT_LIGHT_STATE_ON;
			green = DOT_LIGHT_STATE_OFF;
			posY = FP(1.0);
		}
		else if (timer < DOT_LIGHT_TIMER_RED2 + 1)
		{
			if (sdata->trafficLightsTimer_prevFrame > DOT_LIGHT_TIMER_RED2)
			{
				OtherFX_Play(DOT_LIGHT_FX_COUNTDOWN, DOT_LIGHT_FX_VOL);
			}

			red1 = DOT_LIGHT_STATE_ON;
			red2 = DOT_LIGHT_STATE_ON;
			red3 = DOT_LIGHT_STATE_OFF;
			green = DOT_LIGHT_STATE_OFF;
			posY = FP(1.0);
		}
		else if (timer < DOT_LIGHT_TIMER_RED1 + 1)
		{
			if (sdata->trafficLightsTimer_prevFrame > DOT_LIGHT_TIMER_RED1)
			{
				OtherFX_Play(DOT_LIGHT_FX_COUNTDOWN, DOT_LIGHT_FX_VOL);
			}

			red1 = DOT_LIGHT_STATE_ON;
			red2 = DOT_LIGHT_STATE_OFF;
			red3 = DOT_LIGHT_STATE_OFF;
			green = DOT_LIGHT_STATE_OFF;
			posY = FP(1.0);
		}
		else
		{
			red1 = DOT_LIGHT_STATE_OFF;
			red2 = DOT_LIGHT_STATE_OFF;
			red3 = DOT_LIGHT_STATE_OFF;
			green = DOT_LIGHT_STATE_OFF;
			posY = DotLights_TweenPos((DOT_LIGHT_TIMER_HIDE - gGT->trafficLightsTimer) * FP(1.0));
		}

		DotLights_Video(gGT, red1, red2, red3, green, posY);
	}

	sdata->trafficLightsTimer_prevFrame = gGT->trafficLightsTimer;
}
