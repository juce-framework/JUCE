
#include "../jucedemo_headers.h"

#if JUCE_DIRECTSHOW

//==============================================================================
class DirectShowWindowWithFileBrowser  : public Component,
                                         public FilenameComponentListener
{
public:
    DirectShowWindowWithFileBrowser (DirectShowComponent::VideoRendererType type)
        : fileChooser ("movie", File::nonexistent, true, false, false,
                       "*", String::empty, "(choose a video file to play)"),
          dshowComp (type)
    {
        addAndMakeVisible (&dshowComp);
        addAndMakeVisible (&fileChooser);
        fileChooser.addListener (this);
        fileChooser.setBrowseButtonText ("browse");
    }

    void resized()
    {
        dshowComp.setBounds (0, 0, getWidth(), getHeight() - 60);

        if (transportControl != 0)
            transportControl->setBounds (0, dshowComp.getBottom() + 4, getWidth(), 26);

        fileChooser.setBounds (0, getHeight() - 24, getWidth(), 24);
    }

    void filenameComponentChanged (FilenameComponent*)
    {
        // this is called when the user changes the filename in the file chooser box
        if (dshowComp.loadMovie (fileChooser.getCurrentFile()))
        {
            addAndMakeVisible (transportControl = new TransportControl (dshowComp));
            resized();

            dshowComp.play();
        }
        else
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         "Couldn't load the file!",
                                         "Sorry, DirectShow didn't manage to load that file!");
        }
    }

private:
    DirectShowComponent dshowComp;
    FilenameComponent fileChooser;

    //==============================================================================
    // A quick-and-dirty transport control, containing a play button and a position slider..
    class TransportControl  : public Component,
                              public ButtonListener,
                              public SliderListener,
                              public Timer
    {
    public:
        TransportControl (DirectShowComponent& dshowComp_)
            : playButton ("Play/Pause"),
              position (String::empty),
              dshowComp (dshowComp_)
        {
            addAndMakeVisible (&playButton);
            playButton.addListener (this);

            addAndMakeVisible (&position);
            position.setRange (0, dshowComp.getMovieDuration(), 0);
            position.setSliderStyle (Slider::LinearHorizontal);
            position.setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
            position.addListener (this);

            startTimer (1000 / 50);
        }

        void buttonClicked (Button*)
        {
            if (dshowComp.isPlaying())
                dshowComp.stop();
            else
                dshowComp.play();
        }

        void sliderValueChanged (Slider*)
        {
            dshowComp.setPosition (position.getValue());
        }

        void resized()
        {
            const int playButtonWidth = 90;
            playButton.setBounds (0, 0, playButtonWidth, getHeight());
            position.setBounds (playButtonWidth, 0, getWidth() - playButtonWidth, getHeight());
        }

        void timerCallback()
        {
            if (! position.isMouseButtonDown())
                position.setValue (dshowComp.getPosition(), dontSendNotification);
        }

    private:
        TextButton playButton;
        Slider position;
        DirectShowComponent& dshowComp;
    };

    ScopedPointer<TransportControl> transportControl;
};


//==============================================================================
class DirectShowDemo  : public Component
{
public:
    //==============================================================================
    DirectShowDemo()
        : dsComp1 (DirectShowComponent::dshowVMR7),
          dsComp2 (DirectShowComponent::dshowEVR)
    {
        setName ("DirectShow");

        // add a movie component..
        addAndMakeVisible (&dsComp1);
        addAndMakeVisible (&dsComp2);
    }

    ~DirectShowDemo()
    {
        dsComp1.setVisible (false);
        dsComp2.setVisible (false);
    }

    void resized()
    {
        dsComp1.setBoundsRelative (0.05f, 0.05f, 0.425f, 0.9f);
        dsComp2.setBoundsRelative (0.525f, 0.05f, 0.425f, 0.9f);
    }

private:
    //==============================================================================
    DirectShowWindowWithFileBrowser dsComp1, dsComp2;
};


//==============================================================================
Component* createDirectShowDemo()
{
    return new DirectShowDemo();
}

#endif
