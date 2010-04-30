/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_COMPONENTEDITORCODEVIEW_H_CA5124B0__
#define __JUCER_COMPONENTEDITORCODEVIEW_H_CA5124B0__


//==============================================================================
class ComponentEditor::CodeEditorHolder  : public Component,
                                           public ButtonListener
{
public:
    //==============================================================================
    CodeEditorHolder (ComponentEditor& editor_)
        : editor (editor_), switchFileButton (String::empty), showingHeader (true)
    {
        addAndMakeVisible (&viewport);
        viewport.setScrollBarsShown (true, false);

        addAndMakeVisible (&switchFileButton);

        buttonClicked (0);
        switchFileButton.addButtonListener (this);
    }

    ~CodeEditorHolder()
    {
    }

    void resized()
    {
        viewport.setBounds (getLocalBounds());

        int visWidth = viewport.getMaximumVisibleWidth();
        dynamic_cast <ContentHolder*> (viewport.getViewedComponent())->updateSize (visWidth);

        if (viewport.getMaximumVisibleWidth() != visWidth)
            dynamic_cast <ContentHolder*> (viewport.getViewedComponent())->updateSize (viewport.getMaximumVisibleWidth());

        switchFileButton.setBounds (getWidth() - 150, 4, 120, 20);
    }

    void buttonClicked (Button*)
    {
        showingHeader = ! showingHeader;
        viewport.setViewedComponent (new ContentHolder (editor.getDocument(), showingHeader));
        resized();
        switchFileButton.setButtonText (showingHeader ? "Show CPP file" : "Show header file");
    }

private:
    enum { updateCommandId = 0x23427fa1 };

    //==============================================================================
    class EditorHolder  : public Component,
                          public CodeDocument::Listener
    {
    public:
        EditorHolder (const CodeGenerator::CustomCodeList::CodeDocumentRef::Ptr doc,
                      const String& textBefore, const String& textAfter)
            : document (doc), cppTokeniser(), codeEditor (doc->getDocument(), &cppTokeniser)
        {
            linesBefore.addLines (textBefore);
            linesAfter.addLines (textAfter);

            addAndMakeVisible (&codeEditor);
            doc->getDocument().addListener (this);
        }

        ~EditorHolder()
        {
            document->getDocument().removeListener (this);
        }

        void paint (Graphics& g)
        {
            g.setFont (codeEditor.getFont());
            g.setColour (Colours::darkgrey);

            const int fontHeight = codeEditor.getLineHeight();
            const int fontAscent = (int) codeEditor.getFont().getAscent();
            const int textX = 5;

            int i;
            for (i = 0; i < linesBefore.size(); ++i)
                g.drawSingleLineText (linesBefore[i], textX, i * fontHeight + fontAscent);

            for (i = 0; i < linesAfter.size(); ++i)
                g.drawSingleLineText (linesAfter[i], textX, codeEditor.getBottom() + i * fontHeight + fontAscent);
        }

        void updateSize (int width)
        {
            const int fontHeight = codeEditor.getLineHeight();

            codeEditor.setBounds (0, fontHeight * linesBefore.size() + 1,
                                  width, 2 + codeEditor.getScrollbarThickness()
                                           + fontHeight * jlimit (1, 50, document->getDocument().getNumLines()));

            setSize (width, (linesBefore.size() + linesAfter.size()) * fontHeight + codeEditor.getHeight());
        }

        void codeDocumentChanged (const CodeDocument::Position&, const CodeDocument::Position&)
        {
            int oldHeight = getHeight();
            updateSize (getWidth());
            if (getHeight() != oldHeight)
                getParentComponent()->handleCommandMessage (updateCommandId);
        }

    private:
        const CodeGenerator::CustomCodeList::CodeDocumentRef::Ptr document;
        CPlusPlusCodeTokeniser cppTokeniser;
        CodeEditorComponent codeEditor;
        StringArray linesBefore, linesAfter;
    };

    //==============================================================================
    class ContentHolder  : public Component,
                           public ChangeListener
    {
    public:
        ContentHolder (ComponentDocument& document_, bool isHeader_)
            : document (document_), isHeader (isHeader_)
        {
            setOpaque (true);
            document.getCustomCodeList().addChangeListener (this);
            changeListenerCallback (0);
        }

        ~ContentHolder()
        {
            document.getCustomCodeList().removeChangeListener (this);
        }

        void paint (Graphics& g)
        {
            g.fillAll (Colours::lightgrey);
        }

        void updateSize (int width)
        {
            int y = 2;
            for (int i = 0; i < editors.size(); ++i)
            {
                EditorHolder* const ed = editors.getUnchecked(i);
                ed->updateSize (width - 8);
                ed->setTopLeftPosition (4, y + 1);
                y = ed->getBottom() + 1;
            }

            setSize (width, y + 2);
        }

        void changeListenerCallback (void*)
        {
            editors.clear();

            CodeGenerator::CustomCodeList::Iterator iter (isHeader ? document.getHeaderContent()
                                                                   : document.getCppContent(),
                                                          document.getCustomCodeList());

            while (iter.next())
            {
                EditorHolder* ed = new EditorHolder (iter.codeDocument, iter.textBefore, iter.textAfter);
                editors.add (ed);
                addAndMakeVisible (ed);
            }

            updateSize (getWidth());
        }

        void handleCommandMessage (int commandId)
        {
            if (commandId == updateCommandId)
                updateSize (getWidth());
            else
                Component::handleCommandMessage (commandId);
        }

    private:
        OwnedArray <EditorHolder> editors;
        ComponentDocument& document;
        bool isHeader;
    };

    //==============================================================================
    ComponentEditor& editor;
    Viewport viewport;
    TextButton switchFileButton;
    bool showingHeader;
};


#endif  // __JUCER_COMPONENTEDITORCODEVIEW_H_CA5124B0__
