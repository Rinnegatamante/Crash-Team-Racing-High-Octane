#include <platform/native_input.h>

#include <macros.h>
#include "platform/native_adhoc.h"
#include "psx/libpad.h"

#include <SDL3/SDL.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __vita__
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#endif

#define NATIVE_INPUT_MAX_CONTROLLERS       PLATFORM_INPUT_PAD_COUNT
#define NATIVE_INPUT_PHYSICAL_SLOT_COUNT   2
#define NATIVE_INPUT_PAD_PACKET_BYTES      8
#define NATIVE_INPUT_MULTITAP_HEADER       2
#define NATIVE_INPUT_PAD_DIGITAL           0x41
#define NATIVE_INPUT_PAD_ANALOG            0x73
#define NATIVE_INPUT_PAD_MULTITAP          0x80
#define NATIVE_INPUT_PAD_DISCONNECT        0xff
#define NATIVE_INPUT_RAW_START             0x0008
#define NATIVE_INPUT_AXIS_DEADZONE         500
#define NATIVE_INPUT_MAP_FLAG_AXIS         0x4000
#define NATIVE_INPUT_MAP_FLAG_INVERSE      0x8000
#define NATIVE_INPUT_DEFAULT_KEYBOARD_SLOT 0
#define NATIVE_INPUT_KBM_MOUSE_FLAG         0x10000
#define NATIVE_INPUT_BINDING_OVERRIDE_NONE  0
#define NATIVE_INPUT_BINDING_OVERRIDE_SET   1
#define NATIVE_INPUT_CAPTURE_THRESHOLD      16384
#ifdef __vita__
#define NATIVE_INPUT_VITA_TOUCH_FLAG        0x10000000
#define NATIVE_INPUT_VITA_TOUCH_FRONT_LEFT  (NATIVE_INPUT_VITA_TOUCH_FLAG | 1)
#define NATIVE_INPUT_VITA_TOUCH_FRONT_RIGHT (NATIVE_INPUT_VITA_TOUCH_FLAG | 2)
#define NATIVE_INPUT_VITA_TOUCH_REAR_LEFT   (NATIVE_INPUT_VITA_TOUCH_FLAG | 3)
#define NATIVE_INPUT_VITA_TOUCH_REAR_RIGHT  (NATIVE_INPUT_VITA_TOUCH_FLAG | 4)
#define NATIVE_INPUT_VITA_TOUCH_FRONT_L_BIT (1u << 0)
#define NATIVE_INPUT_VITA_TOUCH_FRONT_R_BIT (1u << 1)
#define NATIVE_INPUT_VITA_TOUCH_REAR_L_BIT  (1u << 2)
#define NATIVE_INPUT_VITA_TOUCH_REAR_R_BIT  (1u << 3)
#endif
// NOTE(aalhendi): Little-endian tag `CTRI` = CTR native Input snapshot.
#define NATIVE_INPUT_STATE_MAGIC           0x49525443
#define NATIVE_INPUT_STATE_VERSION         1

// NOTE(aalhendi): Native input preserves behavior from PsyCross's
// MIT-licensed pad implementation while moving host ownership into ctr-native.
// See THIRD_PARTY_NOTICES.md.

struct NativeInputKeyboardMapping
{
	s32 id;

	s32 kc_square, kc_circle, kc_triangle, kc_cross;

	s32 kc_l1, kc_l2, kc_l3;
	s32 kc_r1, kc_r2, kc_r3;

	s32 kc_start, kc_select;

	s32 kc_dpad_left, kc_dpad_right, kc_dpad_up, kc_dpad_down;
};

struct NativeInputControllerMapping
{
	s32 id;

	s32 gc_square, gc_circle, gc_triangle, gc_cross;

	s32 gc_l1, gc_l2, gc_l3;
	s32 gc_r1, gc_r2, gc_r3;

	s32 gc_start, gc_select;

	s32 gc_dpad_left, gc_dpad_right, gc_dpad_up, gc_dpad_down;

	s32 gc_axis_left_x, gc_axis_left_y;
	s32 gc_axis_right_x, gc_axis_right_y;
};

struct NativeInputController
{
	SDL_JoystickID instanceId;
	SDL_Gamepad *controller;
	s32 analogEnabled;
	s32 switchingAnalog;
	struct PlatformInputPadSnapshot snapshot;
};

struct NativeInputControllerStateSnapshot
{
	struct PlatformInputPadSnapshot snapshot;
	s32 analogEnabled;
	s32 switchingAnalog;
	s32 controllerToSlotMapping;
};

struct NativeInputStateSnapshot
{
	u32 magic;
	u32 version;
	u32 size;
	s32 keyboardControllerSlot;
	s32 lastActiveControllerSlot;
	s32 installedSnapshotsActive;
	struct PlatformInputPadSnapshot installedSnapshots[NATIVE_INPUT_MAX_CONTROLLERS];
	struct NativeInputControllerStateSnapshot controllers[NATIVE_INPUT_MAX_CONTROLLERS];
};

global_variable struct NativeInputControllerMapping s_controllerMapping;
global_variable struct NativeInputKeyboardMapping s_keyboardMapping;
global_variable s32 s_controllerToSlotMapping[NATIVE_INPUT_MAX_CONTROLLERS] = {-1, -1, -1, -1};

global_variable struct NativeInputController s_controllers[NATIVE_INPUT_MAX_CONTROLLERS];
global_variable struct PlatformInputPadSnapshot s_installedSnapshots[NATIVE_INPUT_MAX_CONTROLLERS];
global_variable u8 *s_padSlotData[NATIVE_INPUT_PHYSICAL_SLOT_COUNT];
global_variable const bool *s_keyboardState;
global_variable s32 s_inputInitialized;
global_variable s32 s_installedSnapshotsActive;
global_variable s32 s_keyboardControllerSlot = NATIVE_INPUT_DEFAULT_KEYBOARD_SLOT;
global_variable s32 s_lastActiveControllerSlot = -1;

extern s32 g_padCommEnable;

static const u16 s_bindingRawMasks[PLATFORM_INPUT_BIND_ACTION_COUNT] =
{
	0x4000, // Cross
	0x8000, // Square
	0x2000, // Circle
	0x1000, // Triangle
	0x0400, // L1
	0x0800, // R1
	0x0100, // L2
	0x0200, // R2
	0x0010, // Up
	0x0040, // Down
	0x0080, // Left
	0x0020, // Right
	0x0008, // Start
};

static const char *s_bindingConfigSuffix[PLATFORM_INPUT_BIND_ACTION_COUNT] =
{
	"cross", "square", "circle", "triangle",
	"l1", "r1", "l2", "r2",
	"up", "down", "left", "right",
	"start",
};

global_variable s32 s_bindingOverrideValue[PLATFORM_INPUT_BINDING_DEVICE_COUNT][PLATFORM_INPUT_BIND_ACTION_COUNT];
global_variable u8 s_bindingOverrideState[PLATFORM_INPUT_BINDING_DEVICE_COUNT][PLATFORM_INPUT_BIND_ACTION_COUNT];
global_variable s32 s_bindingCaptureDevice = -1;
global_variable s32 s_bindingCaptureArmed;

#ifdef __vita__
global_variable u32 s_vitaTouchMask;
global_variable s32 s_vitaTouchFrontMidX = 960;
global_variable s32 s_vitaTouchRearMidX = 960;
global_variable SceCtrlData s_vitaLastPad;
global_variable s32 s_vitaLastPadValid;
#endif

internal s32 *NativeInput_KeyboardBindingPtr(s32 action)
{
	switch (action)
	{
	case PLATFORM_INPUT_BIND_CROSS: return &s_keyboardMapping.kc_cross;
	case PLATFORM_INPUT_BIND_SQUARE: return &s_keyboardMapping.kc_square;
	case PLATFORM_INPUT_BIND_CIRCLE: return &s_keyboardMapping.kc_circle;
	case PLATFORM_INPUT_BIND_TRIANGLE: return &s_keyboardMapping.kc_triangle;
	case PLATFORM_INPUT_BIND_L1: return &s_keyboardMapping.kc_l1;
	case PLATFORM_INPUT_BIND_R1: return &s_keyboardMapping.kc_r1;
	case PLATFORM_INPUT_BIND_L2: return &s_keyboardMapping.kc_l2;
	case PLATFORM_INPUT_BIND_R2: return &s_keyboardMapping.kc_r2;
	case PLATFORM_INPUT_BIND_UP: return &s_keyboardMapping.kc_dpad_up;
	case PLATFORM_INPUT_BIND_DOWN: return &s_keyboardMapping.kc_dpad_down;
	case PLATFORM_INPUT_BIND_LEFT: return &s_keyboardMapping.kc_dpad_left;
	case PLATFORM_INPUT_BIND_RIGHT: return &s_keyboardMapping.kc_dpad_right;
	case PLATFORM_INPUT_BIND_START: return &s_keyboardMapping.kc_start;
	default: return NULL;
	}
}

internal s32 *NativeInput_ControllerBindingPtr(s32 action)
{
	switch (action)
	{
	case PLATFORM_INPUT_BIND_CROSS: return &s_controllerMapping.gc_cross;
	case PLATFORM_INPUT_BIND_SQUARE: return &s_controllerMapping.gc_square;
	case PLATFORM_INPUT_BIND_CIRCLE: return &s_controllerMapping.gc_circle;
	case PLATFORM_INPUT_BIND_TRIANGLE: return &s_controllerMapping.gc_triangle;
	case PLATFORM_INPUT_BIND_L1: return &s_controllerMapping.gc_l1;
	case PLATFORM_INPUT_BIND_R1: return &s_controllerMapping.gc_r1;
	case PLATFORM_INPUT_BIND_L2: return &s_controllerMapping.gc_l2;
	case PLATFORM_INPUT_BIND_R2: return &s_controllerMapping.gc_r2;
	case PLATFORM_INPUT_BIND_UP: return &s_controllerMapping.gc_dpad_up;
	case PLATFORM_INPUT_BIND_DOWN: return &s_controllerMapping.gc_dpad_down;
	case PLATFORM_INPUT_BIND_LEFT: return &s_controllerMapping.gc_dpad_left;
	case PLATFORM_INPUT_BIND_RIGHT: return &s_controllerMapping.gc_dpad_right;
	case PLATFORM_INPUT_BIND_START: return &s_controllerMapping.gc_start;
	default: return NULL;
	}
}

internal s32 *NativeInput_BindingPtr(s32 action, s32 device)
{
	if ((action < 0) || (action >= PLATFORM_INPUT_BIND_ACTION_COUNT))
	{
		return NULL;
	}
	if (device == PLATFORM_INPUT_BINDING_KBM)
	{
		return NativeInput_KeyboardBindingPtr(action);
	}
	if (device == PLATFORM_INPUT_BINDING_CONTROLLER)
	{
		return NativeInput_ControllerBindingPtr(action);
	}
	return NULL;
}

void Platform_InputGetBindingConfigKey(int action, int device, char *dst, int dstSize)
{
	const char *prefix;
	if ((dst == NULL) || (dstSize <= 0))
	{
		return;
	}
	dst[0] = 0;
	if ((action < 0) || (action >= PLATFORM_INPUT_BIND_ACTION_COUNT))
	{
		return;
	}
	prefix = device == PLATFORM_INPUT_BINDING_KBM ? "bind_kb_" : device == PLATFORM_INPUT_BINDING_CONTROLLER ? "bind_pad_" : NULL;
	if (prefix == NULL)
	{
		return;
	}
	snprintf(dst, (size_t)dstSize, "%s%s", prefix, s_bindingConfigSuffix[action]);
}

int Platform_InputConfigSetBinding(const char *key, int value)
{
	char candidate[32];
	if (key == NULL)
	{
		return 0;
	}
	for (s32 device = 0; device < PLATFORM_INPUT_BINDING_DEVICE_COUNT; device++)
	{
#ifdef __vita__
		if (device == PLATFORM_INPUT_BINDING_KBM)
		{
			continue;
		}
#endif
		for (s32 action = 0; action < PLATFORM_INPUT_BIND_ACTION_COUNT; action++)
		{
			Platform_InputGetBindingConfigKey(action, device, candidate, sizeof(candidate));
			if (strcmp(key, candidate) == 0)
			{
				s_bindingOverrideValue[device][action] = value;
				s_bindingOverrideState[device][action] = NATIVE_INPUT_BINDING_OVERRIDE_SET;
				if (s_inputInitialized != 0)
				{
					s32 *binding = NativeInput_BindingPtr(action, device);
					if (binding != NULL)
					{
						*binding = value;
					}
				}
				return 1;
			}
		}
	}
	return 0;
}

int Platform_InputGetBinding(int action, int device)
{
	s32 *binding = NativeInput_BindingPtr(action, device);
	return binding != NULL ? *binding : -1;
}


int Platform_InputGetBindingOverlayButton(int action, int device)
{
	s32 binding = Platform_InputGetBinding(action, device);
	if (device != PLATFORM_INPUT_BINDING_CONTROLLER)
	{
		return PLATFORM_INPUT_OVERLAY_UNKNOWN;
	}

#ifdef __vita__
	switch (binding)
	{
	case SCE_CTRL_CROSS: return PLATFORM_INPUT_OVERLAY_SOUTH;
	case SCE_CTRL_SQUARE: return PLATFORM_INPUT_OVERLAY_WEST;
	case SCE_CTRL_CIRCLE: return PLATFORM_INPUT_OVERLAY_EAST;
	case SCE_CTRL_TRIANGLE: return PLATFORM_INPUT_OVERLAY_NORTH;
	case SCE_CTRL_L1: return PLATFORM_INPUT_OVERLAY_L1;
	case SCE_CTRL_R1: return PLATFORM_INPUT_OVERLAY_R1;
	case SCE_CTRL_L2: return PLATFORM_INPUT_OVERLAY_L2;
	case SCE_CTRL_R2: return PLATFORM_INPUT_OVERLAY_R2;
	case SCE_CTRL_UP: return PLATFORM_INPUT_OVERLAY_UP;
	case SCE_CTRL_DOWN: return PLATFORM_INPUT_OVERLAY_DOWN;
	case SCE_CTRL_LEFT: return PLATFORM_INPUT_OVERLAY_LEFT;
	case SCE_CTRL_RIGHT: return PLATFORM_INPUT_OVERLAY_RIGHT;
	case SCE_CTRL_START: return PLATFORM_INPUT_OVERLAY_START;
	case SCE_CTRL_SELECT: return PLATFORM_INPUT_OVERLAY_SELECT;
	case SCE_CTRL_L3: return PLATFORM_INPUT_OVERLAY_L3;
	case SCE_CTRL_R3: return PLATFORM_INPUT_OVERLAY_R3;
	case NATIVE_INPUT_VITA_TOUCH_FRONT_LEFT: return PLATFORM_INPUT_OVERLAY_TOUCH_FRONT_LEFT;
	case NATIVE_INPUT_VITA_TOUCH_FRONT_RIGHT: return PLATFORM_INPUT_OVERLAY_TOUCH_FRONT_RIGHT;
	case NATIVE_INPUT_VITA_TOUCH_REAR_LEFT: return PLATFORM_INPUT_OVERLAY_TOUCH_REAR_LEFT;
	case NATIVE_INPUT_VITA_TOUCH_REAR_RIGHT: return PLATFORM_INPUT_OVERLAY_TOUCH_REAR_RIGHT;
	default: return PLATFORM_INPUT_OVERLAY_UNKNOWN;
	}
#else
	if ((binding & NATIVE_INPUT_MAP_FLAG_AXIS) != 0)
	{
		s32 axis = binding & ~(NATIVE_INPUT_MAP_FLAG_AXIS | NATIVE_INPUT_MAP_FLAG_INVERSE);
		if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) return PLATFORM_INPUT_OVERLAY_L2;
		if (axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) return PLATFORM_INPUT_OVERLAY_R2;
		return PLATFORM_INPUT_OVERLAY_UNKNOWN;
	}

	switch (binding)
	{
	case SDL_GAMEPAD_BUTTON_SOUTH: return PLATFORM_INPUT_OVERLAY_SOUTH;
	case SDL_GAMEPAD_BUTTON_WEST: return PLATFORM_INPUT_OVERLAY_WEST;
	case SDL_GAMEPAD_BUTTON_EAST: return PLATFORM_INPUT_OVERLAY_EAST;
	case SDL_GAMEPAD_BUTTON_NORTH: return PLATFORM_INPUT_OVERLAY_NORTH;
	case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return PLATFORM_INPUT_OVERLAY_L1;
	case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return PLATFORM_INPUT_OVERLAY_R1;
	case SDL_GAMEPAD_BUTTON_DPAD_UP: return PLATFORM_INPUT_OVERLAY_UP;
	case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return PLATFORM_INPUT_OVERLAY_DOWN;
	case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return PLATFORM_INPUT_OVERLAY_LEFT;
	case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return PLATFORM_INPUT_OVERLAY_RIGHT;
	case SDL_GAMEPAD_BUTTON_START: return PLATFORM_INPUT_OVERLAY_START;
	case SDL_GAMEPAD_BUTTON_BACK: return PLATFORM_INPUT_OVERLAY_SELECT;
	case SDL_GAMEPAD_BUTTON_LEFT_STICK: return PLATFORM_INPUT_OVERLAY_L3;
	case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return PLATFORM_INPUT_OVERLAY_R3;
	default: return PLATFORM_INPUT_OVERLAY_UNKNOWN;
	}
#endif
}

void Platform_InputSetBinding(int action, int device, int value)
{
	s32 *binding = NativeInput_BindingPtr(action, device);
	if (binding == NULL)
	{
		return;
	}
	*binding = value;
	s_bindingOverrideValue[device][action] = value;
	s_bindingOverrideState[device][action] = NATIVE_INPUT_BINDING_OVERRIDE_SET;
}

internal void NativeInput_ApplyBindingOverrides(void)
{
	for (s32 device = 0; device < PLATFORM_INPUT_BINDING_DEVICE_COUNT; device++)
	{
#ifdef __vita__
		if (device == PLATFORM_INPUT_BINDING_KBM)
		{
			continue;
		}
#endif
		for (s32 action = 0; action < PLATFORM_INPUT_BIND_ACTION_COUNT; action++)
		{
			if (s_bindingOverrideState[device][action] == NATIVE_INPUT_BINDING_OVERRIDE_SET)
			{
				s32 *binding = NativeInput_BindingPtr(action, device);
				if (binding != NULL)
				{
					*binding = s_bindingOverrideValue[device][action];
				}
			}
		}
	}
}

internal u16 NativeInput_GetSnapshotButtons(const struct PlatformInputPadSnapshot *snapshot)
{
	return (u16)(snapshot->buttons[0] | (snapshot->buttons[1] << 8));
}

internal void NativeInput_SetSnapshotButtons(struct PlatformInputPadSnapshot *snapshot, u16 buttons)
{
	snapshot->buttons[0] = (u8)(buttons & 0xff);
	snapshot->buttons[1] = (u8)(buttons >> 8);
}

internal void NativeInput_ResetSnapshot(s32 slot)
{
	struct PlatformInputPadSnapshot *snapshot = &s_controllers[slot].snapshot;

	snapshot->connected = slot == 0;
	snapshot->status = snapshot->connected ? 0 : NATIVE_INPUT_PAD_DISCONNECT;
	snapshot->id = snapshot->connected ? NATIVE_INPUT_PAD_DIGITAL : NATIVE_INPUT_PAD_DISCONNECT;
	NativeInput_SetSnapshotButtons(snapshot, 0xffff);
	snapshot->analog[0] = 0x80;
	snapshot->analog[1] = 0x80;
	snapshot->analog[2] = 0x80;
	snapshot->analog[3] = 0x80;
	memset(snapshot->reserved, 0, sizeof(snapshot->reserved));
}

internal s32 NativeInput_IsValidControllerSlot(s32 slot)
{
	return (slot >= 0) && (slot < NATIVE_INPUT_MAX_CONTROLLERS);
}

internal s32 NativeInput_NextControllerSlot(s32 slot)
{
	slot++;
	if (slot >= NATIVE_INPUT_MAX_CONTROLLERS)
	{
		slot = 0;
	}

	return slot;
}

internal void NativeInput_MoveKeyboardOffControllerSlot(s32 slot)
{
	if (s_keyboardControllerSlot == slot)
	{
		s_keyboardControllerSlot = NativeInput_NextControllerSlot(s_keyboardControllerSlot);
	}
}

internal void NativeInput_MakeDisconnectedSnapshot(struct PlatformInputPadSnapshot *snapshot)
{
	if (snapshot == NULL)
	{
		return;
	}

	snapshot->connected = 0;
	snapshot->status = NATIVE_INPUT_PAD_DISCONNECT;
	snapshot->id = NATIVE_INPUT_PAD_DISCONNECT;
	NativeInput_SetSnapshotButtons(snapshot, 0xffff);
	snapshot->analog[0] = 0x80;
	snapshot->analog[1] = 0x80;
	snapshot->analog[2] = 0x80;
	snapshot->analog[3] = 0x80;
	memset(snapshot->reserved, 0, sizeof(snapshot->reserved));
}

internal void NativeInput_WritePadPacket(u8 *dst, const struct PlatformInputPadSnapshot *snapshot)
{
	if ((dst == NULL) || (snapshot == NULL))
	{
		return;
	}

	dst[0] = snapshot->status;
	dst[1] = snapshot->id;
	dst[2] = snapshot->buttons[0];
	dst[3] = snapshot->buttons[1];
	dst[4] = snapshot->analog[0];
	dst[5] = snapshot->analog[1];
	dst[6] = snapshot->analog[2];
	dst[7] = snapshot->analog[3];
}

internal s32 NativeInput_UseMultitapBus(void)
{
	s32 slot;

	for (slot = NATIVE_INPUT_PHYSICAL_SLOT_COUNT; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		if (s_controllers[slot].snapshot.connected != 0)
		{
			return 1;
		}
	}

	return 0;
}

internal void NativeInput_WritePadBus(void)
{
	u8 *slot0 = s_padSlotData[0];
	u8 *slot1 = s_padSlotData[1];
	s32 useMultitap = NativeInput_UseMultitapBus();
	s32 slot;

	if (slot0 != NULL)
	{
		if (useMultitap != 0)
		{
			slot0[0] = 0;
			slot0[1] = NATIVE_INPUT_PAD_MULTITAP;
			for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
			{
				NativeInput_WritePadPacket(&slot0[NATIVE_INPUT_MULTITAP_HEADER + (slot * NATIVE_INPUT_PAD_PACKET_BYTES)], &s_controllers[slot].snapshot);
			}
		}
		else
		{
			NativeInput_WritePadPacket(slot0, &s_controllers[0].snapshot);
		}
	}

	if (slot1 != NULL)
	{
		if (useMultitap != 0)
		{
			struct PlatformInputPadSnapshot disconnected;

			NativeInput_MakeDisconnectedSnapshot(&disconnected);
			NativeInput_WritePadPacket(slot1, &disconnected);
		}
		else
		{
			NativeInput_WritePadPacket(slot1, &s_controllers[1].snapshot);
		}
	}
}

internal void NativeInput_WriteInstalledSnapshots(void)
{
	s32 slot;

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		s_controllers[slot].snapshot = s_installedSnapshots[slot];
	}

	NativeInput_WritePadBus();
}

internal void NativeInput_DefaultMappings(void)
{
	s_keyboardMapping.kc_square = SDL_SCANCODE_X;
	s_keyboardMapping.kc_circle = SDL_SCANCODE_V;
	s_keyboardMapping.kc_triangle = SDL_SCANCODE_Z;
	s_keyboardMapping.kc_cross = SDL_SCANCODE_C;

	s_keyboardMapping.kc_l1 = SDL_SCANCODE_LSHIFT;
	s_keyboardMapping.kc_l2 = SDL_SCANCODE_LCTRL;
	s_keyboardMapping.kc_l3 = SDL_SCANCODE_LEFTBRACKET;

	s_keyboardMapping.kc_r1 = SDL_SCANCODE_RSHIFT;
	s_keyboardMapping.kc_r2 = SDL_SCANCODE_RCTRL;
	s_keyboardMapping.kc_r3 = SDL_SCANCODE_RIGHTBRACKET;

	s_keyboardMapping.kc_dpad_up = SDL_SCANCODE_UP;
	s_keyboardMapping.kc_dpad_down = SDL_SCANCODE_DOWN;
	s_keyboardMapping.kc_dpad_left = SDL_SCANCODE_LEFT;
	s_keyboardMapping.kc_dpad_right = SDL_SCANCODE_RIGHT;

	s_keyboardMapping.kc_select = SDL_SCANCODE_SPACE;
	s_keyboardMapping.kc_start = SDL_SCANCODE_RETURN;

#ifdef __vita__
	s_controllerMapping.gc_square = SCE_CTRL_SQUARE;
	s_controllerMapping.gc_circle = SCE_CTRL_CIRCLE;
	s_controllerMapping.gc_triangle = SCE_CTRL_TRIANGLE;
	s_controllerMapping.gc_cross = SCE_CTRL_CROSS;

	s_controllerMapping.gc_l1 = SCE_CTRL_L1;
	s_controllerMapping.gc_l2 = SCE_CTRL_L2;
	s_controllerMapping.gc_l3 = SCE_CTRL_L3;

	s_controllerMapping.gc_r1 = SCE_CTRL_R1;
	s_controllerMapping.gc_r2 = SCE_CTRL_R2;
	s_controllerMapping.gc_r3 = SCE_CTRL_R3;

	s_controllerMapping.gc_dpad_up = SCE_CTRL_UP;
	s_controllerMapping.gc_dpad_down = SCE_CTRL_DOWN;
	s_controllerMapping.gc_dpad_left = SCE_CTRL_LEFT;
	s_controllerMapping.gc_dpad_right = SCE_CTRL_RIGHT;

	s_controllerMapping.gc_select = SCE_CTRL_SELECT;
	s_controllerMapping.gc_start = SCE_CTRL_START;
#else
	s_controllerMapping.gc_square = SDL_GAMEPAD_BUTTON_WEST;
	s_controllerMapping.gc_circle = SDL_GAMEPAD_BUTTON_EAST;
	s_controllerMapping.gc_triangle = SDL_GAMEPAD_BUTTON_NORTH;
	s_controllerMapping.gc_cross = SDL_GAMEPAD_BUTTON_SOUTH;

	s_controllerMapping.gc_l1 = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
	s_controllerMapping.gc_l2 = SDL_GAMEPAD_AXIS_LEFT_TRIGGER | NATIVE_INPUT_MAP_FLAG_AXIS;
	s_controllerMapping.gc_l3 = SDL_GAMEPAD_BUTTON_LEFT_STICK;

	s_controllerMapping.gc_r1 = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
	s_controllerMapping.gc_r2 = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER | NATIVE_INPUT_MAP_FLAG_AXIS;
	s_controllerMapping.gc_r3 = SDL_GAMEPAD_BUTTON_RIGHT_STICK;

	s_controllerMapping.gc_dpad_up = SDL_GAMEPAD_BUTTON_DPAD_UP;
	s_controllerMapping.gc_dpad_down = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
	s_controllerMapping.gc_dpad_left = SDL_GAMEPAD_BUTTON_DPAD_LEFT;
	s_controllerMapping.gc_dpad_right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;

	s_controllerMapping.gc_select = SDL_GAMEPAD_BUTTON_BACK;
	s_controllerMapping.gc_start = SDL_GAMEPAD_BUTTON_START;

	s_controllerMapping.gc_axis_left_x = SDL_GAMEPAD_AXIS_LEFTX | NATIVE_INPUT_MAP_FLAG_AXIS;
	s_controllerMapping.gc_axis_left_y = SDL_GAMEPAD_AXIS_LEFTY | NATIVE_INPUT_MAP_FLAG_AXIS;
	s_controllerMapping.gc_axis_right_x = SDL_GAMEPAD_AXIS_RIGHTX | NATIVE_INPUT_MAP_FLAG_AXIS;
	s_controllerMapping.gc_axis_right_y = SDL_GAMEPAD_AXIS_RIGHTY | NATIVE_INPUT_MAP_FLAG_AXIS;
#endif

	NativeInput_ApplyBindingOverrides();
}

internal s32 NativeInput_ControllerButtonState(SDL_Gamepad *controller, s32 buttonOrAxis)
{
	if (controller == NULL)
	{
		return 0;
	}

	if ((buttonOrAxis & NATIVE_INPUT_MAP_FLAG_AXIS) != 0)
	{
		s32 axis = buttonOrAxis & ~(NATIVE_INPUT_MAP_FLAG_AXIS | NATIVE_INPUT_MAP_FLAG_INVERSE);
		s32 value = SDL_GetGamepadAxis(controller, (SDL_GamepadAxis)axis);

		if ((abs(value) > NATIVE_INPUT_AXIS_DEADZONE) && ((buttonOrAxis & NATIVE_INPUT_MAP_FLAG_INVERSE) != 0))
		{
			value *= -1;
		}

		return value;
	}

	if (buttonOrAxis < 0)
	{
		return 0;
	}

	return SDL_GetGamepadButton(controller, (SDL_GamepadButton)buttonOrAxis) * 32767;
}

internal u8 NativeInput_AxisToByte(s32 axis)
{
	s32 value = (axis / 256) + 128;

	if (value < 0)
	{
		return 0;
	}

	if (value > 0xff)
	{
		return 0xff;
	}

	return (u8)value;
}

internal s32 NativeInput_AxisIsActive(s32 axis)
{
	return abs(axis) > NATIVE_INPUT_AXIS_DEADZONE;
}

#ifndef __vita__
internal s32 NativeInput_KbmBindingState(s32 binding)
{
	if ((binding & NATIVE_INPUT_KBM_MOUSE_FLAG) != 0)
	{
		u32 mouse = SDL_GetMouseState(NULL, NULL);
		s32 button = binding & ~NATIVE_INPUT_KBM_MOUSE_FLAG;
		if ((button <= 0) || (button > 31))
		{
			return 0;
		}
		return (mouse & SDL_BUTTON_MASK(button)) != 0;
	}

	if ((s_keyboardState == NULL) || (binding <= SDL_SCANCODE_UNKNOWN) || (binding >= SDL_SCANCODE_COUNT))
	{
		return 0;
	}
	return s_keyboardState[binding] != 0;
}
#endif

#ifdef __vita__
internal void NativeInput_VitaPollTouch(void)
{
	SceTouchData touch;
	s_vitaTouchMask = 0;

	memset(&touch, 0, sizeof(touch));
	if (sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1) > 0)
	{
		for (u32 i = 0; i < touch.reportNum; i++)
		{
			if (touch.report[i].x < s_vitaTouchFrontMidX)
			{
				s_vitaTouchMask |= NATIVE_INPUT_VITA_TOUCH_FRONT_L_BIT;
			}
			else
			{
				s_vitaTouchMask |= NATIVE_INPUT_VITA_TOUCH_FRONT_R_BIT;
			}
		}
	}

	memset(&touch, 0, sizeof(touch));
	if (sceTouchPeek(SCE_TOUCH_PORT_BACK, &touch, 1) > 0)
	{
		for (u32 i = 0; i < touch.reportNum; i++)
		{
			if (touch.report[i].x < s_vitaTouchRearMidX)
			{
				s_vitaTouchMask |= NATIVE_INPUT_VITA_TOUCH_REAR_L_BIT;
			}
			else
			{
				s_vitaTouchMask |= NATIVE_INPUT_VITA_TOUCH_REAR_R_BIT;
			}
		}
	}
}

internal s32 NativeInput_VitaBindingState(s32 binding, const SceCtrlData *pad)
{
	if ((binding & NATIVE_INPUT_VITA_TOUCH_FLAG) != 0)
	{
		switch (binding)
		{
		case NATIVE_INPUT_VITA_TOUCH_FRONT_LEFT: return (s_vitaTouchMask & NATIVE_INPUT_VITA_TOUCH_FRONT_L_BIT) != 0;
		case NATIVE_INPUT_VITA_TOUCH_FRONT_RIGHT: return (s_vitaTouchMask & NATIVE_INPUT_VITA_TOUCH_FRONT_R_BIT) != 0;
		case NATIVE_INPUT_VITA_TOUCH_REAR_LEFT: return (s_vitaTouchMask & NATIVE_INPUT_VITA_TOUCH_REAR_L_BIT) != 0;
		case NATIVE_INPUT_VITA_TOUCH_REAR_RIGHT: return (s_vitaTouchMask & NATIVE_INPUT_VITA_TOUCH_REAR_R_BIT) != 0;
		default: return 0;
		}
	}
	return (pad != NULL) && ((((u32)binding) & pad->buttons) != 0);
}
#endif

internal void NativeInput_ApplyController(s32 slot)
{
	struct NativeInputController *nativeController = &s_controllers[slot];
	struct PlatformInputPadSnapshot *snapshot = &nativeController->snapshot;
	u16 buttons = 0xffff;
	s32 rightX;
	s32 rightY;
	s32 leftX;
	s32 leftY;

#ifdef __vita__
	SceCtrlData pad;
	if (sceCtrlPeekBufferPositiveExt2(slot ? slot + 1 : 0, &pad, 1) < 0)
	{
		return;
	}

	snapshot->connected = 1;
	snapshot->status = 0;
	snapshot->id = NATIVE_INPUT_PAD_ANALOG;

	for (s32 action = 0; action < PLATFORM_INPUT_BIND_ACTION_COUNT; action++)
	{
		s32 *binding = NativeInput_ControllerBindingPtr(action);
		if (binding == NULL)
		{
			continue;
		}
		if (((*binding & NATIVE_INPUT_VITA_TOUCH_FLAG) != 0) && (slot != 0))
		{
			continue;
		}
		if (NativeInput_VitaBindingState(*binding, &pad))
		{
			buttons &= (u16)~s_bindingRawMasks[action];
		}
	}

	// Select is intentionally not configurable, but keep the original raw
	// input available for code paths that still inspect the PSX Select bit.
	if ((pad.buttons & SCE_CTRL_SELECT) != 0)
	{
		buttons &= (u16)~0x0001;
	}

	if (slot == 0)
	{
		s_vitaLastPad = pad;
		s_vitaLastPadValid = 1;
	}

	rightX = ((s32)pad.rx - 128) * 256;
	rightY = ((s32)pad.ry - 128) * 256;
	leftX = ((s32)pad.lx - 128) * 256;
	leftY = ((s32)pad.ly - 128) * 256;
#else
	const struct NativeInputControllerMapping *mapping = &s_controllerMapping;
	SDL_Gamepad *controller = nativeController->controller;

	if ((controller == NULL) || (SDL_GamepadConnected(controller) == 0))
	{
		return;
	}

	snapshot->connected = 1;
	snapshot->status = 0;
	snapshot->id = nativeController->analogEnabled ? NATIVE_INPUT_PAD_ANALOG : NATIVE_INPUT_PAD_DIGITAL;

	for (s32 action = 0; action < PLATFORM_INPUT_BIND_ACTION_COUNT; action++)
	{
		s32 *binding = NativeInput_ControllerBindingPtr(action);
		if ((binding != NULL) && (NativeInput_ControllerButtonState(controller, *binding) > NATIVE_INPUT_CAPTURE_THRESHOLD))
		{
			buttons &= (u16)~s_bindingRawMasks[action];
		}
	}

	// Select stays on its original fixed host binding and is not exposed in
	// the rebinding screen. L3/R3 are not emitted as PSX actions.
	if (NativeInput_ControllerButtonState(controller, mapping->gc_select) > NATIVE_INPUT_CAPTURE_THRESHOLD)
	{
		buttons &= (u16)~0x0001;
	}

	rightX = NativeInput_ControllerButtonState(controller, mapping->gc_axis_right_x);
	rightY = NativeInput_ControllerButtonState(controller, mapping->gc_axis_right_y);
	leftX = NativeInput_ControllerButtonState(controller, mapping->gc_axis_left_x);
	leftY = NativeInput_ControllerButtonState(controller, mapping->gc_axis_left_y);
#endif

	if ((buttons != 0xffff) || NativeInput_AxisIsActive(rightX) || NativeInput_AxisIsActive(rightY) || NativeInput_AxisIsActive(leftX) ||
	    NativeInput_AxisIsActive(leftY))
	{
		s_lastActiveControllerSlot = slot;
	}

	if (((buttons & 0x1) == 0) && ((buttons & 0x8) == 0))
	{
		buttons = 0xffff;
		if (nativeController->switchingAnalog == 0)
		{
			nativeController->analogEnabled = nativeController->analogEnabled == 0;
		}
		nativeController->switchingAnalog = 1;
	}
	else
	{
		nativeController->switchingAnalog = 0;
	}

	NativeInput_SetSnapshotButtons(snapshot, buttons);
	snapshot->analog[0] = NativeInput_AxisToByte(rightX);
	snapshot->analog[1] = NativeInput_AxisToByte(rightY);
	snapshot->analog[2] = NativeInput_AxisToByte(leftX);
	snapshot->analog[3] = NativeInput_AxisToByte(leftY);
}

internal u16 NativeInput_ReadKeyboard(void)
{
	u16 buttons = 0xffff;
#ifndef __vita__
	if (s_keyboardState == NULL)
	{
		return buttons;
	}

	for (s32 action = 0; action < PLATFORM_INPUT_BIND_ACTION_COUNT; action++)
	{
		s32 *binding = NativeInput_KeyboardBindingPtr(action);
		if ((binding != NULL) && NativeInput_KbmBindingState(*binding))
		{
			buttons &= (u16)~s_bindingRawMasks[action];
		}
	}

	// Select remains fixed (Space by default) rather than being configurable.
	if (NativeInput_KbmBindingState(s_keyboardMapping.kc_select))
	{
		buttons &= (u16)~0x0001;
	}
#endif
	return buttons;
}

internal s32 NativeInput_KeyboardSuppressed(void)
{
	if (s_keyboardState == NULL)
	{
		return 0;
	}

	return s_keyboardState[SDL_SCANCODE_RALT] || s_keyboardState[SDL_SCANCODE_LALT];
}

internal void NativeInput_ApplyKeyboard(s32 slot, u16 keyboardButtons)
{
	struct PlatformInputPadSnapshot *snapshot = &s_controllers[slot].snapshot;
	u16 buttons;

	if (slot != s_keyboardControllerSlot)
	{
		return;
	}

	if (snapshot->connected == 0)
	{
		snapshot->connected = 1;
		snapshot->status = 0;
		snapshot->id = NATIVE_INPUT_PAD_DIGITAL;
	}

	buttons = NativeInput_GetSnapshotButtons(snapshot);
	NativeInput_SetSnapshotButtons(snapshot, buttons & keyboardButtons);
}

internal s32 NativeInput_FindActiveControllerSlot(void)
{
	s32 slot;

	if (NativeInput_IsValidControllerSlot(s_lastActiveControllerSlot) && (s_controllers[s_lastActiveControllerSlot].controller != NULL))
	{
		return s_lastActiveControllerSlot;
	}

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		if (s_controllers[slot].controller != NULL)
		{
			return slot;
		}
	}

	return -1;
}

internal void NativeInput_CopyUpperName(char *dst, s32 dstSize, const char *src)
{
	if ((dst == NULL) || (dstSize <= 0))
	{
		return;
	}
	if (src == NULL)
	{
		src = "UNBOUND";
	}
	s32 i = 0;
	for (; (i < dstSize - 1) && src[i]; i++)
	{
		char c = src[i] == '_' ? ' ' : src[i];
		dst[i] = (char)toupper((unsigned char)c);
	}
	dst[i] = 0;
}

void Platform_InputGetBindingName(int action, int device, char *dst, int dstSize)
{
	s32 binding = Platform_InputGetBinding(action, device);
	if ((dst == NULL) || (dstSize <= 0))
	{
		return;
	}
	dst[0] = 0;

	if (device == PLATFORM_INPUT_BINDING_KBM)
	{
#ifdef __vita__
		NativeInput_CopyUpperName(dst, dstSize, "N/A");
#else
		if ((binding & NATIVE_INPUT_KBM_MOUSE_FLAG) != 0)
		{
			switch (binding & ~NATIVE_INPUT_KBM_MOUSE_FLAG)
			{
			case SDL_BUTTON_LEFT: NativeInput_CopyUpperName(dst, dstSize, "MOUSE 1"); return;
			case SDL_BUTTON_RIGHT: NativeInput_CopyUpperName(dst, dstSize, "MOUSE 2"); return;
			case SDL_BUTTON_MIDDLE: NativeInput_CopyUpperName(dst, dstSize, "MOUSE 3"); return;
			case SDL_BUTTON_X1: NativeInput_CopyUpperName(dst, dstSize, "MOUSE 4"); return;
			case SDL_BUTTON_X2: NativeInput_CopyUpperName(dst, dstSize, "MOUSE 5"); return;
			default: snprintf(dst, (size_t)dstSize, "MOUSE %d", binding & ~NATIVE_INPUT_KBM_MOUSE_FLAG); return;
			}
		}
		if ((binding > SDL_SCANCODE_UNKNOWN) && (binding < SDL_SCANCODE_COUNT))
		{
			NativeInput_CopyUpperName(dst, dstSize, SDL_GetScancodeName((SDL_Scancode)binding));
			return;
		}
		NativeInput_CopyUpperName(dst, dstSize, "UNBOUND");
#endif
		return;
	}

	if (device != PLATFORM_INPUT_BINDING_CONTROLLER)
	{
		NativeInput_CopyUpperName(dst, dstSize, "UNBOUND");
		return;
	}

#ifdef __vita__
	switch (binding)
	{
	case SCE_CTRL_CROSS: NativeInput_CopyUpperName(dst, dstSize, "*"); return;
	case SCE_CTRL_CIRCLE: NativeInput_CopyUpperName(dst, dstSize, "@"); return;
	case SCE_CTRL_SQUARE: NativeInput_CopyUpperName(dst, dstSize, "["); return;
	case SCE_CTRL_TRIANGLE: NativeInput_CopyUpperName(dst, dstSize, "^"); return;
	case SCE_CTRL_L1: NativeInput_CopyUpperName(dst, dstSize, "L1"); return;
	case SCE_CTRL_R1: NativeInput_CopyUpperName(dst, dstSize, "R1"); return;
	case SCE_CTRL_L2: NativeInput_CopyUpperName(dst, dstSize, "L2"); return;
	case SCE_CTRL_R2: NativeInput_CopyUpperName(dst, dstSize, "R2"); return;
	case SCE_CTRL_L3: NativeInput_CopyUpperName(dst, dstSize, "L3"); return;
	case SCE_CTRL_R3: NativeInput_CopyUpperName(dst, dstSize, "R3"); return;
	case SCE_CTRL_UP: NativeInput_CopyUpperName(dst, dstSize, "DPAD UP"); return;
	case SCE_CTRL_DOWN: NativeInput_CopyUpperName(dst, dstSize, "DPAD DOWN"); return;
	case SCE_CTRL_LEFT: NativeInput_CopyUpperName(dst, dstSize, "DPAD LEFT"); return;
	case SCE_CTRL_RIGHT: NativeInput_CopyUpperName(dst, dstSize, "DPAD RIGHT"); return;
	case SCE_CTRL_START: NativeInput_CopyUpperName(dst, dstSize, "START"); return;
	case SCE_CTRL_SELECT: NativeInput_CopyUpperName(dst, dstSize, "SELECT"); return;
	case NATIVE_INPUT_VITA_TOUCH_FRONT_LEFT: NativeInput_CopyUpperName(dst, dstSize, "TOUCHSCREEN L"); return;
	case NATIVE_INPUT_VITA_TOUCH_FRONT_RIGHT: NativeInput_CopyUpperName(dst, dstSize, "TOUCHSCREEN R"); return;
	case NATIVE_INPUT_VITA_TOUCH_REAR_LEFT: NativeInput_CopyUpperName(dst, dstSize, "RETROTOUCH L"); return;
	case NATIVE_INPUT_VITA_TOUCH_REAR_RIGHT: NativeInput_CopyUpperName(dst, dstSize, "RETROTOUCH R"); return;
	default: NativeInput_CopyUpperName(dst, dstSize, "UNBOUND"); return;
	}
#else
	if ((binding & NATIVE_INPUT_MAP_FLAG_AXIS) != 0)
	{
		s32 axis = binding & ~(NATIVE_INPUT_MAP_FLAG_AXIS | NATIVE_INPUT_MAP_FLAG_INVERSE);
		if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER)
		{
			NativeInput_CopyUpperName(dst, dstSize, "L TRIGGER");
			return;
		}
		if (axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
		{
			NativeInput_CopyUpperName(dst, dstSize, "R TRIGGER");
			return;
		}
		NativeInput_CopyUpperName(dst, dstSize, SDL_GetGamepadStringForAxis((SDL_GamepadAxis)axis));
		return;
	}
	if ((binding >= 0) && (binding < SDL_GAMEPAD_BUTTON_COUNT))
	{
		NativeInput_CopyUpperName(dst, dstSize, SDL_GetGamepadStringForButton((SDL_GamepadButton)binding));
		return;
	}
	NativeInput_CopyUpperName(dst, dstSize, "UNBOUND");
#endif
}

#ifndef __vita__
internal s32 NativeInput_FirstActiveKbmBinding(s32 *binding)
{
	if (s_keyboardState != NULL)
	{
		for (s32 scancode = SDL_SCANCODE_UNKNOWN + 1; scancode < SDL_SCANCODE_COUNT; scancode++)
		{
			if (s_keyboardState[scancode])
			{
				if (binding != NULL) *binding = scancode;
				return 1;
			}
		}
	}
	u32 mouse = SDL_GetMouseState(NULL, NULL);
	for (s32 button = SDL_BUTTON_LEFT; button <= SDL_BUTTON_X2; button++)
	{
		if ((mouse & SDL_BUTTON_MASK(button)) != 0)
		{
			if (binding != NULL) *binding = NATIVE_INPUT_KBM_MOUSE_FLAG | button;
			return 1;
		}
	}
	return 0;
}

internal s32 NativeInput_FirstActiveControllerBinding(s32 *binding)
{
	for (s32 slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		SDL_Gamepad *controller = s_controllers[slot].controller;
		if ((controller == NULL) || !SDL_GamepadConnected(controller))
		{
			continue;
		}
		for (s32 button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; button++)
		{
			if (SDL_GetGamepadButton(controller, (SDL_GamepadButton)button))
			{
				if (binding != NULL) *binding = button;
				return 1;
			}
		}
		if (SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > NATIVE_INPUT_CAPTURE_THRESHOLD)
		{
			if (binding != NULL) *binding = SDL_GAMEPAD_AXIS_LEFT_TRIGGER | NATIVE_INPUT_MAP_FLAG_AXIS;
			return 1;
		}
		if (SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > NATIVE_INPUT_CAPTURE_THRESHOLD)
		{
			if (binding != NULL) *binding = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER | NATIVE_INPUT_MAP_FLAG_AXIS;
			return 1;
		}
	}
	return 0;
}
#else
static const s32 s_vitaBindableInputs[] =
{
	SCE_CTRL_CROSS, SCE_CTRL_CIRCLE, SCE_CTRL_SQUARE, SCE_CTRL_TRIANGLE,
	SCE_CTRL_L1, SCE_CTRL_R1, SCE_CTRL_L2, SCE_CTRL_R2, SCE_CTRL_L3, SCE_CTRL_R3,
	SCE_CTRL_UP, SCE_CTRL_DOWN, SCE_CTRL_LEFT, SCE_CTRL_RIGHT, SCE_CTRL_START, SCE_CTRL_SELECT,
	NATIVE_INPUT_VITA_TOUCH_FRONT_LEFT, NATIVE_INPUT_VITA_TOUCH_FRONT_RIGHT,
	NATIVE_INPUT_VITA_TOUCH_REAR_LEFT, NATIVE_INPUT_VITA_TOUCH_REAR_RIGHT,
};

internal s32 NativeInput_FirstActiveControllerBinding(s32 *binding)
{
	if (!s_vitaLastPadValid)
	{
		return 0;
	}
	for (u32 i = 0; i < sizeof(s_vitaBindableInputs) / sizeof(s_vitaBindableInputs[0]); i++)
	{
		if (NativeInput_VitaBindingState(s_vitaBindableInputs[i], &s_vitaLastPad))
		{
			if (binding != NULL) *binding = s_vitaBindableInputs[i];
			return 1;
		}
	}
	return 0;
}
#endif

int Platform_InputBindingIsActive(int device, int binding)
{
	if (device == PLATFORM_INPUT_BINDING_KBM)
	{
#ifdef __vita__
		return 0;
#else
		return NativeInput_KbmBindingState(binding);
#endif
	}
	if (device != PLATFORM_INPUT_BINDING_CONTROLLER)
	{
		return 0;
	}
#ifdef __vita__
	return s_vitaLastPadValid && NativeInput_VitaBindingState(binding, &s_vitaLastPad);
#else
	for (s32 slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		SDL_Gamepad *controller = s_controllers[slot].controller;
		if ((controller != NULL) && SDL_GamepadConnected(controller) &&
		    (NativeInput_ControllerButtonState(controller, binding) > NATIVE_INPUT_CAPTURE_THRESHOLD))
		{
			return 1;
		}
	}
	return 0;
#endif
}

void Platform_InputBeginBindingCapture(int device)
{
	s_bindingCaptureDevice = device;
	s_bindingCaptureArmed = 0;
}

int Platform_InputPollBindingCapture(int device, int *binding)
{
	s32 candidate = -1;
	s32 active = 0;
	if (device != s_bindingCaptureDevice)
	{
		Platform_InputBeginBindingCapture(device);
	}

	if (device == PLATFORM_INPUT_BINDING_KBM)
	{
#ifdef __vita__
		active = 0;
#else
		active = NativeInput_FirstActiveKbmBinding(&candidate);
#endif
	}
	else if (device == PLATFORM_INPUT_BINDING_CONTROLLER)
	{
		active = NativeInput_FirstActiveControllerBinding(&candidate);
	}

	if (!active)
	{
		s_bindingCaptureArmed = 1;
		return 0;
	}
	if (!s_bindingCaptureArmed)
	{
		return 0;
	}
	if (binding != NULL)
	{
		*binding = candidate;
	}
	s_bindingCaptureArmed = 0;
	return 1;
}


internal void NativeInput_SwapControllerSlots(s32 slotA, s32 slotB)
{
	struct NativeInputController controller;
	s32 mapping;

	if (!NativeInput_IsValidControllerSlot(slotA) || !NativeInput_IsValidControllerSlot(slotB) || (slotA == slotB))
	{
		return;
	}

	controller = s_controllers[slotA];
	s_controllers[slotA] = s_controllers[slotB];
	s_controllers[slotB] = controller;

	mapping = s_controllerToSlotMapping[slotA];
	s_controllerToSlotMapping[slotA] = s_controllerToSlotMapping[slotB];
	s_controllerToSlotMapping[slotB] = mapping;

	if (s_lastActiveControllerSlot == slotA)
	{
		s_lastActiveControllerSlot = slotB;
	}
	else if (s_lastActiveControllerSlot == slotB)
	{
		s_lastActiveControllerSlot = slotA;
	}
}

internal s32 NativeInput_FindSlotForDeviceIndex(Sint32 deviceIndex)
{
	s32 slot;

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		if (s_controllerToSlotMapping[slot] == deviceIndex)
		{
			return slot;
		}
	}

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		if ((s_controllerToSlotMapping[slot] < 0) && (s_controllers[slot].controller == NULL))
		{
			return slot;
		}
	}

	return -1;
}

internal void NativeInput_CloseController(s32 slot)
{
	struct NativeInputController *controller;

	if ((slot < 0) || (slot >= NATIVE_INPUT_MAX_CONTROLLERS))
	{
		return;
	}

	controller = &s_controllers[slot];
	if (controller->controller != NULL)
	{
		SDL_CloseGamepad(controller->controller);
	}

	controller->controller = NULL;
	controller->instanceId = -1;
	controller->analogEnabled = 0;
	controller->switchingAnalog = 0;

	if (s_lastActiveControllerSlot == slot)
	{
		s_lastActiveControllerSlot = -1;
	}
}

internal void NativeInput_OpenController(SDL_JoystickID instanceId, s32 slot)
{
	struct NativeInputController *controller;
	SDL_Joystick *joystick;

	if ((slot < 0) || (slot >= NATIVE_INPUT_MAX_CONTROLLERS))
	{
		return;
	}

	if (SDL_IsGamepad(instanceId) == 0)
	{
		return;
	}

	controller = &s_controllers[slot];
	if (controller->controller != NULL)
	{
		return;
	}

	controller->controller = SDL_OpenGamepad(instanceId);
	if (controller->controller == NULL)
	{
		return;
	}

	joystick = SDL_GetGamepadJoystick(controller->controller);
	controller->instanceId = joystick != NULL ? SDL_GetJoystickID(joystick) : instanceId;
	controller->analogEnabled = 1;
	controller->switchingAnalog = 0;
	NativeInput_MoveKeyboardOffControllerSlot(slot);
}

internal void NativeInput_OpenKnownControllers(void)
{
	SDL_JoystickID *gamepads;
	s32 count = 0;
	s32 i;

	gamepads = SDL_GetGamepads(&count);
	for (i = 0; i < count; i++)
	{
		s32 slot = NativeInput_FindSlotForDeviceIndex(gamepads[i]);

		if (slot >= 0)
		{
			NativeInput_OpenController(gamepads[i], slot);
		}
	}
	SDL_free(gamepads);
}

int Platform_InputInit(void)
{
	s32 slot;

	if (s_inputInitialized != 0)
	{
		return 1;
	}

	memset(s_controllers, 0, sizeof(s_controllers));
	memset(s_padSlotData, 0, sizeof(s_padSlotData));
	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		s_controllers[slot].instanceId = -1;
		NativeInput_ResetSnapshot(slot);
		s_installedSnapshots[slot] = s_controllers[slot].snapshot;
	}

	NativeInput_DefaultMappings();
	s_keyboardControllerSlot = NATIVE_INPUT_DEFAULT_KEYBOARD_SLOT;
	s_lastActiveControllerSlot = -1;
	s_installedSnapshotsActive = 0;
	s_keyboardState = SDL_GetKeyboardState(NULL);
#ifdef __vita__
	SceTouchPanelInfo panelInfo;
	s_vitaTouchMask = 0;
	s_vitaLastPadValid = 0;
	memset(&s_vitaLastPad, 0, sizeof(s_vitaLastPad));
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
	if (sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panelInfo) >= 0)
	{
		s_vitaTouchFrontMidX = ((s32)panelInfo.minAaX + (s32)panelInfo.maxAaX) / 2;
	}
	if (sceTouchGetPanelInfo(SCE_TOUCH_PORT_BACK, &panelInfo) >= 0)
	{
		s_vitaTouchRearMidX = ((s32)panelInfo.minAaX + (s32)panelInfo.maxAaX) / 2;
	}
#endif

	if (SDL_InitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC) == 0)
	{
		fprintf(stderr, "[CTR Native] Failed to initialise SDL input subsystem: %s\n", SDL_GetError());
		return 0;
	}

	SDL_AddGamepadMappingsFromFile("gamecontrollerdb.txt");
	NativeInput_OpenKnownControllers();

	s_inputInitialized = 1;
	return 1;
}

void Platform_InputShutdown(void)
{
	s32 slot;

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		NativeInput_CloseController(slot);
	}

	if (s_inputInitialized != 0)
	{
		SDL_QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
	}

	s_inputInitialized = 0;
	s_installedSnapshotsActive = 0;
	s_keyboardControllerSlot = NATIVE_INPUT_DEFAULT_KEYBOARD_SLOT;
	s_lastActiveControllerSlot = -1;
#ifdef __vita__
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_STOP);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_STOP);
	s_vitaTouchMask = 0;
	s_vitaLastPadValid = 0;
#endif
	memset(s_padSlotData, 0, sizeof(s_padSlotData));
	s_keyboardState = NULL;
}

int Platform_InputStartPressed(void)
{
	s32 slot;

	if (s_inputInitialized == 0)
	{
		return 0;
	}

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		const struct PlatformInputPadSnapshot *snapshot = &s_controllers[slot].snapshot;
		if ((snapshot->connected != 0) && ((NativeInput_GetSnapshotButtons(snapshot) & NATIVE_INPUT_RAW_START) == 0))
		{
			return 1;
		}
	}

	return 0;
}

void Platform_InputUpdate(void)
{
	u16 keyboardButtons;
	s32 slot;
	struct PlatformInputPadSnapshot adhocPads[NATIVE_INPUT_MAX_CONTROLLERS];

	if (s_inputInitialized == 0)
	{
		return;
	}

	NativeAdhoc_Update();

	if (s_installedSnapshotsActive != 0)
	{
		// NOTE(aalhendi): replay/state installs PSX-shaped pad bytes here;
		// SDL host state is not serialized.
		NativeInput_WriteInstalledSnapshots();
		return;
	}

	if (g_padCommEnable == 0)
	{
		return;
	}

#ifdef __vita__
	keyboardButtons = 0xffff;
	NativeInput_VitaPollTouch();
#else
	SDL_PumpEvents();
	keyboardButtons = NativeInput_KeyboardSuppressed() ? 0xffff : NativeInput_ReadKeyboard();
#endif

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		NativeInput_ResetSnapshot(slot);
		NativeInput_ApplyController(slot);
#ifndef __vita__
		NativeInput_ApplyKeyboard(slot, keyboardButtons);
#endif
		adhocPads[slot] = s_controllers[slot].snapshot;
	}

	NativeAdhoc_ProcessPadSnapshots(adhocPads, NATIVE_INPUT_MAX_CONTROLLERS);
	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		s_controllers[slot].snapshot = adhocPads[slot];
	}
	NativeInput_WritePadBus();
}

void Platform_InputControllerAdded(int deviceIndex)
{
	s32 slot;

	if (s_inputInitialized == 0)
	{
		return;
	}

	slot = NativeInput_FindSlotForDeviceIndex(deviceIndex);
	if (slot >= 0)
	{
		NativeInput_OpenController(deviceIndex, slot);
	}
}

void Platform_InputControllerRemoved(int instanceId)
{
	s32 slot;

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		if (s_controllers[slot].instanceId == (SDL_JoystickID)instanceId)
		{
			NativeInput_CloseController(slot);
			return;
		}
	}
}

int Platform_InputCycleKeyboardController(void)
{
	s_keyboardControllerSlot = NativeInput_NextControllerSlot(s_keyboardControllerSlot);
	return s_keyboardControllerSlot + 1;
}

int Platform_InputCycleGamepadController(void)
{
	s32 slot = NativeInput_FindActiveControllerSlot();
	s32 nextSlot;

	if (slot < 0)
	{
		return 0;
	}

	nextSlot = NativeInput_NextControllerSlot(slot);
	NativeInput_SwapControllerSlots(slot, nextSlot);
	NativeInput_WritePadBus();
	return nextSlot + 1;
}

void Platform_InputPadInit(int slot, unsigned char *padData)
{
	if ((slot < 0) || (slot >= NATIVE_INPUT_PHYSICAL_SLOT_COUNT))
	{
		return;
	}

	s_padSlotData[slot] = padData;
	NativeInput_WritePadBus();
}

int Platform_InputPadGetState(int port)
{
	s32 physicalSlot = (port >> 4) & 1;
	s32 tap = port & 3;
	s32 slot;

	if (NativeInput_UseMultitapBus() != 0)
	{
		if (physicalSlot != 0)
		{
			return PadStateDiscon;
		}
		slot = tap;
	}
	else
	{
		if (tap != 0)
		{
			return PadStateDiscon;
		}
		slot = physicalSlot;
	}

	if ((slot < 0) || (slot >= NATIVE_INPUT_MAX_CONTROLLERS))
	{
		return PadStateDiscon;
	}

	return s_controllers[slot].snapshot.connected ? PadStateStable : PadStateDiscon;
}

int Platform_InputCapturePadSnapshots(struct PlatformInputPadSnapshot *dst, int count)
{
	s32 slot;

	if ((dst == NULL) || (count < NATIVE_INPUT_MAX_CONTROLLERS))
	{
		return 0;
	}

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		dst[slot] = s_controllers[slot].snapshot;
	}

	return NATIVE_INPUT_MAX_CONTROLLERS;
}

int Platform_InputInstallPadSnapshots(const struct PlatformInputPadSnapshot *src, int count)
{
	s32 slot;

	if ((src == NULL) || (count < NATIVE_INPUT_MAX_CONTROLLERS))
	{
		return 0;
	}

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		s_installedSnapshots[slot] = src[slot];
	}

	s_installedSnapshotsActive = 1;
	NativeInput_WriteInstalledSnapshots();
	return NATIVE_INPUT_MAX_CONTROLLERS;
}

void Platform_InputClearInstalledPadSnapshots(void)
{
	s_installedSnapshotsActive = 0;
}

int Platform_InputGetStateSize(void)
{
	return (int)sizeof(struct NativeInputStateSnapshot);
}

int Platform_InputCaptureState(void *dst, int dstSize)
{
	struct NativeInputStateSnapshot *snapshot = (struct NativeInputStateSnapshot *)dst;
	s32 slot;

	if ((dst == NULL) || (dstSize < (int)sizeof(*snapshot)))
	{
		return 0;
	}

	memset(snapshot, 0, sizeof(*snapshot));
	snapshot->magic = NATIVE_INPUT_STATE_MAGIC;
	snapshot->version = NATIVE_INPUT_STATE_VERSION;
	snapshot->size = sizeof(*snapshot);
	snapshot->keyboardControllerSlot = s_keyboardControllerSlot;
	snapshot->lastActiveControllerSlot = s_lastActiveControllerSlot;
	snapshot->installedSnapshotsActive = s_installedSnapshotsActive;

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		snapshot->installedSnapshots[slot] = s_installedSnapshots[slot];
		snapshot->controllers[slot].snapshot = s_controllers[slot].snapshot;
		snapshot->controllers[slot].analogEnabled = s_controllers[slot].analogEnabled;
		snapshot->controllers[slot].switchingAnalog = s_controllers[slot].switchingAnalog;
		snapshot->controllers[slot].controllerToSlotMapping = s_controllerToSlotMapping[slot];
	}

	return 1;
}

int Platform_InputRestoreState(const void *src, int srcSize)
{
	const struct NativeInputStateSnapshot *snapshot = (const struct NativeInputStateSnapshot *)src;
	s32 slot;

	if ((src == NULL) || (srcSize < (int)sizeof(*snapshot)))
	{
		return 0;
	}
	if ((snapshot->magic != NATIVE_INPUT_STATE_MAGIC) || (snapshot->version != NATIVE_INPUT_STATE_VERSION) || (snapshot->size != sizeof(*snapshot)))
	{
		return 0;
	}
	if (!NativeInput_IsValidControllerSlot(snapshot->keyboardControllerSlot))
	{
		return 0;
	}
	if ((snapshot->lastActiveControllerSlot < -1) || (snapshot->lastActiveControllerSlot >= NATIVE_INPUT_MAX_CONTROLLERS))
	{
		return 0;
	}
	if ((snapshot->installedSnapshotsActive < 0) || (snapshot->installedSnapshotsActive > 1))
	{
		return 0;
	}
	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		if ((snapshot->controllers[slot].analogEnabled < 0) || (snapshot->controllers[slot].analogEnabled > 1))
		{
			return 0;
		}
		if ((snapshot->controllers[slot].switchingAnalog < 0) || (snapshot->controllers[slot].switchingAnalog > 1))
		{
			return 0;
		}
		if (snapshot->controllers[slot].controllerToSlotMapping < -1)
		{
			return 0;
		}
	}

	s_keyboardControllerSlot = snapshot->keyboardControllerSlot;
	s_lastActiveControllerSlot = snapshot->lastActiveControllerSlot;
	s_installedSnapshotsActive = snapshot->installedSnapshotsActive != 0;

	for (slot = 0; slot < NATIVE_INPUT_MAX_CONTROLLERS; slot++)
	{
		s_installedSnapshots[slot] = snapshot->installedSnapshots[slot];
		s_controllers[slot].snapshot = snapshot->controllers[slot].snapshot;
		s_controllers[slot].analogEnabled = snapshot->controllers[slot].analogEnabled;
		s_controllers[slot].switchingAnalog = snapshot->controllers[slot].switchingAnalog;
		s_controllerToSlotMapping[slot] = snapshot->controllers[slot].controllerToSlotMapping;
	}
	NativeInput_WritePadBus();

	return 1;
}

void Platform_InputPadVibrate(int port, unsigned char *table, int len)
{
	s32 physicalSlot = (port >> 4) & 1;
	s32 tap = port & 3;
	s32 slot;
	struct NativeInputController *controller;
	u16 freqHigh;
	u16 freqLow;

	if (NativeInput_UseMultitapBus() != 0)
	{
		if (physicalSlot != 0)
		{
			return;
		}
		slot = tap;
	}
	else
	{
		if (tap != 0)
		{
			return;
		}
		slot = physicalSlot;
	}

	if ((slot < 0) || (slot >= NATIVE_INPUT_MAX_CONTROLLERS) || (table == NULL) || (len <= 0))
	{
		return;
	}

	controller = &s_controllers[slot];
	if (controller->controller == NULL)
	{
		return;
	}

	freqHigh = table[0] * 255;
	freqLow = len > 1 ? table[1] * 255 : 0;

	if ((freqLow != 0) && (freqLow < 4096))
	{
		freqLow = 4096;
	}

	if ((freqHigh != 0) && (freqHigh < 4096))
	{
		freqHigh = 4096;
	}

	SDL_RumbleGamepad(controller->controller, freqLow, freqHigh, 200);
}
