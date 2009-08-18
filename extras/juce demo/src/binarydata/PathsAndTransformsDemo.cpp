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

#include "../jucedemo_headers.h"


//==============================================================================
static float randomNumber()
{
    return Random::getSystemRandom().nextFloat() * 300.0f - 150.0f;
}


//==============================================================================
class PathsAndTransformsDemo  : public Component,
                                public SliderListener,
                                public ComboBoxListener
{
public:
    //==============================================================================
    PathsAndTransformsDemo()
    {
        setName (T("Paths"));

        // No parts of this component are semi-transparent, so calling setOpaque()
        // allows the redraw system to exploit this fact and optimise repainting.
        setOpaque (true);

        generateRandomShape();
        generateImage();
        generateDrawable();
        generateSVGDrawable();

        addAndMakeVisible (typeChooser    = new ComboBox (T("type")));
        addAndMakeVisible (scaleSlider    = new Slider (T("scale")));
        addAndMakeVisible (angleSlider    = new Slider (T("angle")));
        addAndMakeVisible (xSlider        = new Slider (T("x")));
        addAndMakeVisible (ySlider        = new Slider (T("y")));
        addAndMakeVisible (opacitySlider  = new Slider (T("opacity")));

        (new Label (String::empty, T("type:")))     ->attachToComponent (typeChooser, true);
        (new Label (String::empty, T("scale:")))    ->attachToComponent (scaleSlider, true);
        (new Label (String::empty, T("angle:")))    ->attachToComponent (angleSlider, true);
        (new Label (String::empty, T("x offset:"))) ->attachToComponent (xSlider, true);
        (new Label (String::empty, T("y offset:"))) ->attachToComponent (ySlider, true);
        (new Label (String::empty, T("opacity:")))  ->attachToComponent (opacitySlider, true);

        typeChooser->addItem (T("random shape - solid colour"), 1);
        typeChooser->addItem (T("random shape - linear gradient fill"), 2);
        typeChooser->addItem (T("random shape - radial gradient fill"), 3);
        typeChooser->addItem (T("random shape - tiled image fill"), 8);
        typeChooser->addItem (T("image - low quality"), 4);
        typeChooser->addItem (T("image - high quality"), 5);
        typeChooser->addItem (T("image - colour-filled alpha channel"), 6);
        typeChooser->addItem (T("image - gradient-filled alpha channel"), 7);
        typeChooser->addItem (T("image - alphamap-filled alpha channel"), 9);
        typeChooser->addItem (T("drawable object"), 10);
        typeChooser->addItem (T("SVG object"), 11);
        typeChooser->setSelectedId (11);
        typeChooser->addListener (this);

        scaleSlider   ->addListener (this);
        angleSlider   ->addListener (this);
        xSlider       ->addListener (this);
        ySlider       ->addListener (this);
        opacitySlider ->addListener (this);

        scaleSlider->setRange (0.01, 10.0, 0.001);
        scaleSlider->setValue (1.0);

        angleSlider->setRange (-1.0, 1.0, 0.001);
        angleSlider->setValue (0);

        xSlider->setRange (-10, 10, 0.001);
        xSlider->setValue (0);

        ySlider->setRange (-10, 10, 0.001);
        ySlider->setValue (0);

        opacitySlider->setRange (0, 1, 0.01);
        opacitySlider->setValue (1.0);
    }

    ~PathsAndTransformsDemo()
    {
        if (image != 0)
            delete image;

        delete drawable;
        delete svgDrawable;

        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
        g.fillCheckerBoard (0, 0, getWidth(), getHeight(),
                            50, 50,
                            Colour (0xffe0e0e0),
                            Colours::white);

        const int type = typeChooser->getSelectedId();

        if (type == 1)
        {
            g.setColour (Colours::blue.withAlpha ((float) opacitySlider->getValue()));
            g.fillPath (shape, getTransform());
        }
        else if (type == 2 || type == 3)
        {
            GradientBrush gb (Colours::blue.withAlpha ((float) opacitySlider->getValue()),
                              getWidth() * 0.5f, getHeight() * 0.5f,
                              Colours::red.withAlpha ((float) opacitySlider->getValue()),
                              getWidth() * 0.6f, getHeight() * 0.7f,
                              type == 3);

            g.setBrush (&gb);
            g.fillPath (shape, getTransform());
        }
        else if (type == 8)
        {
            ImageBrush ib (image, 100, 100, (float) opacitySlider->getValue());

            g.setBrush (&ib);
            g.fillPath (shape, getTransform());
        }
        else if (type == 4 || type == 5)
        {
            if (type == 4)
                g.setImageResamplingQuality (Graphics::lowResamplingQuality);
            else
                g.setImageResamplingQuality (Graphics::mediumResamplingQuality);

            g.setOpacity ((float) opacitySlider->getValue());

            if (image != 0)
            {
                g.drawImageTransformed (image,
                                        0, 0, image->getWidth(), image->getHeight(),
                                        AffineTransform::translation (-0.5f * image->getWidth(), -0.5f * image->getHeight())
                                            .followedBy (getTransform()),
                                        false);
            }
        }
        else if (type == 6)
        {
            g.setColour (Colours::blue.withAlpha ((float) opacitySlider->getValue()));

            if (image != 0)
            {
                g.drawImageTransformed (image,
                                        0, 0, image->getWidth(), image->getHeight(),
                                        AffineTransform::translation (-0.5f * image->getWidth(), -0.5f * image->getHeight())
                                            .followedBy (getTransform()),
                                        true);
            }
        }
        else if (type == 7)
        {
            GradientBrush gb (Colours::blue.withAlpha ((float) opacitySlider->getValue()),
                              getWidth() * 0.5f, getHeight() * 0.5f,
                              Colours::red.withAlpha ((float) opacitySlider->getValue()),
                              getWidth() * 0.6f, getHeight() * 0.7f,
                              false);

            g.setBrush (&gb);

            if (image != 0)
            {
                g.drawImageTransformed (image,
                                        0, 0, image->getWidth(), image->getHeight(),
                                        AffineTransform::translation (-0.5f * image->getWidth(), -0.5f * image->getHeight())
                                            .followedBy (getTransform()),
                                        true);
            }
        }
        else if (type == 9)
        {
            ImageBrush ib (image, 100, 100, (float) opacitySlider->getValue());
            g.setBrush (&ib);

            if (image != 0)
            {
                g.drawImageTransformed (image,
                                        0, 0, image->getWidth(), image->getHeight(),
                                        AffineTransform::translation (-0.5f * image->getWidth(),
                                                                      -0.5f * image->getHeight())
                                            .followedBy (getTransform()),
                                        true);
            }
        }
        else if (type == 10)
        {
            g.setOpacity ((float) opacitySlider->getValue());

            float x, y, w, h;
            drawable->getBounds (x, y, w, h);

            drawable->draw (g, AffineTransform::translation (-x - 0.5f * w,
                                                             -y - 0.5f * h)
                                .followedBy (getTransform()));
        }
        else if (type == 11)
        {
            g.setOpacity ((float) opacitySlider->getValue());

            float x, y, w, h;
            svgDrawable->getBounds (x, y, w, h);

            svgDrawable->draw (g, AffineTransform::translation (-x - 0.5f * w,
                                                                -y - 0.5f * h)
                                   .followedBy (getTransform()));
        }
    }

    void resized()
    {
        const int x = 100;
        int y = 4;
        typeChooser->setBounds (x, y, 300, 24);
        y += 28;
        scaleSlider->setBounds (x, y, 300, 24);
        y += 28;
        angleSlider->setBounds (x, y, 300, 24);
        y += 28;
        xSlider->setBounds (x, y, 300, 24);
        y += 28;
        ySlider->setBounds (x, y, 300, 24);
        y += 28;
        opacitySlider->setBounds (x, y, 300, 24);
    }

    void sliderValueChanged (Slider*)
    {
        repaint();
    }

    void comboBoxChanged (ComboBox*)
    {
        repaint();
    }

private:
    Path shape;
    Image* image;
    Drawable* drawable;
    DrawableComposite* svgDrawable;

    ComboBox* typeChooser;
    Slider* scaleSlider;
    Slider* angleSlider;
    Slider* xSlider;
    Slider* ySlider;
    Slider* opacitySlider;

    void generateRandomShape()
    {
        shape.startNewSubPath (randomNumber(), randomNumber());

        for (int i = 0; i < 7; ++i)
        {
            shape.lineTo (randomNumber(), randomNumber());

            shape.quadraticTo (randomNumber(), randomNumber(),
                               randomNumber(), randomNumber());
        }

        shape.closeSubPath();
    }

    void generateImage()
    {
        image = ImageFileFormat::loadFrom (BinaryData::juce_png, BinaryData::juce_pngSize);
    }

    void generateDrawable()
    {
        // create a composite drawable object..
        DrawableComposite* dc = new DrawableComposite();
        drawable = dc;

        // ..add a paths drawable to it...
        DrawablePath dp;
        dp.setPath (shape);

        dp.setOutline (4.0f, Colours::blue);

        GradientBrush gb (ColourGradient (Colours::red.withAlpha (0.4f), -100.0f, -100.0f,
                                          Colours::green.withAlpha (0.6f), 100.0f, 100.0f, false));

        dp.setFillBrush (gb);

        dc->insertDrawable (dp);

        // ..add an image drawable..
        DrawableImage di;
        di.setImage (image, false);

        dc->insertDrawable (di, AffineTransform::scale (0.3f, 0.8f));

        // ..and a text object
        DrawableText dt;
        dt.setText (T("JUCE Drawables"), Font (30.0f, Font::bold));
        dt.setColour (Colours::green);

        dc->insertDrawable (dt, AffineTransform::translation (-80.0f, -20.0f)
                                    .scaled (2.0f, 0.8f)
                                    .rotated (-1.3f));
    }

    void generateSVGDrawable()
    {
        svgDrawable = 0;

        MemoryInputStream iconsFileStream (BinaryData::icons_zip, BinaryData::icons_zipSize, false);
        ZipFile icons (&iconsFileStream, false);

        // Load a random SVG file from our embedded icons.zip file.
        InputStream* svgFileStream
            = icons.createStreamForEntry (Random::getSystemRandom().nextInt (icons.getNumEntries()));

        if (svgFileStream != 0)
        {
            Drawable* loadedSVG = Drawable::createFromImageDataStream (*svgFileStream);

            if (loadedSVG != 0)
            {
                // to make our icon the right size, we'll put it inside a DrawableComposite, and apply
                // a transform to get it to the size we want.

                float x, y, w, h;
                loadedSVG->getBounds (x, y, w, h);
                const float scaleFactor = 300.0f / jmax (w, h);

                svgDrawable = new DrawableComposite();
                svgDrawable->insertDrawable (loadedSVG, AffineTransform::scale (scaleFactor, scaleFactor));
            }

            delete svgFileStream;
        }
    }

    const AffineTransform getTransform() const
    {
        return AffineTransform::rotation (float_Pi * 2.0f * (float) angleSlider->getValue())
                .scaled ((float) scaleSlider->getValue(),
                         (float) scaleSlider->getValue())
                .translated (getWidth() * 0.5f + (float) xSlider->getValue(),
                             getHeight() * 0.5f + (float) ySlider->getValue());
    }
};

Component* createPathsAndTransformsDemo()
{
    return new PathsAndTransformsDemo();
}
