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

 name:             MenusDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases menu features.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MenusDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/**
    This struct contains a header component that will be used when the burger menu
    is enabled. It contains an icon that can be used to show the side panel containing
    the menu.
*/
struct BurgerMenuHeader  : public Component
{
    BurgerMenuHeader (SidePanel& sp)
        : sidePanel (sp)
    {
        static const unsigned char burgerMenuPathData[]
            = { 110,109,0,0,128,64,0,0,32,65,108,0,0,224,65,0,0,32,65,98,254,212,232,65,0,0,32,65,0,0,240,65,252,
                169,17,65,0,0,240,65,0,0,0,65,98,0,0,240,65,8,172,220,64,254,212,232,65,0,0,192,64,0,0,224,65,0,0,
                192,64,108,0,0,128,64,0,0,192,64,98,16,88,57,64,0,0,192,64,0,0,0,64,8,172,220,64,0,0,0,64,0,0,0,65,
                98,0,0,0,64,252,169,17,65,16,88,57,64,0,0,32,65,0,0,128,64,0,0,32,65,99,109,0,0,224,65,0,0,96,65,108,
                0,0,128,64,0,0,96,65,98,16,88,57,64,0,0,96,65,0,0,0,64,4,86,110,65,0,0,0,64,0,0,128,65,98,0,0,0,64,
                254,212,136,65,16,88,57,64,0,0,144,65,0,0,128,64,0,0,144,65,108,0,0,224,65,0,0,144,65,98,254,212,232,
                65,0,0,144,65,0,0,240,65,254,212,136,65,0,0,240,65,0,0,128,65,98,0,0,240,65,4,86,110,65,254,212,232,
                65,0,0,96,65,0,0,224,65,0,0,96,65,99,109,0,0,224,65,0,0,176,65,108,0,0,128,64,0,0,176,65,98,16,88,57,
                64,0,0,176,65,0,0,0,64,2,43,183,65,0,0,0,64,0,0,192,65,98,0,0,0,64,254,212,200,65,16,88,57,64,0,0,208,
                65,0,0,128,64,0,0,208,65,108,0,0,224,65,0,0,208,65,98,254,212,232,65,0,0,208,65,0,0,240,65,254,212,
                200,65,0,0,240,65,0,0,192,65,98,0,0,240,65,2,43,183,65,254,212,232,65,0,0,176,65,0,0,224,65,0,0,176,
                65,99,101,0,0 };

        Path p;
        p.loadPathFromData (burgerMenuPathData, sizeof (burgerMenuPathData));
        burgerButton.setShape (p, true, true, false);

        burgerButton.onClick = [this] { showOrHide(); };
        addAndMakeVisible (burgerButton);
    }

    ~BurgerMenuHeader() override
    {
        sidePanel.showOrHide (false);
    }

private:
    void paint (Graphics& g) override
    {
        auto titleBarBackgroundColour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId)
                                                        .darker();

        g.setColour (titleBarBackgroundColour);
        g.fillRect (getLocalBounds());
    }

    void resized() override
    {
        auto r = getLocalBounds();

        burgerButton.setBounds (r.removeFromRight (40).withSizeKeepingCentre (20, 20));

        titleLabel.setFont (Font ((float) getHeight() * 0.5f, Font::plain));
        titleLabel.setBounds (r);
    }

    void showOrHide()
    {
        sidePanel.showOrHide (! sidePanel.isPanelShowing());
    }

    SidePanel& sidePanel;

    Label titleLabel         { "titleLabel", "JUCE Demo" };
    ShapeButton burgerButton { "burgerButton", Colours::lightgrey, Colours::lightgrey, Colours::white };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BurgerMenuHeader)
};

//==============================================================================
class MenusDemo    : public Component,
                     public ApplicationCommandTarget,
                     public MenuBarModel
{
public:
    //==============================================================================
    /** A list of the commands that this demo responds to. */
    enum CommandIDs
    {
        menuPositionInsideWindow = 1,
        menuPositionGlobalMenuBar,
        menuPositionBurgerMenu,
        outerColourRed,
        outerColourGreen,
        outerColourBlue,
        innerColourRed,
        innerColourGreen,
        innerColourBlue
    };

    //==============================================================================
    /** Represents the possible menu positions. */
    enum class MenuBarPosition
    {
        window,
        global,
        burger
    };

    //==============================================================================
    MenusDemo()
    {
        menuBar.reset (new MenuBarComponent (this));
        addAndMakeVisible (menuBar.get());
        setApplicationCommandManagerToWatch (&commandManager);
        commandManager.registerAllCommandsForTarget (this);

        // this ensures that commands invoked on the DemoRunner application are correctly
        // forwarded to this demo
        commandManager.setFirstCommandTarget (this);

        // this lets the command manager use keypresses that arrive in our window to send out commands
        addKeyListener (commandManager.getKeyMappings());

        addChildComponent (menuHeader);
        addAndMakeVisible (outerCommandTarget);
        addAndMakeVisible (sidePanel);

        setWantsKeyboardFocus (true);

        setSize (500, 500);
    }

    ~MenusDemo() override
    {
       #if JUCE_MAC
        MenuBarModel::setMacMainMenu (nullptr);
       #endif

        commandManager.setFirstCommandTarget (nullptr);
    }

    void resized() override
    {
        auto b = getLocalBounds();

        if (menuBarPosition == MenuBarPosition::window)
        {
            menuBar->setBounds (b.removeFromTop (LookAndFeel::getDefaultLookAndFeel()
                                                             .getDefaultMenuBarHeight()));
        }
        else if (menuBarPosition == MenuBarPosition::burger)
        {
            menuHeader.setBounds (b.removeFromTop (40));
        }

        outerCommandTarget.setBounds (b);
    }

    //==============================================================================
    StringArray getMenuBarNames() override
    {
        return { "Menu Position", "Outer Colour", "Inner Colour" };
    }

    PopupMenu getMenuForIndex (int menuIndex, const String& /*menuName*/) override
    {
        PopupMenu menu;

        if (menuIndex == 0)
        {
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionInsideWindow);
           #if JUCE_MAC
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionGlobalMenuBar);
           #endif
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionBurgerMenu);
        }
        else if (menuIndex == 1)
        {
            menu.addCommandItem (&commandManager, CommandIDs::outerColourRed);
            menu.addCommandItem (&commandManager, CommandIDs::outerColourGreen);
            menu.addCommandItem (&commandManager, CommandIDs::outerColourBlue);
        }
        else if (menuIndex == 2)
        {
            menu.addCommandItem (&commandManager, CommandIDs::innerColourRed);
            menu.addCommandItem (&commandManager, CommandIDs::innerColourGreen);
            menu.addCommandItem (&commandManager, CommandIDs::innerColourBlue);
        }

        return menu;
    }

    void menuItemSelected (int /*menuItemID*/, int /*topLevelMenuIndex*/) override {}

    //==============================================================================
    // The following methods implement the ApplicationCommandTarget interface, allowing
    // this window to publish a set of actions it can perform, and which can be mapped
    // onto menus, keypresses, etc.

    ApplicationCommandTarget* getNextCommandTarget() override
    {
        return &outerCommandTarget;
    }

    void getAllCommands (Array<CommandID>& c) override
    {
        Array<CommandID> commands { CommandIDs::menuPositionInsideWindow,
                                    CommandIDs::menuPositionGlobalMenuBar,
                                    CommandIDs::menuPositionBurgerMenu };
        c.addArray (commands);
    }

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
    {
        switch (commandID)
        {
            case CommandIDs::menuPositionInsideWindow:
                result.setInfo ("Inside Window", "Places the menu bar inside the application window", "Menu", 0);
                result.setTicked (menuBarPosition == MenuBarPosition::window);
                result.addDefaultKeypress ('w', ModifierKeys::shiftModifier);
                break;
            case CommandIDs::menuPositionGlobalMenuBar:
                result.setInfo ("Global", "Uses a global menu bar", "Menu", 0);
                result.setTicked (menuBarPosition == MenuBarPosition::global);
                result.addDefaultKeypress ('g', ModifierKeys::shiftModifier);
                break;
            case CommandIDs::menuPositionBurgerMenu:
                result.setInfo ("Burger Menu", "Uses a burger menu", "Menu", 0);
                result.setTicked (menuBarPosition == MenuBarPosition::burger);
                result.addDefaultKeypress ('b', ModifierKeys::shiftModifier);
                break;
            default:
                break;
        }
    }

    bool perform (const InvocationInfo& info) override
    {
        switch (info.commandID)
        {
            case CommandIDs::menuPositionInsideWindow:
                setMenuBarPosition (MenuBarPosition::window);
                break;
            case CommandIDs::menuPositionGlobalMenuBar:
                setMenuBarPosition (MenuBarPosition::global);
                break;
            case CommandIDs::menuPositionBurgerMenu:
                setMenuBarPosition (MenuBarPosition::burger);
                break;
            default:
                return false;
        }

        return true;
    }

    void setMenuBarPosition (MenuBarPosition newPosition)
    {
        if (menuBarPosition != newPosition)
        {
            menuBarPosition = newPosition;

            if (menuBarPosition != MenuBarPosition::burger)
                sidePanel.showOrHide (false);

           #if JUCE_MAC
            MenuBarModel::setMacMainMenu (menuBarPosition == MenuBarPosition::global ? this : nullptr);
           #endif

            menuBar->setVisible   (menuBarPosition == MenuBarPosition::window);
            burgerMenu.setModel   (menuBarPosition == MenuBarPosition::burger ? this : nullptr);
            menuHeader.setVisible (menuBarPosition == MenuBarPosition::burger);

            sidePanel.setContent  (menuBarPosition == MenuBarPosition::burger ? &burgerMenu : nullptr, false);
            menuItemsChanged();

            resized();
        }
    }

private:
   #if JUCE_DEMO_RUNNER
    ApplicationCommandManager& commandManager = getGlobalCommandManager();
   #else
    ApplicationCommandManager commandManager;
   #endif

    std::unique_ptr<MenuBarComponent> menuBar;
    MenuBarPosition menuBarPosition = MenuBarPosition::window;

    SidePanel sidePanel { "Menu", 300, false };

    BurgerMenuComponent burgerMenu;
    BurgerMenuHeader menuHeader { sidePanel };

    //==============================================================================
    /**
        Command messages that aren't handled in the main component will be passed
        to this class to respond to.
    */
    class OuterCommandTarget    : public Component,
                                  public ApplicationCommandTarget
    {
    public:
        OuterCommandTarget (ApplicationCommandManager& m)
            : commandManager (m),
              innerCommandTarget (commandManager)
        {
            commandManager.registerAllCommandsForTarget (this);

            addAndMakeVisible (innerCommandTarget);
        }

        void resized() override
        {
            innerCommandTarget.setBounds (getLocalBounds().reduced (50));
        }

        void paint (Graphics& g) override
        {
            g.fillAll (currentColour);
        }

        //==============================================================================
        ApplicationCommandTarget* getNextCommandTarget() override
        {
            return &innerCommandTarget;
        }

        void getAllCommands (Array<CommandID>& c) override
        {
            Array<CommandID> commands { CommandIDs::outerColourRed,
                                        CommandIDs::outerColourGreen,
                                        CommandIDs::outerColourBlue };

            c.addArray (commands);
        }

        void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
        {
            switch (commandID)
            {
                case CommandIDs::outerColourRed:
                    result.setInfo ("Red", "Sets the outer colour to red", "Outer", 0);
                    result.setTicked (currentColour == Colours::red);
                    result.addDefaultKeypress ('r', ModifierKeys::commandModifier);
                    break;
                case CommandIDs::outerColourGreen:
                    result.setInfo ("Green", "Sets the outer colour to green", "Outer", 0);
                    result.setTicked (currentColour == Colours::green);
                    result.addDefaultKeypress ('g', ModifierKeys::commandModifier);
                    break;
                case CommandIDs::outerColourBlue:
                    result.setInfo ("Blue", "Sets the outer colour to blue", "Outer", 0);
                    result.setTicked (currentColour == Colours::blue);
                    result.addDefaultKeypress ('b', ModifierKeys::commandModifier);
                    break;
                default:
                    break;
            }
        }

        bool perform (const InvocationInfo& info) override
        {
            switch (info.commandID)
            {
                case CommandIDs::outerColourRed:
                    currentColour = Colours::red;
                    break;
                case CommandIDs::outerColourGreen:
                    currentColour = Colours::green;
                    break;
                case CommandIDs::outerColourBlue:
                    currentColour = Colours::blue;
                    break;
                default:
                    return false;
            }

            repaint();
            return true;
        }

    private:
        //==============================================================================
        /**
            Command messages that aren't handled in the OuterCommandTarget will be passed
            to this class to respond to.
        */
        struct InnerCommandTarget    : public Component,
                                       public ApplicationCommandTarget
        {
            InnerCommandTarget (ApplicationCommandManager& m)
                : commandManager (m)
            {
                commandManager.registerAllCommandsForTarget (this);
            }

            void paint (Graphics& g) override
            {
                g.fillAll (currentColour);
            }

            //==============================================================================
            ApplicationCommandTarget* getNextCommandTarget() override
            {
                // this will return the next parent component that is an ApplicationCommandTarget
                return findFirstTargetParentComponent();
            }

            void getAllCommands (Array<CommandID>& c) override
            {
                Array<CommandID> commands { CommandIDs::innerColourRed,
                                            CommandIDs::innerColourGreen,
                                            CommandIDs::innerColourBlue };

                c.addArray (commands);
            }

            void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
            {
                switch (commandID)
                {
                    case CommandIDs::innerColourRed:
                        result.setInfo ("Red", "Sets the inner colour to red", "Inner", 0);
                        result.setTicked (currentColour == Colours::red);
                        result.addDefaultKeypress ('r', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
                        break;
                    case CommandIDs::innerColourGreen:
                        result.setInfo ("Green", "Sets the inner colour to green", "Inner", 0);
                        result.setTicked (currentColour == Colours::green);
                        result.addDefaultKeypress ('g', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
                        break;
                    case CommandIDs::innerColourBlue:
                        result.setInfo ("Blue", "Sets the inner colour to blue", "Inner", 0);
                        result.setTicked (currentColour == Colours::blue);
                        result.addDefaultKeypress ('b', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
                        break;
                    default:
                        break;
                }
            }

            bool perform (const InvocationInfo& info) override
            {
                switch (info.commandID)
                {
                    case CommandIDs::innerColourRed:
                        currentColour = Colours::red;
                        break;
                    case CommandIDs::innerColourGreen:
                        currentColour = Colours::green;
                        break;
                    case CommandIDs::innerColourBlue:
                        currentColour = Colours::blue;
                        break;
                    default:
                        return false;
                }

                repaint();
                return true;
            }

            ApplicationCommandManager& commandManager;

            Colour currentColour { Colours::blue };
        };

        ApplicationCommandManager& commandManager;
        InnerCommandTarget innerCommandTarget;

        Colour currentColour { Colours::red };
    };

    OuterCommandTarget outerCommandTarget { commandManager };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenusDemo)
};
