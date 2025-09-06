/*
 * Copyright 2025, cafeina, <cafeina@world>. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <StringList.h>
#include <SupportDefs.h>
#include "DataFactory.h"

DataFactory::DataFactory()
: BArchivable(),
  fActiveDevice(""),
  fSecondsToRefresh(1),
  fGraphWatermarkShown(true),
  fGraphRunningStatus(true)
{
	DataFactory::FindThermalDevices(&fDevicesList);
}

DataFactory::DataFactory(BMessage* from)
: BArchivable(from)
{
	fActiveDevice = from->GetString("device", "");
	fSecondsToRefresh = from->GetUInt32("window:pulse", 1);
	fGraphWatermarkShown = from->GetBool("graph:watermark", true);
	fGraphRunningStatus = from->GetBool("graph:running", true);
	
	DataFactory::FindThermalDevices(&fDevicesList);
}

DataFactory::DataFactory(const DataFactory& other)
: fActiveDevice(other.fActiveDevice),
  fSecondsToRefresh(other.fSecondsToRefresh),
  fGraphWatermarkShown(other.fGraphWatermarkShown),
  fGraphRunningStatus(other.fGraphRunningStatus)
{
	DataFactory::FindThermalDevices(&fDevicesList);
}

DataFactory::~DataFactory()
{
}

// #pragma mark - BArchivable

status_t DataFactory::AllArchived(BMessage* archive) const
{
	return BArchivable::AllArchived(archive);
}

status_t DataFactory::AllUnarchived(const BMessage* archive)
{
	return BArchivable::AllUnarchived(archive);
}

status_t DataFactory::Archive(BMessage* into, bool deep) const
{
	if(into->ReplaceString("device", fActiveDevice) != B_OK)
		into->AddString("device", fActiveDevice);
	if(into->ReplaceUInt32("window:pulse", fSecondsToRefresh) != B_OK)
		into->AddUInt32("window:pulse", fSecondsToRefresh);
	if(into->ReplaceBool("graph:watermark", fGraphWatermarkShown) != B_OK)
		into->AddBool("graph:watermark", fGraphWatermarkShown);
	if(into->ReplaceBool("graph:running", fGraphRunningStatus) != B_OK)
		into->AddBool("graph:running", fGraphRunningStatus);
	
	return BArchivable::Archive(into, deep);
}

status_t DataFactory::Perform(perform_code opcode, void* argument)
{
	return BArchivable::Perform(opcode, argument);
}

/* static */ 
BArchivable* DataFactory::Instantiate(BMessage* archive)
{
	if(!validate_instantiation(archive, "DataFactory"))
		return NULL;
		
	return new DataFactory(archive);
}

// #pragma mark - Devices

/* static */
status_t DataFactory::FindThermalDevices(BStringList* outList)
{
	BDirectory powerDirectory("/dev/power");
	if(powerDirectory.InitCheck() != B_OK)
		return B_IO_ERROR;

	BEntry powerEntry;
	while(powerDirectory.GetNextEntry(&powerEntry, false) == B_OK) {
		BPath path;
		powerEntry.GetPath(&path);
		if(strstr(path.Leaf(), "thermal") == nullptr)
			continue;

		if(powerEntry.IsDirectory()) {
			BEntry powerSubEntry;
			BDirectory powerEntryDirectory(&powerEntry);
			while(powerEntryDirectory.GetNextEntry(&powerSubEntry, false) == B_OK) {
				if(!powerSubEntry.IsDirectory()) {
					BPath subEntryPath;
					powerSubEntry.GetPath(&subEntryPath);
					outList->Add(BString(subEntryPath.Path()));
				}
			}
		}
		else {
			outList->Add(BString(path.Path()));
		}
	}

	return B_OK;
}

BStringList& DataFactory::ThermalDevices()
{
	return fDevicesList;
}

// #pragma mark - Settings

void DataFactory::SetActiveDevice(const char* devicePath)
{
	fActiveDevice.SetTo(devicePath);
}

const char* DataFactory::ActiveDevice() const
{
	return fActiveDevice.String();
}

void DataFactory::SetRefreshRate(uint32 seconds)
{
	fSecondsToRefresh = seconds;
}

uint32 DataFactory::RefreshRate() const
{
	return fSecondsToRefresh;
}

void DataFactory::SetWatermarkVisibility(bool state)
{
	fGraphWatermarkShown = state;
}

bool DataFactory::WatermarkVisibility() const
{
	return fGraphWatermarkShown;
}

void DataFactory::SetRunningStatus(bool state)
{
	fGraphRunningStatus = state;
}

bool DataFactory::RunningStatus() const
{
	return fGraphRunningStatus;
}
