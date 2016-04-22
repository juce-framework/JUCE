/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"

// (These classes and random functions are used inside the 3rd-party Box2D demo code)
inline float32 RandomFloat()                        { return Random::getSystemRandom().nextFloat() * 2.0f - 1.0f; }
inline float32 RandomFloat (float32 lo, float32 hi) { return Random::getSystemRandom().nextFloat() * (hi - lo) + lo; }

struct Settings
{
    Settings()
        : viewCenter (0.0f, 20.0f),
          hz (60.0f),
          velocityIterations (8),
          positionIterations (3),
          drawShapes (1),
          drawJoints (1),
          drawAABBs (0),
          drawPairs (0),
          drawContactPoints (0),
          drawContactNormals (0),
          drawContactForces (0),
          drawFrictionForces (0),
          drawCOMs (0),
          drawStats (0),
          drawProfile (0),
          enableWarmStarting (1),
          enableContinuous (1),
          enableSubStepping (0),
          pause (0),
          singleStep (0)
    {}

    b2Vec2 viewCenter;
    float32 hz;
    int velocityIterations;
    int positionIterations;
    int drawShapes;
    int drawJoints;
    int drawAABBs;
    int drawPairs;
    int drawContactPoints;
    int drawContactNormals;
    int drawContactForces;
    int drawFrictionForces;
    int drawCOMs;
    int drawStats;
    int drawProfile;
    int enableWarmStarting;
    int enableContinuous;
    int enableSubStepping;
    int pause;
    int singleStep;
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
        if (rowIsSelected)
            g.fillAll (LookAndFeel::getDefaultLookAndFeel().findColour (TextEditor::highlightColourId));

        const Font f (h * 0.7f);
        g.setColour (Colours::black);
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
        testsListBox.setColour (ListBox::backgroundColourId, Colours::lightgrey);

        addAndMakeVisible (instructions);
        instructions.setMultiLine (true);
        instructions.setReadOnly (true);
        instructions.setColour (TextEditor::backgroundColourId, Colours::lightgrey);

        startTimerHz (60);
    }

    ~Box2DDemo()
    {
        testsListModel.removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (4));

        Rectangle<int> area (r.removeFromBottom (150));
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
