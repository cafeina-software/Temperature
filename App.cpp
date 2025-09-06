#include <Screen.h>
#include "App.h"
#include "MainWindow.h"
#include "TemperatureDefs.h"

const char *kTemperaturePath = "/dev/power/acpi_thermal/0";

App::App(void)
	:	BApplication(kTemperatureMime)
{
	mainwin = new MainWindow();
}

void App::ReadyToRun()
{
	BScreen screen(B_MAIN_SCREEN_ID);
	BRect windowFrame = mainwin->Frame();

	if(!screen.Frame().Contains(windowFrame)) {
		mainwin->MoveOnScreen(B_MOVE_IF_PARTIALLY_OFFSCREEN);
	}
	mainwin->Show();
}

int main(int argc, char **argv)
{
	App *app = new App();
	app->Run();
	delete app;
	return 0;
}
