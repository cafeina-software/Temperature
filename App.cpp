#include "App.h"
#include "MainWindow.h"
#include "TemperatureDefs.h"

const char *kTemperaturePath = "/dev/power/acpi_thermal/0";

App::App(void)
	:	BApplication(kTemperatureMime)
{
	MainWindow *mainwin = new MainWindow();
	mainwin->Show();
}


int main(int argc, char **argv)
{
	App *app = new App();
	app->Run();
	delete app;
	return 0;
}
