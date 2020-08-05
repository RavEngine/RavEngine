/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#ifndef SDL_JOYSTICK_HIDAPI_H
#define SDL_JOYSTICK_HIDAPI_H

#include "SDL_atomic.h"
#include "SDL_mutex.h"
#include "SDL_joystick.h"
#include "SDL_gamecontroller.h"
#include "../../hidapi/hidapi/hidapi.h"
#include "../usb_ids.h"

/* This is the full set of HIDAPI drivers available */
#define SDL_JOYSTICK_HIDAPI_PS4
#define SDL_JOYSTICK_HIDAPI_SWITCH
#define SDL_JOYSTICK_HIDAPI_XBOX360
#define SDL_JOYSTICK_HIDAPI_XBOXONE
#define SDL_JOYSTICK_HIDAPI_GAMECUBE

#ifdef __WINDOWS__
/* On Windows, Xbox One controllers are handled by the Xbox 360 driver */
#undef SDL_JOYSTICK_HIDAPI_XBOXONE
/* It turns out HIDAPI for Xbox controllers doesn't allow background input */
#undef SDL_JOYSTICK_HIDAPI_XBOX360
#endif

#ifdef __MACOSX__
/* On Mac OS X, Xbox One controllers are handled by the Xbox 360 driver */
#undef SDL_JOYSTICK_HIDAPI_XBOXONE
#endif

#if defined(__IPHONEOS__) || defined(__TVOS__) || defined(__ANDROID__)
/* Very basic Steam Controller support on mobile devices */
#define SDL_JOYSTICK_HIDAPI_STEAM
#endif

/* The maximum size of a USB packet for HID devices */
#define USB_PACKET_LENGTH   64

/* Forward declaration */
struct _SDL_HIDAPI_DeviceDriver;

typedef struct _SDL_HIDAPI_Device
{
    char *name;
    char *path;
    Uint16 vendor_id;
    Uint16 product_id;
    Uint16 version;
    SDL_JoystickGUID guid;
    int interface_number;   /* Available on Windows and Linux */
    int interface_class;
    int interface_subclass;
    int interface_protocol;
    Uint16 usage_page;      /* Available on Windows and Mac OS X */
    Uint16 usage;           /* Available on Windows and Mac OS X */

    struct _SDL_HIDAPI_DeviceDriver *driver;
    void *context;
    SDL_mutex *dev_lock;
    hid_device *dev;
    SDL_atomic_t rumble_pending;
    int num_joysticks;
    SDL_JoystickID *joysticks;

    /* Used during scanning for device changes */
    SDL_bool seen;

    struct _SDL_HIDAPI_Device *next;
} SDL_HIDAPI_Device;

typedef struct _SDL_HIDAPI_DeviceDriver
{
    const char *hint;
    SDL_bool enabled;
    SDL_bool (*IsSupportedDevice)(const char *name, SDL_GameControllerType type, Uint16 vendor_id, Uint16 product_id, Uint16 version, int interface_number, int interface_class, int interface_subclass, int interface_protocol);
    const char *(*GetDeviceName)(Uint16 vendor_id, Uint16 product_id);
    SDL_bool (*InitDevice)(SDL_HIDAPI_Device *device);
    int (*GetDevicePlayerIndex)(SDL_HIDAPI_Device *device, SDL_JoystickID instance_id);
    void (*SetDevicePlayerIndex)(SDL_HIDAPI_Device *device, SDL_JoystickID instance_id, int player_index);
    SDL_bool (*UpdateDevice)(SDL_HIDAPI_Device *device);
    SDL_bool (*OpenJoystick)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick);
    int (*RumbleJoystick)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble);
    void (*CloseJoystick)(SDL_HIDAPI_Device *device, SDL_Joystick *joystick);
    void (*FreeDevice)(SDL_HIDAPI_Device *device);

} SDL_HIDAPI_DeviceDriver;


/* HIDAPI device support */
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverPS4;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverSteam;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverSwitch;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverXbox360;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverXbox360W;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverXboxOne;
extern SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverGameCube;

/* Return true if a HID device is present and supported as a joystick */
extern SDL_bool HIDAPI_IsDevicePresent(Uint16 vendor_id, Uint16 product_id, Uint16 version, const char *name);

extern void HIDAPI_UpdateDevices(void);
extern SDL_bool HIDAPI_JoystickConnected(SDL_HIDAPI_Device *device, SDL_JoystickID *pJoystickID);
extern void HIDAPI_JoystickDisconnected(SDL_HIDAPI_Device *device, SDL_JoystickID joystickID);

#endif /* SDL_JOYSTICK_HIDAPI_H */

/* vi: set ts=4 sw=4 expandtab: */
