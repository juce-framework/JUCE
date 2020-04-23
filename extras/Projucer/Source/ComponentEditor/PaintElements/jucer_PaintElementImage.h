/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_ColouredElement.h"
#include "../Properties/jucer_FilePropertyComponent.h"
#include "jucer_ImageResourceProperty.h"
#include "jucer_PaintElementUndoableAction.h"

//==============================================================================
class PaintElementImage   : public PaintElement
{
public:
    PaintElementImage (PaintRoutine*);
    ~PaintElementImage() override;

    enum StretchMode
    {
        stretched = 0,
        proportional = 1,
        proportionalReducingOnly = 2
    };

    const Drawable* getDrawable();
    void draw (Graphics&, const ComponentLayout*, const Rectangle<int>&) override;

    //==============================================================================
    void getEditableProperties (Array <PropertyComponent*>&, bool) override;
    void fillInGeneratedCode (GeneratedCode&, String&) override;
    void applyCustomPaintSnippets (StringArray& snippets) override;

    //==============================================================================
    class SetResourceAction   : public PaintElementUndoableAction <PaintElementImage>
    {
    public:
        SetResourceAction (PaintElementImage* const, const String&);

        bool perform();
        bool undo();

    private:
        String newResource, oldResource;
    };

    void setResource (const String&, const bool);

    String getResource() const;

    //==============================================================================
    class SetOpacityAction   : public PaintElementUndoableAction <PaintElementImage>
    {
    public:
        SetOpacityAction (PaintElementImage* const, const double);

        bool perform();
        bool undo();

    private:
        double newOpacity, oldOpacity;
    };

    void setOpacity (double, const bool);
    double getOpacity() const noexcept;

    //==============================================================================
    static const char* getTagName() noexcept;

    void resetToImageSize();

    //==============================================================================
    class SetStretchModeAction   : public PaintElementUndoableAction <PaintElementImage>
    {
    public:
        SetStretchModeAction (PaintElementImage* const, const StretchMode);

        bool perform();
        bool undo();

    private:
        StretchMode newValue, oldValue;
    };

    StretchMode getStretchMode() const noexcept;

    void setStretchMode (const StretchMode, const bool);

    //==============================================================================
    XmlElement* createXml() const override;

    bool loadFromXml (const XmlElement&) override;

private:
    String resourceName;
    double opacity;
    StretchMode mode;
    String customPaintCode;

    //==============================================================================
    class ImageElementResourceProperty   : public ImageResourceProperty <PaintElementImage>
    {
    public:
        ImageElementResourceProperty (PaintElementImage* const);

        void setResource (const String&);
        String getResource() const;
    };

    //==============================================================================
    class OpacityProperty   : public SliderPropertyComponent
    {
    public:
        OpacityProperty (PaintElementImage* const);

        void setValue (double);
        double getValue() const;

        ElementListener<PaintElementImage> listener;
    };

    class StretchModeProperty   : public ChoicePropertyComponent
    {
    public:
        StretchModeProperty (PaintElementImage* const);

        void setIndex (int);
        int getIndex() const;

        ElementListener<PaintElementImage> listener;
    };

    class ResetSizeProperty   : public ButtonPropertyComponent
    {
    public:
        ResetSizeProperty (PaintElementImage* const);

        void buttonClicked();
        String getButtonText() const;

    private:
        PaintElementImage* const element;
    };
};
