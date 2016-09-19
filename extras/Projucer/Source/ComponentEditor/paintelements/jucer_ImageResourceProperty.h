/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_IMAGERESOURCEPROPERTY_H_INCLUDED
#define JUCER_IMAGERESOURCEPROPERTY_H_INCLUDED

#include "../../Project Saving/jucer_ResourceFile.h"

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


#endif   // JUCER_IMAGERESOURCEPROPERTY_H_INCLUDED
