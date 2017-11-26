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

#include "../jucer_PaintRoutine.h"
#include "../Properties/jucer_FilePropertyComponent.h"
#include "jucer_ImageResourceProperty.h"
#include "jucer_PaintElementUndoableAction.h"

//==============================================================================
class PaintElementImage   : public PaintElement
{
public:
    PaintElementImage (PaintRoutine* pr)
        : PaintElement (pr, "Image"),
          opacity (1.0),
          mode (stretched)
    {
    }

    enum StretchMode
    {
        stretched = 0,
        proportional = 1,
        proportionalReducingOnly = 2
    };

    const Drawable* getDrawable()
    {
        if (JucerDocument* const document = getDocument())
            return document->getResources().getDrawable (resourceName);

        return nullptr;
    }

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea) override
    {
        const Rectangle<int> r (position.getRectangle (parentArea, layout));

        if (const Drawable* const image = getDrawable())
        {
            image->drawWithin (g, r.toFloat(),
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
            g.drawText ("(image missing)",
                        r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                        Justification::centred, true);
        }
    }

    //==============================================================================
    void getEditableProperties (Array <PropertyComponent*>& props, bool multipleSelected) override
    {
        PaintElement::getEditableProperties (props, multipleSelected);

        props.add (new ImageElementResourceProperty (this));
        props.add (new StretchModeProperty (this));
        props.add (new OpacityProperty (this));
        props.add (new ResetSizeProperty (this));
    }

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) override
    {
        if (opacity > 0)
        {
            String x, y, w, h, r;
            positionToCode (position, getDocument()->getComponentLayout(), x, y, w, h);
            r << "{\n"
              << "    int x = " << x << ", y = " << y << ", width = " << w << ", height = " << h << ";\n"
              << "    //[UserPaintCustomArguments] Customize the painting arguments here..\n"
              << customPaintCode
              << "    //[/UserPaintCustomArguments]\n";

            if (dynamic_cast<const DrawableImage*> (getDrawable()) != 0)
            {
                const String imageVariable ("cachedImage_" + resourceName.replace ("::", "_") + "_" + String (code.getUniqueSuffix()));

                code.addImageResourceLoader (imageVariable, resourceName);

                if (opacity >= 254.0 / 255.0)
                    r << "    g.setColour (Colours::black);\n";
                else
                    r << "    g.setColour (Colours::black.withAlpha (" << CodeHelpers::floatLiteral (opacity, 3) << "));\n";


                if (mode == stretched)
                {
                    r << "    g.drawImage (" << imageVariable << ",\n"
                      << "                 x, y, width, height,\n"
                      << "                 0, 0, " << imageVariable << ".getWidth(), " << imageVariable << ".getHeight());\n";
                }
                else
                {
                    r << "    g.drawImageWithin (" << imageVariable << ",\n"
                      << "                       x, y, width, height,\n"
                      << "                       ";

                    if (mode == proportionalReducingOnly)
                        r << "RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize";
                    else
                        r << "RectanglePlacement::centred";

                    r << ",\n"
                      << "                       false);\n";
                }

            }
            else
            {
                if (resourceName.isNotEmpty())
                {
                    const String imageVariable ("drawable" + String (code.getUniqueSuffix()));

                    code.privateMemberDeclarations
                        << "ScopedPointer<Drawable> " << imageVariable << ";\n";

                    code.constructorCode
                        << imageVariable << " = Drawable::createFromImageData ("
                        << resourceName << ", " << resourceName << "Size);\n";

                    code.destructorCode
                        << imageVariable << " = nullptr;\n";

                    if (opacity >= 254.0 / 255.0)
                        r << "    g.setColour (Colours::black);\n";
                    else
                        r << "    g.setColour (Colours::black.withAlpha (" << CodeHelpers::floatLiteral (opacity, 3) << "));\n";

                    r << "    jassert (" << imageVariable << " != 0);\n"
                      << "    if (" << imageVariable << " != 0)\n"
                      << "        " << imageVariable  << "->drawWithin (g, Rectangle<float> (x, y, width, height),\n"
                      << "    " << String::repeatedString (" ", imageVariable.length() + 18)
                      << (mode == stretched ? "RectanglePlacement::stretchToFit"
                                            : (mode == proportionalReducingOnly ? "RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize"
                                                                                : "RectanglePlacement::centred"))
                      << ", " << CodeHelpers::floatLiteral (opacity, 3) << ");\n";
                }
            }

            r << "}\n\n";

            paintMethodCode += r;
        }
    }

    void applyCustomPaintSnippets (StringArray& snippets) override
    {
        customPaintCode.clear();

        if (! snippets.isEmpty() && opacity > 0)
        {
            customPaintCode = snippets[0];
            snippets.remove (0);
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
                         "Change image resource");
            }
            else
            {
                resourceName = newName;
                changed();
            }
        }

        repaint();
    }

    String getResource() const
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
                         "Change image opacity");
            }
            else
            {
                opacity = newOpacity;
                changed();
            }
        }
    }

    double getOpacity() const noexcept               { return opacity; }

    //==============================================================================
    static const char* getTagName() noexcept         { return "IMAGE"; }

    void resetToImageSize()
    {
        if (const Drawable* const image = getDrawable())
        {
            if (PaintRoutineEditor* ed = dynamic_cast<PaintRoutineEditor*> (getParentComponent()))
            {
                const Rectangle<int> parentArea (ed->getComponentArea());

                Rectangle<int> r (getCurrentBounds (parentArea));
                Rectangle<float> b (image->getDrawableBounds());

                r.setSize ((int) (b.getWidth()  + 0.999f),
                           (int) (b.getHeight() + 0.999f));

                setCurrentBounds (r, parentArea, true);
            }
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

    StretchMode getStretchMode() const noexcept                        { return mode; }

    void setStretchMode (const StretchMode newMode, const bool undoable)
    {
        if (mode != newMode)
        {
            if (undoable)
            {
                perform (new SetStretchModeAction (this, newMode),
                         "Change image mode");
            }
            else
            {
                mode = newMode;
                changed();
            }
        }
    }

    //==============================================================================
    XmlElement* createXml() const override
    {
        XmlElement* e = new XmlElement (getTagName());
        position.applyToXml (*e);
        e->setAttribute ("resource", resourceName);
        e->setAttribute ("opacity", opacity);
        e->setAttribute ("mode", (int) mode);

        return e;
    }

    bool loadFromXml (const XmlElement& xml) override
    {
        if (xml.hasTagName (getTagName()))
        {
            position.restoreFromXml (xml, position);
            resourceName = xml.getStringAttribute ("resource", String());
            opacity = xml.getDoubleAttribute ("opacity", 1.0);
            mode = (StretchMode) xml.getIntAttribute ("mode", (int) stretched);

            repaint();
            return true;
        }

        jassertfalse;
        return false;
    }

private:
    String resourceName;
    double opacity;
    StretchMode mode;
    String customPaintCode;

    //==============================================================================
    class ImageElementResourceProperty    : public ImageResourceProperty <PaintElementImage>
    {
    public:
        ImageElementResourceProperty (PaintElementImage* const e)
            : ImageResourceProperty <PaintElementImage> (e, "image source")
        {
        }

        void setResource (const String& newName)
        {
            if (element != nullptr)
                element->setResource (newName, true);
        }

        String getResource() const
        {
            if (element != nullptr)
                return element->getResource();

            return {};
        }
    };

    //==============================================================================
    class OpacityProperty  : public SliderPropertyComponent
    {
    public:
        OpacityProperty (PaintElementImage* const e)
            : SliderPropertyComponent ("opacity", 0.0, 1.0, 0.001),
              listener (e)
        {
            listener.setPropertyToRefresh (*this);
        }

        void setValue (double newValue)
        {
            listener.owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();
            listener.owner->setOpacity (newValue, true);
        }

        double getValue() const
        {
            return listener.owner->getOpacity();
        }

        ElementListener<PaintElementImage> listener;
    };

    class StretchModeProperty  : public ChoicePropertyComponent
    {
    public:
        StretchModeProperty (PaintElementImage* const e)
            : ChoicePropertyComponent ("stretch mode"),
              listener (e)
        {
            listener.setPropertyToRefresh (*this);

            choices.add ("Stretched to fit");
            choices.add ("Maintain aspect ratio");
            choices.add ("Maintain aspect ratio, only reduce in size");
        }

        void setIndex (int newIndex)
        {
            listener.owner->setStretchMode ((StretchMode) newIndex, true);
        }

        int getIndex() const
        {
            return (int) listener.owner->getStretchMode();
        }

        ElementListener<PaintElementImage> listener;
    };

    class ResetSizeProperty   : public ButtonPropertyComponent
    {
    public:
        ResetSizeProperty (PaintElementImage* const e)
            : ButtonPropertyComponent ("reset", false),
              element (e)
        {
        }

        void buttonClicked()
        {
            element->resetToImageSize();
        }

        String getButtonText() const          { return "reset to image size"; }

    private:
        PaintElementImage* const element;
    };
};
