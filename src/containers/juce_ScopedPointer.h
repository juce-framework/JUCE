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

#ifndef __JUCE_SCOPEDPOINTER_JUCEHEADER__
#define __JUCE_SCOPEDPOINTER_JUCEHEADER__


//==============================================================================
/**
    This class holds a pointer which is automatically deleted when this object goes
    out of scope.

    Once a pointer has been passed to a ScopedPointer, it will make sure that the pointer
    gets deleted when the ScopedPointer is deleted. Using the ScopedPointer on the stack or
    as member variables is a good way to use RAII to avoid accidentally leaking dynamically
    created objects.

    A ScopedPointer can be used in pretty much the same way that you'd use a normal pointer
    to an object. If you use the assignment operator to assign a different object to a
    ScopedPointer, the old one will be automatically deleted.

    If you need to get a pointer out of a ScopedPointer without it being deleted, you
    can use the release() method.
*/
template <class ObjectType>
class JUCE_API  ScopedPointer
{
public:
    //==============================================================================
    /** Creates a ScopedPointer containing a null pointer. */
    inline ScopedPointer()  : object (0)
    {
    }

    /** Creates a ScopedPointer that owns the specified object. */
    inline ScopedPointer (ObjectType* const objectToTakePossessionOf)
        : object (objectToTakePossessionOf)
    {
    }

    /** Creates a ScopedPointer that takes its pointer from another ScopedPointer.

        Because a pointer can only belong to one ScopedPointer, this transfers
        the pointer from the other object to this one, and the other object is reset to
        be a null pointer.
    */
    ScopedPointer (ScopedPointer& objectToTransferFrom)
        : object (objectToTransferFrom.object)
    {
        objectToTransferFrom.object = 0;
    }

    /** Destructor.
        This will delete the object that this ScopedPointer currently refers to.
    */
    inline ~ScopedPointer()                                                 { delete object; }

    /** Changes this ScopedPointer to point to a new object.

        Because a pointer can only belong to one ScopedPointer, this transfers
        the pointer from the other object to this one, and the other object is reset to
        be a null pointer.

        If this ScopedPointer already points to an object, that object
        will first be deleted.
    */
    const ScopedPointer& operator= (ScopedPointer& objectToTransferFrom)
    {
        if (this != objectToTransferFrom.getAddress())
        {
            // Two ScopedPointers should never be able to refer to the same object - if
            // this happens, you must have done something dodgy!
            jassert (object != objectToTransferFrom.object);

            ObjectType* const oldObject = object;
            object = objectToTransferFrom.object;
            objectToTransferFrom.object = 0;
            delete oldObject;
        }

        return *this;
    }

    /** Changes this ScopedPointer to point to a new object.

        If this ScopedPointer already points to an object, that object
        will first be deleted.

        The pointer that you pass is may be null.
    */
    const ScopedPointer& operator= (ObjectType* const newObjectToTakePossessionOf)
    {
        if (object != newObjectToTakePossessionOf)
        {
            ObjectType* const oldObject = object;
            object = newObjectToTakePossessionOf;
            delete oldObject;
        }

        return *this;
    }

    //==============================================================================
    /** Returns the object that this ScopedPointer refers to.
    */
    inline operator ObjectType*() const                                     { return object; }

    /** Returns the object that this ScopedPointer refers to.
    */
    inline ObjectType& operator*() const                                    { return *object; }

    /** Lets you access methods and properties of the object that this ScopedPointer refers to. */
    inline ObjectType* operator->() const                                   { return object; }

    /** Returns a pointer to the object by casting it to whatever type you need. */
    template <class CastType>
    inline operator CastType*() const                                       { return static_cast <CastType*> (object); }

    /** Returns a reference to the address of the object that this ScopedPointer refers to. */
    inline ObjectType** operator&() const                                   { return (ObjectType**) &object; }

    //==============================================================================
    /** Removes the current object from this ScopedPointer without deleting it.

        This will return the current object, and set the ScopedPointer to a null pointer.
    */
    ObjectType* release()                                                   { ObjectType* const o = object; object = 0; return o; }

    //==============================================================================
    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator== (const ObjectType* const otherPointer) const     { return otherPointer == object; }

    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator!= (const ObjectType* const otherPointer) const     { return otherPointer != object; }

    //==============================================================================
    /** Swaps this object with that of another ScopedPointer.
        The two objects simply exchange their pointers.
    */
    void swapWith (ScopedPointer <ObjectType>& other)
    {
        // Two ScopedPointers should never be able to refer to the same object - if
        // this happens, you must have done something dodgy!
        jassert (object != other.object);

        swapVariables (object, other.object);
    }

private:
    //==============================================================================
    ObjectType* object;

    // (Required as an alternative to the overloaded & operator).
    ScopedPointer* getAddress()                                             { return this; }
};


#endif   // __JUCE_SCOPEDPOINTER_JUCEHEADER__
