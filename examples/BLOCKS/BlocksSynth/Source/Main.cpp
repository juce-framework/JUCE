/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"


//==============================================================================
class BlocksSynthApplication  : public JUCEApplication
{
public:
    //==============================================================================
    BlocksSynthApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }

    //==============================================================================
    void initialise (const String& /*commandLine*/) override    { mainWindow = new MainWindow (getApplicationName()); }
    void shutdown() override                                    { mainWindow = nullptr; }

    //==============================================================================
    struct MainWindow    : public DocumentWindow
    {
        MainWindow (String name)  : DocumentWindow (name,
                                                    LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                                                    DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

            centreWithSize (getWidth(), getHeight());
            setResizable (true, true);
            setVisible (true);

           #if JUCE_IOS
            setFullScreen (true);
           #endif
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        TooltipWindow tooltipWindow;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (BlocksSynthApplication)
