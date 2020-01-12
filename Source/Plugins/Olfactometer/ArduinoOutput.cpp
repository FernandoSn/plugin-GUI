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

#include "ArduinoOutput.h"

#include <stdio.h>


ArduinoOutput::ArduinoOutput()
    : outputChannel         (13)
    , inputChannel          (-1)
    , gateChannel           (-1)
    , state                 (true)
    , acquisitionIsActive   (false)
    , deviceSelected        (false)
{
}


ArduinoOutput::~ArduinoOutput()
{
    if (arduino.isInitialized())
        arduino.disconnect();
}


void ArduinoOutput::setDevice (const String& devName)
{
    if (! acquisitionIsActive)
    {
        Time timer;

        arduino.connect (devName.toStdString());

        if (arduino.isArduinoReady())
        {
            uint32 currentTime = timer.getMillisecondCounter();

            arduino.sendProtocolVersionRequest();
            timer.waitForMillisecondCounter (currentTime + 2000);
            arduino.update();
            arduino.sendFirmwareVersionRequest();

            timer.waitForMillisecondCounter (currentTime + 4000);
            arduino.update();

            std::cout << "firmata v" << arduino.getMajorFirmwareVersion()
                      << "." << arduino.getMinorFirmwareVersion() << std::endl;
        }

        if (arduino.isInitialized())
        {
            std::cout << "Arduino is initialized." << std::endl;
            arduino.sendDigitalPinMode (outputChannel, ARD_OUTPUT);
            CoreServices::sendStatusMessage (("Arduino initialized at" + devName));
            deviceSelected = true;
        }
        else
        {
            std::cout << "Arduino is NOT initialized." << std::endl;
            CoreServices::sendStatusMessage (("Arduino could not be initialized at" + devName));
        }
    }
    else
    {
        CoreServices::sendStatusMessage ("Cannot change device while acquisition is active.");
    }
}

void ArduinoOutput::WriteDigital()
{
    arduino.sendDigital(13, ARD_HIGH);
    arduino.sendDigital(13, ARD_LOW);
}


void ArduinoOutput::setOutputChannel (int chan)
{
    //setParameter (0, chan);
}


void ArduinoOutput::setInputChannel (int chan)
{
    //setParameter (1, chan - 1);
}


void ArduinoOutput::setGateChannel (int chan)
{
    //setParameter (2, chan - 1);
}
