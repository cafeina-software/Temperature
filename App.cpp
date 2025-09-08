#include <Catalog.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <PropertyInfo.h>
#include <Screen.h>
#include <private/interface/AboutWindow.h>
#include <cassert>
#include "App.h"
#include "DataFactory.h"
#include "MainWindow.h"
#include "TemperatureDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PropertyInfo"

static property_info kTemperatureProperties[] = {
	{
		.name       = "ThermalDevice",
		.commands   = { B_GET_PROPERTY, B_SET_PROPERTY, 0 },
		.specifiers = { B_DIRECT_SPECIFIER, 0 },
		.usage      = B_TRANSLATE("The current thermal device"),
		.extra_data = 0,
		.types      = { B_STRING_TYPE }
	},
	{
		.name       = "CurrentTemperature",
		.commands   = { B_GET_PROPERTY, 0 },
		.specifiers = { B_DIRECT_SPECIFIER, 0 },
		.usage      = B_TRANSLATE("The current thermal device's reported temperature"),
		.extra_data = 0,
		.types      = { B_FLOAT_TYPE }
	},
	{
		.name       = "MonitorStatus",
		.commands   = { B_GET_PROPERTY, B_SET_PROPERTY, 0 },
		.specifiers = { B_DIRECT_SPECIFIER, 0 },
		.usage      = B_TRANSLATE("Temperature monitor status: on or off"),
		.extra_data = 0,
		.types      = { B_BOOL_TYPE }
	},
	{ 0 }
};

App::App(void)
	:	BApplication(kTemperatureMime),
	dataRepository(NULL)
{
	LoadSettings();
	mainwin = new MainWindow(dataRepository->WindowRect(), dataRepository);
}

App::~App()
{
	if(dataRepository)
		delete dataRepository;
}

void App::ReadyToRun()
{
	assert(mainwin);

	BScreen screen(B_MAIN_SCREEN_ID);
	if(!screen.Frame().Contains(mainwin->Frame())) {
		mainwin->MoveOnScreen(B_MOVE_IF_PARTIALLY_OFFSCREEN);
	}
	mainwin->Show();
}

bool App::QuitRequested()
{
	SaveSettings();
	return BApplication::QuitRequested();
}

void App::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case B_COUNT_PROPERTIES:
		case B_CREATE_PROPERTY:
		case B_DELETE_PROPERTY:
		case B_EXECUTE_PROPERTY:
		case B_GET_PROPERTY:
		case B_SET_PROPERTY:
			if(!message->HasSpecifiers())
				break;
			HandleScripting(message);
			break;
		case B_ABOUT_REQUESTED:
			AboutRequested();
			break;
		case B_QUIT_REQUESTED:
			QuitRequested();
			break;
		default:
			return BApplication::MessageReceived(message);
	}
}

void App::AboutRequested()
{
	const char* extraCopyrights[] = {
		"2025 cafeina",
		NULL
	};
	BString originalAuthor(B_TRANSLATE("%author% (original author)"));
	originalAuthor.ReplaceAll("%author%", "GatoAmarilloBicolor");
	BString otherAuthor(B_TRANSLATE("%other% (code refactoring)"));
	otherAuthor.ReplaceAll("%other%", "cafeina");
	const char* authors[] = {
		originalAuthor.String(),
		otherAuthor.String(),
		NULL
	};
	const char* website[] = {
		"https://github.com/GatoAmarilloBicolor/Temperature",
		NULL
	};

	BAboutWindow* aboutWindow = new BAboutWindow(kAppName, kTemperatureMime);
	aboutWindow->AddDescription(B_TRANSLATE("Thermal sensors' temperature reader."));
	aboutWindow->AddCopyright(2023, "GatoAmarilloBicolor", extraCopyrights);
	aboutWindow->AddAuthors(authors);
	aboutWindow->AddText(B_TRANSLATE("Website:"), website);

	if(mainwin)
		aboutWindow->CenterIn(mainwin->Frame());
	aboutWindow->Show();
}

BHandler* App::ResolveSpecifier(BMessage* message, int32 index,
	BMessage* specifier, int32 what, const char* property)
{
	BPropertyInfo propertyInfo(kTemperatureProperties);
	if(propertyInfo.FindMatch(message, index, specifier, what, property) >= 0)
		return this;

	return BApplication::ResolveSpecifier(message, index, specifier, what, property);
}

status_t App::GetSupportedSuites(BMessage* message)
{
	message->AddString("suites", kTemperatureSuiteMime);
	BPropertyInfo propertyInfo(kTemperatureProperties);
	message->AddFlat("messages", &propertyInfo);
	return BApplication::GetSupportedSuites(message);
}

void App::HandleScripting(BMessage* message)
{
	BMessage reply(B_REPLY);

	int32 index = 0;
	BMessage specifier;
	int32 what = 0;
	const char* property = NULL;
	if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
		BPropertyInfo propertyInfo(kTemperatureProperties);
		switch(propertyInfo.FindMatch(message, index, &specifier, what, property))
		{
			case 0: // ThermalDevice
			{
				if(message->what == B_GET_PROPERTY) {
					reply.AddString("result", dataRepository->ActiveDevice());
					message->SendReply(&reply);
					return;
				}
				else if(message->what == B_SET_PROPERTY) {
					BString devicePath = message->GetString("data");
					BFile deviceFile(devicePath.String(), B_READ_ONLY);
					status_t status = B_ERROR;
					if((status = deviceFile.InitCheck()) != B_OK) {
						reply.what = B_MESSAGE_NOT_UNDERSTOOD;
						reply.AddString("message", strerror(status));
						message->SendReply(&reply);
						return;
					}
					struct stat st;
					deviceFile.GetStat(&st);
					if(st.st_mode & S_IFDIR || !(st.st_mode & S_IFBLK || st.st_mode & S_IFCHR)) {
						reply.what = B_MESSAGE_NOT_UNDERSTOOD;
						reply.AddString("message", "The provided path is not a device file.\n");
						message->SendReply(&reply);
						return;
					}
					if(devicePath.FindFirst("/dev/power/", 0) < 0) {
						reply.what = B_MESSAGE_NOT_UNDERSTOOD;
						reply.AddString("message", "The device path is not a power-type device.\n");
						message->SendReply(&reply);
						return;
					}
					deviceFile.Unset();

					if(mainwin) { // Notify window as needed
						mainwin->DeviceChanged(devicePath.String());
					}
					return;
				}
				break;
			}
			case 1: // CurrentTemperature
			{
				if(message->what == B_GET_PROPERTY) {
					if(strlen(dataRepository->ActiveDevice()) < 1 ||
					!BEntry(dataRepository->ActiveDevice()).Exists()) {
						reply.what = B_MESSAGE_NOT_UNDERSTOOD;
						reply.AddString("message", "Device not found or not initialized.\n");
						message->SendReply(&reply);
						return;
					}
					ThermalDevice device(dataRepository->ActiveDevice());
					reply.AddFloat("result", device.ReadTemperature());
					message->SendReply(&reply);
					return;
				}
				break;
			}
			case 2: // MonitorStatus
			{
				bool status;
				if(message->what == B_GET_PROPERTY) {
					if(!mainwin)
						status = false;
					else {
						status = mainwin->RunningStatus();
					}
					reply.AddBool("result", status);
					message->SendReply(&reply);
					return;
				}
				else if(message->what == B_SET_PROPERTY) {
					mainwin->SetRunningStatus(message->GetBool("data"));
					return;
				}
				break;
			}
		}
	}
}

void App::LoadSettings()
{
	BPath settingsPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append(kTemperatureConfigFile, true);

	BMessage settings;
	BFile settingsFile(settingsPath.Path(), B_READ_ONLY);
	if(settingsFile.InitCheck() != B_OK || settings.Unflatten(&settingsFile) != B_OK) {
		DataFactory::DefaultSettings(&settings);
	}

	dataRepository = DataFactory::Instantiate(&settings);
	if(dataRepository == NULL) {
		dataRepository = new DataFactory();
	}

	// The data repository currently does not have any active device
	//	so let's select the first one available for the user if...
	if((!dataRepository->ActiveDevice() || // has no device path
	!BEntry(dataRepository->ActiveDevice()).Exists()) && // or the device path is invalid...
	dataRepository->ThermalDevices().CountStrings() > 0) // and has available devices
		dataRepository->SetActiveDevice(dataRepository->ThermalDevices().StringAt(0));
}

void App::SaveSettings()
{
	dataRepository->SetWindowRect(mainwin->Frame());

	BMessage settings;
	if(dataRepository->Archive(&settings, true) == B_OK) {
		BPath settingsPath;
		find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
		settingsPath.Append(kTemperatureConfigFile, true);

		BFile settingsFile(settingsPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		if(settingsFile.InitCheck() == B_OK) {
			settings.Flatten(&settingsFile);
		}
	}
}

int main(int argc, char **argv)
{
	App *app = new App();
	app->Run();
	delete app;
	return 0;
}
