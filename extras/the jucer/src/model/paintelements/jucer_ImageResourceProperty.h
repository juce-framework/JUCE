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

#ifndef __JUCER_IMAGERESOURCEPROPERTY_JUCEHEADER__
#define __JUCER_IMAGERESOURCEPROPERTY_JUCEHEADER__


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
    ImageResourceProperty (JucerDocument& document_,
                           ElementType* const element_,
                           const String& name,
                           const bool allowChoiceOfNoResource_)
        : ChoicePropertyComponent (name),
          element (element_),
          document (document_),
          allowChoiceOfNoResource (allowChoiceOfNoResource_)
    {
        choices.add (T("-- create a new image resource -- "));
        choices.add (String::empty);
        if (allowChoiceOfNoResource_)
            choices.add (T("<< none >>"));
        choices.addArray (document_.getResources().getResourceNames());

        document_.addChangeListener (this);
    }

    ImageResourceProperty (ElementType* const element_,
                           const String& name,
                           const bool allowChoiceOfNoResource_ = false)
        : ChoicePropertyComponent (name),
          element (element_),
          document (*element_->getDocument()),
          allowChoiceOfNoResource (allowChoiceOfNoResource_)
    {
        choices.add (T("-- create a new image resource -- "));
        choices.add (String::empty);
        if (allowChoiceOfNoResource_)
            choices.add (T("<< none >>"));

        choices.addArray (document.getResources().getResourceNames());

        document.addChangeListener (this);
    }

    ~ImageResourceProperty()
    {
        document.removeChangeListener (this);
    }

    //==============================================================================
    virtual void setResource (const String& newName) = 0;

    virtual const String getResource() const = 0;

    //==============================================================================
    void setIndex (const int newIndex)
    {
        if (newIndex == 0)
        {
            String resource (document.getResources()
                     .browseForResource (T("Select an image file to add as a resource"),
                                         T("*.jpg;*.jpeg;*.png;*.gif;*.svg"),
                                         File::nonexistent,
                                         String::empty));

            if (resource.isNotEmpty())
                setResource (resource);
        }
        else
        {
            if (choices[newIndex] == T("<< none >>") && allowChoiceOfNoResource)
                setResource (String::empty);
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

    void changeListenerCallback (void*)
    {
        refresh();
    }

protected:
    ElementType* const element;
    JucerDocument& document;
    const bool allowChoiceOfNoResource;
};


#endif   // __JUCER_IMAGERESOURCEPROPERTY_JUCEHEADER__
