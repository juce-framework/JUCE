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

#ifndef __JUCER_FILLTYPEPROPERTYCOMPONENT_H_88CF1300__
#define __JUCER_FILLTYPEPROPERTYCOMPONENT_H_88CF1300__

#include "../model/Project/jucer_Project.h"
class FillTypeEditorComponent;


//==============================================================================
class PopupFillSelector   : public Component,
                            public ChangeListener,
                            public ValueTree::Listener,
                            public ButtonListener,
                            public AsyncUpdater
{
public:
    PopupFillSelector (const ValueTree& fillState_, const ColourGradient& defaultGradient_,
                       Drawable::ImageProvider* imageProvider_, Project* project, UndoManager* undoManager_)
        : gradientPicker (defaultGradient_),
          defaultGradient (defaultGradient_),
          tilePicker (imageProvider_, project),
          fillState (fillState_),
          imageProvider (imageProvider_),
          undoManager (undoManager_)
    {
        colourButton.setButtonText ("Colour");
        colourButton.setConnectedEdges (TextButton::ConnectedOnRight);
        gradientButton.setButtonText ("Gradient");
        gradientButton.setConnectedEdges (TextButton::ConnectedOnRight | TextButton::ConnectedOnLeft);
        imageButton.setButtonText ("Image");
        imageButton.setConnectedEdges (TextButton::ConnectedOnLeft);

        addAndMakeVisible (&colourButton);
        addAndMakeVisible (&gradientButton);
        addAndMakeVisible (&imageButton);

        addChildComponent (&colourPicker);
        colourPicker.setSize (300, 410);
        colourPicker.setCurrentColour (Colours::green);
        colourPicker.setName ("Colour");
        colourPicker.addChangeListener (this);

        addChildComponent (&gradientPicker);
        gradientPicker.setSize (300, 500);
        gradientPicker.addChangeListener (this);

        addChildComponent (&tilePicker);
        tilePicker.setSize (300, 170);
        tilePicker.addChangeListener (this);

        fillState.addListener (this);

        colourButton.setRadioGroupId (123);
        gradientButton.setRadioGroupId (123);
        imageButton.setRadioGroupId (123);

        colourButton.addButtonListener (this);
        gradientButton.addButtonListener (this);
        imageButton.addButtonListener (this);

        setSize (300, 200);
        refresh();
    }

    ~PopupFillSelector()
    {
        colourButton.removeButtonListener (this);
        gradientButton.removeButtonListener (this);
        imageButton.removeButtonListener (this);
    }

    void resized()
    {
        const int y = 2, w = 80, h = 22;
        gradientButton.setBounds (getWidth() / 2 - w / 2, y, w, h);
        colourButton.setBounds (gradientButton.getX() - w, y, w, h);
        imageButton.setBounds (gradientButton.getRight(), y, w, h);

        colourPicker.setTopLeftPosition (2, y + h + 4);
        gradientPicker.setTopLeftPosition (2, y + h + 4);
        tilePicker.setTopLeftPosition (2, y + h + 4);
    }

    void buttonClicked (Button* b)
    {
        RelativePoint gp1, gp2;
        FillType currentFill (readFillType (&gp1, &gp2));

        if (b == &colourButton)
        {
            if (! currentFill.isColour())
                setFillType (colourPicker.getCurrentColour());
        }
        else if (b == &gradientButton)
        {
            if (! currentFill.isGradient())
            {
                // Use a cunning trick to make the wrapper dig out the earlier gradient settings, if there are any..
                FillType newFill (defaultGradient);
                ValueTree temp ("dummy");
                Drawable::ValueTreeWrapperBase::writeFillType (temp, newFill, 0, 0, 0, 0);

                fillState.setProperty (Drawable::ValueTreeWrapperBase::type, temp [Drawable::ValueTreeWrapperBase::type], undoManager);
                newFill = readFillType (&gp1, &gp2);

                if (newFill.gradient->getNumColours() <= 1)
                {
                    newFill = FillType (defaultGradient);
                    Drawable::ValueTreeWrapperBase::writeFillType (fillState, newFill, 0, 0, imageProvider, undoManager);
                }
                else
                {
                    Drawable::ValueTreeWrapperBase::writeFillType (fillState, newFill, &gp1, &gp2, imageProvider, undoManager);
                }

                refresh();
            }
        }
        else if (b == &imageButton)
        {
            if (! currentFill.isTiledImage())
                setFillType (FillType (StoredSettings::getInstance()->getFallbackImage(),
                                       AffineTransform::identity));
        }
    }

    const FillType readFillType (RelativePoint* gp1, RelativePoint* gp2) const
    {
        return Drawable::ValueTreeWrapperBase::readFillType (fillState, gp1, gp2, 0, imageProvider);
    }

    void setFillType (const FillType& newFill)
    {
        RelativePoint gp1, gp2;
        FillType currentFill (readFillType (&gp1, &gp2));

        if (currentFill != newFill)
        {
            if (undoManager != 0)
                undoManager->undoCurrentTransactionOnly();

            Drawable::ValueTreeWrapperBase::writeFillType (fillState, newFill, &gp1, &gp2, imageProvider, undoManager);
            refresh();
        }
    }

    void changeListenerCallback (void*)
    {
        const FillType currentFill (readFillType (0, 0));

        if (currentFill.isColour())
            setFillType (colourPicker.getCurrentColour());
        else if (currentFill.isGradient())
            setFillType (gradientPicker.getGradient());
        else if (currentFill.isTiledImage())
            setFillType (tilePicker.getFill());
    }

    void refresh()
    {
        FillType newFill (readFillType (0, 0));

        colourPicker.setVisible (newFill.isColour());
        gradientPicker.setVisible (newFill.isGradient());
        tilePicker.setVisible (newFill.isTiledImage());

        if (newFill.isColour())
        {
            setSize (getWidth(), colourPicker.getBottom() + 4);
            colourButton.setToggleState (true, false);
            colourPicker.setCurrentColour (newFill.colour);

        }
        else if (newFill.isGradient())
        {
            setSize (getWidth(), gradientPicker.getBottom() + 4);

            if (newFill.gradient->getNumColours() <= 1)
            {
                newFill = FillType (defaultGradient);
                Drawable::ValueTreeWrapperBase::writeFillType (fillState, newFill, 0, 0, imageProvider, undoManager);
            }

            gradientButton.setToggleState (true, false);
            gradientPicker.setGradient (*newFill.gradient);
        }
        else
        {
            setSize (getWidth(), tilePicker.getBottom() + 4);
            tilePicker.setFill (newFill);
            imageButton.setToggleState (true, false);
        }
    }

    void handleAsyncUpdate()                                                                            { refresh(); }
    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property)  { triggerAsyncUpdate(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)                                 { triggerAsyncUpdate(); }
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)                                  {}

private:
    //==============================================================================
    class GradientDesigner  : public Component,
                              public ChangeBroadcaster,
                              public ChangeListener,
                              public ButtonListener
    {
    public:
        GradientDesigner (const ColourGradient& gradient_)
            : gradient (gradient_),
              selectedPoint (-1),
              dragging (false),
              draggingNewPoint (false),
              draggingPos (0)
        {
            addChildComponent (&colourPicker);

            linearButton.setButtonText ("Linear");
            linearButton.setRadioGroupId (321);
            linearButton.setConnectedEdges (TextButton::ConnectedOnRight | TextButton::ConnectedOnLeft);
            radialButton.setButtonText ("Radial");
            radialButton.setRadioGroupId (321);
            radialButton.setConnectedEdges (TextButton::ConnectedOnRight | TextButton::ConnectedOnLeft);

            addAndMakeVisible (&linearButton);
            addAndMakeVisible (&radialButton);

            linearButton.addButtonListener (this);
            radialButton.addButtonListener (this);
            colourPicker.addChangeListener (this);
        }

        ~GradientDesigner()
        {
        }

        void paint (Graphics& g)
        {
            g.fillAll (getLookAndFeel().findColour (ColourSelector::backgroundColourId));

            g.fillCheckerBoard (previewArea.getX(), previewArea.getY(),
                                previewArea.getWidth(), previewArea.getHeight(), 10, 10,
                                Colour (0xffdddddd), Colour (0xffffffff));

            FillType f (gradient);
            f.gradient->point1.setXY ((float) previewArea.getX(), (float) previewArea.getCentreY());
            f.gradient->point2.setXY ((float) previewArea.getRight(), (float) previewArea.getCentreY());
            g.setFillType (f);
            g.fillRect (previewArea);

            Path marker;
            const float headSize = 4.5f;
            marker.addLineSegment (Line<float> (0.0f, -2.0f, 0.0f, previewArea.getHeight() + 2.0f), 1.5f);
            marker.addTriangle (0.0f, 1.0f, -headSize, -headSize, headSize, -headSize);

            for (int i = 0; i < gradient.getNumColours(); ++i)
            {
                const double pos = gradient.getColourPosition (i);
                const Colour col (gradient.getColour (i));

                const AffineTransform t (AffineTransform::translation (previewArea.getX() + 0.5f + (float) (previewArea.getWidth() * pos),
                                                                       (float) previewArea.getY()));

                g.setColour (Colours::black.withAlpha (0.8f));
                g.strokePath (marker, PathStrokeType (i == selectedPoint ? 2.0f : 1.5f), t);
                g.setColour (i == selectedPoint ? Colours::lightblue : Colours::white);
                g.fillPath (marker, t);
            }
        }

        void resized()
        {
            previewArea.setBounds (7, 35, getWidth() - 14, 24);

            const int w = 60;
            linearButton.setBounds (getWidth() / 2 - w, 2, w, 20);
            radialButton.setBounds (getWidth() / 2, 2, w, 20);

            colourPicker.setBounds (0, previewArea.getBottom() + 16,
                                    getWidth(), getHeight() - previewArea.getBottom() - 16);
        }

        void mouseDown (const MouseEvent& e)
        {
            dragging = false;
            draggingNewPoint = false;
            int point = getPointAt (e.x);

            if (point >= 0)
                setSelectedPoint (point);
        }

        void mouseDrag (const MouseEvent& e)
        {
            if ((! dragging) && ! e.mouseWasClicked())
            {
                preDragGradient = gradient;
                const int mouseDownPoint = getPointAt (e.getMouseDownX());

                if (mouseDownPoint >= 0)
                {
                    if (mouseDownPoint > 0 && mouseDownPoint < gradient.getNumColours() - 1)
                    {
                        dragging = true;
                        draggingNewPoint = false;
                        draggingColour = gradient.getColour (mouseDownPoint);
                        preDragGradient.removeColour (mouseDownPoint);
                        selectedPoint = -1;
                    }
                }
                else
                {
                    dragging = true;
                    draggingNewPoint = true;
                    selectedPoint = -1;
                }
            }

            if (dragging)
            {
                draggingPos = jlimit (0.001, 0.999, (e.x - previewArea.getX()) / (double) previewArea.getWidth());
                gradient = preDragGradient;

                if (previewArea.expanded (6, 6).contains (e.x, e.y))
                {
                    if (draggingNewPoint)
                        draggingColour = preDragGradient.getColourAtPosition (draggingPos);

                    selectedPoint = gradient.addColour (draggingPos, draggingColour);
                    updatePicker();
                }
                else
                {
                    selectedPoint = -1;
                }

                sendChangeMessage (this);
                repaint (previewArea.expanded (30, 30));
            }
        }

        void mouseUp (const MouseEvent& e)
        {
            dragging = false;
        }

        const ColourGradient& getGradient() const throw()   { return gradient; }

        void setGradient (const ColourGradient& newGradient)
        {
            if (newGradient != gradient || selectedPoint < 0)
            {
                jassert (newGradient.getNumColours() > 1);

                gradient = newGradient;

                if (selectedPoint < 0)
                    selectedPoint = 0;

                linearButton.setToggleState (! gradient.isRadial, false);
                radialButton.setToggleState (gradient.isRadial, false);

                updatePicker();
                sendChangeMessage (this);
                repaint();
            }
        }

        void setSelectedPoint (int newIndex)
        {
            if (selectedPoint != newIndex)
            {
                selectedPoint = newIndex;
                updatePicker();
                repaint();
            }
        }

        void changeListenerCallback (void*)
        {
            if (selectedPoint >= 0 && (! dragging) && gradient.getColour (selectedPoint) != colourPicker.getCurrentColour())
            {
                gradient.setColour (selectedPoint, colourPicker.getCurrentColour());
                repaint (previewArea);
                sendChangeMessage (this);
            }
        }

        void buttonClicked (Button* b)
        {
            ColourGradient g (gradient);
            g.isRadial = (b == &radialButton);
            setGradient (g);
        }

    private:
        StoredSettings::ColourSelectorWithSwatches colourPicker;
        ColourGradient gradient;
        int selectedPoint;
        bool dragging, draggingNewPoint;
        double draggingPos;
        Colour draggingColour;
        ColourGradient preDragGradient;

        Rectangle<int> previewArea;
        TextButton linearButton, radialButton;

        void updatePicker()
        {
            colourPicker.setVisible (selectedPoint >= 0);
            if (selectedPoint >= 0)
                colourPicker.setCurrentColour (gradient.getColour (selectedPoint));
        }

        int getPointAt (const int x) const
        {
            int best = -1;
            double bestDiff = 6;

            for (int i = gradient.getNumColours(); --i >= 0;)
            {
                const double pos = previewArea.getX() + previewArea.getWidth() * gradient.getColourPosition (i);
                const double diff = std::abs (pos - x);

                if (diff < bestDiff)
                {
                    bestDiff = diff;
                    best = i;
                }
            }

            return best;
        }
    };

    //==============================================================================
    class TiledFillDesigner  : public Component,
                               public ChangeBroadcaster,
                               public ComboBoxListener,
                               public SliderListener
    {
    public:
        TiledFillDesigner (Drawable::ImageProvider* imageProvider_, Project* project_)
            : imageProvider (imageProvider_), project (project_)
        {
            addAndMakeVisible (&imageBox);
            addAndMakeVisible (&opacitySlider);
            opacitySlider.setRange (0.0, 1.0, 0.001);

            sliderLabel.setText ("Opacity:", false);
            sliderLabel.setColour (Label::textColourId, Colours::white);
            sliderLabel.attachToComponent (&opacitySlider, false);

            OwnedArray<Project::Item> images;
            project->findAllImageItems (images);

            for (int i = 0; i < images.size(); ++i)
                imageBox.addItem (images.getUnchecked(i)->getName().toString(), i + 1);

            imageBox.setTextWhenNothingSelected ("Select an image...");

            opacitySlider.addListener (this);
            imageBox.addListener (this);
        }

        ~TiledFillDesigner()
        {
        }

        const FillType getFill() const
        {
            return fill;
        }

        void setFill (const FillType& newFill)
        {
            if (fill != newFill)
            {
                fill = newFill;

                OwnedArray<Project::Item> images;
                project->findAllImageItems (images);

                const String currentID (imageProvider->getIdentifierForImage (fill.image).toString());
                int idToSelect = -1;
                for (int i = 0; i < images.size(); ++i)
                {
                    if (images.getUnchecked(i)->getImageFileID() == currentID)
                    {
                        idToSelect = i + 1;
                        break;
                    }
                }

                imageBox.setSelectedId (idToSelect, true);
                opacitySlider.setValue (fill.getOpacity(), false, false);
            }
        }

        void resized()
        {
            imageBox.setBounds (20, 10, getWidth() - 40, 22);
            opacitySlider.setBounds (20, 60, getWidth() - 40, 22);
        }

        void sliderValueChanged (Slider* slider)
        {
            if (opacitySlider.getValue() != fill.getOpacity())
            {
                FillType f (fill);
                f.setOpacity ((float) opacitySlider.getValue());
                setFill (f);
                sendChangeMessage (this);
            }
        }

        void comboBoxChanged (ComboBox* comboBoxThatHasChanged)
        {
            OwnedArray<Project::Item> images;
            project->findAllImageItems (images);

            Project::Item* item = images [imageBox.getSelectedId() - 1];
            if (item != 0)
            {
                Image im (imageProvider->getImageForIdentifier (item->getImageFileID()));

                if (im.isValid() && im != fill.image)
                {
                    FillType f (fill);
                    f.image = im;
                    setFill (f);
                    sendChangeMessage (this);
                }
            }
        }

    private:
        FillType fill;
        Drawable::ImageProvider* imageProvider;
        Project* project;

        ComboBox imageBox;
        Slider opacitySlider;
        Label sliderLabel;
    };

    //==============================================================================
    FillTypeEditorComponent* owner;
    StoredSettings::ColourSelectorWithSwatches colourPicker;
    GradientDesigner gradientPicker;
    TiledFillDesigner tilePicker;
    ColourGradient defaultGradient;
    ValueTree fillState;
    Drawable::ImageProvider* imageProvider;
    UndoManager* undoManager;

    TextButton colourButton, gradientButton, imageButton;
};


//==============================================================================
/**
    A component that shows a fill type swatch, and pops up a editor panel
    when you click it.
*/
class FillTypeEditorComponent    : public Component,
                                   public ValueTree::Listener
{
public:
    FillTypeEditorComponent (const ValueTree& fillState_, Drawable::ImageProvider* imageProvider_,
                             Project* project_, UndoManager* undoManager_)
        : fillState (fillState_), undoManager (undoManager_),
          imageProvider (imageProvider_), project (project_)
    {
        fillState.addListener (this);
        refresh();
    }

    ~FillTypeEditorComponent()
    {
    }

    const ColourGradient getDefaultGradient() const;

    void paint (Graphics& g)
    {
        g.setColour (Colours::grey);
        g.drawRect (0, 0, getWidth(), getHeight(), 2);

        g.fillCheckerBoard (2, 2, getWidth() - 4, getHeight() - 4, 10, 10,
                            Colour (0xffdddddd), Colour (0xffffffff));

        FillType f (fillType);

        if (f.gradient != 0)
        {
            f.gradient->point1.setXY (2.0f, getHeight() / 2.0f);
            f.gradient->point2.setXY (getWidth() - 2.0f, getHeight() / 2.0f);
        }

        g.setFillType (f);
        g.fillRect (2, 2, getWidth() - 4, getHeight() - 4);

        if (fillType.isColour())
        {
            g.setColour (Colours::white.overlaidWith (fillType.colour).contrasting());
            g.setFont (getHeight() * 0.6f, Font::bold);
            g.drawFittedText (fillType.colour.toDisplayString (true),
                              2, 1, getWidth() - 4, getHeight() - 1,
                              Justification::centred, 1);
        }
    }

    void refresh()
    {
        const FillType newFill (Drawable::ValueTreeWrapperBase::readFillType (fillState, 0, 0, 0, imageProvider));

        if (newFill != fillType)
        {
            fillType = newFill;
            repaint();
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        undoManager->beginNewTransaction();

        PopupFillSelector popup (fillState, getDefaultGradient(), imageProvider, project, undoManager);
        CallOutBox::showModal (popup, this, 0 /*getTopLevelComponent()*/);
    }

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property)  { refresh(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)                                 { refresh(); }
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)                                  {}

    juce_UseDebuggingNewOperator

private:
    ValueTree fillState;
    Drawable::ImageProvider* imageProvider;
    UndoManager* undoManager;
    Project* project;
    FillType fillType;
};


//==============================================================================
class FillTypePropertyComponent  : public PropertyComponent
{
public:
    //==============================================================================
    FillTypePropertyComponent (UndoManager* undoManager, const String& name, const ValueTree& fill,
                               Drawable::ImageProvider* imageProvider, Project* project)
        : PropertyComponent (name),
          editor (fill, imageProvider, project, undoManager)
    {
        jassert (fill.isValid());
        addAndMakeVisible (&editor);
    }

    ~FillTypePropertyComponent()
    {
    }

    void resized()
    {
        editor.setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
    }

    virtual const ColourGradient getDefaultGradient() = 0;

    void refresh() {}

protected:
    FillTypeEditorComponent editor;
};


#endif  // __JUCER_FILLTYPEPROPERTYCOMPONENT_H_88CF1300__
