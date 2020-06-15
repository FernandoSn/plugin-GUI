/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2019 Allen Institute for Brain Science and Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __MCDAQCOMPONENTS_H__
#define __MCDAQCOMPONENTS_H__

#include <DataThreadHeaders.h>
#include <stdio.h>
#include <string.h>

#include "mc-api/cbw.h"

#define NUM_SOURCE_TYPES 4
#define CHANNEL_BUFFER_SIZE 1000
#define MAX_ANALOG_CHANNELS 16
#define NUM_SAMPLE_RATES 17
#define MAX_DIGITAL_CHANNELS 8
#define ERR_BUFF_SIZE 2048
#define STR2CHR( jString ) ((jString).toUTF8())

#define MAXNUMDEVS 10

//#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
//#define MCDAQErrChk(ULStat) if(ULStat != 0) throw Board::MCCException( __LINE__,__FILE__,ULStat ) This is ideal but OE architecture doesn't support exceptions, I'll try to implement this is in the future. For now I am outputting to console.
#define MCDAQErrChk(ULStat) if(ULStat != 0) std::cout << "MCDAQ error code(ULStat): " << ULStat;

class MCDAQbd;
class InputChannel;
class AnalogIn;
class DigitalIn;
class Trigger;
class Counter;
class UserDefined;

class MCDAQComponent
{
public:
	MCDAQComponent();
	~MCDAQComponent();
	int serial_number;
	virtual void getInfo() = 0;
};

/* API */

class MCDAQAPI
{
public:
	void getInfo();
};

class MCDAQbdDeviceManager
{
public:
	MCDAQbdDeviceManager();
	~MCDAQbdDeviceManager();

	void scanForDevices();

	String getDeviceFromIndex(int deviceIndex);
	String getDeviceFromProductName(String productName);
	const MCDAQ::DaqDeviceDescriptor& GetDeviceDescFromIndex(int deviceIndex);
	const MCDAQ::DaqDeviceDescriptor& GetDeviceDescProductName(String productName);

	int getNumAvailableDevices();

	friend class MCDAQThread;

private:
	int selectedDeviceIndex; //This var seems to be not init.
	int NumberOfDevices = MAXNUMDEVS;
	MCDAQ::DaqDeviceDescriptor DeviceInventory[MAXNUMDEVS];
	float   RevLevel = (float)CURRENTREVNUM;
	
};

struct VRange {
	float vmin, vmax;
	int MCCcode;
	VRange() : vmin(0), vmax(0) {}
	VRange(float rmin, float rmax, int MCCcode)
		: vmin(rmin), vmax(rmax), MCCcode(MCCcode) {}
};

struct SRange {
	unsigned int smin, smaxs, smaxm;
	SRange() : smin(0), smaxs(0), smaxm(0) {}
	SRange(unsigned int smin, unsigned int smaxs, unsigned int smaxm)
		: smin(smin), smaxs(smaxs), smaxm(smaxm) {}
};

enum SOURCE_TYPE {
	RSE1 = 0,
	NRSE2,
	DIFF3,
	PSEUDO_DIFF4
};

class MCDAQbd : public Thread
{
public:
	
	MCDAQbd();
	MCDAQbd(const MCDAQ::DaqDeviceDescriptor& DeviceInfo, int BoardNum);
	~MCDAQbd();

	void connect(); 

	String getProductName();
	String getSerialNumber();

	void getAIChannels();
	void getAIVoltageRanges();
	void getDIChannels();

	//SOURCE_TYPE getSourceTypeForInput(int index);
	void toggleSourceType();

	int getActiveDigitalLines();

	void run();

	friend class MCDAQThread;

private:

	static void CALLBACK ProcBackgroundBoard(int BoardNum, unsigned EventType, unsigned EventData, void* UserData);

private:

	String							deviceName;
	String							productName;
	MCDAQ::DaqDeviceInterface		deviceCategory;
	unsigned int					productNum;
	long long						serialNum;
	int								BoardNum;
	bool							isUSBDevice;
	bool							simAISamplingSupported;
	int								ADCResolution;
	bool							LowRes;
	bool							SupportsDiff;
	SRange 							sampleRateRange;

	Array<VRange>					aiVRanges;
	VRange							voltageRange;

	Array<float>					sampleRates;
	float							samplerate;

	Array<AnalogIn> 				ai;
	SOURCE_TYPE						st;
	//Array<MCDAQ::int32>				terminalConfig;
	//Array<SOURCE_TYPE>				st;
	bool							DiffOn;
	Array<bool>						aiChannelEnabled;

	Array<DigitalIn> 				di;
	Array<bool>						diChannelEnabled;

	//MCDAQ::float64					ai_data[CHANNEL_BUFFER_SIZE * MAX_ANALOG_CHANNELS];
	//uint16_t					ai_data[CHANNEL_BUFFER_SIZE * MAX_ANALOG_CHANNELS];
	//uint16_t					ai_dataRaw[CHANNEL_BUFFER_SIZE * MAX_ANALOG_CHANNELS];
	//MCDAQ::uInt8					di_data_8[CHANNEL_BUFFER_SIZE];  //PXI devices use 8-bit read
	//MCDAQ::uInt32					di_data_32[CHANNEL_BUFFER_SIZE]; //USB devices use 32-bit read
	DOUBLE						ai_data[256];
	DOUBLE						ai_dataCopy[256];

	int64 ai_timestamp;
	uint64 eventCode;

	DataBuffer* aiBuffer;

	bool ProcFinished = false;

};

/* Inputs */ 

class InputChannel
{
public:
	InputChannel();
	InputChannel(String id);
	~InputChannel();

	void setSampleRate(int rateIndex);
	int getSampleRate();

	void setEnabled(bool);

	void startAcquisition();
	void stopAcquisition();

	void setSavingDirectory(File);
	File getSavingDirectory();

	float getFillPercentage();

	friend class MCDAQbd;
	
private:
	String id;
	bool enabled;
	File savingDirectory;

};

class AnalogIn : public InputChannel
{

public:
	AnalogIn();
	AnalogIn(String id);
	~AnalogIn();

	Array<SOURCE_TYPE> getTerminalConfig();

private:
	//MCDAQ::int32 terminalConfig;
};

class DigitalIn : public InputChannel
{
public:
	DigitalIn(String id);
	DigitalIn();
	~DigitalIn();
private:
};

#endif  // __MCDAQCOMPONENTS_H__