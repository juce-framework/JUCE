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

class LiveBuildCodeEditorDocument;


class LiveBuildCodeEditor  : public CppCodeEditorComponent,
                             private Timer
{
public:
    LiveBuildCodeEditor (LiveBuildCodeEditorDocument& edDoc, CodeDocument& doc)
        : CppCodeEditorComponent (edDoc.getFile(), doc),
          editorDoc (edDoc),
          classList (*this, edDoc)
    {
    }

    ~LiveBuildCodeEditor()
    {
        for (int i = getNumChildComponents(); --i >= 0;)
            if (auto* c = dynamic_cast<DiagnosticOverlayComponent*> (getChildComponent (i)))
                delete c;
    }

    CompileEngineChildProcess::Ptr getChildProcess() const
    {
        return editorDoc.getChildProcess();
    }

    Component* addDiagnosticOverlay (CodeDocument::Position start, CodeDocument::Position end,
                                     DiagnosticMessage::Type diagType)
    {
        auto* d = new DiagnosticOverlayComponent (*this, start, end, diagType);
        addAndMakeVisible (d);
        return d;
    }

private:
    LiveBuildCodeEditorDocument& editorDoc;

    //==============================================================================
    struct OverlayComponent : public Component,
                              private GenericCodeEditorComponent::Listener,
                              private CodeDocument::Listener
    {
        OverlayComponent (CodeDocument::Position start,
                          CodeDocument::Position end)
            : startPosition (start),
              endPosition (end)
        {
            startPosition.setPositionMaintained (true);
            endPosition.setPositionMaintained (true);
        }

        ~OverlayComponent()
        {
            setEditor (nullptr);
        }

        void setEditor (GenericCodeEditorComponent* editor)
        {
            if (editor != codeEditor)
            {
                if (codeEditor != nullptr)
                {
                    codeEditor->removeListener (this);
                    codeEditor->getDocument().removeListener (this);
                    codeEditor->removeChildComponent (this);
                }

                codeEditor = editor;

                if (codeEditor != nullptr)
                {
                    codeEditor->addListener (this);
                    codeEditor->getDocument().addListener (this);
                    codeEditor->addAndMakeVisible (this);
                }

                if (editor != nullptr)
                    updatePosition();
            }
        }

        void codeEditorViewportMoved (CodeEditorComponent& editor) override
        {
            setEditor (dynamic_cast<GenericCodeEditorComponent*> (&editor));
            updatePosition();
        }

        void codeDocumentTextInserted (const String&, int) override          { updatePosition(); }
        void codeDocumentTextDeleted (int, int) override                     { updatePosition(); }

        void parentSizeChanged() override
        {
            updatePosition();
        }

        virtual void updatePosition() = 0;

        Component::SafePointer<GenericCodeEditorComponent> codeEditor;
        CodeDocument::Position startPosition, endPosition;
    };

    //==============================================================================
    struct LaunchClassOverlayComponent : public OverlayComponent
    {
        LaunchClassOverlayComponent (GenericCodeEditorComponent& editor,
                                     CodeDocument::Position start, CodeDocument::Position end,
                                     const String className)
            : OverlayComponent (start, end),
              launchButton (className.fromLastOccurrenceOf ("::", false, false)),
              name (className)
        {
            setAlwaysOnTop (true);
            setEditor (&editor);
            addAndMakeVisible (launchButton);
        }

        void updatePosition() override
        {
            if (codeEditor != nullptr)
            {
                jassert (isVisible());

                const auto charArea = codeEditor->getCharacterBounds (startPosition);
                const int height = charArea.getHeight() + 8;

                Font f (height * 0.7f);

                const int width = jmin (height * 2 + f.getStringWidth (launchButton.getName()),
                                        jmax (120, codeEditor->proportionOfWidth (0.2f)));

                setBounds (codeEditor->getWidth() - width - 10, charArea.getY() - 4,
                           width, height);
            }
        }

        void resized() override
        {
            launchButton.setBounds (getLocalBounds());
        }

        struct LaunchButton  : public Button
        {
            LaunchButton (const String& nm)  : Button (nm)
            {
                setMouseCursor (MouseCursor::PointingHandCursor);
            }

            void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
            {
                Colour background (findColour (CodeEditorComponent::backgroundColourId)
                                     .contrasting()
                                     .overlaidWith (Colours::yellow.withAlpha (0.5f))
                                     .withAlpha (0.4f));
                g.setColour (background);
                g.fillRoundedRectangle (getLocalBounds().toFloat(), 3.0f);

                const Path& path = getIcons().play;

                Colour col (background.contrasting (Colours::lightgreen, 0.6f));

                Rectangle<int> r (getLocalBounds().reduced (getHeight() / 5));

                Icon (path, col.withAlpha (isButtonDown ? 1.0f : (isMouseOverButton ? 0.8f : 0.5f)))
                   .draw (g, r.removeFromLeft (getHeight()).toFloat(), false);

                g.setColour (Colours::white);
                g.setFont (getHeight() * 0.7f);
                g.drawFittedText (getName(), r, Justification::centredLeft, 1);
            }

            void clicked() override
            {
                if (auto* l = findParentComponentOfClass<LaunchClassOverlayComponent>())
                    l->launch();
            }
        };

        void launch()
        {
            if (auto* e = findParentComponentOfClass<LiveBuildCodeEditor>())
                e->launch (name);
        }

    private:
        LaunchButton launchButton;
        String name;
    };

    struct ComponentClassList  : private Timer
    {
        ComponentClassList (GenericCodeEditorComponent& e, LiveBuildCodeEditorDocument& edDoc)
            : owner (e),
              childProcess (edDoc.getChildProcess()),
              file (edDoc.getFile())
        {
            startTimer (600);
        }

        ~ComponentClassList()
        {
            deleteOverlays();
        }

        void timerCallback() override
        {
            Array<ClassDatabase::Class*> newClasses;

            if (childProcess != nullptr)
                childProcess->getComponentList().globalNamespace.findClassesDeclaredInFile (newClasses, file);

            for (int i = newClasses.size(); --i >= 0;)
                if (! newClasses.getUnchecked(i)->getInstantiationFlags().canBeInstantiated())
                    newClasses.remove (i);

            if (newClasses != classes)
            {
                classes = newClasses;
                deleteOverlays();

                for (auto& c : classes)
                {
                    CodeDocument::Position pos (owner.getDocument(), c->getClassDeclarationRange().range.getStart());
                    overlays.add (new LaunchClassOverlayComponent (owner, pos, pos, c->getName()));
                }
            }
        }

        void deleteOverlays()
        {
            for (auto& o : overlays)
                o.deleteAndZero();

            overlays.clear();
        }

        GenericCodeEditorComponent& owner;
        CompileEngineChildProcess::Ptr childProcess;
        File file;
        Array<ClassDatabase::Class*> classes;
        Array<Component::SafePointer<Component>> overlays;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentClassList)
    };

    ComponentClassList classList;

    //==============================================================================
    struct DiagnosticOverlayComponent : public OverlayComponent
    {
        DiagnosticOverlayComponent (GenericCodeEditorComponent& editor,
                                    CodeDocument::Position start, CodeDocument::Position end,
                                    DiagnosticMessage::Type diagType)
            : OverlayComponent (start, end), diagnosticType (diagType)
        {
            setInterceptsMouseClicks (false, false);
            setEditor (&editor);
        }

        void updatePosition() override
        {
            if (codeEditor != nullptr)
            {
                jassert (isVisible());

                const auto charStartRect = codeEditor->getCharacterBounds (startPosition);
                const auto charEndRect   = codeEditor->getCharacterBounds (endPosition);

                auto charHeight = charStartRect.getHeight();
                const auto editorBounds = codeEditor->getBounds();

                arrowXMin = static_cast<int> (jmin (charStartRect.getX(), charEndRect.getX()));
                arrowXMax = static_cast<int> (jmax (charStartRect.getX() + charStartRect.getWidth(),  charEndRect.getX() + charEndRect.getWidth()));

                lineYMin = charStartRect.getY();
                lineOffset = charHeight;

                setBounds (0, lineYMin, editorBounds.getWidth(), lineOffset + charHeight);
                repaint();
            }
        }

        void paint (Graphics& g) override
        {
            const auto diagColour = diagnosticType == DiagnosticMessage::Type::error ? Colours::red
                                                                                     : Colour (200, 200, 64);

            g.setColour (diagColour.withAlpha (0.2f));
            g.fillRect (getLocalBounds().withTrimmedBottom (lineOffset));

            Path path;
            const float bottomY = getHeight() - (lineOffset / 2.0f);
            path.addTriangle ((float) arrowXMin, bottomY,
                              (arrowXMax + arrowXMin) / 2.0f, (float) lineOffset,
                              (float) arrowXMax, bottomY);

            g.setColour (diagColour.withAlpha (0.8f));
            g.fillPath (path);
        }

    private:
        int arrowXMin, arrowXMax;
        int lineYMin, lineOffset;
        const DiagnosticMessage::Type diagnosticType;
    };

    //==============================================================================
    void timerCallback() override
    {
        if (isMouseButtonDownAnywhere())
            return;

        MouseInputSource mouse = Desktop::getInstance().getMainMouseSource();
        Component* underMouse = mouse.getComponentUnderMouse();

        if (underMouse != nullptr
             && (dynamic_cast<ControlsComponent*> (underMouse) != nullptr
                   || underMouse->findParentComponentOfClass<ControlsComponent>() != nullptr))
            return;

        overlay = nullptr;

        if (hasKeyboardFocus (true) && underMouse != nullptr
              && (underMouse == this || underMouse->isParentOf (this)))
        {
            Point<int> mousePos = getLocalPoint (nullptr, mouse.getScreenPosition()).toInt();

            CodeDocument::Position start, end;
            getDocument().findTokenContaining (getPositionAt (mousePos.x, mousePos.y), start, end);

            if (end.getPosition() > start.getPosition())
            {
                Range<int> selection = optimiseSelection ({ start.getPosition(), end.getPosition() });

                String text = getTextInRange (selection).toLowerCase();

                if (isIntegerLiteral (text) || isFloatLiteral (text))
                    overlay = new LiteralHighlightOverlay (*this, selection, mightBeColourValue (text));
            }
        }

        startTimerHz (10);
    }

    void hideOverlay()
    {
        stopTimer();
        overlay = nullptr;
    }

    void focusLost (FocusChangeType) override
    {
        if (CompileEngineChildProcess::Ptr childProcess = getChildProcess())
            childProcess->flushEditorChanges();
    }

    void mouseMove (const MouseEvent& e) override
    {
        if (overlay == nullptr)
            startTimer (100);

        CppCodeEditorComponent::mouseMove (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (e.getDistanceFromDragStart() > 0)
            hideOverlay();

        CppCodeEditorComponent::mouseDrag (e);
    }

    void mouseDown (const MouseEvent& e) override
    {
        CppCodeEditorComponent::mouseDown (e);
    }

    void mouseUp (const MouseEvent& e) override
    {
        CppCodeEditorComponent::mouseUp (e);
    }

    bool keyPressed (const KeyPress& key) override
    {
        hideOverlay();
        return CppCodeEditorComponent::keyPressed (key);
    }

    static bool isIntegerLiteral (const String& text)   { return CppParserHelpers::parseSingleToken (text) == CPlusPlusCodeTokeniser::tokenType_integer; }
    static bool isFloatLiteral (const String& text)     { return CppParserHelpers::parseSingleToken (text) == CPlusPlusCodeTokeniser::tokenType_float; }

    static bool mightBeColourValue (const String& text)
    {
        return isIntegerLiteral (text)
                 && text.trim().startsWith ("0x")
                 && text.trim().length() > 7;
    }

    Range<int> optimiseSelection (Range<int> selection)
    {
        String text (getTextInRange (selection));

        if (CharacterFunctions::isDigit (text[0]) || text[0] == '.')
            if (getTextInRange (Range<int> (selection.getStart() - 1, selection.getStart())) == "-")
                selection.setStart (selection.getStart() - 1);

        selection.setStart (selection.getStart() + (text.length() - text.trimStart().length()));
        selection.setEnd (selection.getEnd() - (text.length() - text.trimEnd().length()));

        return selection;
    }

    void launch (const String& name)
    {
        if (CompileEngineChildProcess::Ptr p = getChildProcess())
            if (auto* cls = p->getComponentList().globalNamespace.findClass (name))
                p->openPreview (*cls);
    }

    //==============================================================================
    class ControlsComponent   : public Component,
                                private Slider::Listener,
                                private ChangeListener
    {
    public:
        ControlsComponent (CodeDocument& doc, const Range<int>& selection,
                           CompileEngineChildProcess* cp, bool showColourSelector)
            : document (doc),
              start (doc, selection.getStart()),
              end (doc, selection.getEnd()),
              childProcess (cp)
        {
            slider.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
            slider.setWantsKeyboardFocus (false);
            slider.setMouseClickGrabsKeyboardFocus (false);
            setWantsKeyboardFocus (false);
            setMouseClickGrabsKeyboardFocus (false);
            addAndMakeVisible (&slider);
            updateRange();
            slider.addListener (this);

            if (showColourSelector)
            {
                updateColourSelector();
                selector.setWantsKeyboardFocus (false);
                selector.setMouseClickGrabsKeyboardFocus (false);
                addAndMakeVisible (&selector);
                setSize (400, sliderHeight + 400);
                selector.addChangeListener (this);
            }
            else
            {
                setSize (400, sliderHeight);
            }

            end.setPositionMaintained (true);
        }

        void updateRange()
        {
            double v = getValue();

            if (isFloat())
                slider.setRange (v - 10, v + 10);
            else
                slider.setRange (v - 100, v + 100);

            slider.setValue (v, dontSendNotification);
        }

    private:
        Slider slider;
        ColourSelector selector;

        CodeDocument& document;
        CodeDocument::Position start, end;
        CompileEngineChildProcess::Ptr childProcess;

        static const int sliderHeight = 26;

        void paint (Graphics& g) override
        {
            g.setColour (LiteralHighlightOverlay::getBackgroundColour());
            g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);
        }

        void sliderValueChanged (Slider* s) override
        {
            const String oldText (document.getTextBetween (start, end));
            const String newText (CppParserHelpers::getReplacementStringInSameFormat (oldText, s->getValue()));

            if (oldText != newText)
                document.replaceSection (start.getPosition(), end.getPosition(), newText);

            if (childProcess != nullptr)
                childProcess->flushEditorChanges();

            updateColourSelector();
        }

        void sliderDragStarted (Slider*) override  {}
        void sliderDragEnded (Slider*) override    { updateRange(); }

        void changeListenerCallback (ChangeBroadcaster*) override
        {
            setNewColour (selector.getCurrentColour());
        }

        void updateColourSelector()
        {
            selector.setCurrentColour (getCurrentColour());
        }

        Colour getCurrentColour() const
        {
            int64 val;
            if (CppParserHelpers::parseInt (document.getTextBetween (start, end), val))
                return Colour ((uint32) val);

            return Colours::white;
        }

        void setNewColour (const Colour& c)
        {
            const String oldText (document.getTextBetween (start, end));
            const String newText (CppParserHelpers::getReplacementStringInSameFormat (oldText, (int64) c.getARGB()));

            if (oldText != newText)
                document.replaceSection (start.getPosition(), end.getPosition(), newText);

            if (childProcess != nullptr)
                childProcess->flushEditorChanges();
        }

        void resized() override
        {
            Rectangle<int> r (getLocalBounds());

            slider.setBounds (r.removeFromTop (sliderHeight));

            r.removeFromTop (10);

            if (selector.isVisible())
                selector.setBounds (r);

        }

        double getValue() const
        {
            const String text (document.getTextBetween (start, end));

            if (text.containsChar ('.'))
            {
                double f;
                if (CppParserHelpers::parseFloat (text, f))
                    return f;
            }
            else
            {
                int64 val;
                if (CppParserHelpers::parseInt (text, val))
                    return (double) val;
            }

            jassertfalse;
            return 0;
        }

        bool isFloat() const
        {
            return document.getTextBetween (start, end).containsChar ('.');
        }
    };

    //==============================================================================
    struct LiteralHighlightOverlay  : public Component,
                                      private CodeDocument::Listener
    {
        LiteralHighlightOverlay (LiveBuildCodeEditor& e, Range<int> section, bool showColourSelector)
            : owner (e),
              start (e.getDocument(), section.getStart()),
              end   (e.getDocument(), section.getEnd()),
              controls (e.getDocument(), section, e.getChildProcess(), showColourSelector)
        {
            if (e.hasKeyboardFocus (true))
                previouslyFocused = Component::getCurrentlyFocusedComponent();

            start.setPositionMaintained (true);
            end.setPositionMaintained (true);

            setInterceptsMouseClicks (false, false);

            if (Component* parent = owner.findParentComponentOfClass<ProjectContentComponent>())
                parent->addAndMakeVisible (controls);
            else
                jassertfalse;

            owner.addAndMakeVisible (this);
            toBack();

            updatePosition();

            owner.getDocument().addListener (this);
        }

        ~LiteralHighlightOverlay()
        {
            if (Component* p = getParentComponent())
            {
                p->removeChildComponent (this);

                if (previouslyFocused != nullptr && ! previouslyFocused->hasKeyboardFocus (true))
                    previouslyFocused->grabKeyboardFocus();
            }

            owner.getDocument().removeListener (this);
        }

        void paint (Graphics& g) override
        {
            g.setColour (getBackgroundColour());

            Rectangle<int> r (getLocalBounds());
            g.fillRect (r.removeFromTop (borderSize));
            g.fillRect (r.removeFromLeft (borderSize));
            g.fillRect (r.removeFromRight (borderSize));
        }

        void updatePosition()
        {
            Rectangle<int> area = owner.getCharacterBounds (start)
                                       .getUnion (owner.getCharacterBounds (end.movedBy (-1)))
                                       .expanded (borderSize)
                                       .withTrimmedBottom (borderSize);

            setBounds (getParentComponent()->getLocalArea (&owner, area));

            area.setPosition (area.getX() - controls.getWidth() / 2, area.getBottom());
            area.setSize (controls.getWidth(), controls.getHeight());

            controls.setBounds (controls.getParentComponent()->getLocalArea (&owner, area));
        }

        void codeDocumentTextInserted (const String&, int) override         { updatePosition(); }
        void codeDocumentTextDeleted (int, int) override                    { updatePosition(); }

        LiveBuildCodeEditor& owner;
        CodeDocument::Position start, end;
        ControlsComponent controls;
        Component::SafePointer<Component> previouslyFocused;

        static const int borderSize = 4;
        static Colour getBackgroundColour() { return Colour (0xcb5c7879); }
    };

    ScopedPointer<LiteralHighlightOverlay> overlay;
};

//==============================================================================
class LiveBuildCodeEditorDocument  : public SourceCodeDocument
{
public:
    LiveBuildCodeEditorDocument (Project* project, const File& file)
        : SourceCodeDocument (project, file)
    {
        if (project != nullptr)
            if (CompileEngineChildProcess::Ptr childProcess = getChildProcess())
                childProcess->editorOpened (file, getCodeDocument());
    }

    struct Type  : public SourceCodeDocument::Type
    {
        Document* openFile (Project* proj, const File& file) override
        {
            return new LiveBuildCodeEditorDocument (proj, file);
        }
    };

    Component* createEditor() override
    {
        SourceCodeEditor* e = nullptr;

        if (fileNeedsCppSyntaxHighlighting (getFile()))
            e = new SourceCodeEditor (this, new LiveBuildCodeEditor (*this, getCodeDocument()));
        else
            e = new SourceCodeEditor (this, getCodeDocument());

        applyLastState (*(e->editor));
        return e;
    }

    // override save() to make a few more attempts at saving if it fails, since on Windows
    // the compiler can interfere with things saving..
    bool save() override
    {
        for (int i = 5; --i >= 0;)
        {
            if (SourceCodeDocument::save()) // should already re-try for up to half a second
                return true;

            Thread::sleep (100);
        }

        return false;
    }

    CompileEngineChildProcess::Ptr getChildProcess() const
    {
        return ProjucerApplication::getApp().childProcessCache->getExisting (*project);
    }
};
