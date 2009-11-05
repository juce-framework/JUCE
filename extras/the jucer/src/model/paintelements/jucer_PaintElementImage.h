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

#ifndef __JUCER_PAINTELEMENTIMAGE_JUCEHEADER__
#define __JUCER_PAINTELEMENTIMAGE_JUCEHEADER__

#include "../jucer_PaintRoutine.h"
#include "../../properties/jucer_FilePropertyComponent.h"
#include "jucer_ImageResourceProperty.h"
#include "jucer_PaintElementUndoableAction.h"


//==============================================================================
/**
*/
class PaintElementImage   : public PaintElement
{
public:
    enum StretchMode
    {
        stretched = 0,
        proportional = 1,
        proportionalReducingOnly = 2
    };

    //==============================================================================
    PaintElementImage (PaintRoutine* owner)
        : PaintElement (owner, T("Image")),
          opacity (1.0),
          mode (stretched)
    {
    }

    ~PaintElementImage()
    {
    }

    //==============================================================================
    const Drawable* getDrawable()
    {
        JucerDocument* const document = getDocument();

        if (document != 0)
            return document->getResources().getDrawable (resourceName);

        return 0;
    }

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle& parentArea)
    {
        const Rectangle r (position.getRectangle (parentArea, layout));

        const Drawable* const image = getDrawable();

        if (image != 0)
        {
            image->drawWithin (g, r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                               mode == stretched ? RectanglePlacement::stretchToFit
                                                 : (mode == proportionalReducingOnly ? (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize)
                                                                                     : RectanglePlacement::centred),
                               (float) opacity);
        }
        else
        {
            g.setColour (Colours::grey.withAlpha (0.5f));
            g.fillRect (r);

            g.setColour (Colours::black);
            g.drawText (T("(image missing)"),
                        r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                        Justification::centred, true);
        }
    }

    //==============================================================================
    void getEditableProperties (Array <PropertyComponent*>& properties)
    {
        PaintElement::getEditableProperties (properties);

        properties.add (new ImageElementResourceProperty (this));
        properties.add (new StretchModeProperty (this));
        properties.add (new OpacityProperty (this));
        properties.add (new ResetSizeProperty (this));
    }

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
    {
        String r;

        if (opacity > 0)
        {
            if (dynamic_cast <const DrawableImage*> (getDrawable()) != 0)
            {
                const String imageVariable ("cachedImage_" + resourceName);

                code.addImageResourceLoader (imageVariable, resourceName);

                if (opacity >= 254.0 / 255.0)
                    r << "g.setColour (Colours::black);\n";
                else
                    r << "g.setColour (Colours::black.withAlpha (" << valueToFloat (opacity) << "));\n";

                String x, y, w, h;
                positionToCode (position, getDocument()->getComponentLayout(), x, y, w, h);

                if (mode == stretched)
                {
                    r << "g.drawImage (" << imageVariable << ",\n             "
                      << x << ", " << y << ", " << w << ", " << h
                      << ",\n             0, 0, "
                      << imageVariable << "->getWidth(), "
                      << imageVariable << "->getHeight());\n\n";
                }
                else
                {
                    r << "g.drawImageWithin (" << imageVariable << ",\n                   "
                      << x << ", " << y << ", " << w << ", " << h
                      << ",\n                   ";

                    if (mode == proportionalReducingOnly)
                        r << "RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize";
                    else
                        r << "RectanglePlacement::centred";

                    r << ",\n                   false);\n\n";
                }

                paintMethodCode += r;
            }
            else
            {
                if (resourceName.isNotEmpty())
                {
                    const String imageVariable (T("drawable") + String (code.getUniqueSuffix()));

                    code.privateMemberDeclarations
                        << "Drawable* " << imageVariable << ";\n";

                    code.constructorCode
                        << imageVariable << " = Drawable::createFromImageData ("
                        << resourceName << ", " << resourceName << "Size);\n";

                    code.destructorCode
                        << "deleteAndZero (" << imageVariable << ");\n";

                    if (opacity >= 254.0 / 255.0)
                        r << "g.setColour (Colours::black);\n";
                    else
                        r << "g.setColour (Colours::black.withAlpha (" << valueToFloat (opacity) << "));\n";

                    String x, y, w, h;
                    positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

                    r << "jassert (" << imageVariable << " != 0);\n"
                      << "if (" << imageVariable << " != 0)\n    "
                      << imageVariable  << "->drawWithin (g, "
                      << x << ", " << y << ", " << w << ", " << h
                      << ",\n"
                      << String::repeatedString (T(" "), imageVariable.length() + 18)
                      << (mode == stretched ? "RectanglePlacement::stretchToFit"
                                            : (mode == proportionalReducingOnly ? "RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize"
                                                                                : "RectanglePlacement::centred"))
                      << ");\n\n";

                    paintMethodCode += r;
                }
                else
                {
                    jassertfalse // this resource isn't valid!
                }
            }
        }
    }

    //==============================================================================
    class SetResourceAction   : public PaintElementUndoableAction <PaintElementImage>
    {
    public:
        SetResourceAction (PaintElementImage* const element, const String& newResource_)
            : PaintElementUndoableAction <PaintElementImage> (element),
              newResource (newResource_)
        {
            oldResource = element->getResource();
        }

        bool perform()
        {
            showCorrectTab();
            getElement()->setResource (newResource, false);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getElement()->setResource (oldResource, false);
            return true;
        }

    private:
        String newResource, oldResource;
    };

    void setResource (const String& newName, const bool undoable)
    {
        if (resourceName != newName)
        {
            if (undoable)
            {
                perform (new SetResourceAction (this, newName),
                         T("Change image resource"));
            }
            else
            {
                resourceName = newName;
                changed();
            }
        }

        repaint();
    }

    const String getResource() const
    {
        return resourceName;
    }

    //==============================================================================
    class SetOpacityAction   : public PaintElementUndoableAction <PaintElementImage>
    {
    public:
        SetOpacityAction (PaintElementImage* const element, const double newOpacity_)
            : PaintElementUndoableAction <PaintElementImage> (element),
              newOpacity (newOpacity_)
        {
            oldOpacity = element->getOpacity();
        }

        bool perform()
        {
            showCorrectTab();
            getElement()->setOpacity (newOpacity, false);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getElement()->setOpacity (oldOpacity, false);
            return true;
        }

    private:
        double newOpacity, oldOpacity;
    };

    void setOpacity (double newOpacity, const bool undoable)
    {
        newOpacity = jlimit (0.0, 1.0, newOpacity);

        if (opacity != newOpacity)
        {
            if (undoable)
            {
                perform (new SetOpacityAction (this, newOpacity),
                         T("Change image opacity"));
            }
            else
            {
                opacity = newOpacity;
                changed();
            }
        }
    }

    double getOpacity() const throw()               { return opacity; }

    //==============================================================================
    static const tchar* getTagName() throw()        { return T("IMAGE"); }

    void resetToImageSize()
    {
        const Drawable* const image = getDrawable();

        if (image != 0 && getParentComponent() != 0)
        {
            const Rectangle parentArea (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

            Rectangle r (getCurrentBounds (parentArea));

            float x, y, w, h;
            image->getBounds (x, y, w, h);

            r.setSize ((int) (w + 1.0f), (int) (h + 1.0f));

            setCurrentBounds (r, parentArea, true);
        }
    }

    //==============================================================================
    class SetStretchModeAction  : public PaintElementUndoableAction <PaintElementImage>
    {
    public:
        SetStretchModeAction (PaintElementImage* const element, const StretchMode newValue_)
            : PaintElementUndoableAction <PaintElementImage> (element),
              newValue (newValue_)
        {
            oldValue = element->getStretchMode();
        }

        bool perform()
        {
            showCorrectTab();
            getElement()->setStretchMode (newValue, false);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getElement()->setStretchMode (oldValue, false);
            return true;
        }

    private:
        StretchMode newValue, oldValue;
    };

    StretchMode getStretchMode() const throw()                        { return mode; }

    void setStretchMode (const StretchMode newMode, const bool undoable)
    {
        if (mode != newMode)
        {
            if (undoable)
            {
                perform (new SetStretchModeAction (this, newMode),
                         T("Change image mode"));
            }
            else
            {
                mode = newMode;
                changed();
            }
        }
    }

    //==============================================================================
    XmlElement* createXml() const
    {
        XmlElement* e = new XmlElement (getTagName());
        position.applyToXml (*e);
        e->setAttribute (T("resource"), resourceName);
        e->setAttribute (T("opacity"), opacity);
        e->setAttribute (T("mode"), (int) mode);

        return e;
    }

    bool loadFromXml (const XmlElement& xml)
    {
        if (xml.hasTagName (getTagName()))
        {
            position.restoreFromXml (xml, position);
            resourceName = xml.getStringAttribute (T("resource"), String::empty);
            opacity = xml.getDoubleAttribute (T("opacity"), 1.0);
            mode = (StretchMode) xml.getIntAttribute (T("mode"), (int) stretched);

            repaint();
            return true;
        }
        else
        {
            jassertfalse
            return false;
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String resourceName;
    double opacity;
    StretchMode mode;

    //==============================================================================
    class ImageElementResourceProperty    : public ImageResourceProperty <PaintElementImage>
    {
    public:
        ImageElementResourceProperty (PaintElementImage* const element_)
            : ImageResourceProperty <PaintElementImage> (element_, T("image source"))
        {
        }

        //==============================================================================
        void setResource (const String& newName)
        {
            element->setResource (newName, true);
        }

        const String getResource() const
        {
            return element->getResource();
        }
    };

    //==============================================================================
    class OpacityProperty  : public SliderPropertyComponent,
                             private ChangeListener
    {
    public:
        OpacityProperty (PaintElementImage* const element_)
            : SliderPropertyComponent (T("opacity"), 0.0, 1.0, 0.001),
              element (element_)
        {
            element->getDocument()->addChangeListener (this);
        }

        ~OpacityProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setValue (const double newValue)
        {
            element->getDocument()->getUndoManager().undoCurrentTransactionOnly();

            element->setOpacity (newValue, true);
        }

        const double getValue() const
        {
            return element->getOpacity();
        }

        void changeListenerCallback (void*)
        {
            refresh();
        }

    private:
        PaintElementImage* const element;
    };

    class StretchModeProperty  : public ChoicePropertyComponent,
                                 private ChangeListener
    {
    public:
        StretchModeProperty (PaintElementImage* const element_)
            : ChoicePropertyComponent (T("stretch mode")),
              element (element_)
        {
            choices.add (T("Stretched to fit"));
            choices.add (T("Maintain aspect ratio"));
            choices.add (T("Maintain aspect ratio, only reduce in size"));

            element->getDocument()->addChangeListener (this);
        }

        ~StretchModeProperty()
        {
            element->getDocument()->removeChangeListener (this);
        }

        void setIndex (const int newIndex)
        {
            element->setStretchMode ((StretchMode) newIndex, true);
        }

        int getIndex() const
        {
            return (int) element->getStretchMode();
        }

        void changeListenerCallback (void*)
        {
            refresh();
        }

    private:
        PaintElementImage* const element;
    };

    class ResetSizeProperty   : public ButtonPropertyComponent
    {
    public:
        ResetSizeProperty (PaintElementImage* const element_)
            : ButtonPropertyComponent (T("reset"), false),
              element (element_)
        {
        }

        void buttonClicked()
        {
            element->resetToImageSize();
        }

        const String getButtonText() const          { return T("reset to image size"); }

    private:
        PaintElementImage* const element;
    };
};


#endif   // __JUCER_PAINTELEMENTIMAGE_JUCEHEADER__
