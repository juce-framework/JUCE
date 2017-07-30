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

#include "../JuceDemoHeader.h"

// (These classes and random functions are used inside the 3rd-party Box2D demo code)
inline float32 RandomFloat()                        { return Random::getSystemRandom().nextFloat() * 2.0f - 1.0f; }
inline float32 RandomFloat (float32 lo, float32 hi) { return Random::getSystemRandom().nextFloat() * (hi - lo) + lo; }

struct Settings
{
    b2Vec2 viewCenter { 0.0f, 20.0f };
    float32 hz = 60.0f;
    int velocityIterations = 8;
    int positionIterations = 3;
    int drawShapes = 1;
    int drawJoints = 1;
    int drawAABBs = 0;
    int drawPairs = 0;
    int drawContactPoints = 0;
    int drawContactNormals = 0;
    int drawContactForces = 0;
    int drawFrictionForces = 0;
    int drawCOMs = 0;
    int drawStats = 0;
    int drawProfile = 0;
    int enableWarmStarting = 1;
    int enableContinuous = 1;
    int enableSubStepping = 0;
    int pause = 0;
    int singleStep = 0;
};

struct Test
{
    Test() : m_world (new b2World (b2Vec2 (0.0f, -10.0f))) {}
    virtual ~Test() {}

    virtual void Keyboard (unsigned char /*key*/) {}
    virtual void KeyboardUp (unsigned char /*key*/) {}

    ScopedPointer<b2World> m_world;
};

#include "Box2DTests/AddPair.h"
#include "Box2DTests/ApplyForce.h"
#include "Box2DTests/Dominos.h"
#include "Box2DTests/Chain.h"

//==============================================================================
/** This list box just displays a StringArray and broadcasts a change message when the
    selected row changes.
*/
class Box2DTestList : public ListBoxModel,
                      public ChangeBroadcaster
{
public:
    Box2DTestList (const StringArray& testList)   : tests (testList)
    {
    }

    int getNumRows() override           { return tests.size(); }

    void paintListBoxItem (int row, Graphics& g, int w, int h, bool rowIsSelected) override
    {
        auto& lf = LookAndFeel::getDefaultLookAndFeel();

        if (rowIsSelected)
            g.fillAll (Colour::contrasting (lf.findColour (ListBox::textColourId),
                                            lf.findColour (ListBox::backgroundColourId)));

        const Font f (h * 0.7f);
        g.setColour (lf.findColour (ListBox::textColourId));
        g.setFont (f);
        g.drawText (tests[row], Rectangle<int> (0, 0, w, h).reduced (2),
                    Justification::centredLeft, true);
    }

    void selectedRowsChanged (int /*lastRowSelected*/) override
    {
        sendChangeMessage();
    }

private:
    StringArray tests;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box2DTestList)
};

//==============================================================================
struct Box2DRenderComponent  : public Component
{
    Box2DRenderComponent()
    {
        setOpaque (true);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);

        if (currentTest != nullptr)
        {
            Box2DRenderer renderer;

            renderer.render (g, *currentTest->m_world,
                             -16.0f, 30.0f, 16.0f, -1.0f,
                             getLocalBounds().toFloat().reduced (8.0f));
        }
    }

    ScopedPointer<Test> currentTest;
};

//==============================================================================
class Box2DDemo : public Component,
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
        : testsList (getTestsList()),
          testsListModel (testsList)
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
    }

    ~Box2DDemo()
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
        if (renderComponent.currentTest != nullptr)
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
    Box2DTestList testsListModel;

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
            {
                String s;
                s << "Keys:" << newLine
                  << newLine
                  << "Left: \'a\'" << newLine
                  << "Right: \'d\'" << newLine
                  << "Forward: \'w\'";

                return s;
            }

            default:
                break;
        }

        return String();
    }

    void checkKeys()
    {
        if (renderComponent.currentTest == nullptr)
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
        if (renderComponent.currentTest == nullptr)
            return;

        grabKeyboardFocus();
        checkKeys();
        renderComponent.currentTest->m_world->Step (1.0f / 60.0f, 6, 2);
        repaint();
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &testsListModel)
        {
            const int index = testsListBox.getSelectedRow();

            renderComponent.currentTest = createTest (index);
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
        const char* tests[] =
        {
            "Add Pair Stress Test",
            "Apply Force",
            "Dominoes",
            "Chain"
        };

        jassert (numElementsInArray (tests) == numTests);

        return StringArray (tests, numElementsInArray (tests));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box2DDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<Box2DDemo> demo ("29 Graphics: Box 2D");
