//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

//-----------------------------------------------------------------------------

#define TUSB_CFG_CONTROLLER_0_MODE  (TUSB_MODE_HOST)

#define TUSB_CFG_HOST_HUB                       1
#define TUSB_CFG_HOST_HID_KEYBOARD              1
#define TUSB_CFG_HOST_HID_MOUSE                 1
#define TUSB_CFG_HOST_HID_GENERIC               0 // (not yet supported)
#define TUSB_CFG_HOST_MSC                       1
#define TUSB_CFG_HOST_CDC                       1

#define TUSB_CFG_HOST_DEVICE_MAX                (TUSB_CFG_HOST_HUB ? 5 : 1) // normal hub has 4 ports

#define TUSB_CFG_DEBUG                2

#define TUSB_CFG_OS                   TUSB_OS_NONE
#define TUSB_CFG_TICKS_HZ             1000

#define TUSB_CFG_ATTR_USBRAM

//-----------------------------------------------------------------------------

#endif // TUSB_CONFIG_H

//-----------------------------------------------------------------------------
