/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Plugins/IOConfigurationWindow.h"

class PluginGraph;

/**
    A window that shows a log of parameter change messages sent by the plugin.
*/
class PluginDebugWindow : public AudioProcessorEditor,
                          public AudioProcessorParameter::Listener,
                          public ListBoxModel,
                          public AsyncUpdater
{
public:
    PluginDebugWindow (AudioProcessor& proc)
        : AudioProcessorEditor (proc), audioProc (proc)
    {
        setSize (500, 200);
        addAndMakeVisible (list);

        for (auto* p : audioProc.getParameters())
            p->addListener (this);

        log.add ("Parameter debug log started");
    }

    void parameterValueChanged (int parameterIndex, float newValue) override
    {
        auto* param = audioProc.getParameters()[parameterIndex];
        auto value = param->getCurrentValueAsText().quoted() + " (" + String (newValue, 4) + ")";

        appendToLog ("parameter change", *param, value);
    }

    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override
    {
        auto* param = audioProc.getParameters()[parameterIndex];
        appendToLog ("gesture", *param, gestureIsStarting ? "start" : "end");
    }

private:
    void appendToLog (StringRef action, AudioProcessorParameter& param, StringRef value)
    {
        String entry (action + " " + param.getName (30).quoted() + " [" + String (param.getParameterIndex()) + "]: " + value);

        {
            ScopedLock lock (pendingLogLock);
            pendingLogEntries.add (entry);
        }

        triggerAsyncUpdate();
    }

    void resized() override
    {
        list.setBounds(getLocalBounds());
    }

    int getNumRows() override
    {
        return log.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool) override
    {
        g.setColour (getLookAndFeel().findColour (TextEditor::textColourId));

        if (isPositiveAndBelow (rowNumber, log.size()))
            g.drawText (log[rowNumber], Rectangle<int> { 0, 0, width, height }, Justification::left, true);
    }

    void handleAsyncUpdate() override
    {
        if (log.size() > logSizeTrimThreshold)
            log.removeRange (0, log.size() - maxLogSize);

        {
            ScopedLock lock (pendingLogLock);
            log.addArray (pendingLogEntries);
            pendingLogEntries.clear();
        }

        list.updateContent();
        list.scrollToEnsureRowIsOnscreen (log.size() - 1);
    }

    constexpr static const int maxLogSize = 300;
    constexpr static const int logSizeTrimThreshold = 400;

    ListBox list { "Log", this };

    StringArray log;
    StringArray pendingLogEntries;
    CriticalSection pendingLogLock;

    AudioProcessor& audioProc;
};

//==============================================================================
/**
    A desktop window containing a plugin's GUI.
*/
class PluginWindow  : public DocumentWindow
{
public:
    enum class Type
    {
        normal = 0,
        generic,
        programs,
        audioIO,
        debug,
        numTypes
    };

    PluginWindow (AudioProcessorGraph::Node* n, Type t, OwnedArray<PluginWindow>& windowList)
       : DocumentWindow (n->getProcessor()->getName(),
                         LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                         DocumentWindow::minimiseButton | DocumentWindow::closeButton),
         activeWindowList (windowList),
         node (n), type (t)
    {
        setResizable (true, false);
        setSize (400, 300);

        if (auto* ui = createProcessorEditor (*node->getProcessor(), type))
            setContentOwned (ui, true);

       #if JUCE_IOS || JUCE_ANDROID
        auto screenBounds = Desktop::getInstance().getDisplays().getTotalBounds (true).toFloat();
        auto scaleFactor = jmin ((screenBounds.getWidth() - 50) / getWidth(), (screenBounds.getHeight() - 50) / getHeight());

        if (scaleFactor < 1.0f)
            setSize ((int) (getWidth() * scaleFactor), (int) (getHeight() * scaleFactor));

        setTopLeftPosition (20, 20);
       #else
        setTopLeftPosition (node->properties.getWithDefault (getLastXProp (type), Random::getSystemRandom().nextInt (500)),
                            node->properties.getWithDefault (getLastYProp (type), Random::getSystemRandom().nextInt (500)));
       #endif

        node->properties.set (getOpenProp (type), true);

        setVisible (true);
    }

    ~PluginWindow() override
    {
        clearContentComponent();
    }

    void moved() override
    {
        node->properties.set (getLastXProp (type), getX());
        node->properties.set (getLastYProp (type), getY());
    }

    void closeButtonPressed() override
    {
        node->properties.set (getOpenProp (type), false);
        activeWindowList.removeObject (this);
    }

    static String getLastXProp (Type type)    { return "uiLastX_" + getTypeName (type); }
    static String getLastYProp (Type type)    { return "uiLastY_" + getTypeName (type); }
    static String getOpenProp  (Type type)    { return "uiopen_"  + getTypeName (type); }

    OwnedArray<PluginWindow>& activeWindowList;
    const AudioProcessorGraph::Node::Ptr node;
    const Type type;

    BorderSize<int> getBorderThickness() override
    {
       #if JUCE_IOS || JUCE_ANDROID
        const int border = 10;
        return { border, border, border, border };
       #else
        return DocumentWindow::getBorderThickness();
       #endif
    }

private:
    float getDesktopScaleFactor() const override     { return 1.0f; }

    static AudioProcessorEditor* createProcessorEditor (AudioProcessor& processor,
                                                        PluginWindow::Type type)
    {
        if (type == PluginWindow::Type::normal)
        {
            if (processor.hasEditor())
                if (auto* ui = processor.createEditorIfNeeded())
                    return ui;

            type = PluginWindow::Type::generic;
        }

        if (type == PluginWindow::Type::generic)  return new GenericAudioProcessorEditor (processor);
        if (type == PluginWindow::Type::programs) return new ProgramAudioProcessorEditor (processor);
        if (type == PluginWindow::Type::audioIO)  return new IOConfigurationWindow (processor);
        if (type == PluginWindow::Type::debug)    return new PluginDebugWindow (processor);

        jassertfalse;
        return {};
    }

    static String getTypeName (Type type)
    {
        switch (type)
        {
            case Type::normal:     return "Normal";
            case Type::generic:    return "Generic";
            case Type::programs:   return "Programs";
            case Type::audioIO:    return "IO";
            case Type::debug:      return "Debug";
            case Type::numTypes:
            default:               return {};
        }
    }

    //==============================================================================
    struct ProgramAudioProcessorEditor  : public AudioProcessorEditor
    {
        ProgramAudioProcessorEditor (AudioProcessor& p)  : AudioProcessorEditor (p)
        {
            setOpaque (true);

            addAndMakeVisible (panel);

            Array<PropertyComponent*> programs;

            auto numPrograms = p.getNumPrograms();
            int totalHeight = 0;

            for (int i = 0; i < numPrograms; ++i)
            {
                auto name = p.getProgramName (i).trim();

                if (name.isEmpty())
                    name = "Unnamed";

                auto pc = new PropertyComp (name, p);
                programs.add (pc);
                totalHeight += pc->getPreferredHeight();
            }

            panel.addProperties (programs);

            setSize (400, jlimit (25, 400, totalHeight));
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::grey);
        }

        void resized() override
        {
            panel.setBounds (getLocalBounds());
        }

    private:
        struct PropertyComp  : public PropertyComponent,
                               private AudioProcessorListener
        {
            PropertyComp (const String& name, AudioProcessor& p)  : PropertyComponent (name), owner (p)
            {
                owner.addListener (this);
            }

            ~PropertyComp() override
            {
                owner.removeListener (this);
            }

            void refresh() override {}
            void audioProcessorChanged (AudioProcessor*, const ChangeDetails&) override {}
            void audioProcessorParameterChanged (AudioProcessor*, int, float) override {}

            AudioProcessor& owner;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyComp)
        };

        PropertyPanel panel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramAudioProcessorEditor)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginWindow)
};
