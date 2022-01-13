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

#include "../../Application/jucer_CommonHeaders.h"
#include "../../Application/jucer_Application.h"

//==============================================================================
class MessagesPopupWindow  : public Component,
                             private ComponentMovementWatcher
{
public:
    MessagesPopupWindow (Component& target, Component& parent, Project& project)
        : ComponentMovementWatcher (&parent),
          targetComponent (target),
          parentComponent (parent),
          messagesListComponent (*this, project)
    {
        parentComponent.addAndMakeVisible (this);
        setAlwaysOnTop (true);

        addAndMakeVisible (viewport);
        viewport.setScrollBarsShown (true, false);
        viewport.setViewedComponent (&messagesListComponent, false);
        viewport.setWantsKeyboardFocus (false);

        setOpaque (true);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    void resized() override
    {
        viewport.setBounds (getLocalBounds());
    }

    bool isListShowing() const
    {
        return messagesListComponent.getRequiredHeight() > 0;
    }

    void updateBounds (bool animate)
    {
        auto targetBounds = parentComponent.getLocalArea (&targetComponent, targetComponent.getLocalBounds());

        auto height = jmin (messagesListComponent.getRequiredHeight(), maxHeight);
        auto yPos = jmax (indent, targetBounds.getY() - height);

        Rectangle<int> bounds (targetBounds.getX(), yPos,
                               jmin (width, parentComponent.getWidth() - targetBounds.getX() - indent), targetBounds.getY() - yPos);

        auto& animator = Desktop::getInstance().getAnimator();

        if (animate)
        {
            setBounds (bounds.withY (targetBounds.getY()));
            animator.animateComponent (this, bounds, 1.0f, 150, false, 1.0, 1.0);
        }
        else
        {
            if (animator.isAnimating (this))
                animator.cancelAnimation (this, false);

            setBounds (bounds);
        }

        messagesListComponent.resized();
    }

private:
    //==============================================================================
    class MessagesListComponent  : public Component,
                                   private ValueTree::Listener,
                                   private AsyncUpdater
    {
    public:
        MessagesListComponent (MessagesPopupWindow& o, Project& currentProject)
            : owner (o),
              project (currentProject)
        {
            messagesTree = project.getProjectMessages();
            messagesTree.addListener (this);

            setOpaque (true);

            messagesChanged();
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            auto numMessages = messages.size();

            for (size_t i = 0; i < numMessages; ++i)
            {
                messages[i]->setBounds (bounds.removeFromTop (messageHeight));

                if (numMessages > 1 && i != (numMessages - 1))
                    bounds.removeFromTop (messageSpacing);
            }
        }

        void paint (Graphics& g) override
        {
            g.fillAll (findColour (backgroundColourId).contrasting (0.2f));
        }

        int getRequiredHeight() const
        {
            auto numMessages = (int) messages.size();

            if (numMessages > 0)
                return (numMessages * messageHeight) + ((numMessages - 1) * messageSpacing);

            return 0;
        }

        void updateSize (int parentWidth)
        {
            setSize (parentWidth, getRequiredHeight());
        }

    private:
        static constexpr int messageHeight = 65;
        static constexpr int messageSpacing = 2;

        //==============================================================================
        struct MessageComponent  : public Component
        {
            MessageComponent (MessagesListComponent& listComponent,
                              const Identifier& messageToDisplay,
                              std::vector<ProjectMessages::MessageAction> messageActions)
               : message (messageToDisplay)
            {
                for (auto& action : messageActions)
                {
                    auto button = std::make_unique<TextButton> (action.first);
                    addAndMakeVisible (*button);
                    button->onClick = action.second;

                    buttons.push_back (std::move (button));
                }

                icon = (ProjectMessages::getTypeForMessage (message) == ProjectMessages::Ids::warning ? getIcons().warning : getIcons().info);

                messageTitleLabel.setText (ProjectMessages::getTitleForMessage (message), dontSendNotification);
                messageTitleLabel.setFont (Font (11.0f).boldened());
                addAndMakeVisible (messageTitleLabel);

                messageDescriptionLabel.setText (ProjectMessages::getDescriptionForMessage (message), dontSendNotification);
                messageDescriptionLabel.setFont (Font (11.0f));
                messageDescriptionLabel.setJustificationType (Justification::topLeft);
                addAndMakeVisible (messageDescriptionLabel);

                dismissButton.setShape (getLookAndFeel().getCrossShape (1.0f), false, true, false);
                addAndMakeVisible (dismissButton);

                dismissButton.onClick = [this, &listComponent]
                {
                    listComponent.messagesTree.getChildWithName (ProjectMessages::getTypeForMessage (message))
                                              .getChildWithName (message)
                                              .setProperty (ProjectMessages::Ids::isVisible, false, nullptr);
                };
            }

            void paint (Graphics& g) override
            {
                g.fillAll (findColour (secondaryBackgroundColourId).contrasting (0.1f));

                auto bounds = getLocalBounds().reduced (5);

                g.setColour (findColour (defaultIconColourId));
                g.fillPath (icon, icon.getTransformToScaleToFit (bounds.removeFromTop (messageTitleHeight)
                                                                       .removeFromLeft (messageTitleHeight).toFloat(), true));
            }

            void resized() override
            {
                auto bounds = getLocalBounds().reduced (5);

                auto topSlice = bounds.removeFromTop (messageTitleHeight);

                topSlice.removeFromLeft (messageTitleHeight + 5);
                topSlice.removeFromRight (5);

                dismissButton.setBounds (topSlice.removeFromRight (messageTitleHeight));
                messageTitleLabel.setBounds (topSlice);
                bounds.removeFromTop (5);

                auto numButtons = (int) buttons.size();

                if (numButtons > 0)
                {
                    auto buttonBounds = bounds.removeFromBottom (buttonHeight);

                    auto buttonWidth = roundToInt ((float) buttonBounds.getWidth() / 3.5f);
                    auto requiredWidth = (numButtons * buttonWidth) + ((numButtons - 1) * buttonSpacing);
                    buttonBounds.reduce ((buttonBounds.getWidth() - requiredWidth) / 2, 0);

                    for (auto& b : buttons)
                    {
                        b->setBounds (buttonBounds.removeFromLeft (buttonWidth));
                        buttonBounds.removeFromLeft (buttonSpacing);
                    }

                    bounds.removeFromBottom (5);
                }

                messageDescriptionLabel.setBounds (bounds);
            }

            static constexpr int messageTitleHeight = 11;
            static constexpr int buttonHeight = messageHeight / 4;
            static constexpr int buttonSpacing = 5;

            Identifier message;

            Path icon;
            Label messageTitleLabel, messageDescriptionLabel;
            std::vector<std::unique_ptr<TextButton>> buttons;
            ShapeButton dismissButton { {},
                                        findColour (treeIconColourId),
                                        findColour (treeIconColourId).overlaidWith (findColour (defaultHighlightedTextColourId).withAlpha (0.2f)),
                                        findColour (treeIconColourId).overlaidWith (findColour (defaultHighlightedTextColourId).withAlpha (0.4f)) };

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageComponent)
        };

        //==============================================================================
        void valueTreePropertyChanged   (ValueTree&, const Identifier&) override  { messagesChanged(); }
        void valueTreeChildAdded        (ValueTree&, ValueTree&)        override  { messagesChanged(); }
        void valueTreeChildRemoved      (ValueTree&, ValueTree&, int)   override  { messagesChanged(); }
        void valueTreeChildOrderChanged (ValueTree&, int, int)          override  { messagesChanged(); }
        void valueTreeParentChanged     (ValueTree&)                    override  { messagesChanged(); }
        void valueTreeRedirected        (ValueTree&)                    override  { messagesChanged(); }

        void handleAsyncUpdate() override
        {
            messagesChanged();
        }

        void messagesChanged()
        {
            auto listWasShowing = (getHeight() > 0);

            auto warningsTree = messagesTree.getChildWithName (ProjectMessages::Ids::warning);
            auto notificationsTree = messagesTree.getChildWithName (ProjectMessages::Ids::notification);

            auto removePredicate = [warningsTree, notificationsTree] (std::unique_ptr<MessageComponent>& messageComponent)
            {
                for (int i = 0; i < warningsTree.getNumChildren(); ++i)
                {
                    auto child = warningsTree.getChild (i);

                    if (child.getType() == messageComponent->message
                        && child.getProperty (ProjectMessages::Ids::isVisible))
                    {
                        return false;
                    }
                }

                for (int i = 0; i < notificationsTree.getNumChildren(); ++i)
                {
                    auto child = notificationsTree.getChild (i);

                    if (child.getType() == messageComponent->message
                        && child.getProperty (ProjectMessages::Ids::isVisible))
                    {
                        return false;
                    }
                }

                return true;
            };

            messages.erase (std::remove_if (messages.begin(), messages.end(), removePredicate),
                                            messages.end());

            for (auto* tree : { &warningsTree, &notificationsTree })
            {
                for (int i = 0; i < tree->getNumChildren(); ++i)
                {
                    auto child = tree->getChild (i);

                    if (! child.getProperty (ProjectMessages::Ids::isVisible))
                        continue;

                    const auto messageMatchesType = [&child] (const auto& messageComponent)
                    {
                        return messageComponent->message == child.getType();
                    };

                    if (std::none_of (messages.begin(), messages.end(), messageMatchesType))
                    {
                        messages.push_back (std::make_unique<MessageComponent> (*this,
                                                                                child.getType(),
                                                                                project.getMessageActions (child.getType())));
                        addAndMakeVisible (*messages.back());
                    }
                }
            }

            const auto isNowShowing = (messages.size() > 0);

            owner.updateBounds (isNowShowing != listWasShowing);
            updateSize (owner.getWidth());
        }

        //==============================================================================
        MessagesPopupWindow& owner;
        Project& project;

        ValueTree messagesTree;
        std::vector<std::unique_ptr<MessageComponent>> messages;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessagesListComponent)
    };

    //==============================================================================
    void componentMovedOrResized (bool, bool) override
    {
        if (isListShowing())
            updateBounds (false);
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    void componentPeerChanged() override
    {
        if (isListShowing())
            updateBounds (false);
    }

    void componentVisibilityChanged() override
    {
        if (isListShowing())
            updateBounds (false);
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    //==============================================================================
    static constexpr int maxHeight = 500, width = 350, indent = 20;

    Component& targetComponent;
    Component& parentComponent;

    Viewport viewport;
    MessagesListComponent messagesListComponent;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessagesPopupWindow)
};

//==============================================================================
class ProjectMessagesComponent  : public Component
{
public:
    ProjectMessagesComponent()
    {
        setFocusContainerType (FocusContainerType::focusContainer);
        setTitle ("Project Messages");

        addAndMakeVisible (warningsComponent);
        addAndMakeVisible (notificationsComponent);

        warningsComponent.addMouseListener (this, true);
        notificationsComponent.addMouseListener (this, true);

        setOpaque (true);
    }

    //==============================================================================
    void resized() override
    {
        auto b = getLocalBounds();

        warningsComponent.setBounds (b.removeFromLeft (b.getWidth() / 2).reduced (5));
        notificationsComponent.setBounds (b.reduced (5));
    }

    void paint (Graphics& g) override
    {
        auto backgroundColour = findColour (backgroundColourId);

        if (isMouseDown || isMouseOver)
            backgroundColour = backgroundColour.overlaidWith (findColour (defaultHighlightColourId)
                                                                 .withAlpha (isMouseDown ? 1.0f : 0.8f));

        g.fillAll (backgroundColour);
    }

    //==============================================================================
    void mouseEnter (const MouseEvent&) override
    {
        isMouseOver = true;
        repaint();
    }

    void mouseExit (const MouseEvent&) override
    {
        isMouseOver = false;
        repaint();
    }

    void mouseDown (const MouseEvent&) override
    {
        isMouseDown = true;
        repaint();
    }

    void mouseUp (const MouseEvent&) override
    {
        isMouseDown = false;
        repaint();

        showOrHideMessagesWindow();
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this,
                                                       AccessibilityRole::button,
                                                       AccessibilityActions().addAction (AccessibilityActionType::press,
                                                                                         [this] { showOrHideMessagesWindow(); }));
    }

    //==============================================================================
    void setProject (Project* newProject)
    {
        if (currentProject != newProject)
        {
            currentProject = newProject;

            if (currentProject != nullptr)
            {
                if (auto* projectWindow = ProjucerApplication::getApp().mainWindowList.getMainWindowForFile (currentProject->getFile()))
                    messagesWindow = std::make_unique<MessagesPopupWindow> (*this, *projectWindow, *currentProject);

                auto projectMessagesTree = currentProject->getProjectMessages();

                warningsComponent.setTree (projectMessagesTree.getChildWithName (ProjectMessages::Ids::warning));
                notificationsComponent.setTree (projectMessagesTree.getChildWithName (ProjectMessages::Ids::notification));
            }
            else
            {
                warningsComponent.setTree ({});
                notificationsComponent.setTree ({});
            }
        }
    }

    void numMessagesChanged()
    {
        const auto total = warningsComponent.getNumMessages()
                           + notificationsComponent.getNumMessages();

        setHelpText (String (total) + (total == 1 ? " message" : " messages"));
    }

    void showOrHideMessagesWindow()
    {
        if (messagesWindow != nullptr)
            showOrHideAllMessages (! messagesWindow->isListShowing());
    }

private:
    //==============================================================================
    struct MessageCountComponent  : public Component,
                                    private ValueTree::Listener
    {
        MessageCountComponent (ProjectMessagesComponent& o, Path pathToUse)
          : owner (o),
            path (pathToUse)
        {
            setInterceptsMouseClicks (false, false);
        }

        void paint (Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();

            g.setColour (findColour ((owner.isMouseDown || owner.isMouseOver) ? defaultHighlightedTextColourId : treeIconColourId));
            g.fillPath (path, path.getTransformToScaleToFit (b.removeFromLeft (b.getWidth() / 2.0f), true));

            b.removeFromLeft (5);
            g.drawFittedText (String (numMessages), b.getSmallestIntegerContainer(), Justification::centredLeft, 1);
        }

        void setTree (ValueTree tree)
        {
            messagesTree = tree;

            if (messagesTree.isValid())
                messagesTree.addListener (this);

            updateNumMessages();
        }

        void updateNumMessages()
        {
            numMessages = messagesTree.getNumChildren();
            owner.numMessagesChanged();
            repaint();
        }

        int getNumMessages() const noexcept  { return numMessages; }

    private:
        void valueTreeChildAdded   (ValueTree&, ValueTree&)        override  { updateNumMessages(); }
        void valueTreeChildRemoved (ValueTree&, ValueTree&, int)   override  { updateNumMessages(); }

        ProjectMessagesComponent& owner;
        ValueTree messagesTree;

        Path path;
        int numMessages = 0;
    };

    void showOrHideAllMessages (bool shouldBeVisible)
    {
        if (currentProject != nullptr)
        {
            auto messagesTree = currentProject->getProjectMessages();

            auto setVisible = [shouldBeVisible] (ValueTree subTree)
            {
                for (int i = 0; i < subTree.getNumChildren(); ++i)
                    subTree.getChild (i).setProperty (ProjectMessages::Ids::isVisible, shouldBeVisible, nullptr);
            };

            setVisible (messagesTree.getChildWithName (ProjectMessages::Ids::warning));
            setVisible (messagesTree.getChildWithName (ProjectMessages::Ids::notification));
        }
    }

    //==============================================================================
    Project* currentProject = nullptr;
    bool isMouseOver = false, isMouseDown = false;

    MessageCountComponent warningsComponent { *this, getIcons().warning },
                          notificationsComponent { *this, getIcons().info };

    std::unique_ptr<MessagesPopupWindow> messagesWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectMessagesComponent)
};
