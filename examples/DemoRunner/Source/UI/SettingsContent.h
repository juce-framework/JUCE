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

        setFocusContainerType (FocusContainerType::focusContainer);
        setTitle ("DemoRunner Settings");

        setOpaque (true);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        constexpr int minimumWidth  = 350;
        constexpr int minimumHeight = 550;

        auto r = getLocalBounds();
        const auto scrollBarWidth = getLookAndFeel().getDefaultScrollbarWidth();

        innerContent.setSize (jmax (r.getWidth() - scrollBarWidth, minimumWidth),
                              jmax (r.getHeight(), minimumHeight));

        settingsViewport.setBounds (r);
    }

private:
    static constexpr float titleLabelFontHeight = 18.0f;
    static constexpr int itemHeight = 30;
    static constexpr int itemSpacing = 7;

    class GraphicsSettingsGroup  : public Component,
                                   private ComponentMovementWatcher
    {
    public:
        GraphicsSettingsGroup (MainComponent& comp)
            : ComponentMovementWatcher (&comp),
              mainComponent (comp)
        {
            addAndMakeVisible (titleLabel);
            titleLabel.setFont (titleLabelFontHeight);

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

            setFocusContainerType (FocusContainerType::focusContainer);
            setTitle ("Graphics Settings");
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            titleLabel.setBounds (bounds.removeFromTop (itemHeight));
            bounds.removeFromTop (itemSpacing);

            const auto xPos  = roundToInt ((float) bounds.getX() + ((float) bounds.getWidth() * 0.35f));
            const auto width = roundToInt ((float) bounds.getWidth() * 0.6f);

            lookAndFeelSelector.setBounds (bounds.removeFromTop (itemHeight).withWidth (width).withX (xPos));
            bounds.removeFromTop (itemSpacing);

            rendererSelector.setBounds (bounds.removeFromTop (itemHeight).withWidth (width).withX (xPos));
        }

    private:
        void componentMovedOrResized (bool, bool) override    {}
        using ComponentListener::componentMovedOrResized;

        void componentVisibilityChanged() override            {}
        using ComponentListener::componentVisibilityChanged;

        void componentPeerChanged() override
        {
            auto* newPeer = mainComponent.getPeer();

            if (peer != newPeer)
            {
                peer = newPeer;

                if (peer != nullptr)
                    refreshRenderingEngineSelector();
            }
        }

        void refreshRenderingEngineSelector()
        {
            rendererSelector.clear (NotificationType::dontSendNotification);

            rendererSelector.addItemList (mainComponent.getRenderingEngines(), 1);
            rendererSelector.setSelectedItemIndex (mainComponent.getCurrentRenderingEngine());
        }

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

        MainComponent& mainComponent;
        ComponentPeer* peer = nullptr;

        Label titleLabel       { {}, "Graphics" },
              lookAndFeelLabel { {}, "LookAndFeel:" },
              rendererLabel    { {}, "Renderer:" };

        ComboBox lookAndFeelSelector, rendererSelector;
        StringArray lookAndFeelNames;
        OwnedArray<LookAndFeel> lookAndFeels;
    };

    class AudioSettingsGroup  : public Component
    {
    public:
        AudioSettingsGroup()
            : deviceSelectorComp (getSharedAudioDeviceManager(), 0, 256, 0, 256, true, true, true, false)
        {
            addAndMakeVisible (titleLabel);
            titleLabel.setFont (titleLabelFontHeight);

            addAndMakeVisible (deviceSelectorComp);
            deviceSelectorComp.setItemHeight (itemHeight);

            setFocusContainerType (FocusContainerType::focusContainer);
            setTitle ("Audio Settings");
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            titleLabel.setBounds (bounds.removeFromTop (itemHeight));
            bounds.removeFromTop (itemSpacing);

            deviceSelectorComp.setBounds (bounds);
        }

    private:
        Label titleLabel { {}, "Audio" };
        AudioDeviceSelectorComponent deviceSelectorComp;
    };

    //==============================================================================
    class InnerContent    : public Component
    {
    public:
        InnerContent (MainComponent& mainComponent)
            : graphicsSettings (mainComponent)
        {
            addAndMakeVisible (graphicsSettings);
            addAndMakeVisible (audioSettings);

            setOpaque (true);
        }

        void paint (Graphics& g) override
        {
            g.fillAll (findColour (ResizableWindow::backgroundColourId).contrasting (0.2f));
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            graphicsSettings.setBounds (bounds.removeFromTop (150));
            audioSettings.setBounds (bounds);
        }

        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            return createIgnoredAccessibilityHandler (*this);
        }

    private:
        GraphicsSettingsGroup graphicsSettings;
        AudioSettingsGroup audioSettings;
    };

    Viewport settingsViewport;
    InnerContent innerContent;
};
