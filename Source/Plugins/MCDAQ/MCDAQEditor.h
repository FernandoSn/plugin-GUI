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

#ifndef __MCDAQEDITOR_H__
#define __MCDAQEDITOR_H__

#include <ProcessorHeaders.h>
#include <EditorHeaders.h>
#include "MCDAQThread.h"

class UtilityButton;
/**

User interface for MCDAQbd devices.

@see SourceNode

*/
class SourceNode;

class MCDAQEditor;
class MCDAQCanvas;
class MCDAQInterface;
class Annotation;
class ColorSelector;

class EditorBackground : public Component
{
public:
	EditorBackground(int nAI, int nDI);

private:
	void paint(Graphics& g);
	int nAI;
	int nDI;

};

class AIButton : public ToggleButton, public Timer
{
public:
	AIButton(int id, MCDAQThread* thread);

	void setId(int id);
	int getId(); 
	void setEnabled(bool);
	void timerCallback();

	MCDAQThread* thread;

	friend class MCDAQEditor;

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

	int id;
	bool enabled;

};

class DIButton : public ToggleButton, public Timer
{
public:
	DIButton(int id, MCDAQThread* thread);

	void setId(int id);
	int getId();
	void setEnabled(bool);
	void timerCallback() override;

	MCDAQThread* thread;

	friend class MCDAQEditor;

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

	int id;
	bool enabled;

};

class SourceTypeButton : public TextButton, public Timer
{
public:
	SourceTypeButton(int id, MCDAQThread* thread, SOURCE_TYPE source);

	void setId(int id);
	int getId();
	void toggleSourceType();
	void timerCallback();

	void update(SOURCE_TYPE sourceType);

	MCDAQThread* thread;

	friend class MCDAQEditor;

private:

	int id;
	bool enabled;

};

class FifoMonitor : public Component, public Timer
{
public:
	FifoMonitor(MCDAQThread* thread);

	void setFillPercentage(float percentage);

	void timerCallback();

private:
	void paint(Graphics& g);

	float fillPercentage;
	MCDAQThread* thread;
	int id;
};

class BackgroundLoader : public Thread
{
public:
	BackgroundLoader(MCDAQThread* t, MCDAQEditor* e);
	~BackgroundLoader();
	void run();
private:
	MCDAQThread* t;
	MCDAQEditor* e;
};

class MCDAQEditor : public GenericEditor, public ComboBox::Listener
{
public:
	MCDAQEditor(GenericProcessor* parentNode, MCDAQThread* thread, bool useDefaultParameterEditors);
	virtual ~MCDAQEditor();

	void draw();

	void buttonEvent(Button* button);
	void comboBoxChanged(ComboBox*);

	void saveCustomParameters(XmlElement*);
	void loadCustomParameters(XmlElement*);

private:

	OwnedArray<AIButton> aiButtons;
	OwnedArray<TextButton> sourceTypeButtons;
	OwnedArray<DIButton> diButtons;

	ScopedPointer<ComboBox> sampleRateSelectBox;
	ScopedPointer<ComboBox> voltageRangeSelectBox;
	ScopedPointer<FifoMonitor> fifoMonitor;

	ScopedPointer<UtilityButton> swapDeviceButton;

	Array<File> savingDirectories;

	ScopedPointer<BackgroundLoader> uiLoader;
	ScopedPointer<EditorBackground> background;

	MCDAQThread* thread;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MCDAQEditor);

};


#endif  // __RHD2000EDITOR_H_2AD3C591__
