#ifndef __TEMPERATURE_DEFS__
#define __TEMPERATURE_DEFS__

/* Application identity */
#define kAppName "Temperature"
#define kTemperatureSuiteMime "suite/vnd.Loa-Temperature"
#define kTemperatureConfigFile kAppName ".settings"
#define kTemperatureMime "application/x-vnd.Loa-Temperature"

/* Configuration names */
#define kConfigDevicePath   "device"
#define kConfigBaseWnd      "window:"
#define kConfigBaseGraph    "graph:"
#define kConfigWndFrame     kConfigBaseWnd   "frame"
#define kConfigWndPulse     kConfigBaseWnd   "pulse"
#define kConfigGraphRun     kConfigBaseGraph "running"
#define kConfigGraphWMark   kConfigBaseGraph "watermark"

#endif /* __TEMPERATURE_DEFS__ */
