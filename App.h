#ifndef APP_H
#define APP_H

#include <Application.h>
#include "MainWindow.h"
#include "DataFactory.h"

class App : public BApplication
{
public:
	App(void);
	~App();

	void ReadyToRun() override;
	bool QuitRequested() override;
	void MessageReceived(BMessage* message) override;
	void AboutRequested() override;

	BHandler* ResolveSpecifier(BMessage* message, int32 index,
		BMessage* specifier, int32 form, const char* property) override;
	status_t GetSupportedSuites(BMessage* data) override;
	void HandleScripting(BMessage* message);

	void LoadSettings();
	void SaveSettings();
private:
	DataFactory* dataRepository;
	MainWindow *mainwin;
};

#endif
