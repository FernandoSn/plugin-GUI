/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include "Olfactometer.h"
#include "OlfactometerEditor.h"

#include <stdio.h>


Olfactometer::Olfactometer()
    : GenericProcessor      ("Olfactometer")
    , state                 (true)
    , acquisitionIsActive   (false)
    , deviceSelected        (false)
{
    setProcessorType (PROCESSOR_TYPE_SINK);
}


Olfactometer::~Olfactometer()
{
    if (arduino.isInitialized())
        arduino.disconnect();
}


AudioProcessorEditor* Olfactometer::createEditor()
{
    editor = new OlfactometerEditor (this, true);
    return editor;
}

void Olfactometer::setOlfactometer(const String& OlfactometerName)
{
    if (!acquisitionIsActive)
    {
        Time timer;

        arduino.connect(OlfactometerName.toStdString());

        if (arduino.isArduinoReady())
        {
            uint32 currentTime = timer.getMillisecondCounter();

            arduino.sendProtocolVersionRequest();
            timer.waitForMillisecondCounter(currentTime + 2000);
            arduino.update();
            arduino.sendFirmwareVersionRequest();

            timer.waitForMillisecondCounter(currentTime + 4000);
            arduino.update();

            std::cout << "firmata v" << arduino.getMajorFirmwareVersion()
                << "." << arduino.getMinorFirmwareVersion() << std::endl;
        }

        if (arduino.isInitialized())
        {
            std::cout << "Arduino is initialized." << std::endl;
            //arduino.sendDigitalPinMode(13, ARD_OUTPUT);
            CoreServices::sendStatusMessage(("Arduino initialized at" + OlfactometerName));
            deviceSelected = true;
        }
        else
        {
            std::cout << "Arduino is NOT initialized." << std::endl;
            CoreServices::sendStatusMessage(("Arduino could not be initialized at" + OlfactometerName));
        }
    }
    else
    {
        CoreServices::sendStatusMessage("Cannot change device while acquisition is active.");
    }
}
//#include <fstream>
void Olfactometer::StartOdorPres()
{
    arduino.sendDigital(13, ARD_HIGH);
    arduino.sendDigital(13, ARD_LOW);
}


void Olfactometer::handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int sampleNum)
{
  //  if (Event::getEventType(event) == EventChannel::TTL)
  //  {
		//TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);

  //      //int eventNodeId = *(dataptr+1);
  //      const int eventId         = ttl->getState() ? 1: 0;
  //      const int eventChannel    = ttl->getChannel();

  //      // std::cout << "Received event from " << eventNodeId
  //      //           << " on channel " << eventChannel
  //      //           << " with value " << eventId << std::endl;

  //      if (eventChannel == gateChannel)
  //      {
  //          if (eventId == 1)
  //              state = true;
  //          else
  //              state = false;
  //      }

  //      if (state)
  //      {
  //          if (inputChannel == -1 || eventChannel == inputChannel)
  //          {
  //              if (eventId == 0)
  //              {
  //                  arduino.sendDigital (outputChannel, ARD_LOW);
  //              }
  //              else
  //              {
  //                  arduino.sendDigital (outputChannel, ARD_HIGH);
  //              }
  //          }
  //      }
  //  }
}


void Olfactometer::setParameter (int parameterIndex, float newValue)
{
    // make sure current output channel is off:
    /*arduino.sendDigital(outputChannel, ARD_LOW);

    if (parameterIndex == 0)
    {
        outputChannel = (int) newValue;
    }
    else if (parameterIndex == 1)
    {
        inputChannel = (int) newValue;
    }
    else if (parameterIndex == 2)
    {
        gateChannel = (int) newValue;

        if (gateChannel == -1)
            state = true;
        else
            state = false;
    }*/
}


bool Olfactometer::enable()
{
    acquisitionIsActive = true;

    return deviceSelected;
}


bool Olfactometer::disable()
{
    //arduino.sendDigital (outputChannel, ARD_LOW);
    acquisitionIsActive = false;

    return true;
}


void Olfactometer::process (AudioSampleBuffer& buffer)
{
    checkForEvents ();
}
