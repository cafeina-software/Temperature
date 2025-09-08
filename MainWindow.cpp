#include "MainWindow.h"
#include "DataFactory.h"
#include "TemperatureDefs.h"
#include "TemperatureUtils.h"
#include "ThermalDevice.h"

#include <Alert.h>
#include <Application.h>
#include <Window.h>
#include <LayoutBuilder.h>
#include <View.h>
#include <Button.h>
#include <String.h>
#include <File.h>
#include <Catalog.h>
#include <NumberFormat.h>

#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>

#include <cassert>
#include <unordered_map>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Main window"

static std::unordered_map<int32, std::pair<char, BString>> tempMenuItems = {
	{ 0, { SCALE_CELSIUS, B_TRANSLATE("Celsius") } },
	{ 1, { SCALE_FAHRENHEIT, B_TRANSLATE("Fahrenheit") } },
	{ 2, { SCALE_KELVIN, B_TRANSLATE("Kelvin") } }
};

MainWindow::MainWindow(BRect frame, DataFactory* dataRepo)
	:	BWindow(frame, B_TRANSLATE_SYSTEM_NAME("Temperature"), B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS),
	dataRepository(dataRepo),
	activeDevice(NULL),
	shouldStopUpdater(false)
{
	assert(dataRepository != NULL);

	SetPulseRate(dataRepository->RefreshRate() * 1000000);

    // Device selector
    devicesField = new BMenuField("devices", B_TRANSLATE("Device"), new BPopUpMenu("", true, true));
	for(int32 i = 0; i < dataRepository->ThermalDevices().CountStrings(); i++) {
		BString deviceString(dataRepository->ThermalDevices().StringAt(i));
		BMessage* deviceMessage = new BMessage(M_DEVICE_CHANGED);
		deviceMessage->AddString("target", deviceString);
		devicesField->Menu()->AddItem(new BMenuItem(deviceString, deviceMessage));
	}
	if(devicesField->Menu()->FindItem(dataRepository->ActiveDevice()))
		devicesField->Menu()->FindItem(dataRepository->ActiveDevice())->SetMarked(true);

    // Reported temperatures
    criticalTempControl = new BTextControl("critical_temp", B_TRANSLATE("Critical:"), NULL, NULL);
	criticalTempControl->TextView()->MakeEditable(false);
	criticalTempControl->SetModificationMessage(new BMessage(B_MODIFIERS_CHANGED));

	currentTempControl = new BTextControl("current_temp", B_TRANSLATE("Current:"), NULL, NULL);
	currentTempControl->TextView()->MakeEditable(false);
	currentTempControl->SetModificationMessage(new BMessage(B_MODIFIERS_CHANGED));

	// Temperature settings
	BMenu* temperatureMenu = new BMenu("⚙️");
	for(int32 i = 0; i < SCALE_COUNT; i++) {
		BMessage* scaleMessage = new BMessage(M_SCALE_CHANGED);
		auto scale = tempMenuItems[i].first;
		scaleMessage->AddData(kConfigTempScale, B_CHAR_TYPE, &scale, sizeof(scale));
		scaleMessage->AddInt32("menu_item", i);
		temperatureMenu->AddItem(new BMenuItem(tempMenuItems[i].second, scaleMessage));
	}

	temperatureField = new BMenuField("temperature_options", "️", temperatureMenu);
	for(int32 i = 0; i < SCALE_COUNT; i++) { // Initialize temperature scale in menu
		auto scale = dataRepository->TemperatureScale();
		temperatureField->Menu()->ItemAt(i)->SetMarked(scale == tempMenuItems[i].first);
	}

    // Layouting
    BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGrid()
			.SetSpacing(B_USE_SMALL_SPACING, B_USE_SMALL_SPACING)
			.Add(devicesField->CreateLabelLayoutItem(), 0, 0)
			.Add(devicesField->CreateMenuBarLayoutItem(), 1, 0, 4)
			.Add(currentTempControl->CreateLabelLayoutItem(), 0, 1)
			.Add(currentTempControl->CreateTextViewLayoutItem(), 1, 1)
			.Add(criticalTempControl->CreateLabelLayoutItem(), 2, 1)
			.Add(criticalTempControl->CreateTextViewLayoutItem(), 3, 1)
			.Add(temperatureField, 4, 1)
		.End()
	.End();

	ResizeToPreferred();

	// Shortcuts
	AddShortcut('1', B_COMMAND_KEY, new BMessage(B_ABOUT_REQUESTED));
	AddShortcut(B_DELETE, B_COMMAND_KEY, new BMessage(M_RESTORE_DEFAULTS));

	// Start live monitoring
	tempUpdaterThread = spawn_thread(CallUpdateTemperature, "Temperature updater",
		B_NORMAL_PRIORITY, this);
	resume_thread(tempUpdaterThread);
}

MainWindow::~MainWindow()
{
	shouldStopUpdater.store(true, std::memory_order_release); // exit the thread
}

void MainWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(B_ABOUT_REQUESTED);
			break;
		case B_QUIT_REQUESTED:
			QuitRequested();
			break;
		case M_RESTORE_DEFAULTS:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Restore default values"),
			B_TRANSLATE("Do you want to restore the configuration to the default values?"),
			B_TRANSLATE("Cancel"), B_TRANSLATE("Restore defaults"), NULL,
			B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->SetDefaultButton(alert->ButtonAt(0));
			alert->SetShortcut(0, B_ESCAPE);

			if(alert->Go() == 1) {
				dataRepository->Perform(static_cast<perform_code>(msg->what), NULL);
				DeviceChanged(dataRepository->ThermalDevices().StringAt(0));
			}
			break;
		}
		case M_DEVICE_CHANGED:
		{
			BString target;
			if(msg->FindString("target", &target) == B_OK) {
				// Check if it exists...
				BEntry targetEntry(target);
				if(!targetEntry.Exists())
					break;

				// And if it is openable
				BFile targetFile(&targetEntry, B_READ_ONLY);
				if(targetFile.InitCheck() != B_OK)
					break;

				DeviceChanged(target);
			}
			break;
		}
		case M_SCALE_CHANGED:
		{
			const void* ptr = NULL;
			ssize_t length = 0;
			if(msg->FindData(kConfigTempScale, B_CHAR_TYPE, &ptr, &length) == B_OK) {
				char c = *(static_cast<const char*>(ptr));
				if(!IsValidScale(c))
					break;

				dataRepository->SetTemperatureScale(c);

				int32 index = 0;
				msg->FindInt32("menu_item", &index);
				for(int32 i = 0; i < SCALE_COUNT; i++)
					temperatureField->Menu()->ItemAt(i)->SetMarked(i == index);
			}
			break;
		}
		case M_STARTED_RUNNING:
		case M_STOPPED_RUNNING:
			// Notify as needed...
			break;
		case M_REFRESH_RATE:
		{
			uint32 rate = -1;
			if(msg->FindUInt32("rate", 0, &rate) == B_OK) {
				dataRepository->SetRefreshRate(rate);
				SetPulseRate(dataRepository->RefreshRate() * 1000000);
			}
			break;
		}
		default:
			BWindow::MessageReceived(msg);
			break;
	}
}

bool MainWindow::QuitRequested(void)
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void MainWindow::DeviceChanged(const char* devicePath)
{
	if(!devicePath)
		return;

	// Update target
	dataRepository->SetActiveDevice(devicePath);

	// Wait for thread to end
	if(tempUpdaterThread >= 0) {
		shouldStopUpdater.store(true, std::memory_order_release);
		status_t exitCode = B_OK;
		wait_for_thread(tempUpdaterThread, &exitCode);
		shouldStopUpdater.store(true, std::memory_order_release);
	}

	// and restart updater
	tempUpdaterThread = spawn_thread(CallUpdateTemperature, "Temperature updater",
		B_NORMAL_PRIORITY, this);
	shouldStopUpdater.store(false, std::memory_order_release);
	resume_thread(tempUpdaterThread);

	// Update interface
	LockLooper();
	devicesField->Menu()->FindItem(devicePath)->SetMarked(true);
	UnlockLooper();
}

/* static */
int32 MainWindow::CallUpdateTemperature(void* data)
{
	MainWindow* window = (MainWindow*)data;
	if(!window)
		return B_ERROR;

	on_exit_thread(CallNotifyStopped, data);
	window->Update();
	return B_OK;
}

void MainWindow::Update()
{
	PostMessage(M_STARTED_RUNNING);

	while(!shouldStopUpdater.load(std::memory_order_acquire)) {
		Lock();

		if(strcmp(activeDevice.Location(), dataRepository->ActiveDevice()) != 0)
			activeDevice.SetTo(dataRepository->ActiveDevice());

		auto scale = dataRepository->TemperatureScale();
		float currentTemp = activeDevice.ReadTemperature(TEMPERATURE_CURRENT);
		float convertedTemp = ConvertToScale(currentTemp, SCALE_CELSIUS, scale);

		BString currentTempString;
		BNumberFormat numberFormat;
		numberFormat.Format(currentTempString, static_cast<double>(convertedTemp));
		currentTempString.Append(SymbolForScale(scale, 1));
		currentTempControl->SetText(currentTempString);

		BString criticalTempString;
		if(activeDevice.IsTemperatureReported(TEMPERATURE_CRITICAL)) {
			float criticalTemp = activeDevice.ReadTemperature(TEMPERATURE_CRITICAL);
			float criticalConverted = ConvertToScale(criticalTemp, SCALE_CELSIUS, scale);

			BNumberFormat criticalFormat;
			criticalFormat.Format(criticalTempString, static_cast<double>(criticalConverted));
			criticalTempString.Append(SymbolForScale(scale, 1));
		}
		else
			criticalTempString.SetTo(B_TRANSLATE_COMMENT("N/A",
				"Abbreviated: when something is not available."));
		criticalTempControl->SetText(criticalTempString);

		Unlock();
		snooze(1000000);
	}
}

/* static */
void MainWindow::CallNotifyStopped(void* data)
{
	MainWindow* window = (MainWindow*)data;
	if(window) {
		window->PostMessage(M_STOPPED_RUNNING);
	}
}
