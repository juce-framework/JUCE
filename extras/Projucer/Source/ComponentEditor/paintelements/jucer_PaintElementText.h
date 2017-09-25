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

#include "jucer_ColouredElement.h"
#include "../properties/jucer_FontPropertyComponent.h"
#include "../properties/jucer_JustificationProperty.h"


//==============================================================================
class PaintElementText   : public ColouredElement
{
public:
    PaintElementText (PaintRoutine* pr)
        : ColouredElement (pr, "Text", false, false),
          text ("Your text goes here"),
          font (15.0f),
          typefaceName (FontPropertyComponent::getDefaultFont()),
          justification (Justification::centred)
    {
        fillType.colour = Colours::black;
        position.rect.setWidth (200);
        position.rect.setHeight (30);
    }

    //==============================================================================
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea) override
    {
        fillType.setFillType (g, getDocument(), parentArea);

        font = FontPropertyComponent::applyNameToFont (typefaceName, font);
        g.setFont (font);

        g.drawText (replaceStringTranslations (text, owner->getDocument()),
                    position.getRectangle (parentArea, layout), justification, true);
    }

    static String replaceStringTranslations (String s, JucerDocument* document)
    {
        s = s.replace ("%%getName()%%", document->getComponentName());
        s = s.replace ("%%getButtonText()%%", document->getComponentName());
        return s;
    }

    void getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ColouredElement::getEditableProperties (props, multipleSelected);

        if (multipleSelected)
            return;
        
        props.add (new TextProperty (this));
        props.add (new FontNameProperty (this));
        props.add (new FontStyleProperty (this));
        props.add (new FontSizeProperty (this));
        props.add (new FontKerningProperty (this));
        props.add (new TextJustificationProperty (this));
        props.add (new TextToPathProperty (this));
    }

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) override
    {
        if (! fillType.isInvisible())
        {
            String x, y, w, h, r;
            positionToCode (position, code.document->getComponentLayout(), x, y, w, h);
            r << "{\n"
              << "    int x = " << x << ", y = " << y << ", width = " << w << ", height = " << h << ";\n"
              << "    String text (" << quotedString (text, code.shouldUseTransMacro()) << ");\n"
              << "    " << fillType.generateVariablesCode ("fill")
              << "    //[UserPaintCustomArguments] Customize the painting arguments here..\n"
              << customPaintCode
              << "    //[/UserPaintCustomArguments]\n"
              << "    ";
            fillType.fillInGeneratedCode ("fill", position, code, r);
            r << "    g.setFont (" << FontPropertyComponent::getCompleteFontCode (font, typefaceName) << ");\n"
              << "    g.drawText (text, x, y, width, height,\n"
              << "                " << CodeHelpers::justificationToCode (justification) << ", true);\n"
              << "}\n\n";
            
            paintMethodCode += r;
        }
    }

    void applyCustomPaintSnippets (StringArray& snippets) override
    {
        customPaintCode.clear();
        
        if (! snippets.isEmpty() && ! fillType.isInvisible())
        {
            customPaintCode = snippets[0];
            snippets.remove (0);
        }
    }

    static const char* getTagName() noexcept        { return "TEXT"; }

    XmlElement* createXml() const override
    {
        XmlElement* e = new XmlElement (getTagName());
        position.applyToXml (*e);
        addColourAttributes (e);
        e->setAttribute ("text", text);
        e->setAttribute ("fontname", typefaceName);
        e->setAttribute ("fontsize", roundToInt (font.getHeight() * 100.0) / 100.0);
        e->setAttribute ("kerning", roundToInt (font.getExtraKerningFactor() * 1000.0) / 1000.0);
        e->setAttribute ("bold", font.isBold());
        e->setAttribute ("italic", font.isItalic());
        e->setAttribute ("justification", justification.getFlags());
        if (font.getTypefaceStyle() != "Regular")
        {
            e->setAttribute ("typefaceStyle", font.getTypefaceStyle());
        }

        return e;
    }

    bool loadFromXml (const XmlElement& xml) override
    {
        if (xml.hasTagName (getTagName()))
        {
            position.restoreFromXml (xml, position);
            loadColourAttributes (xml);

            text = xml.getStringAttribute ("text", "Hello World");
            typefaceName = xml.getStringAttribute ("fontname", FontPropertyComponent::getDefaultFont());
            font.setHeight ((float) xml.getDoubleAttribute ("fontsize", 15.0));
            font.setBold (xml.getBoolAttribute ("bold", false));
            font.setItalic (xml.getBoolAttribute ("italic", false));
            font.setExtraKerningFactor ((float) xml.getDoubleAttribute ("kerning", 0.0));
            justification = Justification (xml.getIntAttribute ("justification", Justification::centred));
            auto fontStyle = xml.getStringAttribute ("typefaceStyle");
            if (! fontStyle.isEmpty())
                font.setTypefaceStyle (fontStyle);

            return true;
        }

        jassertfalse;
        return false;
    }

    //==============================================================================
    const String& getText() const noexcept                           { return text; }

    class SetTextAction   : public PaintElementUndoableAction <PaintElementText>
    {
    public:
        SetTextAction (PaintElementText* const element, const String& newText_)
            : PaintElementUndoableAction <PaintElementText> (element),
              newText (newText_),
              oldText (element->getText())
        {
        }

        bool perform()
        {
            showCorrectTab();
            getElement()->setText (newText, false);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getElement()->setText (oldText, false);
            return true;
        }

    private:
        String newText, oldText;
    };

    void setText (const String& t, const bool undoable)
    {
        if (t != text)
        {
            if (undoable)
            {
                perform (new SetTextAction (this, t),
                         "Change text element text");
            }
            else
            {
                text = t;
                changed();
            }
        }
    }

    //==============================================================================
    const Font& getFont() const                                     { return font; }

    class SetFontAction   : public PaintElementUndoableAction <PaintElementText>
    {
    public:
        SetFontAction (PaintElementText* const element, const Font& newFont_)
            : PaintElementUndoableAction <PaintElementText> (element),
              newFont (newFont_),
              oldFont (element->getFont())
        {
        }

        bool perform()
        {
            showCorrectTab();
            getElement()->setFont (newFont, false);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getElement()->setFont (oldFont, false);
            return true;
        }

    private:
        Font newFont, oldFont;
    };

    void setFont (const Font& newFont, const bool undoable)
    {
        if (font != newFont)
        {
            if (undoable)
            {
                perform (new SetFontAction (this, newFont),
                         "Change text element font");
            }
            else
            {
                font = newFont;
                changed();
            }
        }
    }

    //==============================================================================
    class SetTypefaceAction   : public PaintElementUndoableAction <PaintElementText>
    {
    public:
        SetTypefaceAction (PaintElementText* const element, const String& newValue_)
            : PaintElementUndoableAction <PaintElementText> (element),
              newValue (newValue_),
              oldValue (element->getTypefaceName())
        {
        }

        bool perform()
        {
            showCorrectTab();
            getElement()->setTypefaceName (newValue, false);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getElement()->setTypefaceName (oldValue, false);
            return true;
        }

    private:
        String newValue, oldValue;
    };

    void setTypefaceName (const String& newFontName, const bool undoable)
    {
        if (undoable)
        {
            perform (new SetTypefaceAction (this, newFontName),
                     "Change text element typeface");
        }
        else
        {
            typefaceName = newFontName;
            changed();
        }
    }

    String getTypefaceName() const noexcept                         { return typefaceName; }

    //==============================================================================
    Justification getJustification() const noexcept                 { return justification; }

    class SetJustifyAction   : public PaintElementUndoableAction <PaintElementText>
    {
    public:
        SetJustifyAction (PaintElementText* const element, Justification newValue_)
            : PaintElementUndoableAction <PaintElementText> (element),
              newValue (newValue_),
              oldValue (element->getJustification())
        {
        }

        bool perform()
        {
            showCorrectTab();
            getElement()->setJustification (newValue, false);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getElement()->setJustification (oldValue, false);
            return true;
        }

    private:
        Justification newValue, oldValue;
    };

    void setJustification (Justification j, const bool undoable)
    {
        if (justification.getFlags() != j.getFlags())
        {
            if (undoable)
            {
                perform (new SetJustifyAction (this, j),
                         "Change text element justification");
            }
            else
            {
                justification = j;
                changed();
            }
        }
    }

    void convertToPath()
    {
        if (PaintRoutineEditor* parent = dynamic_cast<PaintRoutineEditor*> (getParentComponent()))
        {

            font = FontPropertyComponent::applyNameToFont (typefaceName, font);

            const Rectangle<int> r =
                getCurrentBounds (parent->getComponentArea().withZeroOrigin());

            GlyphArrangement arr;
            arr.addCurtailedLineOfText (font, text,
                                        0.0f, 0.0f, (float) r.getWidth(),
                                        true);

            arr.justifyGlyphs (0, arr.getNumGlyphs(),
                               (float) r.getX(), (float) r.getY(),
                               (float) r.getWidth(), (float) r.getHeight(),
                               justification);

            Path path;
            arr.createPath (path);

            convertToNewPathElement (path);
        }
        else
        {
            jassertfalse;
        }
    }

private:
    String text;
    Font font;
    String typefaceName;
    Justification justification;
    String customPaintCode;
    
    Array <Justification> justificationTypes;

    //==============================================================================
    class TextProperty  : public TextPropertyComponent,
                          public ChangeListener
    {
    public:
        TextProperty (PaintElementText* const e)
            : TextPropertyComponent ("text", 2048, false),
              element (e)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~TextProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setText (const String& newText) override    { element->setText (newText, true); }
        String getText() const override                  { return element->getText(); }

        void changeListenerCallback (ChangeBroadcaster*) override     { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class FontNameProperty  : public FontPropertyComponent,
                              public ChangeListener
    {
    public:
        FontNameProperty (PaintElementText* const e)
            : FontPropertyComponent ("font"),
              element (e)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~FontNameProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setTypefaceName (const String& newFontName)    { element->setTypefaceName (newFontName, true); }
        String getTypefaceName() const                      { return element->getTypefaceName(); }

        void changeListenerCallback (ChangeBroadcaster*)                 { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class FontStyleProperty  : public ChoicePropertyComponent,
                               public ChangeListener
    {
    public:
        FontStyleProperty (PaintElementText* const e)
            : ChoicePropertyComponent ("style"),
              element (e)
        {
            element->getDocument()->addChangeListener (this);

            updateStylesList (element->getTypefaceName());
        }

        ~FontStyleProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void updateStylesList (const String& name)
        {
            if (getNumChildComponents() > 0)
            {
                if (auto cb = dynamic_cast<ComboBox*> (getChildComponent (0)))
                    cb->clear();

                getChildComponent (0)->setVisible (false);
                removeAllChildren();
            }

            choices.clear();

            choices.add ("Regular");
            choices.add ("Bold");
            choices.add ("Italic");
            choices.add ("Bold Italic");

            choices.mergeArray (Font::findAllTypefaceStyles (name));
            refresh();
        }

        void setIndex (int newIndex)
        {
            Font f (element->getFont());

            if (f.getAvailableStyles().contains (choices[newIndex]))
            {
                f.setBold   (false);
                f.setItalic (false);
                f.setTypefaceStyle (choices[newIndex]);
            }
            else
            {
                f.setTypefaceStyle ("Regular");
                f.setBold   (newIndex == 1 || newIndex == 3);
                f.setItalic (newIndex == 2 || newIndex == 3);
            }

            element->setFont (f, true);
        }

        int getIndex() const
        {
            auto f = element->getFont();

            const auto typefaceIndex = choices.indexOf (f.getTypefaceStyle());
            if (typefaceIndex == -1)
            {
                if (f.isBold() && f.isItalic())
                    return 3;
                else if (f.isBold())
                    return 1;
                else if (f.isItalic())
                    return 2;

                return 0;
            }

            return typefaceIndex;
        }

        void changeListenerCallback (ChangeBroadcaster*)
        {
            updateStylesList (element->getTypefaceName());
        }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class FontSizeProperty  : public SliderPropertyComponent,
                              public ChangeListener
    {
    public:
        FontSizeProperty (PaintElementText* const e)
            : SliderPropertyComponent ("size", 1.0, 250.0, 0.1, 0.3),
              element (e)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~FontSizeProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setValue (double newValue)
        {
            element->getDocument()->getUndoManager().undoCurrentTransactionOnly();

            Font f (element->getFont());
            f.setHeight ((float) newValue);

            element->setFont (f, true);
        }

        double getValue() const
        {
            return element->getFont().getHeight();
        }

        void changeListenerCallback (ChangeBroadcaster*)     { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class FontKerningProperty  : public SliderPropertyComponent,
                                 public ChangeListener
    {
    public:
        FontKerningProperty (PaintElementText* const e)
            : SliderPropertyComponent ("kerning", -0.5, 0.5, 0.001),
              element (e)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~FontKerningProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setValue (double newValue)
        {
            element->getDocument()->getUndoManager().undoCurrentTransactionOnly();

            Font f (element->getFont());
            f.setExtraKerningFactor ((float) newValue);

            element->setFont (f, true);
        }

        double getValue() const
        {
            return element->getFont().getExtraKerningFactor();
        }

        void changeListenerCallback (ChangeBroadcaster*)
        {
            refresh();
        }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class TextJustificationProperty  : public JustificationProperty,
                                       public ChangeListener
    {
    public:
        TextJustificationProperty (PaintElementText* const e)
            : JustificationProperty ("layout", false),
              element (e)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~TextJustificationProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setJustification (Justification newJustification)
        {
            element->setJustification (newJustification, true);
        }

        Justification getJustification() const
        {
            return element->getJustification();
        }

        void changeListenerCallback (ChangeBroadcaster*)     { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class TextToPathProperty  : public ButtonPropertyComponent
    {
    public:
        TextToPathProperty (PaintElementText* const e)
            : ButtonPropertyComponent ("path", false),
              element (e)
        {
        }

        void buttonClicked()
        {
            element->convertToPath();
        }

        String getButtonText() const
        {
            return "convert text to a path";
        }

    private:
        PaintElementText* const element;
    };
};
