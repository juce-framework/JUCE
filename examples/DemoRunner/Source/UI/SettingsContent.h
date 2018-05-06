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

#pragma once

#include "MainComponent.h"

//==============================================================================
class SettingsContent    : public Component
{
public:
    SettingsContent (MainComponent& topLevelComponent)
        : innerContent (topLevelComponent)
    {
        settingsViewport.setViewedComponent (&innerContent, false);
        addAndMakeVisible (settingsViewport);

        setOpaque (true);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto scrollBarWidth = getLookAndFeel().getDefaultScrollbarWidth();

        innerContent.setSize (jmax (r.getWidth() - scrollBarWidth, innerContent.getMinimumWidth()),
                              jmax (r.getHeight(), innerContent.getMinimumHeight()));

        settingsViewport.setBounds (r);
    }

private:
    Viewport settingsViewport;

    //==============================================================================
    class InnerContent    : public Component,
                            public ComponentMovementWatcher
    {
    public:
        InnerContent (MainComponent& topLevelComponent)
            : ComponentMovementWatcher (this), mainComponent (topLevelComponent)
        {
            addAndMakeVisible (graphicsTitleLabel);
            graphicsTitleLabel.setFont (18.0f);

            addAndMakeVisible (audioTitleLabel);
            audioTitleLabel.setFont (18.0f);

            addLookAndFeels();

            addAndMakeVisible (lookAndFeelSelector);
            for (int i = 0; i < lookAndFeelNames.size(); ++i)
                lookAndFeelSelector.addItem (lookAndFeelNames.getReference (i), i + 1);

            lookAndFeelSelector.setSelectedItemIndex (lookAndFeelNames.indexOf ("LookAndFeel_V4 (Dark)"));
            lookAndFeelSelector.onChange = [this]
            {
                auto* lf = lookAndFeels.getUnchecked (lookAndFeelSelector.getSelectedItemIndex());
                Desktop::getInstance().setDefaultLookAndFeel (lf);
            };

            addAndMakeVisible (lookAndFeelLabel);
            lookAndFeelLabel.setJustificationType (Justification::centredRight);
            lookAndFeelLabel.attachToComponent (&lookAndFeelSelector, true);

            addAndMakeVisible (rendererSelector);
            rendererSelector.onChange = [this] { mainComponent.setRenderingEngine (rendererSelector.getSelectedItemIndex()); };

            addAndMakeVisible (rendererLabel);
            rendererLabel.setJustificationType (Justification::centredRight);
            rendererLabel.attachToComponent (&rendererSelector, true);

            audioSettings.reset (new AudioDeviceSelectorComponent (getSharedAudioDeviceManager(),
                                                                   0, 256, 0, 256, true, true, true, false));
            addAndMakeVisible (audioSettings.get());
            audioSettings->setItemHeight (itemHeight);

            setOpaque (true);
        }

        void paint (Graphics& g) override
        {
            g.fillAll (findColour (ResizableWindow::backgroundColourId).contrasting (0.2f));
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            auto space = itemHeight / 4;

            graphicsTitleLabel.setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (space);

            auto xPos = bounds.getX() + (bounds.getWidth() * 0.35f);
            auto width = bounds.getWidth() * 0.6f;

            lookAndFeelSelector.setBounds (bounds.removeFromTop (itemHeight).withWidth ((int) width).withX ((int) xPos));

            bounds.removeFromTop (space);

            rendererSelector.setBounds (bounds.removeFromTop (itemHeight).withWidth ((int) width).withX ((int) xPos));

            bounds.removeFromTop (space);
            audioTitleLabel.setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (space);

            audioSettings->setBounds (bounds);
        }

        //==============================================================================
        int getMinimumHeight() const noexcept    { return 550; }
        int getMinimumWidth() const noexcept     { return 350; }

    private:
        MainComponent& mainComponent;
        ComponentPeer* peer = nullptr;

        const int itemHeight = 30;

        Label graphicsTitleLabel  { {}, "Graphics" },
              audioTitleLabel     { {}, "Audio" },
              lookAndFeelLabel    { {}, "LookAndFeel:" },
              rendererLabel       { {}, "Renderer:" };

        ComboBox lookAndFeelSelector, rendererSelector;

        StringArray lookAndFeelNames;
        OwnedArray<LookAndFeel> lookAndFeels;

        std::unique_ptr<AudioDeviceSelectorComponent> audioSettings;

        //==============================================================================
        void refreshRenderingEngineSelector()
        {
            StringArray renderingEngines (mainComponent.getRenderingEngines());

            rendererSelector.clear (NotificationType::dontSendNotification);

            for (int i = 0; i < renderingEngines.size(); ++i)
                rendererSelector.addItem (renderingEngines.getReference (i), i + 1);

            rendererSelector.setSelectedItemIndex (mainComponent.getCurrentRenderingEngine());
        }

        //==============================================================================
        void componentMovedOrResized (bool, bool) override    {}
        void componentVisibilityChanged() override            {}
        void componentPeerChanged() override
        {
            auto* newPeer = getPeer();
            if (peer != newPeer)
            {
                peer = newPeer;

                if (peer != nullptr)
                    refreshRenderingEngineSelector();
            }
        }

        //==============================================================================
        void addLookAndFeels()
        {
            lookAndFeelNames.addArray ({ "LookAndFeel_V1", "LookAndFeel_V2", "LookAndFeel_V3",
                                         "LookAndFeel_V4 (Dark)", "LookAndFeel_V4 (Midnight)",
                                         "LookAndFeel_V4 (Grey)", "LookAndFeel_V4 (Light)" });

            lookAndFeels.add (new LookAndFeel_V1());
            lookAndFeels.add (new LookAndFeel_V2());
            lookAndFeels.add (new LookAndFeel_V3());
            lookAndFeels.add (new LookAndFeel_V4 (LookAndFeel_V4::getDarkColourScheme()));
            lookAndFeels.add (new LookAndFeel_V4 (LookAndFeel_V4::getMidnightColourScheme()));
            lookAndFeels.add (new LookAndFeel_V4 (LookAndFeel_V4::getGreyColourScheme()));
            lookAndFeels.add (new LookAndFeel_V4 (LookAndFeel_V4::getLightColourScheme()));
        }
    };

    InnerContent innerContent;
};
