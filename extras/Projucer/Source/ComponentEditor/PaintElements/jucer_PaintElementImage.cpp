/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_PaintElementImage.h"

PaintElementImage::PaintElementImage (PaintRoutine* pr)
    : PaintElement (pr, "Image"),
      opacity (1.0),
      mode (stretched)
{
}

PaintElementImage::~PaintElementImage() {}

const Drawable* PaintElementImage::getDrawable()
{
    if (JucerDocument* const document = getDocument())
        return document->getResources().getDrawable (resourceName);

    return nullptr;
}

void PaintElementImage::draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea)
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
void PaintElementImage::getEditableProperties (Array <PropertyComponent*>& props, bool multipleSelected)
{
    PaintElement::getEditableProperties (props, multipleSelected);

    props.add (new ImageElementResourceProperty (this));
    props.add (new StretchModeProperty (this));
    props.add (new OpacityProperty (this));
    props.add (new ResetSizeProperty (this));
}

void PaintElementImage::fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
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

        if (dynamic_cast<const DrawableImage*> (getDrawable()))
        {
            const String imageVariable ("cachedImage_" + resourceName.replace ("::", "_") + "_" + String (code.getUniqueSuffix()));

            code.addImageResourceLoader (imageVariable, resourceName);

            if (opacity >= 254.0 / 255.0)
                r << "    g.setColour (juce::Colours::black);\n";
            else
                r << "    g.setColour (juce::Colours::black.withAlpha (" << CodeHelpers::floatLiteral (opacity, 3) << "));\n";

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
                    r << "juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize";
                else
                    r << "juce::RectanglePlacement::centred";

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
                    << "std::unique_ptr<juce::Drawable> " << imageVariable << ";\n";

                code.constructorCode
                    << imageVariable << " = juce::Drawable::createFromImageData ("
                    << resourceName << ", " << resourceName << "Size);\n";

                code.destructorCode
                    << imageVariable << " = nullptr;\n";

                if (opacity >= 254.0 / 255.0)
                    r << "    g.setColour (juce::Colours::black);\n";
                else
                    r << "    g.setColour (juce::Colours::black.withAlpha (" << CodeHelpers::floatLiteral (opacity, 3) << "));\n";

                r << "    jassert (" << imageVariable << " != nullptr);\n"
                  << "    if (" << imageVariable << " != nullptr)\n"
                  << "        " << imageVariable  << "->drawWithin (g, juce::Rectangle<int> (x, y, width, height).toFloat(),\n"
                  << "    " << String::repeatedString (" ", imageVariable.length() + 18)
                  << (mode == stretched ? "juce::RectanglePlacement::stretchToFit"
                                        : (mode == proportionalReducingOnly ? "juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize"
                                                                            : "juce::RectanglePlacement::centred"))
                  << ", " << CodeHelpers::floatLiteral (opacity, 3) << ");\n";
            }
        }

        r << "}\n\n";

        paintMethodCode += r;
    }
}

void PaintElementImage::applyCustomPaintSnippets (StringArray& snippets)
{
    customPaintCode.clear();

    if (! snippets.isEmpty() && opacity > 0)
    {
        customPaintCode = snippets[0];
        snippets.remove (0);
    }
}

//==============================================================================
PaintElementImage::SetResourceAction::SetResourceAction (PaintElementImage* const element, const String& newResource_)
    : PaintElementUndoableAction <PaintElementImage> (element),
      newResource (newResource_)
{
    oldResource = element->getResource();
}

bool PaintElementImage::SetResourceAction::perform()
{
    showCorrectTab();
    getElement()->setResource (newResource, false);
    return true;
}

bool PaintElementImage::SetResourceAction::undo()
{
    showCorrectTab();
    getElement()->setResource (oldResource, false);
    return true;
}

void PaintElementImage::setResource (const String& newName, const bool undoable)
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

String PaintElementImage::getResource() const
{
    return resourceName;
}

//==============================================================================
PaintElementImage::SetOpacityAction::SetOpacityAction (PaintElementImage* const element, const double newOpacity_)
    : PaintElementUndoableAction <PaintElementImage> (element),
      newOpacity (newOpacity_)
{
    oldOpacity = element->getOpacity();
}

bool PaintElementImage::SetOpacityAction::perform()
{
    showCorrectTab();
    getElement()->setOpacity (newOpacity, false);
    return true;
}

bool PaintElementImage::SetOpacityAction::undo()
{
    showCorrectTab();
    getElement()->setOpacity (oldOpacity, false);
    return true;
}

void PaintElementImage::setOpacity (double newOpacity, const bool undoable)
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

double PaintElementImage::getOpacity() const noexcept               { return opacity; }

//==============================================================================
const char* PaintElementImage::getTagName() noexcept         { return "IMAGE"; }

void PaintElementImage::resetToImageSize()
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
PaintElementImage::SetStretchModeAction::SetStretchModeAction (PaintElementImage* const element, const StretchMode newValue_)
    : PaintElementUndoableAction <PaintElementImage> (element),
      newValue (newValue_)
{
    oldValue = element->getStretchMode();
}

bool PaintElementImage::SetStretchModeAction::perform()
{
    showCorrectTab();
    getElement()->setStretchMode (newValue, false);
    return true;
}

bool PaintElementImage::SetStretchModeAction::undo()
{
    showCorrectTab();
    getElement()->setStretchMode (oldValue, false);
    return true;
}

PaintElementImage::StretchMode PaintElementImage::getStretchMode() const noexcept   { return mode; }

void PaintElementImage::setStretchMode (const StretchMode newMode, const bool undoable)
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
XmlElement* PaintElementImage::createXml() const
{
    XmlElement* e = new XmlElement (getTagName());
    position.applyToXml (*e);
    e->setAttribute ("resource", resourceName);
    e->setAttribute ("opacity", opacity);
    e->setAttribute ("mode", (int) mode);

    return e;
}

bool PaintElementImage::loadFromXml (const XmlElement& xml)
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

//==============================================================================
PaintElementImage::ImageElementResourceProperty::ImageElementResourceProperty (PaintElementImage* const e)
    : ImageResourceProperty <PaintElementImage> (e, "image source")
{
}

void PaintElementImage::ImageElementResourceProperty::setResource (const String& newName)
{
    if (element != nullptr)
        element->setResource (newName, true);
}

String PaintElementImage::ImageElementResourceProperty::getResource() const
{
    if (element != nullptr)
        return element->getResource();

    return {};
}


//==============================================================================
PaintElementImage::OpacityProperty::OpacityProperty (PaintElementImage* const e)
    : SliderPropertyComponent ("opacity", 0.0, 1.0, 0.001),
      listener (e)
{
    listener.setPropertyToRefresh (*this);
}

void PaintElementImage::OpacityProperty::setValue (double newValue)
{
    listener.owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();
    listener.owner->setOpacity (newValue, true);
}

double PaintElementImage::OpacityProperty::getValue() const
{
    return listener.owner->getOpacity();
}

PaintElementImage::StretchModeProperty::StretchModeProperty (PaintElementImage* const e)
    : ChoicePropertyComponent ("stretch mode"),
      listener (e)
{
    listener.setPropertyToRefresh (*this);

    choices.add ("Stretched to fit");
    choices.add ("Maintain aspect ratio");
    choices.add ("Maintain aspect ratio, only reduce in size");
}

void PaintElementImage::StretchModeProperty::setIndex (int newIndex)
{
    listener.owner->setStretchMode ((StretchMode) newIndex, true);
}

int PaintElementImage::StretchModeProperty::getIndex() const
{
    return (int) listener.owner->getStretchMode();
}

PaintElementImage::ResetSizeProperty::ResetSizeProperty (PaintElementImage* const e)
    : ButtonPropertyComponent ("reset", false),
      element (e)
{
}

void PaintElementImage::ResetSizeProperty::buttonClicked()
{
    element->resetToImageSize();
}

String PaintElementImage::ResetSizeProperty::getButtonText() const   { return "reset to image size"; }
