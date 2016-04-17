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

#ifndef JUCE_OSCBUNDLE_H_INCLUDED
#define JUCE_OSCBUNDLE_H_INCLUDED

//==============================================================================
/**
    An OSC bundle.

    An OSCBundle contains an OSCTimeTag and zero or more OSCBundle Elements.
    The elements of a bundle can be OSC messages or other OSC bundles (this
    means that OSC bundles can be nested).

    This is an advanced OSC structure useful to bundle OSC messages together
    whose effects must occur simultaneously at some given time. For most
    use cases it is probably enough to send and receive plain OSC messages.
*/
class JUCE_API  OSCBundle
{
public:
    //==============================================================================
    /** Constructs an OSCBundle with no content and a default time tag ("immediately"). */
    OSCBundle();

    /** Constructs an OSCBundle with no content and a given time tag. */
    OSCBundle (OSCTimeTag timeTag);

    //==============================================================================
    /** Sets the OSCBundle's OSC time tag. */
    void setTimeTag (OSCTimeTag newTimeTag) noexcept        { timeTag = newTimeTag; }

    /** Returns the OSCBundle's OSC time tag. */
    OSCTimeTag getTimeTag() const noexcept                  { return timeTag; }

    //==============================================================================
    /**
        An OSC bundle element.

        An OSCBundle Element contains either one OSCMessage or one OSCBundle.
     */
    class Element
    {
    public:
        //==============================================================================
        /** Constructs an OSCBundle Element from an OSCMessage. */
        Element (OSCMessage message);

        /** Constructs an OSCBundle Element from an OSCBundle. */
        Element (OSCBundle bundle);

        /** Copy constructor. */
        Element (const Element& other);

        /** Destructor. */
        ~Element();

        /** Returns true if the OSCBundle element is an OSCMessage. */
        bool isMessage() const noexcept;

        /** Returns true if the OSCBundle element is an OSCBundle. */
        bool isBundle() const noexcept;

        /** Returns a reference to the contained OSCMessage.
            If the OSCBundle element is not an OSCMessage, behaviour is undefined.
         */
        const OSCMessage& getMessage() const;

        /** Returns a reference to the contained OSCBundle.
            If the OSCBundle element is not an OSCBundle, behaviour is undefined.
        */
        const OSCBundle& getBundle() const;

    private:
        //==============================================================================
        ScopedPointer<OSCMessage> message;
        ScopedPointer<OSCBundle> bundle;
    };

    //==============================================================================
    /** Returns the number of elements contained in the bundle. */
    int size() const noexcept                                    { return elements.size(); }

    /** Returns true if the bundle contains no elements; false otherwise. */
    bool isEmpty() const noexcept                                { return elements.isEmpty(); }

    /** Returns a reference to the OSCBundle element at index i in this bundle.
        This method does not check the range and results in undefined behaviour
        in case i < 0 or i >= size().
    */
    OSCBundle::Element& operator[] (const int i) const noexcept
    {
        return elements.getReference (i);
    }

    /** Adds an OSCBundleElement to the OSCBundle's content. s*/
    void addElement (const OSCBundle::Element& element)          { elements.add (element); }

    /** Returns a pointer to the first element of the OSCBundle. */
    OSCBundle::Element* begin() const noexcept                   { return elements.begin(); }

    /** Returns a pointer past the last element of the OSCBundle. */
    OSCBundle::Element* end() const noexcept                     { return elements.end(); }

private:
    //==============================================================================
    Array<OSCBundle::Element> elements;
    OSCTimeTag timeTag;
};


#endif // JUCE_OSCBUNDLE_H_INCLUDED
