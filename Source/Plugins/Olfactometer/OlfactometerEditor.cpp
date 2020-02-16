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
#include "OlfactometersSN.h"
//#include "Olfactometer.h"
//#include <SerialLib.h>
#include <stdio.h>
#include <bitset>

//std::ofstream DebugOlEd("OlEd.txt");


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
    : 
    GenericEditor(parentNode, useDefaultParameterEditors)
{

    desiredWidth = 350;

    Olfac = (Olfactometer*) parentNode;


    std::vector<ofSerialDeviceInfo> devices = serial.getDeviceList();

    FindLabOlfactometers(devices);

    deviceSelector = std::make_unique<ComboBox>();
    deviceSelector->setBounds(10,47,70,20);
    deviceSelector->addListener(this);
    deviceSelector->addItem("Device",1);
    
    int ComboBoxIdx = 0;
    for (auto iter = OlfactometerCOMS.begin(); iter != OlfactometerCOMS.end(); ++iter, ComboBoxIdx++) 
    {
        deviceSelector->addItem(iter->first, ComboBoxIdx + 2);
    }

    deviceSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(deviceSelector.get());



    LastSeriesNoStr = " ";
    LastTrialLengthStr = " ";
    LastOpenTimeStr = " ";

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
    SeriesNoValue->setEditable(false);
    //SeriesNoValue->addListener(this);
    SeriesNoValue->setTooltip("Set the low cut for the selected channels");
    addAndMakeVisible(SeriesNoValue.get());

    TrialLengthValue = std::make_unique<Label>("trial length value", LastTrialLengthStr);
    TrialLengthValue->setBounds(97, 49, 60, 18);
    TrialLengthValue->setFont(Font("Default", 15, Font::plain));
    TrialLengthValue->setColour(Label::textColourId, Colours::white);
    TrialLengthValue->setColour(Label::backgroundColourId, Colours::grey);
    TrialLengthValue->setEditable(false);
    //TrialLengthValue->addListener(this);
    TrialLengthValue->setTooltip("Set the high cut for the selected channels");
    addAndMakeVisible(TrialLengthValue.get());

    OpenTimeValue = std::make_unique<Label>("open time value", LastOpenTimeStr);
    OpenTimeValue->setBounds(97, 94, 60, 18);
    OpenTimeValue->setFont(Font("Default", 15, Font::plain));
    OpenTimeValue->setColour(Label::textColourId, Colours::white);
    OpenTimeValue->setColour(Label::backgroundColourId, Colours::grey);
    OpenTimeValue->setEditable(false);
    //SeriesNoValue->addListener(this);
    OpenTimeValue->setTooltip("sdfsdfsd");
    addAndMakeVisible(OpenTimeValue.get());


    /*WriteDigButton = std::make_unique<UtilityButton>("W", Font("Small Text", 13, Font::bold));
    WriteDigButton->setRadius(3.0f);
    WriteDigButton->setBounds(95, 60, 65, 25);
    WriteDigButton->addListener(this);

    addAndMakeVisible(WriteDigButton.get());*/

}

OlfactometerEditor::~OlfactometerEditor()
{
    if(OlfactometerInit)
        SaveOlfacParams();
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
        if ((juce::String)"Device" != deviceSelector->getText())
        {
            OlfacName = deviceSelector->getText().toStdString();

            //This is bad. If this params are not defined before the functions below are beign called, the plugin will behave weird.

            NoOdorButtons = Olfactometer::BruceChNo; ///I need to rewrite this to capture all posible olfactometers.
            FirstOdorButtonID = Olfactometer::BruceFirstChan;
            ActiveButtons = std::vector<char>(NoOdorButtons,false);
            //////////////////////////////////////////////////////////////////////////////////////////////////////////

            //Olfactometer Connection.
            OlfactometerInit = Olfac->InitOlfactometer(OlfactometerCOMS.find(OlfacName)->second);

            LoadOlfacParams();

            OdorChButtons.ensureStorageAllocated(NoOdorButtons);

            DrawOdorChans(NoOdorButtons, FirstOdorButtonID);

            SeriesNoValue->setEditable(true);
            TrialLengthValue->setEditable(true);
            OpenTimeValue->setEditable(true);


            //Setting Olfactometer vars.
            Olfac->SetSeriesNo(LastSeriesNoStr.getIntValue());
            Olfac->SetTrialLength(LastTrialLengthStr.getDoubleValue());
            Olfac->SetOpenTime(LastOpenTimeStr.getDoubleValue());
        }
        else
        {
            Olfac->FinOlfactometer();
            SeriesNoValue->setEditable(false);
            TrialLengthValue->setEditable(false);
            OpenTimeValue->setEditable(false);

            OlfactometerInit = false;
        }
    }
}

void OlfactometerEditor::buttonEvent(Button* button)
{
    if(OdorChButtons.contains((OdorChButton*)button))
    {
        OdorChButton* OdorB = reinterpret_cast<OdorChButton*>(button);
        bool IsEnabled = OdorB->enabled;
        OdorB->setEnabled(!IsEnabled);
        int idx = OdorB->getId() - FirstOdorButtonID;
        ActiveButtons[idx] = !IsEnabled;
        //thread->toggleDIChannel(((DIButton*)button)->getId());
        //DebugOlEd << " Odor butt \n";
        repaint();
    }

    //DebugOlEd << " Other butt \n";
}

void OlfactometerEditor::labelTextChanged(Label* label)
{
    
    Value val = label->getTextValue();

    if (label == SeriesNoValue.get())
    {
        Olfac->SetSeriesNo(int(val.getValue()));
        LastSeriesNoStr = label->getText();
    }
    else if (label == TrialLengthValue.get())
    {
        //double requestedValue = double(val.getValue());
        Olfac->SetTrialLength(double(val.getValue()));
        LastTrialLengthStr = label->getText();
    }
    else if (label == OpenTimeValue.get())
    {
        double requestedValue = double(val.getValue());
        if (requestedValue > Olfac->GetTrialLength())
        {
            CoreServices::sendStatusMessage("Open time should be less than trial length");
            label->setText(LastOpenTimeStr, dontSendNotification);
            LastOpenTimeStr = label->getText();
        }
        else
        {
            Olfac->SetOpenTime(requestedValue);
            LastOpenTimeStr = label->getText();
        }
    }
}

void OlfactometerEditor::updateSettings()
{
    /*if (!isUpdated)
    {

        Olfac->setOdorVec(ActiveButtons, FirstOdorButtonID); 
        DebugOlEd << "Child upd \n"; 
        isUpdated = true;

    }*/
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
        b->setEnabled(ActiveButtons[i]);
        //b->setEnabled(true);
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

void OlfactometerEditor::FindLabOlfactometers(std::vector<ofSerialDeviceInfo>& Devices)
{
    //This code is slow and makes the GUI crash. I need to 
    //std::ofstream DebFil("debpr.txt");


    //std::cout << "si entro func "<< "\n";
    std::string file_name = "TempOlfac.txt";

    for (auto it = Devices.begin(); it < Devices.end(); ++it) //Iterate over all COM Devices.
    {
        // Get the Serial Numbers of COM devices and store them in TempOlfac.
        std::system(("wmic path Win32_PnPEntity where \"Name like '%" +
            it->getDevicePath() + "%'\" get DeviceID > " + file_name).c_str());
        
        
        std::ifstream file(file_name);
        //Get the trash string in the File.
        std::string DevicesSNBad = { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

        //New string to get every other character. 
        std::string DevicesSN;
        DevicesSN.resize(DevicesSNBad.size() / 2);

        
        //Getting every other character.
        for (auto BadIt = DevicesSNBad.begin(), GoodIt = DevicesSN.begin(); 
            BadIt < DevicesSNBad.end(); BadIt += 2, ++GoodIt)
        {
            *GoodIt = *BadIt;
        }
        
       
        //Search for Olfactometers/Arduinos associated with a particular COM port.
        for (std::string* ptrOlfacArd = OlfacArd, *ptrOlfacSer = OlfacSer, *ptrOlfacNames = OlfacNames;
            ptrOlfacArd < OlfacArd + MAX_OLFACTOMETERS; ++ptrOlfacArd, ++ptrOlfacSer,++ptrOlfacNames)
        {
            //If an arduino/olfactometer is found put it in the map
            size_t found = DevicesSN.find(*ptrOlfacArd);
            if (found != string::npos)
            {
                auto MapIt = OlfactometerCOMS.find(*ptrOlfacNames);
                if (MapIt != OlfactometerCOMS.end())
                {
                    MapIt->second.first = it->getDevicePath();
                }
                else
                {
                    std::pair<std::string, std::string> ComPair;
                    ComPair.first = it->getDevicePath();
                    OlfactometerCOMS.emplace(*ptrOlfacNames, ComPair);
                }
                //DebFil << "Encontr" << "\n";
            }

            found = DevicesSN.find(*ptrOlfacSer);
            if (found != string::npos)
            {
                auto MapIt = OlfactometerCOMS.find(*ptrOlfacNames);
                if (MapIt != OlfactometerCOMS.end())
                {
                    MapIt->second.second = it->getDevicePath();
                }
                else
                {
                    std::pair<std::string, std::string> ComPair;
                    ComPair.second = it->getDevicePath();
                    OlfactometerCOMS.emplace(*ptrOlfacNames, ComPair);
                }
            }
        }
    }
}

void OlfactometerEditor::LoadOlfacParams()
{

    OlfactometerParamsIn.open(OlfacName);
    char StrBuff[3];
    StrBuff[2] = 0;

    OlfactometerParamsIn.read(StrBuff, 2);
    if (StrBuff[0] == 48)
        LastSeriesNoStr = StrBuff + 1;
    else
        LastSeriesNoStr = StrBuff;

    OlfactometerParamsIn.read(StrBuff, 2);
    if (StrBuff[0] == 48)
        LastTrialLengthStr = StrBuff + 1;
    else
        LastTrialLengthStr = StrBuff;

    OlfactometerParamsIn.read(StrBuff, 2);
    if (StrBuff[0] == 48)
        LastOpenTimeStr = StrBuff + 1;
    else
        LastOpenTimeStr = StrBuff;

    SeriesNoValue->setText(LastSeriesNoStr, dontSendNotification);
    TrialLengthValue->setText(LastTrialLengthStr, dontSendNotification);
    OpenTimeValue->setText(LastOpenTimeStr, dontSendNotification);


    OlfactometerParamsIn.read(reinterpret_cast<char*>(ActiveButtons.data()), NoOdorButtons);

    OlfactometerParamsIn.close();
}

void OlfactometerEditor::SaveOlfacParams()
{

    LastSeriesNoStr = SeriesNoValue->getText();
    LastTrialLengthStr = TrialLengthValue->getText();
    LastOpenTimeStr = OpenTimeValue->getText();

    if (LastSeriesNoStr.length() == 1)
        LastSeriesNoStr = "0" + LastSeriesNoStr;

    if (LastTrialLengthStr.length() == 1)
        LastTrialLengthStr = "0" + LastTrialLengthStr;

    if (LastOpenTimeStr.length() == 1)
        LastOpenTimeStr = "0" + LastOpenTimeStr;

    OlfactometerParamsOut.open(OlfacName);

    OlfactometerParamsOut << LastSeriesNoStr
        << LastTrialLengthStr << LastOpenTimeStr;
    
    for (auto it = ActiveButtons.begin(); it < ActiveButtons.end(); ++it)
    {
        OlfactometerParamsOut << *it;
        //a = *it;
    }

    OlfactometerParamsOut.close();

}

//void OlfactometerEditor::saveCustomParameters(XmlElement* xml)
//{
//    Deb << "save \n";
//
//    xml->setAttribute("Type", "OlfactometerEditor");
//
//    LastSeriesNoStr = SeriesNoValue->getText();
//    LastTrialLengthStr = TrialLengthValue->getText();
//    LastOpenTimeStr = OpenTimeValue->getText();
//
//    XmlElement* textLabelValues = xml->createNewChildElement("OLFPARAMS");
//    textLabelValues->setAttribute("no of series value", LastSeriesNoStr);
//    textLabelValues->setAttribute("trial length value", LastTrialLengthStr);
//    textLabelValues->setAttribute("open time value", LastOpenTimeStr);
//
//}
//
//void OlfactometerEditor::loadCustomParameters(XmlElement* xml)
//{
//    Deb << "load \n";
//    forEachXmlChildElement(*xml, xmlNode)
//    {
//        if (xmlNode->hasTagName("OLFPARAMS"))
//        {
//            LastSeriesNoStr = xmlNode->getStringAttribute("no of series value", LastSeriesNoStr);
//            LastTrialLengthStr = xmlNode->getStringAttribute("trial length value", LastTrialLengthStr);
//            LastOpenTimeStr = xmlNode->getStringAttribute("open time value", LastOpenTimeStr);
//
//            SeriesNoValue->setText(LastSeriesNoStr, dontSendNotification);
//            TrialLengthValue->setText(LastTrialLengthStr, dontSendNotification);
//            OpenTimeValue->setText(LastOpenTimeStr, dontSendNotification);
//        }
//    }
//}