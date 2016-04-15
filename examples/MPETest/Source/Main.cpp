/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/


#include "../JuceLibraryCode/JuceHeader.h"
#include <array>


struct MPETestClasses
{
    #include "MPESetupComponent.h"
    #include "ZoneColourPicker.h"
    #include "ZoneLayoutComponent.h"
    #include "MPEDemoSynthVoice.h"
    #include "Visualiser.h"
    #include "MainComponent.h"
};


//==============================================================================
class MPETestApplication  : public JUCEApplication
{
public:
    //==============================================================================
    MPETestApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String&) override
    {
        mainWindow = new MainWindow (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    //==============================================================================
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow (String name)
            : DocumentWindow (name, Colours::lightgrey, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MPETestClasses::MainComponent(), true);

            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    //==============================================================================
    ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (MPETestApplication)
