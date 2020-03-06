/*
------------------------------------------------------------------

Fernando Santos Valencia
The Franks Lab
the MCC plugin is currently under development based on the NIDAQmx plugin.
In my opinion NIDAQmx plugin needs improvement.
I will not change the architecture for now due to time constrains.

Additionally, I found Juce library not the best to handle strings.
all the String management could be done better with the standard library.

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

#include <chrono>
#include <math.h>

#include "MCDAQComponents.h"
#include "mc-api/Utilities.h"
#include <fstream>
#include <string>

std::ofstream DebugMCFile("DebugMCFile.txt");

static int32 GetTerminalNameWithDevPrefix(MCDAQ::TaskHandle taskHandle, const char terminalName[], char triggerName[]);

static int32 GetTerminalNameWithDevPrefix(MCDAQ::TaskHandle taskHandle, const char terminalName[], char triggerName[])
{

	MCDAQ::int32	error = 0;
	char			device[256];
	MCDAQ::int32	productCategory;
	MCDAQ::uInt32	numDevices, i = 1;

	DAQmxErrChk(MCDAQ::DAQmxGetTaskNumDevices(taskHandle, &numDevices));
	while (i <= numDevices) {
		DAQmxErrChk(MCDAQ::DAQmxGetNthTaskDevice(taskHandle, i++, device, 256));
		DAQmxErrChk(MCDAQ::DAQmxGetDevProductCategory(device, &productCategory));
		if (productCategory != DAQmx_Val_CSeriesModule && productCategory != DAQmx_Val_SCXIModule) {
			*triggerName++ = '/';
			strcat(strcat(strcpy(triggerName, device), "/"), terminalName);
			break;
		}
	}

Error:
	return error;
}

MCDAQComponent::MCDAQComponent() : serial_number(0) {}
MCDAQComponent::~MCDAQComponent() {}

void MCDAQAPI::getInfo()
{
	//TODO
}

MCDAQbdDeviceManager::MCDAQbdDeviceManager() 
{
	//Disabling MCC error system so we can handle them ourselves.
	MCDAQErrChk(MCDAQ::cbErrHandling(DONTPRINT, DONTSTOP));
	//Ignore instacall
	MCDAQErrChk(MCDAQ::cbIgnoreInstaCal());
	//Set the version of the api.
	MCDAQErrChk(MCDAQ::cbDeclareRevision(&RevLevel));
	
}

MCDAQbdDeviceManager::~MCDAQbdDeviceManager() {}

void MCDAQbdDeviceManager::scanForDevices()
{
	MCDAQErrChk(MCDAQ::cbGetDaqDeviceInventory(MCDAQ::DaqDeviceInterface::ANY_IFC, DeviceInventory, &NumberOfDevices));
}

String MCDAQbdDeviceManager::getDeviceFromIndex(int deviceIndex)
{
	return String(DeviceInventory[deviceIndex].ProductName);
}

String MCDAQbdDeviceManager::getDeviceFromProductName(String productName)
{
	//I don't really get why are they doing this redundant instantiation
	//Anyway I will adapt my code to this and replace Juce ScopedPointers to Std unique ptr. In the future I'll rewrite this but for now this should work
	//I dont think this gonna impact performance, plus memory is being freed so it is fine for now.

	//Update: I dont think I am going to use this func at all. I believe this was created because for NIDAQmx the device name and product name are different.
	//for MC the "device name" doesnt exist.

	for (auto it = DeviceInventory.begin(), end = DeviceInventory.end(); it < end; ++it)
	{
		
		std::unique_ptr<MCDAQbd> n = std::make_unique<MCDAQbd>(it->ProductName);
		if (n->getProductName() == productName)
			return String(it->ProductName);

	}

	return "";
}

const MCDAQ::DaqDeviceDescriptor& MCDAQbdDeviceManager::GetDeviceDescFromIndex(int deviceIndex)
{
	return DeviceInventory[deviceIndex];
}

const MCDAQ::DaqDeviceDescriptor& MCDAQbdDeviceManager::GetDeviceDescProductName(int deviceIndex)
{
	// TODO: insert return statement here
}

int MCDAQbdDeviceManager::getNumAvailableDevices()
{
	return NumberOfDevices;
}

MCDAQbd::MCDAQbd() : Thread("MCDAQbd_Thread") {};

MCDAQbd::MCDAQbd(MCDAQ::DaqDeviceDescriptor DeviceInfo, int BoardNum)
	: Thread("MCDAQbd_Thread"),
	deviceName(DeviceInfo.DevString),
	productName(DeviceInfo.ProductName),
	deviceCategory(DeviceInfo.InterfaceType),
	productNum(DeviceInfo.ProductID),
	serialNum(DeviceInfo.NUID),
	BoardNum(BoardNum)
{
	//Creating Board on MCC Lib.
	MCDAQErrChk(MCDAQ::cbCreateDaqDevice(BoardNum, DeviceInfo));

	//TODO: Aadd a check for diff channels. Test this with Usb-204 board.
	/*if (MCDAQ::cbAInputMode(BoardNum, SINGLE_ENDED) != 0)
		DiffCh = false;*/

	//Getting the ADC Resolution.
	MCDAQErrChk(MCDAQ::cbGetConfig(BOARDINFO, BoardNum, 0, BIADRES, &ADCResolution));

	//Getting the resolution bool.
	ADCResolution > 16 ? LowRes = false : LowRes = true;



	connect();

	//this could be static.
	float sample_rates[NUM_SAMPLE_RATES] = {
		1000.0f, 1250.0f, 1500.0f,
		2000.0f, 2500.0f,
		3000.0f, 3330.0f,
		4000.0f,
		5000.0f,
		6250.0f,
		8000.0f,
		10000.0f,
		12500.0f,
		15000.0f,
		20000.0f,
		25000.0f,
		30000.0f
	};

	int idx = 0;
	while (sample_rates[idx] <= sampleRateRange.smaxm && idx < NUM_SAMPLE_RATES)
		sampleRates.add(sample_rates[idx++]);

	// Default to highest sample rate
	samplerate = sampleRates[sampleRates.size() - 1];

	// Default to largest voltage range
	voltageRange = aiVRanges[aiVRanges.size() - 1];

	// Enable all channels by default
	for (int i = 0; i < aiChannelEnabled.size(); i++)
		aiChannelEnabled.set(i, true);

	for (int i = 0; i < diChannelEnabled.size(); i++)
		diChannelEnabled.set(i, true);


}

MCDAQbd::~MCDAQbd() {
}

String MCDAQbd::getProductName()
{
	return productName;
}

String MCDAQbd::getSerialNumber()
{
	return String(serialNum);
}

void MCDAQbd::connect()
{


	fflush(stdout);

	getAIVoltageRanges();
	getAIChannels();
	getDIChannels();

	if (!simAISamplingSupported)
		smaxm = smaxs / ai.size();

	sampleRateRange = SRange(smin, smaxs, smaxm);

	printf("Min sample rate: %1.2f\n", sampleRateRange.smin);
	printf("Max single channel sample rate: %1.2f\n", sampleRateRange.smaxs);
	printf("Max multi channel sample rate: %1.2f\n", sampleRateRange.smaxm);

}

void MCDAQbd::getAIVoltageRanges()
{
	char RangeString[RANGENAMELEN];
	double RangeVolts;

	if (LowRes)
	{
		unsigned short dataValue;
		for (int RangeCode = 0; RangeCode < 20; i++)
		{
			if (!MCDAQ::cbAIn(BoardNum, 0, RangeCode, &dataValue))
			{
				GetRangeInfo(RangeCode, RangeString, &RangeVolts);
				aiVRanges.add(VRange(0.0 - RangeVolts/2.0, RangeVolts/2.0));
			}
		}
	}
	fflush(stdout);
}

void MCDAQbd::getAIChannels()
{
	//TODO: Toggle between SE or DIFF
	MCDAQErrChk(MCDAQ::cbAInputMode(BoardNum, SINGLE_ENDED));
	DiffCh = true;

	int NumberOfChannels;

	MCDAQErrChk(MCDAQ::cbGetConfig(BOARDINFO, BoardNum, 0, DINUMBITS, &NumberOfChannels));

	int aiCount = 0;

	VRange vRange = aiVRanges[0];

	for (int i = 0; i < NumberOfChannels; i++)
	{
		if (aiCount++ < MAX_ANALOG_CHANNELS)
		{

			ai.add(AnalogIn(std::to_string(i)));

			//terminalConfig.add(termCfgs);

			aiChannelEnabled.add(true);

		}
	}

	fflush(stdout);
}

void MCDAQbd::getDIChannels()
{

	char data[2048];
	//MCDAQ::DAQmxGetDevTerminals(STR2CHR(deviceName), &data[0], sizeof(data)); //gets all terminals
	//MCDAQ::DAQmxGetDevDIPorts(STR2CHR(deviceName), &data[0], sizeof(data));	//gets line name
	MCDAQ::DAQmxGetDevDILines(STR2CHR(deviceName), &data[0], sizeof(data));	//gets ports on line
	printf("Found digital inputs: \n");

	StringArray channel_list;
	channel_list.addTokens(&data[0], ", ", "\"");

	int diCount = 0;

	for (int i = 0; i < channel_list.size(); i++)
	{
		StringArray channel_type;
		channel_type.addTokens(channel_list[i], "/", "\"");
		if (channel_list[i].length() > 0 && diCount++ < MAX_DIGITAL_CHANNELS)
		{
			printf("%s\n", channel_list[i].toUTF8());
			di.add(DigitalIn(channel_list[i].toUTF8()));
			diChannelEnabled.add(false);
		}
	}

	fflush(stdout);

}

int MCDAQbd::getActiveDigitalLines()
{
	uint16 linesEnabled = 0;
	for (int i = 0; i < diChannelEnabled.size(); i++)
	{
		if (diChannelEnabled[i])
			linesEnabled += pow(2, i);
	}
	return linesEnabled;
}

void MCDAQbd::toggleSourceType(int index)
{

	SOURCE_TYPE current = st[index];
	int next = (static_cast<int>(current)+1) % NUM_SOURCE_TYPES;
	SOURCE_TYPE source = static_cast<SOURCE_TYPE>(next);
	while (!((1 << next) & terminalConfig[index]))
		source = static_cast<SOURCE_TYPE>(++next % NUM_SOURCE_TYPES);
	st.set(index,source);

}

void MCDAQbd::run()
{
	/* Derived from MCDAQbd: ANSI C Example program: ContAI-ReadDigChan.c */

	MCDAQ::int32	error = 0;
	char			errBuff[ERR_BUFF_SIZE] = { '\0' };

	/**************************************/
	/********CONFIG ANALOG CHANNELS********/
	/**************************************/

	MCDAQ::int32		ai_read = 0;
	static int			totalAIRead = 0;
	MCDAQ::TaskHandle	taskHandleAI = 0;

	String usePort; //Temporary digital port restriction until software buffering is implemented

	/* Create an analog input task */
	if (isUSBDevice)
		DAQmxErrChk(MCDAQ::DAQmxCreateTask(STR2CHR("AITask_USB" + getSerialNumber()), &taskHandleAI));
	else
		DAQmxErrChk(MCDAQ::DAQmxCreateTask(STR2CHR("AITask_PXI" + getSerialNumber()), &taskHandleAI));


	/* Create a voltage channel for each analog input */
	for (int i = 0; i < ai.size(); i++)
	{
		MCDAQ::int32 termConfig;

		switch (st[i]) {
		case SOURCE_TYPE::RSE:
			termConfig = DAQmx_Val_RSE;
		case SOURCE_TYPE::NRSE:
			termConfig = DAQmx_Val_NRSE;
		case SOURCE_TYPE::DIFF:
			termConfig = DAQmx_Val_Diff;
		case SOURCE_TYPE::PSEUDO_DIFF:
			termConfig = DAQmx_Val_PseudoDiff;
		default:
			termConfig = DAQmx_Val_Cfg_Default;
		}

		DAQmxErrChk(MCDAQ::DAQmxCreateAIVoltageChan(
			taskHandleAI,					//task handle
			STR2CHR(ai[i].id),			//MCDAQ physical channel name (e.g. dev1/ai1)
			"",							//user-defined channel name (optional)
			termConfig,					//input terminal configuration
			voltageRange.vmin,			//min input voltage
			voltageRange.vmax,			//max input voltage
			DAQmx_Val_Volts,			//voltage units
			NULL));

	}

	/* Configure sample clock timing */
	DAQmxErrChk(MCDAQ::DAQmxCfgSampClkTiming(
		taskHandleAI,
		"",													//source : NULL means use internal clock
		samplerate,											//rate : samples per second per channel
		DAQmx_Val_Rising,									//activeEdge : (DAQmc_Val_Rising || DAQmx_Val_Falling)
		DAQmx_Val_ContSamps,								//sampleMode : (DAQmx_Val_FiniteSamps || DAQmx_Val_ContSamps || DAQmx_Val_HWTimedSinglePoint)
		MAX_ANALOG_CHANNELS * CHANNEL_BUFFER_SIZE));		//sampsPerChanToAcquire : 
																//If sampleMode == DAQmx_Val_FiniteSamps : # of samples to acquire for each channel
																//Elif sampleMode == DAQmx_Val_ContSamps : circular buffer size


	/* Get handle to analog trigger to sync with digital inputs */
	char trigName[256];
	DAQmxErrChk(GetTerminalNameWithDevPrefix(taskHandleAI, "ai/SampleClock", trigName));

	/************************************/
	/********CONFIG DIGITAL LINES********/
	/************************************/

	MCDAQ::int32		di_read = 0;
	static int			totalDIRead = 0;
	MCDAQ::TaskHandle	taskHandleDI = 0;

	char ports[2048];
	MCDAQ::DAQmxGetDevDIPorts(STR2CHR(deviceName), &ports[0], sizeof(ports));

	/* For now, restrict max num digital inputs until software buffering is implemented */
	if (MAX_DIGITAL_CHANNELS <= 8)
	{
		StringArray port_list;
		port_list.addTokens(&ports[0], ", ", "\"");
		usePort = port_list[0];
	}

	/* Create a digital input task using device serial number to gurantee unique task name per device */
	if (isUSBDevice)
		DAQmxErrChk(MCDAQ::DAQmxCreateTask(STR2CHR("DITask_USB"+getSerialNumber()), &taskHandleDI));
	else
		DAQmxErrChk(MCDAQ::DAQmxCreateTask(STR2CHR("DITask_PXI"+getSerialNumber()), &taskHandleDI));

	/* Create a channel for each digital input */
	DAQmxErrChk(MCDAQ::DAQmxCreateDIChan(
		taskHandleDI,
		STR2CHR(usePort),
		"",
		DAQmx_Val_ChanForAllLines));

	
	if (!isUSBDevice) //USB devices do not have an internal clock and instead use CPU, so we can't configure the sample clock timing
		DAQmxErrChk(MCDAQ::DAQmxCfgSampClkTiming(
			taskHandleDI,							//task handle
			trigName,								//source : NULL means use internal clock, we will sync to analog input clock
			samplerate,								//rate : samples per second per channel
			DAQmx_Val_Rising,						//activeEdge : (DAQmc_Val_Rising || DAQmx_Val_Falling)
			DAQmx_Val_ContSamps,					//sampleMode : (DAQmx_Val_FiniteSamps || DAQmx_Val_ContSamps || DAQmx_Val_HWTimedSinglePoint)
			CHANNEL_BUFFER_SIZE));					//sampsPerChanToAcquire : want to sync with analog samples per channel
														//If sampleMode == DAQmx_Val_FiniteSamps : # of samples to acquire for each channel
														//Elif sampleMode == DAQmx_Val_ContSamps : circular buffer size

	DAQmxErrChk(MCDAQ::DAQmxTaskControl(taskHandleAI, DAQmx_Val_Task_Commit));
	DAQmxErrChk(MCDAQ::DAQmxTaskControl(taskHandleDI, DAQmx_Val_Task_Commit));

	DAQmxErrChk(MCDAQ::DAQmxStartTask(taskHandleDI));
	DAQmxErrChk(MCDAQ::DAQmxStartTask(taskHandleAI));

	MCDAQ::int32 numSampsPerChan;
	MCDAQ::float64 timeout;
	if (isUSBDevice)
	{
		//This is an arbitrary number, if the main loop takes longer that 300 samples per chan to complete 
		//some samples are not gonna get written on file. You can increase this num up to CHANNEL_BUFFER_SIZE.
		numSampsPerChan = 300;
		timeout = -1;
	}
	else
	{
		numSampsPerChan = CHANNEL_BUFFER_SIZE;
		timeout = -1;
	}

	int TotalAnalogChans = ai.size();
	MCDAQ::int32 arraySizeInSamps = TotalAnalogChans * numSampsPerChan;

	uint64 linesEnabled = 0;

	ai_timestamp = 0;
	eventCode = 0;

	while (!threadShouldExit())
	{
		//-1 is passed to read all the samples currently available in the board buffer. 
		//For buffer size look into the manual of your board.
		DAQmxErrChk(MCDAQ::DAQmxReadAnalogF64(
			taskHandleAI,
			-1,
			timeout,
			DAQmx_Val_GroupByScanNumber, //DAQmx_Val_GroupByScanNumber
			ai_data,
			arraySizeInSamps,
			&ai_read,
			NULL));

		int DigitalLines = getActiveDigitalLines();

		if (DigitalLines)
		{
			if (isUSBDevice)
				DAQmxErrChk(MCDAQ::DAQmxReadDigitalU32(
					taskHandleDI,
					-1,
					timeout,
					DAQmx_Val_GroupByScanNumber,
					di_data_32,
					numSampsPerChan,
					&di_read,
					NULL));
			else 
				DAQmxErrChk(MCDAQ::DAQmxReadDigitalU8(
					taskHandleDI,
					-1,
					timeout,
					DAQmx_Val_GroupByScanNumber,
					di_data_8,
					numSampsPerChan,
					&di_read,
					NULL));
		}

		/*
		std::chrono::milliseconds last_time;
		std::chrono::milliseconds t = std::chrono::duration_cast< std::chrono::milliseconds >(
			std::chrono::system_clock::now().time_since_epoch());
		long long t_ms = t.count()*std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den;
		if (ai_read>0) {
			printf("Read @ %i | ", t_ms);
			printf("Acquired %d AI samples. Total %d | ", (int)ai_read, (int)(totalAIRead += ai_read));
			printf("Acquired %d DI samples. Total %d\n", (int)di_read, (int)(totalDIRead += di_read));
			fflush(stdout);
		}
		*/

		//Loop to handle Digital inputs
		int count = 0;
		for (int i = 0; i < di_read; i++)
		{
				if (DigitalLines) //i% MAX_ANALOG_CHANNELS == 0 && //This gate could be added
				{
					//Bitwise operations to get the mask for the eventCode.
					if (isUSBDevice)
						eventCode = di_data_32[count++] & DigitalLines;
					else
						eventCode = di_data_8[count++] & DigitalLines;
				}
		}

		//Loop to handle Analog inputs.
		//Actually it seems that analog and digital are added to the same or different buffers with the same func call
		//ie. addToBuffer.

		float aiSamples[MAX_ANALOG_CHANNELS];
		for (int i = 0; i < TotalAnalogChans * ai_read; i++)
		{
	
			int channel = i % MAX_ANALOG_CHANNELS;

			aiSamples[channel] = 0;
			if (aiChannelEnabled[channel])
				aiSamples[channel] = ai_data[i];

			if (i % MAX_ANALOG_CHANNELS == 0)
			{
				ai_timestamp++;
				aiBuffer->addToBuffer(aiSamples, &ai_timestamp, &eventCode, 1);
			}

		}
		fflush(stdout);
	}

	/*********************************************/
	// DAQmx Stop Code
	/*********************************************/

	MCDAQ::DAQmxStopTask(taskHandleAI);
	MCDAQ::DAQmxClearTask(taskHandleAI);
	MCDAQ::DAQmxStopTask(taskHandleDI);
	MCDAQ::DAQmxClearTask(taskHandleDI);

	return;

Error:

	if (DAQmxFailed(error))
		MCDAQ::DAQmxGetExtendedErrorInfo(errBuff, ERR_BUFF_SIZE);

	if (taskHandleAI != 0) {
		// DAQmx Stop Code
		MCDAQ::DAQmxStopTask(taskHandleAI);
		MCDAQ::DAQmxClearTask(taskHandleAI);
	}

	if (taskHandleDI != 0) {
		// DAQmx Stop Code
		MCDAQ::DAQmxStopTask(taskHandleDI);
		MCDAQ::DAQmxClearTask(taskHandleDI);
	}
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
		fflush(stdout);

	return;

}

InputChannel::InputChannel()
{

}

InputChannel::InputChannel(String id) : id(id), enabled(true)
{
}

InputChannel::~InputChannel()
{
}

void InputChannel::setEnabled(bool enable)
{
	enabled = enable;
}

AnalogIn::AnalogIn()
{
}

AnalogIn::AnalogIn(String id) : InputChannel(id)
{
	
}

AnalogIn::~AnalogIn()
{

}

DigitalIn::DigitalIn(String id) : InputChannel(id)
{

}

DigitalIn::DigitalIn()
{

}

DigitalIn::~DigitalIn()
{

}



