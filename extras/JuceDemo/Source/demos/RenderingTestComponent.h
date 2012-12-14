/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  21 Sep 2012 12:12:19pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_RENDERINGTESTCOMPONENT_RENDERINGTESTCOMPONENT_1C802450__
#define __JUCER_HEADER_RENDERINGTESTCOMPONENT_RENDERINGTESTCOMPONENT_1C802450__

//[Headers]     -- You can add your own extra header files here --
#include "../jucedemo_headers.h"
class RenderingTestCanvas;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class RenderingTestComponent  : public Component,
                                public ComboBoxListener,
                                public SliderListener
{
public:
    //==============================================================================
    RenderingTestComponent ();
    ~RenderingTestComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void sliderValueChanged (Slider* sliderThatWasMoved);

    // Binary resources:
    static const char* demoJpeg_jpg;
    static const int demoJpeg_jpgSize;
    static const char* demoPng_png;
    static const int demoPng_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    friend class RenderingTestCanvas;
    //[/UserVariables]

    //==============================================================================
    ComboBox* testTypeComboBox;
    Label* testTypeLabel;
    Label* speedLabel;
    RenderingTestCanvas* testCanvas;
    Slider* opacitySlider;
    ToggleButton* highQualityToggle;
    ToggleButton* animateSizeToggle;
    ToggleButton* animateRotationToggle;
    ToggleButton* animatePositionToggle;
    ToggleButton* animateFillToggle;
    Label* opacityLabel;
    Slider* xSlider;
    Slider* ySlider;
    Slider* sizeSlider;
    Slider* angleSlider;
    Label* xSliderLabel;
    Label* ySliderLabel;
    Label* sizeSliderLabel;
    Label* angleSliderLabel;
    ToggleButton* clipToRectangleToggle;
    ToggleButton* clipToPathToggle;
    ToggleButton* clipToImageToggle;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderingTestComponent)
};


#endif   // __JUCER_HEADER_RENDERINGTESTCOMPONENT_RENDERINGTESTCOMPONENT_1C802450__
