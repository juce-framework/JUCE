/*
  ==============================================================================

    This file was auto-generated and contains the startup code for a PIP.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "%%filename%%"

%%component_begin%%
class Application    : public JUCEApplication
{
public:
    //==============================================================================
    Application() {}

    const String getApplicationName() override       { return "%%project_name%%"; }
    const String getApplicationVersion() override    { return "%%project_version%%"; }

    void initialize (const String&) override         { %%startup%% }
    void shutdown() override                         { %%shutdown%% }

private:
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow (const String& name, Component* c)  : DocumentWindow (name,
                                                                         Desktop::getInstance().getDefaultLookAndFeel()
                                                                                               .findColor (ResizableWindow::backgroundColorId),
                                                                         DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (c, true);

           #if JUCE_ANDROID || JUCE_IOS
            setFullScreen (true);
           #else
            setResizable (true, false);
            setResizeLimits (300, 250, 10000, 10000);
            centerWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };
    ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (Application)
%%component_end%%

%%audioprocessor_begin%%
//==============================================================================
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new %%class_name%%();
}
%%audioprocessor_end%%

%%console_begin%%
%%console_end%%
