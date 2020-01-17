/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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
#ifndef __OlfactometerEDITOR_H_28EB4CC9__
#define __OlfactometerEDITOR_H_28EB4CC9__


#include <EditorHeaders.h>
#include "Olfactometer.h"
#include <SerialLib.h>
#include "../../../JuceLibraryCode/modules/juce_graphics/contexts/juce_GraphicsContext.h"

class ImageIcon;

//Odor Channel Button class

class OdorChButton : public ToggleButton, public Timer
{

public:
    
    
    OdorChButton(int id);
    void setId(int id);
    int getId();
    void setEnabled(bool);
    void timerCallback() override;




    friend class OlfactometerEditor;

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    int id;
    bool enabled;

    //JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OdorChButton)

};

/**

  User interface for the Olfactometer processor.

  @see Olfactometer

*/

class OlfactometerEditor : public GenericEditor,
                             public ComboBox::Listener,public Label::Listener

{
public:
    OlfactometerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~OlfactometerEditor();

    void receivedEvent();

    ImageIcon* icon;

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

    void buttonEvent(Button* button) override; //FER
    void labelTextChanged(Label* label) override;

    //void saveCustomParameters(XmlElement* xml) override;
    //void loadCustomParameters(XmlElement* xml) override;

private:

    void timerCallback();

    void DrawOdorChans(uint8_t ChNo, uint8_t FirstCh);

    void FindLabOlfactometers(std::vector<ofSerialDeviceInfo>& Devices);

public:



    Olfactometer* Olfac;

    ofSerial serial;

private:

    std::unique_ptr<ComboBox> inputChannelSelector;
    std::unique_ptr<ComboBox> outputChannelSelector;
    std::unique_ptr<ComboBox> gateChannelSelector;
    std::unique_ptr<ComboBox> deviceSelector;

    //Text boxes
    String LastSeriesNoStr;
    String LastTrialLengthStr;
    String LastOpenTimeStr;

    std::unique_ptr<Label> SeriesNoLabel;
    std::unique_ptr<Label> TrialLengthLabel;
    std::unique_ptr<Label> OpenTimeLabel;

    std::unique_ptr<Label> SeriesNoValue;
    std::unique_ptr<Label> TrialLengthValue;
    std::unique_ptr<Label> OpenTimeValue;

    //Buttons
    std::unique_ptr<UtilityButton> WriteDigButton;

    OwnedArray<OdorChButton> OdorChButtons;
    std::unique_ptr<Label> OdorChLabel;
    std::fstream OlfactometerParams;
    std::map<std::string,std::pair<std::string, std::string>> OlfactometerCOMS;

    static const uint8_t BruceChNo = 11;
    static const uint8_t BruceFirstChan = 2;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OlfactometerEditor);

};
#endif  // __OlfactometerEDITOR_H_28EB4CC9__
