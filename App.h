#ifndef APP_H
#define APP_H

#include <Application.h>
#include "MainWindow.h"

class App : public BApplication
{
public:
	App(void);

	void ReadyToRun() override;
private:
	MainWindow *mainwin;
};

#endif
