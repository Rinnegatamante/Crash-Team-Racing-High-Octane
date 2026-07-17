#include <common.h>

enum
{
	RACING_WHEEL_DEFAULT_CENTER = 0x80,
	RACING_WHEEL_DEFAULT_DEAD_ZONE = 0x30,
	RACING_WHEEL_DEFAULT_RANGE = 0x7f,
	// NOTE(aalhendi): Retail's null-wheel strength path uses 0x5e directly,
	// not default range minus default dead zone.
	RACING_WHEEL_DEFAULT_STRENGTH_DISTANCE = 0x5e,
	JOYSTICK_STRENGTH_CURVE_SEGMENTS = 5,
};

CTR_STATIC_ASSERT(RACING_WHEEL_DEFAULT_CENTER == 0x80);
CTR_STATIC_ASSERT(RACING_WHEEL_DEFAULT_DEAD_ZONE == 0x30);
CTR_STATIC_ASSERT(RACING_WHEEL_DEFAULT_RANGE == 0x7f);
CTR_STATIC_ASSERT(RACING_WHEEL_DEFAULT_STRENGTH_DISTANCE == 0x5e);
CTR_STATIC_ASSERT(JOYSTICK_STRENGTH_CURVE_SEGMENTS == 5);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8006163c-0x800616b0.
int VehPhysJoystick_ReturnToRest(int stickVal, int half, struct RacingWheelData *rwd)
{
	int center = RACING_WHEEL_DEFAULT_CENTER;
	int deadZone = RACING_WHEEL_DEFAULT_DEAD_ZONE;
	int range = RACING_WHEEL_DEFAULT_RANGE;

	if (rwd != 0)
	{
		center = rwd->gamepadCenter;
		deadZone = rwd->deadZone;
		range = rwd->range;
	}

	stickVal = CTR_MipsSubLo(stickVal, center);
	if (stickVal < 0)
	{
		return CTR_MipsNegLo(VehCalc_MapToRange(CTR_MipsNegLo(stickVal), deadZone, range, 0, half));
	}

	return VehCalc_MapToRange(stickVal, deadZone, range, 0, half);
}

struct RacingWheelData rwd_default = {
    .gamepadCenter = RACING_WHEEL_DEFAULT_CENTER,
    .deadZone = RACING_WHEEL_DEFAULT_DEAD_ZONE,
    .range = RACING_WHEEL_DEFAULT_RANGE,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800616b0-0x800617cc.
int VehPhysJoystick_GetStrength(int val, int max, struct RacingWheelData *rwd)
{
	int dead = RACING_WHEEL_DEFAULT_DEAD_ZONE;
	int range = RACING_WHEEL_DEFAULT_RANGE;
	int dist = RACING_WHEEL_DEFAULT_STRENGTH_DISTANCE;
	if (rwd != 0)
	{
		dead = rwd->deadZone;
		range = rwd->range;
		dist = CTR_MipsSubLo(range, dead);
	}

	if (val < dead)
	{
		return 0;
	}

	dead = CTR_MipsSubLo(val, dead);

	if (range <= val)
	{
		return max;
	}

	int halfDist = CTR_MipsSra(CTR_MipsAddLo(dist, (u32)dist >> 31), 1);
	int maxFifth = max / JOYSTICK_STRENGTH_CURVE_SEGMENTS;

	if (halfDist <= dead)
	{
		dead = CTR_MipsSubLo(dead, halfDist);
		dead = CTR_MipsMulLo(dead, CTR_MipsSubLo(max, maxFifth));
		dead = CTR_MipsSll(dead, 1);
		return CTR_MipsAddLo(CTR_MipsDiv(dead, dist), maxFifth);
	}

	dead = CTR_MipsMulLo(dead, CTR_MipsSll(maxFifth, 1));
	return CTR_MipsDiv(dead, dist);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800617cc-0x8006181c.
int VehPhysJoystick_GetStrengthAbsolute(int stickVal, int maxSteer, struct RacingWheelData *rwd)
{
	int center = RACING_WHEEL_DEFAULT_CENTER;
	if (rwd != NULL)
	{
		center = rwd->gamepadCenter;
	}

	int distFromCenter = CTR_MipsSubLo(stickVal, center);

	// if steering right
	if (distFromCenter < 0)
	{
		return CTR_MipsNegLo(VehPhysJoystick_GetStrength(CTR_MipsNegLo(distFromCenter), maxSteer, rwd));
	}

	// steer left
	return VehPhysJoystick_GetStrength(distFromCenter, maxSteer, rwd);
}
