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

#include "MCDAQThread.h"
#include "MCDAQEditor.h"
#include <stdexcept>

DataThread* MCDAQThread::createDataThread(SourceNode *sn)
{
	return new MCDAQThread(sn);
}

GenericEditor* MCDAQThread::createEditor(SourceNode* sn)
{
    return new MCDAQEditor(sn, this, true);
}

MCDAQThread::MCDAQThread(SourceNode* sn) : DataThread(sn), inputAvailable(false)
{

	dm = std::make_unique<MCDAQbdDeviceManager>();

	dm->scanForDevices();

	if (dm->getNumAvailableDevices() == 0)
	{
		//Okay for now as plugin-GUI handles source init runtime errors. 
		throw std::runtime_error("No MCDAQ devices detected!");
	}

	inputAvailable = true;
	openConnection();

}


MCDAQThread::~MCDAQThread()
{
}

int MCDAQThread::openConnection()
{
	//mMCDAQ = std::make_unique<MCDAQbd>(STR2CHR(dm->getDeviceFromIndex(0)));

	mMCDAQ = std::make_unique<MCDAQbd>(dm->GetDeviceDescFromIndex(0),0);

	sourceBuffers.add(new DataBuffer(getNumAnalogInputs(), 10000));

	mMCDAQ->aiBuffer = sourceBuffers.getLast();

	sampleRateIndex = mMCDAQ->sampleRates.size() - 1;
	setSampleRate(sampleRateIndex);

	voltageRangeIndex = mMCDAQ->aiVRanges.size() - 1;
	setVoltageRange(voltageRangeIndex);

	return 0;

}

int MCDAQThread::getNumAvailableDevices()
{
	return dm->getNumAvailableDevices();
}

void MCDAQThread::selectFromAvailableDevices()
{

	PopupMenu deviceSelect;
	StringArray productNames;
	for (int i = 0; i < getNumAvailableDevices(); i++)
	{
		//ScopedPointer<MCDAQbd> n = new MCDAQbd(STR2CHR(dm->getDeviceFromIndex(i)));
		std::unique_ptr<MCDAQbd> n = std::make_unique<MCDAQbd>(dm->GetDeviceDescFromIndex(i), i);
		if (!(n->getProductName() == getProductName()))
		{
			deviceSelect.addItem(productNames.size() + 1, "Swap to " + n->getProductName());
			productNames.add(n->getProductName());
		}
	}
	int selectedDeviceIndex = deviceSelect.show();
	if (selectedDeviceIndex == 0) //user clicked outside of popup window
		return;

	swapConnection(productNames[selectedDeviceIndex - 1]);
}

String MCDAQThread::getProductName() const
{
	return mMCDAQ->productName;
}

int MCDAQThread::swapConnection(String productName)
{

	if (!dm->getDeviceFromProductName(productName).isEmpty())
	{
		//mMCDAQ = new MCDAQbd(STR2CHR(dm->getDeviceFromProductName(productName)));
		mMCDAQ = std::make_unique<MCDAQbd>(dm->GetDeviceDescProductName(productName), 0);

		sourceBuffers.removeLast();
		sourceBuffers.add(new DataBuffer(getNumAnalogInputs(), 10000));
		mMCDAQ->aiBuffer = sourceBuffers.getLast();

		sampleRateIndex = mMCDAQ->sampleRates.size() - 1;
		setSampleRate(sampleRateIndex);

		voltageRangeIndex = mMCDAQ->aiVRanges.size() - 1;
		setVoltageRange(voltageRangeIndex);

		return 0;
	}
	return 1;

}

void MCDAQThread::toggleSourceType()
{
	mMCDAQ->toggleSourceType();
}

bool MCDAQThread::supportsDiffSourceType()
{
	return mMCDAQ->SupportsDiff;
}

SOURCE_TYPE MCDAQThread::getSourceTypeForInput()
{
	return mMCDAQ->st;
	//return SOURCE_TYPE::RSE1;
}

void MCDAQThread::closeConnection()
{
}

int MCDAQThread::getNumAnalogInputs() const
{
	return mMCDAQ->ai.size();
}

int MCDAQThread::getNumDigitalInputs() const
{
	return mMCDAQ->di.size();
}

void MCDAQThread::toggleAIChannel(int index)
{
	mMCDAQ->aiChannelEnabled.set(index, !mMCDAQ->aiChannelEnabled[index]);
}

void MCDAQThread::toggleDIChannel(int index)
{
	mMCDAQ->diChannelEnabled.set(index, !mMCDAQ->diChannelEnabled[index]);
}

void MCDAQThread::setVoltageRange(int rangeIndex)
{
	voltageRangeIndex = rangeIndex;
	mMCDAQ->voltageRange = mMCDAQ->aiVRanges[rangeIndex];
}

void MCDAQThread::setSampleRate(int rateIndex)
{
	sampleRateIndex = rateIndex;
	mMCDAQ->samplerate = mMCDAQ->sampleRates[rateIndex];
}

int MCDAQThread::getVoltageRangeIndex()
{
	return voltageRangeIndex;
}

int MCDAQThread::getSampleRateIndex()
{
	return sampleRateIndex;
}

Array<String> MCDAQThread::getVoltageRanges()
{
	Array<String> voltageRanges;
	for (auto range : mMCDAQ->aiVRanges)
	{
		voltageRanges.add(String(range.vmin) + "-" + String(range.vmax) + " V");
	}
	return voltageRanges;
}

Array<String> MCDAQThread::getSampleRates()
{
	Array<String> sampleRates;
	for (auto rate : mMCDAQ->sampleRates)
	{
		sampleRates.add(String(rate) + " S/s");
	}
	return sampleRates;
}

bool MCDAQThread::foundInputSource()
{
    return inputAvailable;
}

XmlElement MCDAQThread::getInfoXml()
{

	//TODO: 
	XmlElement MCDAQ_info("NI-DAQmx");
	XmlElement* api_info = new XmlElement("API");
	//api_info->setAttribute("version", api.version);
	MCDAQ_info.addChildElement(api_info);

	return MCDAQ_info;

}

/** Initializes data transfer.*/
bool MCDAQThread::startAcquisition()
{
	//TODO:
	mMCDAQ->startThread();
	this->startThread();
    return true;
}

/** Stops data transfer.*/
bool MCDAQThread::stopAcquisition()
{

	if (isThreadRunning())
	{
		signalThreadShouldExit();
	}
	if (mMCDAQ->isThreadRunning())
	{
		mMCDAQ->signalThreadShouldExit();
	}
    return true;
}

void MCDAQThread::setDefaultChannelNames()
{

	for (int i = 0; i < getNumAnalogInputs(); i++)
	{
		ChannelCustomInfo info;
		info.name = "AI" + String(i + 1);
		info.gain = mMCDAQ->voltageRange.vmax / float(0x7fff);
		channelInfo.set(i, info);
	}

}

bool MCDAQThread::usesCustomNames() const
{
	return true;
}

/** Returns the number of virtual subprocessors this source can generate */
unsigned int MCDAQThread::getNumSubProcessors() const
{
	return 1;
}

/** Returns the number of continuous headstage channels the data source can provide.*/
int MCDAQThread::getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const
{
	if (subProcessorIdx > 0) return 0;

	int numChans = 0;

	if (type == DataChannel::ADC_CHANNEL)
	{
		numChans = getNumAnalogInputs();
	}

	return numChans;
}

/** Returns the number of TTL channels that each subprocessor generates*/
int MCDAQThread::getNumTTLOutputs(int subProcessorIdx) const
{
	return getNumDigitalInputs();
}

/** Returns the sample rate of the data source.*/
float MCDAQThread::getSampleRate(int subProcessorIdx) const
{
	return mMCDAQ->samplerate;
}

float MCDAQThread::getBitVolts(const DataChannel* chan) const
{
	return mMCDAQ->voltageRange.vmax / float(0x7fff);
}



void MCDAQThread::setTriggerMode(bool trigger)
{
    //TODO
}

void MCDAQThread::setAutoRestart(bool restart)
{
	//TODO
}

bool MCDAQThread::updateBuffer()
{
	return true;
}

