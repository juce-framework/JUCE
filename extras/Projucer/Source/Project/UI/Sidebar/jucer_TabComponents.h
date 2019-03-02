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


//==============================================================================
class ConcertinaHeader    : public Component,
                            public ChangeBroadcaster
{
public:
    ConcertinaHeader (String n, Path p)
        : Component (n), name (n), iconPath (p)
    {
        panelIcon = Icon (iconPath, Colours::white);

        nameLabel.setText (name, dontSendNotification);
        nameLabel.setJustificationType (Justification::centredLeft);
        nameLabel.setInterceptsMouseClicks (false, false);
        nameLabel.setColour (Label::textColourId, Colours::white);

        addAndMakeVisible (nameLabel);
    }

    void resized() override
    {
        auto b = getLocalBounds().toFloat();

        iconBounds = b.removeFromLeft (b.getHeight()).reduced (7, 7);
        arrowBounds = b.removeFromRight (b.getHeight());
        nameLabel.setBounds (b.toNearestInt());
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (defaultButtonBackgroundColourId));
        g.fillRoundedRectangle (getLocalBounds().reduced (2, 3).toFloat(), 2.0f);

        g.setColour (Colours::white);
        g.fillPath (arrowPath = ProjucerLookAndFeel::getArrowPath (arrowBounds,
                                                                   getParentComponent()->getBoundsInParent().getY() == yPosition ? 2 : 0,
                                                                   true, Justification::centred));

        panelIcon.draw (g, iconBounds.toFloat(), false);
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (! e.mouseWasDraggedSinceMouseDown())
            sendChangeMessage();
    }

    int direction = 0;
    int yPosition = 0;

private:
    String name;
    Label nameLabel;

    Path iconPath;
    Icon panelIcon;

    Rectangle<float> arrowBounds, iconBounds;
    Path arrowPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaHeader)
};

//==============================================================================
class FindPanel    : public Component,
                     private Timer,
                     private FocusChangeListener
{
public:
    FindPanel (std::function<void (const String&)> cb)
        : callback (cb)
    {
        addAndMakeVisible (editor);
        editor.onTextChange = [this] { startTimer (250); };
        editor.onFocusLost  = [this]
        {
            isFocused = false;
            repaint();
        };

        Desktop::getInstance().addFocusChangeListener (this);

        lookAndFeelChanged();
    }

    ~FindPanel() override
    {
        Desktop::getInstance().removeFocusChangeListener (this);
    }

    void paintOverChildren (Graphics& g) override
    {
        if (! isFocused)
            return;

        g.setColour (findColour (defaultHighlightColourId));

        Path p;
        p.addRoundedRectangle (getLocalBounds().reduced (2), 3.0f);
        g.strokePath (p, PathStrokeType (2.0f));
    }


    void resized() override
    {
        editor.setBounds (getLocalBounds().reduced (2));
    }

private:
    TextEditor editor;
    bool isFocused = false;
    std::function<void (const String&)> callback;

    //==============================================================================
    void lookAndFeelChanged() override
    {
        editor.setTextToShowWhenEmpty ("Filter...", findColour (widgetTextColourId).withAlpha (0.3f));
    }

    void globalFocusChanged (Component* focusedComponent) override
    {
        if (focusedComponent == &editor)
        {
            isFocused = true;
            repaint();
        }
    }

    void timerCallback() override
    {
        stopTimer();
        callback (editor.getText());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FindPanel)
};

//==============================================================================
class ConcertinaTreeComponent    : public Component
{
public:
    ConcertinaTreeComponent (TreePanelBase* tree, bool hasAddButton = false,
                             bool hasSettingsButton = false, bool hasFindPanel = false)
         : treeToDisplay (tree)
    {
        if (hasAddButton)
        {
            addButton.reset (new IconButton ("Add", &getIcons().plus));
            addAndMakeVisible (addButton.get());
            addButton->onClick = [this] { showAddMenu(); };
        }

        if (hasSettingsButton)
        {
            settingsButton.reset (new IconButton ("Settings", &getIcons().settings));
            addAndMakeVisible (settingsButton.get());
            settingsButton->onClick = [this] { showSettings(); };
        }

        if (hasFindPanel)
        {
            findPanel.reset (new FindPanel ([this] (const String& filter) { treeToDisplay->rootItem->setSearchFilter (filter); }));
            addAndMakeVisible (findPanel.get());
        }

        addAndMakeVisible (treeToDisplay.get());
    }

    ~ConcertinaTreeComponent() override
    {
        treeToDisplay.reset();
        addButton.reset();
        findPanel.reset();
        settingsButton.reset();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        if (addButton != nullptr || settingsButton != nullptr || findPanel != nullptr)
        {
            auto bottomSlice = bounds.removeFromBottom (25);
            bottomSlice.removeFromRight (3);

            if (addButton != nullptr)
                addButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

            if (settingsButton != nullptr)
                settingsButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

            if (findPanel != nullptr)
                findPanel->setBounds (bottomSlice.reduced (2));
        }

        treeToDisplay->setBounds (bounds);
    }

    TreePanelBase* getTree() const noexcept    { return treeToDisplay.get(); }

private:
    std::unique_ptr<TreePanelBase> treeToDisplay;
    std::unique_ptr<IconButton> addButton, settingsButton;
    std::unique_ptr<FindPanel> findPanel;

    void showAddMenu()
    {
        auto numSelected = treeToDisplay->tree.getNumSelectedItems();

        if (numSelected > 1)
            return;

        if (numSelected == 0)
        {
            if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
                root->showPopupMenu();
        }
        else
        {
            if (auto* item = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getSelectedItem (0)))
                item->showAddMenu();
        }
    }

    void showSettings()
    {
        if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
        {
            treeToDisplay->tree.clearSelectedItems();
            root->showDocument();
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaTreeComponent)
};
