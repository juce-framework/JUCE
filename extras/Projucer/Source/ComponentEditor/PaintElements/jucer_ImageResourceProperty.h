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

#include "../../ProjectSaving/jucer_ResourceFile.h"

//==============================================================================
/**
    A property that lets you pick a resource to use as an image, or create a
    new one with a file selector.
*/
template <class ElementType>
class ImageResourceProperty    : public ChoicePropertyComponent,
                                 private ChangeListener
{
public:
    ImageResourceProperty (JucerDocument& doc,
                           ElementType* const e,
                           const String& name,
                           const bool allowChoiceOfNoResource_)
        : ChoicePropertyComponent (name),
          element (e), document (doc),
          allowChoiceOfNoResource (allowChoiceOfNoResource_)
    {
        refreshChoices();
        doc.addChangeListener (this);
    }

    ImageResourceProperty (ElementType* const e, const String& name,
                           const bool allowChoiceOfNoResource_ = false)
        : ChoicePropertyComponent (name),
          element (e), document (*e->getDocument()),
          allowChoiceOfNoResource (allowChoiceOfNoResource_)
    {
        refreshChoices();
        document.addChangeListener (this);
    }

    ~ImageResourceProperty()
    {
        document.removeChangeListener (this);
    }

    //==============================================================================
    virtual void setResource (const String& newName) = 0;

    virtual String getResource() const = 0;

    //==============================================================================
    void setIndex (int newIndex)
    {
        if (newIndex == 0)
        {
            String resource (document.getResources()
                     .browseForResource ("Select an image file to add as a resource",
                                         "*.jpg;*.jpeg;*.png;*.gif;*.svg",
                                         File(),
                                         String()));

            if (resource.isNotEmpty())
                setResource (resource);
        }
        else
        {
            if (choices[newIndex] == getNoneText() && allowChoiceOfNoResource)
                setResource (String());
            else
                setResource (choices [newIndex]);
        }
    }

    int getIndex() const
    {
        if (getResource().isEmpty())
            return -1;

        return choices.indexOf (getResource());
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

    void refreshChoices()
    {
        choices.clear();

        choices.add ("-- create a new image resource -- ");
        choices.add (String());

        if (allowChoiceOfNoResource)
            choices.add (getNoneText());

        choices.addArray (document.getResources().getResourceNames());

        const SourceCodeDocument& cpp = document.getCppDocument();

        if (Project* project = cpp.getProject())
        {
            ResourceFile resourceFile (*project);

            for (int i = 0; i < resourceFile.getNumFiles(); ++i)
            {
                const File& file = resourceFile.getFile(i);

                if (ImageFileFormat::findImageFormatForFileExtension(file))
                    choices.add (resourceFile.getClassName() + "::" + resourceFile.getDataVariableFor (file));
            }
        }
    }

    const char* getNoneText() noexcept      { return "<< none >>"; }

protected:
    mutable Component::SafePointer<ElementType> element;
    JucerDocument& document;
    const bool allowChoiceOfNoResource;
};
