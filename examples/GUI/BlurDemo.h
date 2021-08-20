/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             BlurDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Demonstrates various image blurring techniques available in
                   JUCE.

 dependencies:     juce_gui_basics
 exporters:        xcode_mac, vs2019, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        BlurDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct DemoGaussianFilter : public ImageEffectFilter
{
    int blurRadius {0};

    void applyEffect (Image& sourceImage,
                      Graphics& destContext,
                      float /*scaleFactor*/,
                      float /*alpha*/) final
    {
        ImageConvolutionKernel kernel { blurRadius * 2 + 1 };
        kernel.createGaussianBlur (blurRadius);

        auto blurredImage = sourceImage.createCopy();
        kernel.applyToImage (blurredImage, sourceImage, sourceImage.getBounds());

        destContext.drawImageAt (blurredImage, 0, 0);
    }
};

class TimedImageComponent : public Component
{
public:
    explicit TimedImageComponent(Label& labelToUse)
        : statusLabel {labelToUse}
    {
    }

    void paint (Graphics& g) final
    {
        g.drawImageAt (blurredImage, 0, 0);
    }

    void setImage (Image imageToDraw)
    {
        sourceImage = imageToDraw;
        updateBlurredImage();
    }

    Image& getImage()
    {
        return sourceImage;
    }

    void setFilter (ImageEffectFilter* newFilter)
    {
        filter = newFilter;
        updateBlurredImage();
    }

private:
    String getRenderTimeMessage (double renderTime)
    {
        if (renderTime < 1.0)
            return "Rendered in " + String {renderTime * 1000.0} + " milliseconds.";

        return "Rendered in " + String {renderTime} + " seconds.";
    }

    void updateBlurredImage()
    {
        blurredImage = sourceImage.createCopy();
        blurredImage.clear (blurredImage.getBounds());
        Graphics g {blurredImage};
        const auto scale = g.getInternalContext().getPhysicalPixelScaleFactor();

        auto renderTime = 0.0;

        {
            ScopedTimeMeasurement timer {renderTime};

            if (filter != nullptr)
                filter->applyEffect (sourceImage, g, scale, 1.0f);
            else
                g.drawImageAt (sourceImage, 0, 0);
        }

       statusLabel.setText (getRenderTimeMessage (renderTime),
                            dontSendNotification);

        repaint();
    }

    Label& statusLabel;
    Image sourceImage;
    Image blurredImage;
    ImageEffectFilter* filter = nullptr;
};

//==============================================================================
class BlurDemo : public Component
{
public:
    BlurDemo()
    {
        addAndMakeVisible (blurTechniqueBox);
        blurTechniqueBox.addItemList ({ "No Blur",
                                        "Gaussian Blur",
                                        "Stack Blur" }, 1);
        blurTechniqueBox.onChange = [this]() {
            updateBlurTechnique();
        };
        blurTechniqueBox.setSelectedItemIndex (0);

        addChildComponent (threadPoolButton);
        threadPoolButton.setButtonText ("Use Thread Pool?");
        threadPoolButton.setClickingTogglesState (true);
        threadPoolButton.setToggleState (true, dontSendNotification);
        threadPoolButton.onClick = [this]() {
            updateBlurTechnique();
        };

        image.setImage (getImageFromAssets ("portmeirion.jpg"));
        addAndMakeVisible (image);

        addAndMakeVisible (radiusSlider);
        radiusSlider.setRange (0.0, 25.0, 1.0);
        radiusSlider.onValueChange = [this]() {
            updateBlurTechnique();
        };
        radiusSlider.setValue (10.0);

        addAndMakeVisible (statusLabel);

        setSize (500, 500);
    }

    void paint(juce::Graphics& g) final
    {
        g.setColour (Colours::white);
        g.drawRect (image.getBounds().expanded (1));
    }

    void resized() final
    {
        auto bounds = getLocalBounds().reduced (50);
        const auto imageSize = image.getImage().getBounds();
        const auto imageBounds = bounds.removeFromTop (imageSize.getHeight()).withSizeKeepingCentre (imageSize.getWidth(), imageSize.getHeight());
        image.setBounds (imageBounds);

        bounds = bounds.withSizeKeepingCentre (jmin (300, bounds.getWidth()), bounds.getHeight());

        radiusSlider.setBounds (bounds.removeFromTop (45).reduced (0, 10));

        FlexBox flex;
        flex.flexDirection = FlexBox::Direction::column;
        flex.items = {
            FlexItem {blurTechniqueBox}.withHeight (25.0f),
            FlexItem {statusLabel}.withHeight (25.0f),
            FlexItem {threadPoolButton}.withHeight (25.0f)
        };
        flex.performLayout (bounds);
    }

private:
    enum class BlurTechnique
    {
        none,
        gaussian,
        stack
    };

    void updateBlurTechnique()
    {
        const auto blurTechnique = static_cast<BlurTechnique> (blurTechniqueBox.getSelectedItemIndex());

        radiusSlider.setEnabled (blurTechnique != BlurTechnique::none);
        threadPoolButton.setVisible (blurTechnique == BlurTechnique::stack);

        gaussianBlur.blurRadius = getBlurRadius();
        stackBlur.setBlurRadius (getBlurRadius());
        stackBlur.setUseThreadPool (threadPoolButton.getToggleState());

        switch (blurTechnique)
        {
            case BlurTechnique::none:
                image.setFilter (nullptr);
                break;
            case BlurTechnique::gaussian:
                image.setFilter (&gaussianBlur);
                break;
            case BlurTechnique::stack:
                image.setFilter (&stackBlur);
                break;
        }

        repaint();
    }

    int getBlurRadius() const
    {
        return roundToInt (radiusSlider.getValue());
    }

    ComboBox blurTechniqueBox;
    ToggleButton threadPoolButton;
    Slider radiusSlider;
    Label statusLabel;
    TimedImageComponent image {statusLabel};

    DemoGaussianFilter gaussianBlur;
    StackBlurEffect stackBlur;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlurDemo)
};
