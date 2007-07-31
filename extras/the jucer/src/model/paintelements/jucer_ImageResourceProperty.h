/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
    ImageResourceProperty (ElementType* const element_, const String& name)
        : ChoicePropertyComponent (name),
          element (element_)
    {
        choices.add (T("-- create a new image resource -- "));
        choices.add (String::empty);
        choices.addArray (element_->getDocument()->getResources().getResourceNames());

        element_->getDocument()->addChangeListener (this);
    }

    ~ImageResourceProperty()
    {
        element->getDocument()->removeChangeListener (this);
    }

    //==============================================================================
    virtual void setResource (const String& newName) = 0;

    virtual const String getResource() const = 0;

    //==============================================================================
    void setIndex (const int newIndex)
    {
        if (newIndex == 0)
        {
            String resource (element->getDocument()->getResources()
                     .browseForResource (T("Select an image file to add as a resource"),
                                         T("*.jpg;*.jpeg;*.png;*.gif"),
                                         File::nonexistent,
                                         String::empty));

            if (resource.isNotEmpty())
                setResource (resource);
        }
        else
        {
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
};


#endif   // __JUCER_IMAGERESOURCEPROPERTY_JUCEHEADER__
