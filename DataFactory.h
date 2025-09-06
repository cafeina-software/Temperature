/*
 * Copyright 2025, cafeina, <cafeina@world>. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef __DATA_FACTORY__
#define __DATA_FACTORY__

#include <Archivable.h>
#include <String.h>
#include <StringList.h>

class DataFactory : public BArchivable
{
public:
	DataFactory();
	DataFactory(BMessage* from);
	DataFactory(const DataFactory& other);
	~DataFactory() override;
	
	// BArchivable interface
	status_t AllArchived(BMessage* archive) const override;
	status_t AllUnarchived(const BMessage* archive) override;
	status_t Archive(BMessage* into, bool deep = true) const override;
	status_t Perform(perform_code opcode, void* argument) override;
	
	static BArchivable* Instantiate(BMessage* archive);
	
	// Device utils
	static status_t FindThermalDevices(BStringList* outList);
	BStringList& ThermalDevices() const;
	
	// Application settings
	void SetActiveDevice(const char* devicePath);
	const char* ActiveDevice() const;
	
	void SetRefreshRate(uint32 seconds);
	uint32 RefreshRate() const;
	
	void SetWatermarkVisibility(bool state);
	bool WatermarkVisibility() const;
	
	void SetRunningStatus(bool state);
	bool RunningStatus() const;
private:
	BStringList fDevicesList;
	
	BString fActiveDevice;
	uint32 fSecondsToRefresh;
	bool fGraphWatermarkShown;
	bool fGraphRunningStatus;
};

#endif /* __DATA_FACTORY__ */
