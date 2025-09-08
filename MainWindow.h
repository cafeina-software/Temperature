#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h>
#include <MenuBar.h>
#include <StringView.h>

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Button.h>
#include <String.h>
#include <TextControl.h>
#include <atomic>

#include "DataFactory.h"
#include "ThermalDevice.h"

class MainWindow : public BWindow
{
public:
						MainWindow(BRect frame, DataFactory* dataRepo);
						~MainWindow();

			void		MessageReceived(BMessage *msg);
			bool		QuitRequested(void);

			void		DeviceChanged(const char* devicePath);
	static	int32 		CallUpdateTemperature(void* data);
			void		Update();
	static	void		CallNotifyStopped(void* data);

			bool		RunningStatus() const { return !shouldStopUpdater; }
			void		SetRunningStatus(bool shouldRun)
							{ shouldStopUpdater = !shouldRun; }
private:
		DataFactory*	dataRepository;
		ThermalDevice 	activeDevice;

		BMenuField*		devicesField;
		BMenuField*		temperatureField;
		BTextControl*	criticalTempControl;
		BTextControl*	currentTempControl;

		thread_id		tempUpdaterThread;
		std::atomic<bool> shouldStopUpdater;
};

#endif
