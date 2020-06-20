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


//TODOs: Make Pin 31 or 22 of the Arduino firmdata mega usable for synch NPX and NIDAQ.
//Make Timer robust. If timer resets it may interrupt the func loop.
//Implement the vector for selecting Odor chans with the buttons.
//Handle the Arduino clock problem for recordings that last more than 1 hour.
//Reorgnize the arch to accept more olfactometers and avoid dead memory.
//Add advance section to modify Arduino pins functions.
//Test if a filled buffer is bad for the arduinos.

#include "Olfactometer.h"
#include "OlfactometerEditor.h"

#include <stdio.h>
#include <chrono>
#include <thread>
#include <array>
#include <numeric>
#include <fstream>

//std::ofstream DebugOlfac1("DebugOlfac1.txt");
//std::ofstream DebugOlfac2("DebugOlfac2.txt");
//std::ofstream DebugOlfac3("DebugOlfac3.txt");
//std::ofstream DebugOlfac4("DebugOlfac4.txt");
//std::ofstream DebugOlfac5("DebugOlfac5.txt");

Olfactometer::Olfactometer()
    : GenericProcessor      ("Olfactometer")
    , state                 (true)
    , acquisitionIsActive   (false)
    , deviceSelected        (false)
    , SeriesNo              (0)
    , TrialLength           (0)
    , OpenTime              (0)
    , timer                 ()
    , OlfactometerProc      (&Olfactometer::OdorValveOpener)
    , Generator             (Rd())
    , Predictor             ()
{
    setProcessorType (PROCESSOR_TYPE_SINK);
   /* std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Predictor.setFrequency(1000.0);
    Predictor.setAmplitude(0.5f);*/
    //uint32 FT = timer.getMillisecondCounter();
    //timer.waitForMillisecondCounter(FT + 10000);

}


Olfactometer::~Olfactometer()
{
    FinOlfactometer();
}


AudioProcessorEditor* Olfactometer::createEditor()
{
    editor = new OlfactometerEditor (this, true);
    return editor;
}

bool Olfactometer::InitOlfactometer(const std::pair<std::string, std::string>& COMPair)
{
    //this func is based on ArduinoOutput::setDevice method, thus it has all the downsides and upsides of that method.
    if (!acquisitionIsActive)
    {
        if ((!OlfacArduino.connect(COMPair.first)) || (!OlfacSerial.setup(COMPair.second.c_str(), 9600)))
        {
            CoreServices::sendStatusMessage(("Error: Check Olfactometer connections (" + COMPair.first
                + " and " + COMPair.second + ")"));
            String message = "Connection cannot be established. Make sure both boards are connected.";
            AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "ERROR", message, "OK");
            return false;
        }

        if (OlfacArduino.isArduinoReady())
        {
            uint32 currentTime = timer.getMillisecondCounter();

            OlfacArduino.sendProtocolVersionRequest();
            timer.waitForMillisecondCounter(currentTime + 2000);
            OlfacArduino.update();
            OlfacArduino.sendFirmwareVersionRequest();

            timer.waitForMillisecondCounter(currentTime + 4000);
            OlfacArduino.update();

            std::cout << "firmata v" << OlfacArduino.getMajorFirmwareVersion()
                << "." << OlfacArduino.getMinorFirmwareVersion() << std::endl;

            if (!ResetOlfactometer())
            {
                CoreServices::sendStatusMessage(("OlfacSerial data unavailable " + COMPair.second));
                String message = "Disconnect and restart the Olfactometer to empty the buffer of both boards.";
                AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Data unavailable", message, "OK");
                FinOlfactometer();

                return false;
            }

        }

        if (OlfacArduino.isInitialized())
        {
            std::cout << "OlfacArduino is initialized." << std::endl;
            CoreServices::sendStatusMessage(("OlfacArduino initialized at " + COMPair.first));
            deviceSelected = true; //Not sure why I added this. DEBUG ME PLEASE
            return true;
        }
        else
        {
            std::cout << "OlfacArduino is NOT initialized." << std::endl;
            CoreServices::sendStatusMessage(("OlfacArduino could not be initialized at " + COMPair.first));
            String message = "Disconnect and restart the Olfactometer.";
            AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Unknown connection Error", message, "OK");
            FinOlfactometer();
            
            return false;
        }
    }
    else
    {
        CoreServices::sendStatusMessage("Cannot change device while acquisition is active.");
        return false;
    }
}

void Olfactometer::FinOlfactometer()
{
    if (OlfacArduino.isInitialized())
        OlfacArduino.disconnect();

    OlfacSerial.close();
}

void Olfactometer::RunOdorPres()
{
    OlfacArduino.sendDigital(5, ARD_LOW); //Mineral Oil Valve always open.

    std::vector<int> OdorChannels = { 5,6,7 };

    //std::vector<int>::iterator asd = OdorChannels.begin();

    for (auto OdorChannel = OdorChannels.begin(); OdorChannel < OdorChannels.end(); ++OdorChannel)
    {

        uint32 FT = timer.getMillisecondCounter();

        OlfacArduino.sendDigital(BruceA2SOdorPin, ARD_HIGH);

        if (*OdorChannel != 5)
        {
            OlfacArduino.sendDigital(*OdorChannel, ARD_HIGH);
            OlfacArduino.sendDigital(5, ARD_HIGH);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(6000));

        uint32 currentTime = timer.getMillisecondCounter();
        //timer.waitForMillisecondCounter(currentTime + 4000);
        while (timer.getMillisecondCounter() - currentTime < 10000)
        {
            OlfacArduino.sendDigital(BruceA2SFVPin, ARD_LOW);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            break;
        }

        OlfacArduino.sendDigital(BruceA2SFVPin, ARD_HIGH);


        OlfacArduino.sendDigital(BruceA2SOdorPin, ARD_LOW);

        if (*OdorChannel != 5)
        {
            OlfacArduino.sendDigital(5, ARD_LOW);
            OlfacArduino.sendDigital(*OdorChannel, ARD_LOW);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(timer.getMillisecondCounter() - FT));

    }


}

bool Olfactometer::ResetOlfactometer()
{
    //Flush serial arduino.
    OlfacSerial.flush(true, true);

    //Set all pins to zero.
    for (int i = 5; i < 13; i++)
    {
        OlfacArduino.sendDigital(i, ARD_LOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //timer.waitForMillisecondCounter(currentTime + 4000);
    }

    OlfacArduino.sendDigital(BruceSynchPin, ARD_LOW);

    OlfacArduino.sendDigital(2, ARD_HIGH);
    OlfacArduino.sendDigital(3, ARD_HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));


    for (int IntTest = 1; IntTest < 6; IntTest++)
    {
        OlfacArduino.sendDigital(13, ARD_HIGH);
        OlfacArduino.sendDigital(13, ARD_LOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        int AvailableInts = (OlfacSerial.available() / 4);
        std::vector<uint32_t> TempBuff(AvailableInts, 0);
        int BytesRead = OlfacSerial.readBytes((uint8_t*)TempBuff.data(), AvailableInts * 4);

        uint32_t TimeTag = TempBuff.data()[0] % 1000;

        std::array<uint32_t, 4> TempArr = { 1,2,3,4 };

        //DebugFile << AvailableInts << " " << BytesRead <<" "<< TimeTag << "\n";

        if (std::find(TempArr.begin(), TempArr.end(), TimeTag) == TempArr.end())
        {
            //DebugFile << "init testing " << IntTest << "\n";
        }
        else
        {
            //DebugFile << "Good data.\n";
            return true;
        }

    }

    return false;
}

void Olfactometer::InitOdorPres()
{
    OlfacArduino.sendDigital(BruceMO, ARD_LOW); //Mineral Oil Valve always open.

    //editor->updateSettings();

    //Select the odors. Numbers are pins in the Arduino mega. 
    OdorVec.push_back(5);
    OdorVec.push_back(6);
    OdorVec.push_back(7);
    OdorVec.push_back(8);
    OdorVec.push_back(9);
    OdorVec.push_back(10);
    OdorVec.push_back(11);


    CurrentOdor = OdorVec.begin();
    PastLastOdor = OdorVec.end();

    //Shuffle odors.
    std::shuffle(CurrentOdor, PastLastOdor, Generator);

    SerialTime = timer.getMillisecondCounter();


    //TargetTime = (uint32_t)(TrialLength * 1000.0);

    //TimeCounter = timer.getMillisecondCounter();
}

void Olfactometer::OpenFinalValve()
{
    //Sync TTL
    OlfacArduino.sendDigital(BruceSynchPin, ARD_HIGH);
    //OpenValve
    OlfacArduino.sendDigital(BruceA2SFVPin, ARD_LOW);

    //Reset vars;
    TestEpoch = 0;
    TestBuffPtr = TestBuffer;

    TimeCounter = CurrentTime;
    TargetTime = (uint32_t)(OpenTime * 1000.0); //Open time of the final valve
    OlfactometerProc = &Olfactometer::ValvesCloser;
}

void Olfactometer::OdorValveOpener(AudioSampleBuffer& buffer)
{
    //DebugOlfac1 << "ODORValveOpener \n";
    LoopTime = timer.getMillisecondCounter();
    ////DebugOlfac3 << LoopTime <<"\n";
    //if (CurrentTime >= TimeCounter + TargetTime)
    //{

    OlfacArduino.sendDigital(BruceA2SOdorPin, ARD_HIGH);
    if (*CurrentOdor != BruceMO)
    {
        OlfacArduino.sendDigital(*CurrentOdor, ARD_HIGH);
        OlfacArduino.sendDigital(BruceMO, ARD_HIGH);
    }

    TimeCounter = timer.getMillisecondCounter();
    //TimeCounter = CurrentTime;
    TargetTime = EquilibrationTime; //Equilibrate for...

    CoreServices::sendStatusMessage( "Series: " + juce::String(CurrentSeries+1) + "/"
        + juce::String(SeriesNo) + ", Odor Chan: " + juce::String((int)(*CurrentOdor)));

    OlfactometerProc = &Olfactometer::Equilibrate6Sec;
    //}
}

void Olfactometer::Equilibrate6Sec(AudioSampleBuffer& buffer)
{
    CurrentTime = timer.getMillisecondCounter();
    //DebugOlfac1 << "Fuera6sec \n";
    if (CurrentTime >= TimeCounter + TargetTime)
    {
        //DebugOlfac2 << "Dentro6sec \n";
        TimeCounter = CurrentTime;
        TargetTime = RespEpochTime; //Set baseline respiration epoch
        OlfactometerProc = &Olfactometer::RespProc;
    }
}

void Olfactometer::RespProc(AudioSampleBuffer& buffer)
{
    //DebugOlfac1 << "FueraRespProc \n";
    CurrentTime = timer.getMillisecondCounter();

    if (CurrentTime <= TimeCounter + TargetTime)
    {
        //DebugOlfac2 << "DentroRespProcIF \n";

        //Extracting data from NIDAQ(or any source) and copying it into RespBuffer;

        uint32_t SamNo = getNumSamples(RespChannel);


        const float* RawRespPtr = buffer.getReadPointer(RespChannel);

        std::memcpy(RespBuffPtr, RawRespPtr, SamNo * sizeof(float));

        RespBuffPtr += SamNo;

        //SamplesinBuffer += SamNo;
    }
    else
    {
        //DebugOlfac2 << "DentroRespProcELSE \n";
        float* BuffPtrCpy = RespBuffer;

        float SamplesInBuffer = (float)std::distance(RespBuffer, RespBuffPtr);
        float Sum = (float)std::accumulate(RespBuffer, RespBuffPtr, 0.0f);

        RespMean = Sum / SamplesInBuffer;

        float BinVariance = 0.0f;

        for (; BuffPtrCpy < RespBuffPtr; ++BuffPtrCpy)
        {
            BinVariance += ((*BuffPtrCpy) - RespMean) * ((*BuffPtrCpy) - RespMean);
            //DebugOlfac5 << *BuffPtrCpy << "\n";
        }

        BinVariance /= (SamplesInBuffer - 1.0f); // this is Variance over N. Matlab uses Bessels correction to compute STD. Actually Im gonna use Bessels correction.

        RespStd = std::sqrt(BinVariance); // Stand deviation to my STD vector.

        //Reset vars;
        RespBuffPtr = RespBuffer;

        TimeCounter = CurrentTime;
        TargetTime = RespToleranceTime; //MaxTime for final Valve Opener
        //DebugOlfac3 << "Number of Samples in RespBuff: " << SamplesInBuffer << "\n";
        //DebugOlfac3 << "sum in RespBuff: " << Sum << "\n";
        //DebugOlfac3 << "Mean in RespBuff: " << RespMean << "\n";
        //DebugOlfac3 << "STD in RespBuff: " << RespStd << "\n";
        OlfactometerProc = &Olfactometer::FinalValveOpener;
        //DebugOlfac5 << 20 << "\n";
    }
}

void Olfactometer::FinalValveOpener(AudioSampleBuffer& buffer)
{

    CurrentTime = timer.getMillisecondCounter();
    //DebugOlfac1 << "FueraFVO \n";
    if (CurrentTime <= TimeCounter + TargetTime)
    {
        //DebugOlfac2 << "DentroFVOIF \n";
        //DebugOlfac3 << "DentroFVOIF \n";
        if (TestEpoch < 200)
        {
            //Filling the test Respirtion buffer up to 200 samples. to deliver odor
            uint32_t SamNo = getNumSamples(RespChannel);

            const float* RawRespPtr = buffer.getReadPointer(RespChannel);

            std::memcpy(TestBuffPtr, RawRespPtr, SamNo * sizeof(float));

            TestBuffPtr += SamNo;

            TestEpoch += SamNo;

            //DebugOlfac3 << " Filling Test Buff, samples: "<< TestEpoch<<"\n";
        }
        else
        {
            float Thresh = RespMean - RespStd;

            //DebugOlfac3 << " Test Thresh: " << Thresh << "\n";
            /////
            float* temp = TestBuffer;

            for (; temp<TestBuffPtr ; ++temp)
            {
                //DebugOlfac4 << *temp << "\n";

            }

            //DebugOlfac3 << " Test Thresh: " << Thresh << "\n";
            ////
            if (std::any_of(TestBuffer, TestBuffPtr, [&Thresh](float& Sample) 
                {
                    return Sample < Thresh;

                }) &&
                (*(TestBuffPtr - 1) > RespMean))
            {
                //OlfacArduino.sendDigital(BruceSynchPin, ARD_HIGH);
                //OlfacArduino.sendDigital(BruceA2SFVPin, ARD_LOW);

                ////Reset vars;
                //TestEpoch = 0;
                //TestBuffPtr = TestBuffer;

                //TimeCounter = CurrentTime;
                //TargetTime = (uint32_t)(OpenTime * 1000.0); //Open time of the final valve
                ////DebugOlfac3 << " Test Crossed: " << Thresh << "\n";
                //OlfactometerProc = &Olfactometer::ValvesCloser;
                OpenFinalValve();
            }
            else
            {
                //DebugOlfac3 << " Test not Crossed: " << Thresh << "\n";
                TestEpoch = 0;
                TestBuffPtr = TestBuffer;
            }


            //DebugOlfac4 << 20 << "\n";
        }
    }
    else
    {
        ////DebugOlfac2 << "DentroFVOELSE \n";
        //OlfacArduino.sendDigital(BruceSynchPin, ARD_HIGH);
        //OlfacArduino.sendDigital(BruceA2SFVPin, ARD_LOW);

        ////Reset vars;
        //TestEpoch = 0;
        //TestBuffPtr = TestBuffer;

        //TimeCounter = CurrentTime;
        //TargetTime = (uint32_t)(OpenTime * 1000.0); //Open time of the final valve
        ////DebugOlfac3 << " No thresh cross final else \n";
        //OlfactometerProc = &Olfactometer::ValvesCloser;

        OpenFinalValve();
    }

}

void Olfactometer::ValvesCloser(AudioSampleBuffer& buffer)
{
    CurrentTime = timer.getMillisecondCounter();
    //DebugOlfac1 << "FueraValveCloser \n";
    if (CurrentTime >= TimeCounter + TargetTime)
    {
        //DebugOlfac2 << "DentroValveCloser \n";
        OlfacArduino.sendDigital(BruceA2SFVPin, ARD_HIGH);
        OlfacArduino.sendDigital(BruceA2SOdorPin, ARD_LOW);
        OlfacArduino.sendDigital(BruceSynchPin, ARD_LOW);

        if (*CurrentOdor != BruceMO)
        {
            OlfacArduino.sendDigital(BruceMO, ARD_LOW);
            OlfacArduino.sendDigital(*CurrentOdor, ARD_LOW);
        }
        TimeCounter = CurrentTime;
        TargetTime = (uint32_t)(TrialLength * 1000.0);
        OlfactometerProc = &Olfactometer::RestartFuncLoop;
    }

}

void Olfactometer::RestartFuncLoop(AudioSampleBuffer& buffer)
{
    CurrentTime = timer.getMillisecondCounter();
    //DebugOlfac1 << "FueraREstLoop \n";
    if (CurrentTime >= LoopTime + TargetTime)
    {
        //DebugOlfac2 << "DentroREstLoop \n";
        ++CurrentOdor;

        if (CurrentOdor < PastLastOdor)
        {
            //Advance to the next odor in OdorVec
            OlfactometerProc = &Olfactometer::CheckSerialTime;

        }
        else
        {
            CurrentSeries++;

            if (CurrentSeries < SeriesNo)
            {
                //Advance to the next trial
                CurrentOdor = OdorVec.begin();
                //Shuffle the odors for the next trial
                std::shuffle(CurrentOdor, PastLastOdor, Generator);
                OlfactometerProc = &Olfactometer::CheckSerialTime;
            }
            else
            {
                //If all trials has passed, end the loop.
                CoreServices::sendStatusMessage("Odor presentation finished");
                OlfactometerProc = &Olfactometer::EmptyFunc;
            }
        }
    }
}

void Olfactometer::CheckSerialTime(AudioSampleBuffer& buffer)
{
    //DebugOlfac1 << "FueraValveCloser \n";


    if (timer.getMillisecondCounter() > SerialTime + 3600000)
    {
        //If one hour has passed, reset arduinos.
        ResetOlfactometer();
        SerialTime = timer.getMillisecondCounter();
        OlfactometerProc = &Olfactometer::OdorValveOpener;
    }
    else 
    {

        OlfactometerProc = &Olfactometer::OdorValveOpener;

    }

}

void Olfactometer::EmptyFunc(AudioSampleBuffer& buffer)
{
}

void Olfactometer::SetSeriesNo(int SN)
{
    SeriesNo = SN;
}

void Olfactometer::SetTrialLength(double TL)
{
    TrialLength = TL;
}

void Olfactometer::SetOpenTime(double OT)
{
    OpenTime = OT;
}

void Olfactometer::setOdorVec(const std::vector<char> ActiveButtons, uint8_t IdxShift)
{
    //Getting the active odor Channels from the Active Buttons char vec from OlfactometerEditor. 
    uint8_t IdxCount = 0;
    for (auto it = ActiveButtons.begin(); it < ActiveButtons.end(); ++it)
    {
        if (*it != 0)
        {
            OdorVec.push_back(IdxCount + IdxShift);
        }
        IdxCount++;
    }
    CurrentOdor = OdorVec.begin(); //Iterators to get individual odor valves in the main Process Loop.
    PastLastOdor = OdorVec.end();
}

int Olfactometer::GetSeriesNo() const
{
    return SeriesNo;
}

double Olfactometer::GetTrialLength() const
{
    return TrialLength;
}

double Olfactometer::GetOpenTime() const
{
    return OpenTime;
}


void Olfactometer::handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int sampleNum)
{

    //DebugFile << "h" << "\n";
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
  //                  OlfacArduino.sendDigital (outputChannel, ARD_LOW);
  //              }
  //              else
  //              {
  //                  OlfacArduino.sendDigital (outputChannel, ARD_HIGH);
  //              }
  //          }
  //      }
  //  }
}


void Olfactometer::setParameter (int parameterIndex, float newValue)
{
    // make sure current output channel is off:
    /*OlfacArduino.sendDigital(outputChannel, ARD_LOW);

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
    InitOdorPres();
    return deviceSelected;
}


bool Olfactometer::disable()
{
    //OlfacArduino.sendDigital (outputChannel, ARD_LOW);
    acquisitionIsActive = false;
    
    return true;
}

//int asd = 0;
void Olfactometer::process (AudioSampleBuffer& buffer)
{   
    //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    //std::chrono::steady_clock::time_point start1 = std::chrono::steady_clock::now();
    //DebugFile << Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks()) << "\n";
    //uint32 NowJuce = Time::getMillisecondCounter();
    ////DebugOlfac2 << "Process \n";

    (this->*OlfactometerProc)(buffer);
    

    /*uint32 SamNo = getNumSamples(2);

    const float* ptr = buffer.getReadPointer(2);

    const float* endBuff = ptr + SamNo;

    for (; ptr < endBuff; ++ptr)
    {

        //DebugOlfac1 << *ptr << "\n";


    }*/


    /*uint32_t SamNo = getNumSamples(RespChannel);


    const float* RawRespPtr = buffer.getReadPointer(RespChannel);

    std::memcpy(RespBuffPtr, RawRespPtr, SamNo * sizeof(float));

    float* temp = RespBuffPtr;

    for (; RespBuffPtr < temp + SamNo; ++RespBuffPtr)
    {

        //DebugOlfac1 << *RespBuffPtr << "\n";


    }*/
    //RespBuffPtr += SamNo;





    ////std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    //////DebugOlfac2 << getTimestamp(2) << "\n";// << Time::getMillisecondCounter() << "\n";;

    //std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
    //std::chrono::duration<float> duration1 = end1 - start1;
    //float asd = duration1.count();

    //uint32 NowJuce2 = Time::getMillisecondCounter()-NowJuce;

    ////DebugOlfac1 << NowJuce2 << "\n";

    //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //std::chrono::duration<float> duration = end - start;

    

    ////DebugOlfac2 << duration.count() << "\n";


    ////DebugOlfac2 << Time::getMillisecondCounter() - NowJuce << "\n";;

    //RunOdorPres();
    //chec
    //checkForEvents ();
}

SimpleTone::SimpleTone()
    :
    frequency(0.0),
    sampleRate(44100.0),
    currentPhase(0.0),
    phasePerSample(0.0),
    amplitude(0.0f)
{

    setAudioChannels(0, 2);

}

SimpleTone::~SimpleTone()
{
    shutdownAudio();
}

void SimpleTone::prepareToPlay(int samplesPerBlockExpected, double rate)
{
    currentPhase = 0.0;
    phasePerSample = 0.0;
    sampleRate = rate;
}

void SimpleTone::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    if (frequency == 0)
    {
        //If frequency is set to 0 play some white noise.
        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            // Get a pointer to the start sample in the buffer for this audio output channel
            auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

            // Fill the required number of samples with noise between -0.125 and +0.125
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                  buffer[sample] = (random.nextFloat() * 2.0f - 1.0f) * amplitude;
                //buffer[sample] = random.nextFloat() * 0.25f - 0.125f;
        }
    }
    else
    {
        //Play a tone with the desired frequency.
        if (phasePerSample == 0.0)
            phasePerSample = double_Pi * 2.0 / (sampleRate / frequency);

        for (int i = 0; i < bufferToFill.numSamples; ++i)
        {
            const float sample = amplitude * (float)std::sin(currentPhase);
            //DebugOlfac1 << sample << "\n";
            currentPhase += phasePerSample;

            for (int j = bufferToFill.buffer->getNumChannels(); --j >= 0;)
                bufferToFill.buffer->setSample(j, bufferToFill.startSample + i, sample);
        }
    }
}

void SimpleTone::releaseResources()
{ 
}

void SimpleTone::setAudioChannels(int numInputChannels, int numOutputChannels)
{
    String audioError = deviceManager.initialise(numInputChannels, numOutputChannels, nullptr, true);
    jassert(audioError.isEmpty());

    deviceManager.addAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(this);
}

void SimpleTone::shutdownAudio()
{
    audioSourcePlayer.setSource(nullptr);
    deviceManager.removeAudioCallback(&audioSourcePlayer);
    deviceManager.closeAudioDevice();
}

void SimpleTone::setAmplitude(float newAmplitude)
{
    amplitude = newAmplitude;
}

void SimpleTone::setFrequency(double newFrequencyHz)
{
    frequency = newFrequencyHz;
    phasePerSample = 0.0;
}
