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

namespace juce
{

#if JUCE_ENABLE_LIVE_CONSTANT_EDITOR

namespace LiveConstantEditor
{

//==============================================================================
class AllComponentRepainter  : private Timer,
                               private DeletedAtShutdown
{
public:
    AllComponentRepainter()  {}
    ~AllComponentRepainter() override  { clearSingletonInstance(); }

    JUCE_DECLARE_SINGLETON (AllComponentRepainter, false)

    void trigger()
    {
        if (! isTimerRunning())
            startTimer (100);
    }

private:
    void timerCallback() override
    {
        stopTimer();

        Array<Component*> alreadyDone;

        for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
            if (auto* c = TopLevelWindow::getTopLevelWindow(i))
                repaintAndResizeAllComps (c, alreadyDone);

        auto& desktop = Desktop::getInstance();

        for (int i = desktop.getNumComponents(); --i >= 0;)
            if (auto* c = desktop.getComponent(i))
                repaintAndResizeAllComps (c, alreadyDone);
    }

    static void repaintAndResizeAllComps (Component::SafePointer<Component> c,
                                          Array<Component*>& alreadyDone)
    {
        if (c->isVisible() && ! alreadyDone.contains (c))
        {
            c->repaint();
            c->resized();

            for (int i = c->getNumChildComponents(); --i >= 0;)
            {
                if (auto* child = c->getChildComponent(i))
                {
                    repaintAndResizeAllComps (child, alreadyDone);
                    alreadyDone.add (child);
                }

                if (c == nullptr)
                    break;
            }
        }
    }
};

JUCE_IMPLEMENT_SINGLETON (AllComponentRepainter)
JUCE_IMPLEMENT_SINGLETON (ValueList)

//==============================================================================
int64 parseInt (String s)
{
    s = s.trimStart();

    if (s.startsWithChar ('-'))
        return -parseInt (s.substring (1));

    if (s.startsWith ("0x"))
        return s.substring(2).getHexValue64();

    return s.getLargeIntValue();
}

double parseDouble (const String& s)
{
    return s.retainCharacters ("0123456789.eE-").getDoubleValue();
}

String intToString (int   v, bool preferHex)    { return preferHex ? "0x" + String::toHexString (v) : String (v); }
String intToString (int64 v, bool preferHex)    { return preferHex ? "0x" + String::toHexString (v) : String (v); }

//==============================================================================
LiveValueBase::LiveValueBase (const char* file, int line)
    : sourceFile (file), sourceLine (line)
{
    name = File (sourceFile).getFileName() + " : " + String (sourceLine);
}

LiveValueBase::~LiveValueBase()
{
}

//==============================================================================
LivePropertyEditorBase::LivePropertyEditorBase (LiveValueBase& v, CodeDocument& d)
    : value (v), document (d), sourceEditor (document, &tokeniser)
{
    setSize (600, 100);

    addAndMakeVisible (name);
    addAndMakeVisible (resetButton);
    addAndMakeVisible (valueEditor);
    addAndMakeVisible (sourceEditor);

    findOriginalValueInCode();
    selectOriginalValue();

    name.setFont (13.0f);
    name.setText (v.name, dontSendNotification);
    valueEditor.setMultiLine (v.isString());
    valueEditor.setReturnKeyStartsNewLine (v.isString());
    valueEditor.setText (v.getStringValue (wasHex), dontSendNotification);
    valueEditor.onTextChange = [this] { applyNewValue (valueEditor.getText()); };
    sourceEditor.setReadOnly (true);
    sourceEditor.setFont (sourceEditor.getFont().withHeight (13.0f));
    resetButton.onClick = [this] { applyNewValue (value.getOriginalStringValue (wasHex)); };
}

void LivePropertyEditorBase::paint (Graphics& g)
{
    g.setColour (Colours::white);
    g.fillRect (getLocalBounds().removeFromBottom (1));
}

void LivePropertyEditorBase::resized()
{
    auto r = getLocalBounds().reduced (0, 3).withTrimmedBottom (1);

    auto left = r.removeFromLeft (jmax (200, r.getWidth() / 3));

    auto top = left.removeFromTop (25);
    resetButton.setBounds (top.removeFromRight (35).reduced (0, 3));
    name.setBounds (top);

    if (customComp != nullptr)
    {
        valueEditor.setBounds (left.removeFromTop (25));
        left.removeFromTop (2);
        customComp->setBounds (left);
    }
    else
    {
        valueEditor.setBounds (left);
    }

    r.removeFromLeft (4);
    sourceEditor.setBounds (r);
}

void LivePropertyEditorBase::applyNewValue (const String& s)
{
    value.setStringValue (s);

    document.replaceSection (valueStart.getPosition(), valueEnd.getPosition(), value.getCodeValue (wasHex));
    document.clearUndoHistory();
    selectOriginalValue();

    valueEditor.setText (s, dontSendNotification);
    AllComponentRepainter::getInstance()->trigger();
}

void LivePropertyEditorBase::selectOriginalValue()
{
    sourceEditor.selectRegion (valueStart, valueEnd);
}

void LivePropertyEditorBase::findOriginalValueInCode()
{
    CodeDocument::Position pos (document, value.sourceLine, 0);
    auto line = pos.getLineText();
    auto p = line.getCharPointer();

    p = CharacterFunctions::find (p, CharPointer_ASCII ("JUCE_LIVE_CONSTANT"));

    if (p.isEmpty())
    {
        // Not sure how this would happen - some kind of mix-up between source code and line numbers..
        jassertfalse;
        return;
    }

    p += (int) (sizeof ("JUCE_LIVE_CONSTANT") - 1);
    p = p.findEndOfWhitespace();

    if (! CharacterFunctions::find (p, CharPointer_ASCII ("JUCE_LIVE_CONSTANT")).isEmpty())
    {
        // Aargh! You've added two JUCE_LIVE_CONSTANT macros on the same line!
        // They're identified by their line number, so you must make sure each
        // one goes on a separate line!
        jassertfalse;
    }

    if (p.getAndAdvance() == '(')
    {
        auto start = p, end = p;

        int depth = 1;

        while (! end.isEmpty())
        {
            auto c = end.getAndAdvance();

            if (c == '(')  ++depth;
            if (c == ')')  --depth;

            if (depth == 0)
            {
                --end;
                break;
            }
        }

        if (end > start)
        {
            valueStart = CodeDocument::Position (document, value.sourceLine, (int) (start - line.getCharPointer()));
            valueEnd   = CodeDocument::Position (document, value.sourceLine, (int) (end   - line.getCharPointer()));

            valueStart.setPositionMaintained (true);
            valueEnd.setPositionMaintained (true);

            wasHex = String (start, end).containsIgnoreCase ("0x");
        }
    }
}

//==============================================================================
class ValueListHolderComponent  : public Component
{
public:
    ValueListHolderComponent (ValueList& l) : valueList (l)
    {
        setVisible (true);
    }

    void addItem (int width, LiveValueBase& v, CodeDocument& doc)
    {
        addAndMakeVisible (editors.add (v.createPropertyComponent (doc)));
        layout (width);
    }

    void layout (int width)
    {
        setSize (width, editors.size() * itemHeight);
        resized();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (2, 0);

        for (int i = 0; i < editors.size(); ++i)
            editors.getUnchecked(i)->setBounds (r.removeFromTop (itemHeight));
    }

    enum { itemHeight = 120 };

    ValueList& valueList;
    OwnedArray<LivePropertyEditorBase> editors;
};

//==============================================================================
class ValueList::EditorWindow  : public DocumentWindow,
                                 private DeletedAtShutdown
{
public:
    EditorWindow (ValueList& list)
        : DocumentWindow ("Live Values", Colours::lightgrey, DocumentWindow::closeButton)
    {
        setLookAndFeel (&lookAndFeel);
        setUsingNativeTitleBar (true);

        viewport.setViewedComponent (new ValueListHolderComponent (list), true);
        viewport.setSize (700, 600);
        viewport.setScrollBarsShown (true, false);

        setContentNonOwned (&viewport, true);
        setResizable (true, false);
        setResizeLimits (500, 400, 10000, 10000);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    ~EditorWindow() override
    {
        setLookAndFeel (nullptr);
    }

    void closeButtonPressed() override
    {
        setVisible (false);
    }

    void updateItems (ValueList& list)
    {
        if (auto* l = dynamic_cast<ValueListHolderComponent*> (viewport.getViewedComponent()))
        {
            while (l->getNumChildComponents() < list.values.size())
            {
                if (auto* v = list.values [l->getNumChildComponents()])
                    l->addItem (viewport.getMaximumVisibleWidth(), *v, list.getDocument (v->sourceFile));
                else
                    break;
            }

            setVisible (true);
        }
    }

    void resized() override
    {
        DocumentWindow::resized();

        if (auto* l = dynamic_cast<ValueListHolderComponent*> (viewport.getViewedComponent()))
            l->layout (viewport.getMaximumVisibleWidth());
    }

    Viewport viewport;
    LookAndFeel_V3 lookAndFeel;
};

//==============================================================================
ValueList::ValueList()  {}
ValueList::~ValueList() { clearSingletonInstance(); }

void ValueList::addValue (LiveValueBase* v)
{
    values.add (v);
    triggerAsyncUpdate();
}

void ValueList::handleAsyncUpdate()
{
    if (editorWindow == nullptr)
        editorWindow = new EditorWindow (*this);

    editorWindow->updateItems (*this);
}

CodeDocument& ValueList::getDocument (const File& file)
{
    const int index = documentFiles.indexOf (file.getFullPathName());

    if (index >= 0)
        return *documents.getUnchecked (index);

    auto* doc = documents.add (new CodeDocument());
    documentFiles.add (file);
    doc->replaceAllContent (file.loadFileAsString());
    doc->clearUndoHistory();
    return *doc;
}

//==============================================================================
struct ColourEditorComp  : public Component,
                           private ChangeListener
{
    ColourEditorComp (LivePropertyEditorBase& e)  : editor (e)
    {
        setMouseCursor (MouseCursor::PointingHandCursor);
    }

    Colour getColour() const
    {
        return Colour ((uint32) parseInt (editor.value.getStringValue (false)));
    }

    void paint (Graphics& g) override
    {
        g.fillCheckerBoard (getLocalBounds().toFloat(), 6.0f, 6.0f,
                            Colour (0xffdddddd).overlaidWith (getColour()),
                            Colour (0xffffffff).overlaidWith (getColour()));
    }

    void mouseDown (const MouseEvent&) override
    {
        auto* colourSelector = new ColourSelector();
        colourSelector->setName ("Colour");
        colourSelector->setCurrentColour (getColour());
        colourSelector->addChangeListener (this);
        colourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
        colourSelector->setSize (300, 400);

        CallOutBox::launchAsynchronously (colourSelector, getScreenBounds(), nullptr);
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (auto* cs = dynamic_cast<ColourSelector*> (source))
            editor.applyNewValue (getAsString (cs->getCurrentColour(), true));

        repaint();
    }

    LivePropertyEditorBase& editor;
};

Component* createColourEditor (LivePropertyEditorBase& editor)
{
    return new ColourEditorComp (editor);
}

//==============================================================================
struct SliderComp   : public Component
{
    SliderComp (LivePropertyEditorBase& e, bool useFloat)
        : editor (e), isFloat (useFloat)
    {
        slider.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
        addAndMakeVisible (slider);
        updateRange();
        slider.onDragEnd = [this] { updateRange(); };
        slider.onValueChange = [this]
        {
            editor.applyNewValue (isFloat ? getAsString ((double) slider.getValue(), editor.wasHex)
                                          : getAsString ((int64)  slider.getValue(), editor.wasHex));
        };
    }

    virtual void updateRange()
    {
        double v = isFloat ? parseDouble (editor.value.getStringValue (false))
                           : (double) parseInt (editor.value.getStringValue (false));

        double range = isFloat ? 10 : 100;

        slider.setRange (v - range, v + range);
        slider.setValue (v, dontSendNotification);
    }

    void resized() override
    {
        slider.setBounds (getLocalBounds().removeFromTop (25));
    }

    LivePropertyEditorBase& editor;
    Slider slider;
    bool isFloat;
};

//==============================================================================
struct BoolSliderComp  : public SliderComp
{
    BoolSliderComp (LivePropertyEditorBase& e)
        : SliderComp (e, false)
    {
        slider.onValueChange = [this] { editor.applyNewValue (slider.getValue() > 0.5 ? "true" : "false"); };
    }

    void updateRange() override
    {
        slider.setRange (0.0, 1.0, dontSendNotification);
        slider.setValue (editor.value.getStringValue (false) == "true", dontSendNotification);
    }
};

Component* createIntegerSlider (LivePropertyEditorBase& editor)  { return new SliderComp (editor, false); }
Component* createFloatSlider   (LivePropertyEditorBase& editor)  { return new SliderComp (editor, true);  }
Component* createBoolSlider    (LivePropertyEditorBase& editor)  { return new BoolSliderComp (editor); }

}

#endif

} // namespace juce
