#ifndef PLATFORM_NATIVE_INPUT_H
#define PLATFORM_NATIVE_INPUT_H

#include <macros.h>

#define PLATFORM_INPUT_PAD_COUNT 4

enum PlatformInputBindingDevice
{
	PLATFORM_INPUT_BINDING_KBM = 0,
	PLATFORM_INPUT_BINDING_CONTROLLER,
	PLATFORM_INPUT_BINDING_DEVICE_COUNT
};

enum PlatformInputBindingAction
{
	PLATFORM_INPUT_BIND_CROSS = 0,
	PLATFORM_INPUT_BIND_SQUARE,
	PLATFORM_INPUT_BIND_CIRCLE,
	PLATFORM_INPUT_BIND_TRIANGLE,
	PLATFORM_INPUT_BIND_L1,
	PLATFORM_INPUT_BIND_R1,
	PLATFORM_INPUT_BIND_L2,
	PLATFORM_INPUT_BIND_R2,
	PLATFORM_INPUT_BIND_UP,
	PLATFORM_INPUT_BIND_DOWN,
	PLATFORM_INPUT_BIND_LEFT,
	PLATFORM_INPUT_BIND_RIGHT,
	PLATFORM_INPUT_BIND_START,
	PLATFORM_INPUT_BIND_ACTION_COUNT
};

enum PlatformInputOverlayButton
{
	PLATFORM_INPUT_OVERLAY_SOUTH = 0,
	PLATFORM_INPUT_OVERLAY_WEST,
	PLATFORM_INPUT_OVERLAY_EAST,
	PLATFORM_INPUT_OVERLAY_NORTH,
	PLATFORM_INPUT_OVERLAY_L1,
	PLATFORM_INPUT_OVERLAY_R1,
	PLATFORM_INPUT_OVERLAY_L2,
	PLATFORM_INPUT_OVERLAY_R2,
	PLATFORM_INPUT_OVERLAY_UP,
	PLATFORM_INPUT_OVERLAY_DOWN,
	PLATFORM_INPUT_OVERLAY_LEFT,
	PLATFORM_INPUT_OVERLAY_RIGHT,
	PLATFORM_INPUT_OVERLAY_START,
	PLATFORM_INPUT_OVERLAY_SELECT,
	PLATFORM_INPUT_OVERLAY_L3,
	PLATFORM_INPUT_OVERLAY_R3,
	PLATFORM_INPUT_OVERLAY_TOUCH_FRONT_LEFT,
	PLATFORM_INPUT_OVERLAY_TOUCH_FRONT_RIGHT,
	PLATFORM_INPUT_OVERLAY_TOUCH_REAR_LEFT,
	PLATFORM_INPUT_OVERLAY_TOUCH_REAR_RIGHT,
	PLATFORM_INPUT_OVERLAY_COUNT,
	PLATFORM_INPUT_OVERLAY_UNKNOWN = 31,
};

struct PlatformInputPadSnapshot
{
	u8 status;
	u8 id;
	u8 buttons[2];
	u8 analog[4];
	u8 connected;
	u8 reserved[3];
};

int Platform_InputInit(void);
void Platform_InputShutdown(void);
void Platform_InputUpdate(void);
void Platform_InputControllerAdded(int deviceIndex);
void Platform_InputControllerRemoved(int instanceId);
int Platform_InputCycleKeyboardController(void);
int Platform_InputCycleGamepadController(void);

int Platform_InputConfigSetBinding(const char *key, int value);
void Platform_InputGetBindingConfigKey(int action, int device, char *dst, int dstSize);
int Platform_InputGetBinding(int action, int device);
int Platform_InputGetBindingOverlayButton(int action, int device);
void Platform_InputSetBinding(int action, int device, int binding);
void Platform_InputGetBindingName(int action, int device, char *dst, int dstSize);
void Platform_InputBeginBindingCapture(int device);
int Platform_InputPollBindingCapture(int device, int *binding);
int Platform_InputBindingIsActive(int device, int binding);

void Platform_InputPadInit(int slot, unsigned char *padData);
int Platform_InputPadGetState(int port);
void Platform_InputPadVibrate(int port, unsigned char *table, int len);
int Platform_InputCapturePadSnapshots(struct PlatformInputPadSnapshot *dst, int count);
int Platform_InputInstallPadSnapshots(const struct PlatformInputPadSnapshot *src, int count);
void Platform_InputClearInstalledPadSnapshots(void);
int Platform_InputGetStateSize(void);
int Platform_InputCaptureState(void *dst, int dstSize);
int Platform_InputRestoreState(const void *src, int srcSize);

#endif
