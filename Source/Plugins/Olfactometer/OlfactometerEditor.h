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

#ifndef __OlfactometerEDITOR_H_28EB4CC9__
#define __OlfactometerEDITOR_H_28EB4CC9__


#include <EditorHeaders.h>
#include "Olfactometer.h"
#include <SerialLib.h>

class ImageIcon;

//Odor Channel Button class

class OdorChButton : public ToggleButton, public Timer
{
    friend class OlfactometerEditor;

public:
    
    
    OdorChButton(int id);
    void setId(int id);
    int getId();
    void setEnabled(bool);
    void timerCallback();


private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    int id;
    bool enabled;

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

    void buttonClicked(Button* button) override; //FER

    void labelTextChanged(Label* label) override;

    void saveCustomParameters(XmlElement* xml) override;
    void loadCustomParameters(XmlElement* xml) override;

    Olfactometer* Olfac;

    ofSerial serial;

private:

   // ScopedPointer<UtilityButton> triggerButton;
    std::unique_ptr<ComboBox> inputChannelSelector;
    std::unique_ptr<ComboBox> outputChannelSelector;
    std::unique_ptr<ComboBox> gateChannelSelector;
    std::unique_ptr<ComboBox> deviceSelector;

    //Text boxes
    String LastSeriesNoStr;
    String LastTrialLengthStr;
    String LastOpenTimeStr;
    String LastOdorConcStr;

    std::unique_ptr<Label> SeriesNoLabel;
    std::unique_ptr<Label> TrialLengthLabel;
    std::unique_ptr<Label> OpenTimeLabel;
    std::unique_ptr<Label> OdorConcLabel;

    std::unique_ptr<Label> SeriesNoValue;
    std::unique_ptr<Label> TrialLengthValue;
    std::unique_ptr<Label> OpenTimeValue;
    std::unique_ptr<Label> OdorConcValue;

    //Buttons
    std::unique_ptr<UtilityButton> WriteDigButton;

    void timerCallback();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OlfactometerEditor);

};




#endif  // __OlfactometerEDITOR_H_28EB4CC9__
