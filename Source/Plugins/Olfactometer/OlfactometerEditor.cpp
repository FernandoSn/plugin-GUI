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

#include "OlfactometerEditor.h"
//#include "Olfactometer.h"
//#include <SerialLib.h>
#include <stdio.h>

OdorChButton::OdorChButton(int id) : id(id)
{
    startTimer(500);
}

void OdorChButton::setId(int id_)
{
    id = id_;
}

int OdorChButton::getId()
{
    return id;
}

void OdorChButton::setEnabled(bool enable)
{
    enabled = enable;
}

void OdorChButton::timerCallback()
{
}

void OdorChButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver && enabled)
        g.setColour(Colours::antiquewhite);
    else
        g.setColour(Colours::darkgrey);
    g.fillEllipse(14, 0, 10, 10);

    if (enabled)
    {
        if (isMouseOver)
            g.setColour(Colours::lightgreen);
        else
            g.setColour(Colours::forestgreen);
    }
    else
    {
        if (isMouseOver)
            g.setColour(Colours::lightgrey);
        else
            g.setColour(Colours::lightgrey);
    }
    g.fillEllipse(16, 2, 6, 6);

   /* g.setColour(Colours::lightgrey);
    g.drawRoundedRectangle(0,0,30,15, 4, 3);

    g.setColour(Colours::darkgrey);
    g.drawRoundedRectangle(0, 0, 30, 15, 4, 1);*/
    g.setColour(Colours::darkgrey);
    g.setFont(13);
    g.drawText(
        String(id),
        0,
        0,
        20, 10, Justification::centredLeft);
}

OlfactometerEditor::OlfactometerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{

    // accumulator = 0;

    desiredWidth = 350;

    Olfac = (Olfactometer*) parentNode;

    vector <ofSerialDeviceInfo> devices = serial.getDeviceList();



    deviceSelector = std::make_unique<ComboBox>();
    deviceSelector->setBounds(10,47,70,20);
    deviceSelector->addListener(this);
    deviceSelector->addItem("Device",1);
    
    for (int i = 0; i < devices.size(); i++)
    {
        deviceSelector->addItem(devices[i].getDevicePath(),i+2);
    }

    deviceSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(deviceSelector.get());



    LastSeriesNoStr = " ";
    LastTrialLengthStr = " ";
    LastOpenTimeStr = " ";
    LastOdorConcStr = " ";

    SeriesNoLabel = std::make_unique<Label>("Series No Label", "No. of Series:");
    SeriesNoLabel->setBounds(5, 77, 80, 20);
    SeriesNoLabel->setFont(Font("Small Text", 12, Font::plain));
    SeriesNoLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(SeriesNoLabel.get());

    TrialLengthLabel = std::make_unique<Label>("Trial Length Label", "Trial Length(s):");
    TrialLengthLabel->setBounds(92, 32, 80, 20);
    TrialLengthLabel->setFont(Font("Small Text", 12, Font::plain));
    TrialLengthLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(TrialLengthLabel.get());

    OpenTimeLabel = std::make_unique<Label>("Open Time Label", "Open Time(s):");
    OpenTimeLabel->setBounds(92, 77, 80, 20);
    OpenTimeLabel->setFont(Font("Small Text", 12, Font::plain));
    OpenTimeLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(OpenTimeLabel.get());

    SeriesNoValue = std::make_unique<Label>("no of series value", LastSeriesNoStr);
    SeriesNoValue->setBounds(10, 94, 60, 18);
    SeriesNoValue->setFont(Font("Default", 15, Font::plain));
    SeriesNoValue->setColour(Label::textColourId, Colours::white);
    SeriesNoValue->setColour(Label::backgroundColourId, Colours::grey);
    SeriesNoValue->setEditable(true);
    //SeriesNoValue->addListener(this);
    SeriesNoValue->setTooltip("Set the low cut for the selected channels");
    addAndMakeVisible(SeriesNoValue.get());

    TrialLengthValue = std::make_unique<Label>("trial length value", LastTrialLengthStr);
    TrialLengthValue->setBounds(97, 49, 60, 18);
    TrialLengthValue->setFont(Font("Default", 15, Font::plain));
    TrialLengthValue->setColour(Label::textColourId, Colours::white);
    TrialLengthValue->setColour(Label::backgroundColourId, Colours::grey);
    TrialLengthValue->setEditable(true);
    //TrialLengthValue->addListener(this);
    TrialLengthValue->setTooltip("Set the high cut for the selected channels");
    addAndMakeVisible(TrialLengthValue.get());

    OpenTimeValue = std::make_unique<Label>("open time value", LastSeriesNoStr);
    OpenTimeValue->setBounds(97, 94, 60, 18);
    OpenTimeValue->setFont(Font("Default", 15, Font::plain));
    OpenTimeValue->setColour(Label::textColourId, Colours::white);
    OpenTimeValue->setColour(Label::backgroundColourId, Colours::grey);
    OpenTimeValue->setEditable(true);
    //SeriesNoValue->addListener(this);
    OpenTimeValue->setTooltip("sdfsdfsd");
    addAndMakeVisible(OpenTimeValue.get());

    DrawOdorChans(BruceChNo,BruceFirstChan);

     /*Image im;
     im = ImageCache::getFromMemory(BinaryData::ArduinoIcon_png,
                                    BinaryData::ArduinoIcon_pngSize);

     icon = new ImageIcon(im);
     addAndMakeVisible(icon);
     icon->setBounds(75,15,50,50);

     icon->setOpacity(0.3f);*/

    //deviceSelector = new ComboBox(); 
    //deviceSelector = std::make_unique<ComboBox>();
    //deviceSelector->setBounds(10,105,125,20);
    //deviceSelector->addListener(this);
    //deviceSelector->addItem("Device",1);
    //
    //for (int i = 0; i < devices.size(); i++)
    //{
    //    deviceSelector->addItem(devices[i].getDevicePath(),i+2);
    //}

    //deviceSelector->setSelectedId(1, dontSendNotification);
    //addAndMakeVisible(deviceSelector.get());

    ////inputChannelSelector = new ComboBox();
    //inputChannelSelector = std::make_unique<ComboBox>();
    //inputChannelSelector->setBounds(10,30,55,20);
    //inputChannelSelector->addListener(this);
    //inputChannelSelector->addItem("Trig",1);

    //for (int i = 0; i < 16; i++)
    //    inputChannelSelector->addItem(String(i+1),i+2); // start numbering at one for
    //// user-visible channels

    //inputChannelSelector->setSelectedId(1, dontSendNotification);
    //addAndMakeVisible(inputChannelSelector.get());

    ////outputChannelSelector = new ComboBox();
    //outputChannelSelector = std::make_unique<ComboBox>();
    //outputChannelSelector->setBounds(10,80,80,20);
    //outputChannelSelector->addListener(this);
    //outputChannelSelector->addItem("Output CH",1);

    //for (int i = 1; i < 13; i++)
    //    outputChannelSelector->addItem(String(i+1),i+2);

    //outputChannelSelector->setSelectedId(14, dontSendNotification);
    //addAndMakeVisible(outputChannelSelector.get());

    ////gateChannelSelector = new ComboBox();
    //gateChannelSelector = std::make_unique<ComboBox>();
    //gateChannelSelector->setBounds(10,55,55,20);
    //gateChannelSelector->addListener(this);
    //gateChannelSelector->addItem("Gate",1);

    //for (int i = 0; i < 16; i++)
    //    gateChannelSelector->addItem(String(i+1),i+2); // start numbering at one for
    //// user-visible channels

    //gateChannelSelector->setSelectedId(1, dontSendNotification);
    //addAndMakeVisible(gateChannelSelector.get());

    //WriteDigButton = std::make_unique<UtilityButton>("W", Font("Small Text", 13, Font::bold));
    //WriteDigButton->setRadius(3.0f);
    //WriteDigButton->setBounds(95, 60, 65, 25);
    //WriteDigButton->addListener(this);

    //addAndMakeVisible(WriteDigButton.get());

}

OlfactometerEditor::~OlfactometerEditor()
{
}

void OlfactometerEditor::receivedEvent()
{

    icon->setOpacity(0.8f);
    startTimer(50);

}

void OlfactometerEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == deviceSelector.get())
    {
        Olfac->setOlfactometer(deviceSelector->getText());
    }/* else if (comboBoxThatHasChanged == outputChannelSelector.get())
    {
        arduino->setOutputChannel(outputChannelSelector->getSelectedId()-1);
    } else if (comboBoxThatHasChanged == inputChannelSelector.get())
    {
        arduino->setInputChannel(inputChannelSelector->getSelectedId()-1);
    } else if (comboBoxThatHasChanged == gateChannelSelector.get())
    {
        arduino->setGateChannel(gateChannelSelector->getSelectedId()-1);
    }*/
}

void OlfactometerEditor::buttonEvent(Button* button)
{

    //Olfac->StartOdorPres();

    if(OdorChButtons.contains((OdorChButton*)button))
    {
    ((OdorChButton*)button)->setEnabled(!((OdorChButton*)button)->enabled);
    //thread->toggleDIChannel(((DIButton*)button)->getId());
    repaint();
    }

}

void OlfactometerEditor::timerCallback()
{

    repaint();

    accumulator++;

    if (isFading)
    {

        if (accumulator > 15.0)
        {
            stopTimer();
            isFading = false;
        }

    }
    else
    {

        if (accumulator < 10.0)
        {
            icon->setOpacity(0.8f-(0.05*float(accumulator)));
            accumulator++;
        }
        else
        {
            icon->setOpacity(0.3f);
            stopTimer();
            accumulator = 0;
        }
    }
}

void OlfactometerEditor::DrawOdorChans(uint8_t ChNo, uint8_t FirstCh)
{

    OdorChLabel = std::make_unique<Label>("Odor Channels Label", "Channels");
    OdorChLabel->setBounds(160, 67, 80, 20);
    OdorChLabel->setFont(Font("Small Text", 12, Font::plain));
    OdorChLabel->setColour(Label::textColourId, Colours::darkgrey);
    OdorChLabel->setTransform(AffineTransform::rotation(-3.1416f / 2.0f,
        OdorChLabel->getWidth() * 0.5f + 160,
        OdorChLabel->getHeight() * 0.5f + 67));
    addAndMakeVisible(OdorChLabel.get());


    int maxChannelsPerColumn = 6;

    int ChannelsPerColumn = ChNo > 0 && ChNo < maxChannelsPerColumn ? ChNo : maxChannelsPerColumn;

    /*for (int i = 0; i < nAI; i++)
    {

        int colIndex = i / aiChannelsPerColumn;
        int rowIndex = i % aiChannelsPerColumn + 1;
        int xOffset = colIndex * 75 + 40;
        int y_pos = 5 + rowIndex * 26;

        AIButton* a = new AIButton(i, thread);
        a->setBounds(xOffset, y_pos, 15, 15);
        a->addListener(this);
        addAndMakeVisible(a);
        aiButtons.add(a);

        SOURCE_TYPE sourceType = thread->getSourceTypeForInput(i);
        printf("Got source type for input %d: %d\n", i, sourceType);

        SourceTypeButton* b = new SourceTypeButton(i, thread, sourceType);
        b->setBounds(xOffset + 18, y_pos - 2, 26, 17);
        b->addListener(this);
        addAndMakeVisible(b);
        sourceTypeButtons.add(b);

    }*/


    for (int i = 0, id = FirstCh; i < ChNo; i++, id++)
    {
        int colIndex = i / ChannelsPerColumn;
        int rowIndex = i % ChannelsPerColumn + 1;
        int xOffset = colIndex * 30 + 210;
        int y_pos = 20 + rowIndex * 15;

        OdorChButton* b = new OdorChButton(id);
        
        b->setBounds(xOffset, y_pos, 25, 15);
        b->addListener(this);
        b->setEnabled(true);
        addAndMakeVisible(b);
        OdorChButtons.add(b);
    }


    //int xOffset;
    //if (Beast)
    //{

    //    //TODO: Odor concentration stuff
    //    xOffset = (((int)ChNo % maxChannelsPerColumn == 0 ? 0 : 1) + (int)ChNo / ChannelsPerColumn) * 75 + 38 + colIndex * 45;
    //    xOffset = xOffset + 25;

    //}

}
