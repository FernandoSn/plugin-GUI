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

	/*for (auto it = DeviceInventory.begin(), end = DeviceInventory.end(); it < end; ++it)
	{
		
		std::unique_ptr<MCDAQbd> n = std::make_unique<MCDAQbd>(it->ProductName);
		if (n->getProductName() == productName)
			return String(it->ProductName);

	}*/

	return "";
}

const MCDAQ::DaqDeviceDescriptor& MCDAQbdDeviceManager::GetDeviceDescFromIndex(int deviceIndex)
{
	return DeviceInventory[deviceIndex];
}

const MCDAQ::DaqDeviceDescriptor& MCDAQbdDeviceManager::GetDeviceDescProductName(String productName)
{
	// TODO: insert return statement here
	return DeviceInventory[0];
}

int MCDAQbdDeviceManager::getNumAvailableDevices()
{
	return NumberOfDevices;
}

MCDAQbd::MCDAQbd() : Thread("MCDAQbd_Thread") {};

MCDAQbd::MCDAQbd(const MCDAQ::DaqDeviceDescriptor& DeviceInfo, int BoardNum)
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


	sampleRateRange = SRange(500, 30000, 30000); //this is arbitrary but MCC library doesnt support checking max sampling rate before acqusitionn.

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
		for (int RangeCode = 0; RangeCode < 20; RangeCode++)
		{
			if (!MCDAQ::cbAIn(BoardNum, 0, RangeCode, &dataValue))
			{
				GetRangeInfo(RangeCode, RangeString, &RangeVolts);
				aiVRanges.add(VRange(0.0 - RangeVolts/2.0, RangeVolts/2.0,RangeCode));
			}
		}
	}
	fflush(stdout);
}

void MCDAQbd::getAIChannels()
{
	//TODO: Toggle between SE or DIFF
	MCDAQErrChk(MCDAQ::cbAInputMode(BoardNum, SINGLE_ENDED));
	SupportsDiff = true;
	DiffOn = false;

	int NumberOfAIChannels;

	MCDAQErrChk(MCDAQ::cbGetConfig(BOARDINFO, BoardNum, 0, BINUMADCHANS, &NumberOfAIChannels));
	//std::cout << "Channel MCC: " << NumberOfAIChannels << "\n";
	int aiCount = 0;

	//VRange vRange = aiVRanges[0];

	for (int i = 0; i < NumberOfAIChannels; i++)
	{
		if (aiCount++ < MAX_ANALOG_CHANNELS)
		{
			std::string temp = std::to_string(i);

			ai.add(AnalogIn(String(temp.c_str())));
			//std::cout << "FEEEEr channel :" << ai[i].id.getIntValue() << "\n";
			//terminalConfig.add(termCfgs);

			aiChannelEnabled.add(true);

		}
	}

	fflush(stdout);
}

void MCDAQbd::getDIChannels()
{

	int NumberOfDIChannels;
	MCDAQErrChk(MCDAQ::cbGetConfig(BOARDINFO, BoardNum, 0, DICONFIG, &NumberOfDIChannels));

	NumberOfDIChannels /= 8; //This works for USB-1608G. I am assuming DICONFIG returns the total bits that the board can handle.
	
	int diCount = 0;

	for (int i = 0; i < NumberOfDIChannels; i++)
	{
		if (diCount++ < MAX_DIGITAL_CHANNELS)
		{
			std::string temp = std::to_string(i);

			di.add(DigitalIn(String(temp.c_str())));

			//terminalConfig.add(termCfgs);

			diChannelEnabled.add(true);

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

	if (DiffOn)
		DiffOn = false;
	else
		DiffOn = true;
	
}

void MCDAQbd::run()
{
	
	int Packet20Hz = 256;
	int LowChan = 0;
	int HighChan;

	if (DiffOn)
		HighChan = 7;
	else
		HighChan = 15;

	//long Rate = 2000;
	long Rate = samplerate;
	//std::cout << "Rate: " << Rate << "\n";

	int Gain = voltageRange.MCCcode;
	//unsigned Options = CONVERTDATA + BACKGROUND + CONTINUOUS + BLOCKIO; //CONVERTDATA
	unsigned Options = SCALEDATA + CONVERTDATA + BACKGROUND + CONTINUOUS + SINGLEIO; //SCALEDATA OUTPUTS DOUBLE PREC.



	//Enable CALLBACK Board::ProcBackgroundBoard
	MCDAQErrChk(MCDAQ::cbEnableEvent(BoardNum, ON_DATA_AVAILABLE, Packet20Hz, MCDAQbd::ProcBackgroundBoard, this));

	// IMPORTANT : Putting this thread to sleep to get the correct Packets.
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Starting Scan.
	MCDAQErrChk(MCDAQ::cbAInScan(BoardNum, LowChan, HighChan, Packet20Hz, &Rate, Gain,
		reinterpret_cast<void*>(ai_data), Options));

	ai_timestamp = 0;
	eventCode = 0;

	while (!threadShouldExit())
	{
		
		
		//Loop to handle Analog inputs.
		//Actually it seems that analog and digital are added to the same or different buffers with the same func call
		//ie. addToBuffer.
		if (ProcFinished)
		{
			ProcFinished = false;
			float aiSamples[MAX_ANALOG_CHANNELS];
			for (int i = 0; i < Packet20Hz; i++)
			{
	
				int channel = i % MAX_ANALOG_CHANNELS;

				aiSamples[channel] = 0;
				if (aiChannelEnabled[channel])
					aiSamples[channel] = (float)ai_dataCopy[i];

				if (i % MAX_ANALOG_CHANNELS == 0)
				{
					ai_timestamp++;
					aiBuffer->addToBuffer(aiSamples, &ai_timestamp, &eventCode, 1);
					//DebugMCFile << ai_data[channel] << "\n";
				}

			}
			
			fflush(stdout);


		}

	}

	/*********************************************/
	// DAQmx Stop Code
	/*********************************************/

	MCDAQErrChk(MCDAQ::cbStopBackground(BoardNum, AIFUNCTION));

	fflush(stdout);

	return;

}

void MCDAQbd::ProcBackgroundBoard(int BoardNum, unsigned EventType, unsigned EventData, void* UserData)
{
	//NOTE: this Proc could be implemented passing a pointer to This Board on the UserData Param.
	//However I found that it is a bit slower. Static variables do the work faster, however the code looks messier.
	// Im gonna stick with static variables for now, but be aware that it is posible to use pure member variables.


	//ftRec.StartFrame();
	//std::memcpy(TempRecData, RecordingData, TempCpySize);
	//RecordingFile.write(reinterpret_cast<char*>(TempRecData), TempCpySize);
	//BoardGfxReady = true;
	//ftRec.StopFrame(logfileRec);

	//DebugMCFile << EventData << "\n";
	//Conteo = EventData;

	std::memcpy(((MCDAQbd*)UserData)->ai_dataCopy, ((MCDAQbd*)UserData)->ai_data, 256 * 8);
	//std::memcpy(((NIDAQmx*)UserData)->ai_dataMCCcopy, ((NIDAQmx*)UserData)->ai_dataMCC, 256 * 8);
	((MCDAQbd*)UserData)->ProcFinished = true;
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



