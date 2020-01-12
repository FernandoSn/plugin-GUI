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

#ifndef __ARDUINOOUTPUTEDITOR_H_28EB4CC9__
#define __ARDUINOOUTPUTEDITOR_H_28EB4CC9__


#include <EditorHeaders.h>
#include "ArduinoOutput.h"
#include <SerialLib.h>

class ImageIcon;

/**

  User interface for the ArduinoOutput processor.

  @see ArduinoOutput

*/

class ArduinoOutputEditor : public GenericEditor,
                            public ComboBox::Listener

{
public:
    ArduinoOutputEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~ArduinoOutputEditor();

    void receivedEvent();

    ImageIcon* icon;

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged);

    void buttonClicked(Button* button) override; //FER

    ArduinoOutput* arduino;

    ofSerial serial;

private:

   // ScopedPointer<UtilityButton> triggerButton;
    std::unique_ptr<ComboBox> inputChannelSelector;
    std::unique_ptr<ComboBox> outputChannelSelector;
    std::unique_ptr<ComboBox> gateChannelSelector;
    std::unique_ptr<ComboBox> deviceSelector;

    //Buttons
    std::unique_ptr<UtilityButton> WriteDigButton;

    void timerCallback();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArduinoOutputEditor);

};




#endif  // __ARDUINOOUTPUTEDITOR_H_28EB4CC9__
