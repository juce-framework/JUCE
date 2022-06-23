/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class ContentViewComponent  : public Component
{
public:
    ContentViewComponent()
    {
        setTitle ("Content");
        setFocusContainerType (Component::FocusContainerType::focusContainer);

        addAndMakeVisible (logoComponent);

        addAndMakeVisible (fileNameLabel);
        fileNameLabel.setJustificationType (Justification::centred);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        fileNameLabel.setBounds (bounds.removeFromTop (15));

        if (content != nullptr)
            content->setBounds (bounds);
        else
            logoComponent.setBounds (bounds);
    }

    Component* getCurrentComponent() noexcept
    {
        return content.get();
    }

    void setContent (std::unique_ptr<Component> newContent,
                     const String& labelText)
    {
        content = std::move (newContent);
        addAndMakeVisible (content.get());

        fileNameLabel.setVisible (labelText.isNotEmpty());
        fileNameLabel.setText (labelText, dontSendNotification);

        resized();
    }

private:
    class LogoComponent  : public Component
    {
    public:
        void paint (Graphics& g) override
        {
            g.setColour (findColour (defaultTextColourId));

            auto bounds = getLocalBounds();
            bounds.reduce (bounds.getWidth() / 6, bounds.getHeight() / 6);

            g.setFont (15.0f);
            g.drawFittedText (versionInfo, bounds.removeFromBottom (50), Justification::centredBottom, 3);

            if (logo != nullptr)
                logo->drawWithin (g, bounds.withTrimmedBottom (bounds.getHeight() / 4).toFloat(),
                                  RectanglePlacement (RectanglePlacement::centred), 1.0f);
        }

    private:
        std::unique_ptr<Drawable> logo = []() -> std::unique_ptr<Drawable>
        {
            if (auto svg = parseXML (BinaryData::background_logo_svg))
                return Drawable::createFromSVG (*svg);

            jassertfalse;
            return {};
        }();

        String versionInfo = SystemStats::getJUCEVersion()
                             + newLine
                             + ProjucerApplication::getApp().getVersionDescription();
    };

    std::unique_ptr<Component> content;
    LogoComponent logoComponent;
    Label fileNameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentViewComponent)
};
