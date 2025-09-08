#ifndef __TEMPERATURE_DEFS__
#define __TEMPERATURE_DEFS__

enum {
    M_DEVICE_CHANGED = 'idev',
	M_SCALE_CHANGED = 'k   ',
	M_STARTED_RUNNING = 'strt',
	M_STOPPED_RUNNING = 'stop',
	M_REFRESH_RATE = 'rate',
	M_RESTORE_DEFAULTS = 'rstr'
};

/* Application identity */
#define kAppName "Temperature"
#define kTemperatureSuiteMime "suite/vnd.Loa-Temperature"
#define kTemperatureConfigFile kAppName ".settings"
#define kTemperatureMime "application/x-vnd.Loa-Temperature"

/* Configuration names */
#define kConfigDevicePath   "device"
#define kConfigTempScale    "scale"
#define kConfigBaseWnd      "window:"
#define kConfigBaseGraph    "graph:"
#define kConfigWndFrame     kConfigBaseWnd   "frame"
#define kConfigWndPulse     kConfigBaseWnd   "pulse"
#define kConfigGraphRun     kConfigBaseGraph "running"
#define kConfigGraphWMark   kConfigBaseGraph "watermark"

#endif /* __TEMPERATURE_DEFS__ */
