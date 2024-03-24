/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             Box2DDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases 2D graphics features.

 dependencies:     juce_box2d, juce_core, juce_data_structures, juce_events,
                   juce_graphics, juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        Box2DDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
// (These classes and random functions are used inside the 3rd-party Box2D demo code)
inline float32 RandomFloat()                           { return Random::getSystemRandom().nextFloat() * 2.0f - 1.0f; }
inline float32 RandomFloat (float32 lo, float32 hi)    { return Random::getSystemRandom().nextFloat() * (hi - lo) + lo; }

struct Settings
{
    b2Vec2 viewCenter  { 0.0f, 20.0f };
    float32 hz = 60.0f;
    int velocityIterations = 8;
    int positionIterations = 3;
    int drawShapes         = 1;
    int drawJoints         = 1;
    int drawAABBs          = 0;
    int drawPairs          = 0;
    int drawContactPoints  = 0;
    int drawContactNormals = 0;
    int drawContactForces  = 0;
    int drawFrictionForces = 0;
    int drawCOMs           = 0;
    int drawStats          = 0;
    int drawProfile        = 0;
    int enableWarmStarting = 1;
    int enableContinuous   = 1;
    int enableSubStepping  = 0;
    int pause              = 0;
    int singleStep         = 0;
};

struct Test
{
    Test()          = default;
    virtual ~Test() = default;

    virtual void Keyboard (unsigned char /*key*/)   {}
    virtual void KeyboardUp (unsigned char /*key*/) {}

    std::unique_ptr<b2World> m_world  { new b2World (b2Vec2 (0.0f, -10.0f)) };
};

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wimplicit-int-float-conversion", "-Wsuggest-override")

#include "../Assets/Box2DTests/AddPair.h"
#include "../Assets/Box2DTests/ApplyForce.h"
#include "../Assets/Box2DTests/Dominos.h"
#include "../Assets/Box2DTests/Chain.h"

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
/** This list box just displays a StringArray and broadcasts a change message when the
    selected row changes.
*/
class Box2DTestList final : public ListBoxModel,
                            public ChangeBroadcaster
{
public:
    Box2DTestList (const StringArray& testList)
        : tests (testList)
    {}

    int getNumRows() override                                      { return tests.size(); }

    void selectedRowsChanged (int /*lastRowSelected*/) override    { sendChangeMessage(); }

    void paintListBoxItem (int row, Graphics& g, int w, int h, bool rowIsSelected) override
    {
        auto& lf = LookAndFeel::getDefaultLookAndFeel();

        if (rowIsSelected)
            g.fillAll (Colour::contrasting (lf.findColour (ListBox::textColourId),
                                            lf.findColour (ListBox::backgroundColourId)));

        g.setColour (lf.findColour (ListBox::textColourId));
        g.setFont ((float) h * 0.7f);
        g.drawText (tests[row], Rectangle<int> (0, 0, w, h).reduced (2),
                    Justification::centredLeft, true);
    }

private:
    StringArray tests;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box2DTestList)
};

//==============================================================================
struct Box2DRenderComponent final : public Component
{
    Box2DRenderComponent()
    {
        setOpaque (true);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);

        if (currentTest.get() != nullptr)
        {
            Box2DRenderer renderer;

            renderer.render (g, *currentTest->m_world,
                             -16.0f, 30.0f, 16.0f, -1.0f,
                             getLocalBounds().toFloat().reduced (8.0f));
        }
    }

    std::unique_ptr<Test> currentTest;
};

//==============================================================================
class Box2DDemo final : public Component,
                        private Timer,
                        private ChangeListener
{
public:
    enum Demos
    {
        addPair = 0,
        applyForce,
        dominoes,
        chain,
        numTests
    };

    Box2DDemo()
        : testsList (getTestsList())
    {
        setOpaque (true);
        setWantsKeyboardFocus (true);

        testsListModel.addChangeListener (this);

        addAndMakeVisible (renderComponent);

        addAndMakeVisible (testsListBox);
        testsListBox.setModel (&testsListModel);
        testsListBox.selectRow (dominoes);

        addAndMakeVisible (instructions);
        instructions.setMultiLine (true);
        instructions.setReadOnly (true);

        startTimerHz (60);

        setSize (500, 500);
    }

    ~Box2DDemo() override
    {
        testsListModel.removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (4);

        auto area = r.removeFromBottom (150);
        testsListBox.setBounds (area.removeFromLeft (150));

        area.removeFromLeft (4);
        instructions.setBounds (area);

        r.removeFromBottom (6);
        renderComponent.setBounds (r);
    }

    bool keyPressed (const KeyPress& key) override
    {
        if (renderComponent.currentTest.get() != nullptr)
        {
            // We override this to avoid the system beeping for an unused keypress
            switch (key.getTextCharacter())
            {
                case 'a':
                case 'w':
                case 'd':
                    return true;

                default:
                    break;
            }
        }

        return false;
    }

private:
    StringArray testsList;
    Box2DTestList testsListModel  { testsList };

    Box2DRenderComponent renderComponent;
    ListBox testsListBox;
    TextEditor instructions;

    static Test* createTest (int index)
    {
        switch (index)
        {
            case addPair:       return new AddPair();
            case applyForce:    return new ApplyForce();
            case dominoes:      return new Dominos();
            case chain:         return new Chain();
            default:            break;
        }

        return nullptr;
    }

    static String getInstructions (int index)
    {
        switch (index)
        {
            case applyForce:
                return String ("Keys:") + newLine + "Left: \'a\'" + newLine
                        + "Right: \'d\'" + newLine + "Forward: \'w\'";

            default:
                break;
        }

        return {};
    }

    void checkKeys()
    {
        if (renderComponent.currentTest.get() == nullptr)
            return;

        checkKeyCode ('a');
        checkKeyCode ('w');
        checkKeyCode ('d');
    }

    void checkKeyCode (const int keyCode)
    {
        if (KeyPress::isKeyCurrentlyDown (keyCode))
            renderComponent.currentTest->Keyboard ((unsigned char) keyCode);
    }

    void timerCallback() override
    {
        if (renderComponent.currentTest.get() == nullptr)
            return;

        if (isShowing())
            grabKeyboardFocus();

        checkKeys();
        renderComponent.currentTest->m_world->Step (1.0f / 60.0f, 6, 2);
        repaint();
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &testsListModel)
        {
            auto index = testsListBox.getSelectedRow();

            renderComponent.currentTest.reset (createTest (index));
            instructions.setText (getInstructions (index));

            repaint();
        }
    }

    void lookAndFeelChanged() override
    {
        instructions.applyFontToAllText (instructions.getFont());
    }

    static StringArray getTestsList()
    {
        return { "Add Pair Stress Test", "Apply Force", "Dominoes", "Chain" };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box2DDemo)
};
