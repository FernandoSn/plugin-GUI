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
//#pragma once
#ifndef __Olfactometer_H_F7BDA585__
#define __Olfactometer_H_F7BDA585__

#include <random>
#include <SerialLib.h>
#include <ProcessorHeaders.h>
#include "serial/ofArduino.h"
#include "FerTimer.h"

//AudioAppComponent
//It seems that the dll for this class is not linked.
//ToneGeneratorAudioSource
class SimpleTone : AudioSource
{
public:
    SimpleTone();
    ~SimpleTone();
    void prepareToPlay(int samplesPerBlockExpected, double rate) override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void setAudioChannels(int numInputChannels, int numOutputChannels);
    void shutdownAudio();
    void setAmplitude(float newAmplitude);
    void setFrequency(double newFrequencyHz);


    AudioDeviceManager deviceManager;

private:

    AudioSourcePlayer audioSourcePlayer;
    double frequency, sampleRate;
    double currentPhase, phasePerSample;
    float amplitude;
    Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleTone)
};



/**
    *UNDER CONSTRUCTION*

    Provides a serial interface to an Arduino board.

    Based on Open Frameworks ofArduino class.

    @see GenericProcessor
 */
class Olfactometer : public GenericProcessor
{
public:
    Olfactometer();
    ~Olfactometer();

    /** Searches for events and triggers the Arduino output when appropriate. */
    void process (AudioSampleBuffer& buffer) override;

    /** Currently unused. Future uses may include changing the TTL trigger channel
    or the output channel of the Arduino. */
    void setParameter (int parameterIndex, float newValue) override;

    /** Convenient interface for responding to incoming events. */
    void handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int sampleNum) override;

    /** Called immediately prior to the start of data acquisition. */
    bool enable() override;

    /** Called immediately after the end of data acquisition. */
    bool disable() override;

    /** Creates the OlfactometerEditor. */
    AudioProcessorEditor* createEditor() override;




    bool InitOlfactometer(const std::pair<std::string,std::string>& COMPair);
    void FinOlfactometer();



    int GetSeriesNo() const;
    double GetTrialLength() const;
    double GetOpenTime() const;

    void SetSeriesNo(int SN);
    void SetTrialLength(double TL);
    void SetOpenTime(double OT);
    void setOdorVec(const std::vector<char> ActiveButtons, uint8_t IdxShift);

private:


    void RunOdorPres(); //deprecated

    bool ResetOlfactometer();

    void InitOdorPres();

    void OpenFinalValve(); //code to open Final Valve.

    void setToneOn(float newAmplitude, double newFrequency);

    void setToneOff();

    void SetContext();

    //Rotating functions for the Process Loop

    void OdorValveOpener(AudioSampleBuffer& buffer);

    void Equilibrate6Sec(AudioSampleBuffer& buffer);

    void RespProc(AudioSampleBuffer& buffer);

    void FinalValveOpener(AudioSampleBuffer& buffer);

    void ValvesCloser(AudioSampleBuffer& buffer);

    void RestartFuncLoop(AudioSampleBuffer& buffer);

    void CheckSerialTime(AudioSampleBuffer& buffer);

    void EmptyFunc(AudioSampleBuffer& buffer);



    ////////////////////////////////////////////////////////

public:

    static const uint8_t RespChannel = 4;

    enum class OlfactometersID : uint8_t
    {
        BRUCE = 1,
        CHANCE,
        PETEY,
        SHADOW,
        BEAST
    };

    static const uint8_t BruceChNo = 11;
    static const uint8_t BruceFirstChan = 2;



    static const uint8_t BruceA2SOdorPin = 13;
    static const uint8_t BruceA2SFVPin = 3;//3
    //static const uint8_t BruceMO = 5;
    uint8_t BruceBlanks[2];
    uint8_t BruceMFCs[3];
    //static const uint8_t BruceSynchPin = 4;//4
    static const uint8_t BruceTonePin = 2;


private:
    /** An open-frameworks Arduino object. */
    ofArduino OlfacArduino;
    ofSerial OlfacSerial;
    Time timer;

    bool state;
    bool acquisitionIsActive;
    bool deviceSelected;

    uint8_t OlfacName;
    int SeriesNo;
    double TrialLength;
    double OpenTime;
    std::vector<uint8_t> OdorVec; //Up to 256 odor channels. Want more? What kind fo olfactomer are you building!!!!???????
    std::vector<bool> ToneBoolVec;
    std::vector<int> MFC0Vec;
    std::vector<uint8_t> OdorVec1;
    int OdorCount;
    int TotalOdors;

    
    int CurrentSeries = 0;
    uint32_t TimeCounter;
    uint32_t CurrentTime;
    uint32_t TargetTime;
    uint32_t LoopTime;
    uint32_t SerialTime;
    std::vector<uint8_t>::iterator CurrentOdor;
    std::vector<uint8_t>::iterator PastLastOdor;
    std::vector<uint8_t>::iterator CurrentOdor1;
    std::vector<uint8_t>::iterator PastLastOdor1;
    std::vector<int>::iterator CurrentMFC0;
    std::vector<int>::iterator PastLastMFC0;
    std::vector<bool>::iterator CurrentToneBool;
    std::vector<bool>::iterator PastLastToneBool;
    float RespBuffer[2000 * 3]; //2000 Sampling Rate. 3 secs. I actually using 2 sec but some extra memory to avoid any unwanted leaks.
    float* RespBuffPtr = RespBuffer;
    float RespMean;
    float RespStd;

    float TestBuffer[2000];
    float* TestBuffPtr = TestBuffer;
    uint32_t TestEpoch = 0;
    //uint32_t SamplesinBuffer = 0;


	//Func ptr for the proc loop.
    void(Olfactometer::* OlfactometerProc)(AudioSampleBuffer& buffer);

	//Statics for vars inside the func loop

	static const uint32_t EquilibrationTime = 6000; //Do not change this vars
	static const uint32_t RespEpochTime = 2000;
	static const uint32_t RespToleranceTime = 5000;
    static const uint32_t ContextTime = 10000;
    static const uint32_t BaselineTime = 15 * 60 * 1000;
    //uint32_t ContextTime;

	//static const uint32_t EquilibrationTime = 0;
	//static const uint32_t RespEpochTime = 0;
	//static const uint32_t RespToleranceTime = 0;



    std::random_device Rd;
    std::default_random_engine Generator;
    std::uniform_int_distribution<> Distr4TrialLength;
    SimpleTone Tone;

    bool ToneOn = false;
    bool LightOn = false;
    bool ContextReady = false; //Var for mycontext experiments
    bool BaselineOn;
    bool RandomITI;
    bool RandomOdors;
    bool ContextExperiment;
    bool MorphingExperiment;
    //bool TonePres;
    std::ofstream OlfacFile;


    //This tempalte is based on std::shuffle C++17.
    //CAUTION!!!! THIS IS NOT A SAFE TEMPLATE. If one of the two containers contains more elements than the other, this template method will have undefined behavior.
    // We cannot have different types of container ie. vector and map; however, we can have differnt types data of the same container. ie, vector<int> and vector<bool>,
    //This is because iterators for the same container require the same amount of memory.
    //Since this is not safe this should only be used with vector<int>,vector<bool> (I have only tested with these vector types).
    //The intended purpose is to shuffle two vectors with the same random distribution.
    template<class RandomIt1, class RandomIt2, class URBG>
    void shuffle(RandomIt1 first, RandomIt1 last, RandomIt2 first2, URBG&& g)
    {
        typedef typename std::iterator_traits<RandomIt1>::difference_type diff_t;
        typedef std::uniform_int_distribution<diff_t> distr_t;
        typedef typename distr_t::param_type param_t;

        distr_t D;
        diff_t n = last - first;
        for (diff_t i = n - 1; i > 0; --i) {
            using std::swap;
            auto randIdx = D(g, param_t(0, i));
            swap(first[i], first[randIdx]);
            swap(first2[i], first2[randIdx]);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Olfactometer);
};




#endif  // __Olfactometer_H_F7BDA585__
