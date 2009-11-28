/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_PAINTELEMENTTEXT_JUCEHEADER__
#define __JUCER_PAINTELEMENTTEXT_JUCEHEADER__

#include "jucer_ColouredElement.h"
#include "../../properties/jucer_FontPropertyComponent.h"
#include "../../properties/jucer_JustificationProperty.h"


//==============================================================================
/**
*/
class PaintElementText   : public ColouredElement
{
public:
    //==============================================================================
    PaintElementText (PaintRoutine* owner)
        : ColouredElement (owner, T("Text"), false, false),
          text (T("Your text goes here")),
          font (15.0f),
          typefaceName (FontPropertyComponent::defaultFont),
          justification (Justification::centred)
    {
        fillType.colour = Colours::black;
        position.rect.setWidth (200);
        position.rect.setHeight (30);
    }

    ~PaintElementText()
    {
    }

    //==============================================================================
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle& parentArea)
    {
        fillType.setFillType (g, getDocument(), parentArea);

        font = FontPropertyComponent::applyNameToFont (typefaceName, font);
        g.setFont (font);

        const Rectangle r (position.getRectangle (parentArea, layout));
        g.drawText (replaceStringTranslations (text, owner->getDocument()),
                    r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                    justification, true);
    }

    void getEditableProperties (Array <PropertyComponent*>& properties)
    {
        ColouredElement::getEditableProperties (properties);

        properties.add (new TextProperty (this));
        properties.add (new FontNameProperty (this));
        properties.add (new FontStyleProperty (this));
        properties.add (new FontSizeProperty (this));
        properties.add (new TextJustificationProperty (this));
        properties.add (new TextToPathProperty (this));
    }

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
    {
        if (! fillType.isInvisible())
        {
            String r;

            fillType.fillInGeneratedCode (code, paintMethodCode);

            String x, y, w, h;
            positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

            r << "g.setFont ("
              << FontPropertyComponent::getCompleteFontCode (font, typefaceName)
              << ");\ng.drawText ("
              << quotedString (text)
              << ",\n            "
              << x << ", " << y << ", " << w << ", " << h
              << ",\n            "
              << justificationToCode (justification)
              << ", true);\n\n";

            paintMethodCode += r;
        }
    }

    static const tchar* getTagName() throw()        { return T("TEXT"); }

    XmlElement* createXml() const
    {
        XmlElement* e = new XmlElement (getTagName());
        position.applyToXml (*e);
        addColourAttributes (e);
        e->setAttribute (T("text"), text);
        e->setAttribute (T("fontname"), typefaceName);
        e->setAttribute (T("fontsize"), roundDoubleToInt (font.getHeight() * 100.0) / 100.0);
        e->setAttribute (T("bold"), font.isBold());
        e->setAttribute (T("italic"), font.isItalic());
        e->setAttribute (T("justification"), justification.getFlags());

        return e;
    }

    bool loadFromXml (const XmlElement& xml)
    {
        if (xml.hasTagName (getTagName()))
        {
            position.restoreFromXml (xml, position);
            loadColourAttributes (xml);

            text = xml.getStringAttribute (T("text"), T("Hello World"));
            typefaceName = xml.getStringAttribute (T("fontname"), FontPropertyComponent::defaultFont);
            font.setHeight ((float) xml.getDoubleAttribute (T("fontsize"), 15.0));
            font.setBold (xml.getBoolAttribute (T("bold"), false));
            font.setItalic (xml.getBoolAttribute (T("italic"), false));
            justification = Justification (xml.getIntAttribute (T("justification"), Justification::centred));

            return true;
        }
        else
        {
            jassertfalse
            return false;
        }
    }

    //==============================================================================
    const String& getText() const throw()                           { return text; }

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
                         T("Change text element text"));
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
                         T("Change text element font"));
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
                     T("Change text element typeface"));
        }
        else
        {
            typefaceName = newFontName;
            changed();
        }
    }

    const String getTypefaceName() const throw()                    { return typefaceName; }

    //==============================================================================
    const Justification& getJustification() const throw()           { return justification; }

    class SetJustifyAction   : public PaintElementUndoableAction <PaintElementText>
    {
    public:
        SetJustifyAction (PaintElementText* const element, const Justification& newValue_)
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

    void setJustification (const Justification& j, const bool undoable)
    {
        if (justification.getFlags() != j.getFlags())
        {
            if (undoable)
            {
                perform (new SetJustifyAction (this, j),
                         T("Change text element justification"));
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
        font = FontPropertyComponent::applyNameToFont (typefaceName, font);

        const Rectangle r (getCurrentAbsoluteBounds());

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

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String text;
    Font font;
    String typefaceName;
    Justification justification;

    Array <Justification> justificationTypes;

    //==============================================================================
    class TextProperty  : public TextPropertyComponent,
                          public ChangeListener
    {
    public:
        TextProperty (PaintElementText* const element_)
            : TextPropertyComponent (T("text"), 2048, false),
              element (element_)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~TextProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setText (const String& newText)    { element->setText (newText, true); }
        const String getText() const            { return element->getText(); }

        void changeListenerCallback (void*)     { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class FontNameProperty  : public FontPropertyComponent,
                              public ChangeListener
    {
    public:
        FontNameProperty (PaintElementText* const element_)
            : FontPropertyComponent (T("font")),
              element (element_)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~FontNameProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setTypefaceName (const String& newFontName)    { element->setTypefaceName (newFontName, true); }
        const String getTypefaceName() const                { return element->getTypefaceName(); }

        void changeListenerCallback (void*)                 { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class FontStyleProperty  : public ChoicePropertyComponent,
                               public ChangeListener
    {
    public:
        FontStyleProperty (PaintElementText* const element_)
            : ChoicePropertyComponent (T("style")),
              element (element_)
        {
            element->getDocument()->addChangeListener (this);

            choices.add (T("normal"));
            choices.add (T("bold"));
            choices.add (T("italic"));
            choices.add (T("bold + italic"));
        }

        ~FontStyleProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setIndex (const int newIndex)
        {
            Font f (element->getFont());

            f.setBold (newIndex == 1 || newIndex == 3);
            f.setItalic (newIndex == 2 || newIndex == 3);

            element->setFont (f, true);
        }

        int getIndex() const
        {
            if (element->getFont().isBold() && element->getFont().isItalic())
                return 3;
            else if (element->getFont().isBold())
                return 1;
            else if (element->getFont().isItalic())
                return 2;

            return 0;
        }

        void changeListenerCallback (void*)     { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class FontSizeProperty  : public SliderPropertyComponent,
                              public ChangeListener
    {
    public:
        FontSizeProperty (PaintElementText* const element_)
            : SliderPropertyComponent (T("size"), 1.0, 250.0, 0.1, 0.3),
              element (element_)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~FontSizeProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setValue (const double newValue)
        {
            element->getDocument()->getUndoManager().undoCurrentTransactionOnly();

            Font f (element->getFont());
            f.setHeight ((float) newValue);

            element->setFont (f, true);
        }

        const double getValue() const
        {
            return element->getFont().getHeight();
        }

        void changeListenerCallback (void*)     { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class TextJustificationProperty  : public JustificationProperty,
                                       public ChangeListener
    {
    public:
        TextJustificationProperty (PaintElementText* const element_)
            : JustificationProperty (T("layout"), false),
              element (element_)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~TextJustificationProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setJustification (const Justification& newJustification)
        {
            element->setJustification (newJustification, true);
        }

        const Justification getJustification() const
        {
            return element->getJustification();
        }

        void changeListenerCallback (void*)     { refresh(); }

    private:
        PaintElementText* const element;
    };

    //==============================================================================
    class TextToPathProperty  : public ButtonPropertyComponent
    {
    public:
        TextToPathProperty (PaintElementText* const element_)
            : ButtonPropertyComponent (T("path"), false),
              element (element_)
        {
        }

        void buttonClicked()
        {
            element->convertToPath();
        }

        const String getButtonText() const
        {
            return T("convert text to a path");
        }

    private:
        PaintElementText* const element;
    };
};


#endif   // __JUCER_PAINTELEMENTTEXT_JUCEHEADER__
