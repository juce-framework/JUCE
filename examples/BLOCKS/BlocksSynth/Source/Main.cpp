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
                                                    Colours::lightgrey,
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
