/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_OPTIONALSCOPEDPOINTER_JUCEHEADER__
#define __JUCE_OPTIONALSCOPEDPOINTER_JUCEHEADER__

#include "juce_ScopedPointer.h"


//==============================================================================
/**
    Holds a pointer to an object which can optionally be deleted when this pointer
    goes out of scope.

    This acts in many ways like a ScopedPointer, but allows you to specify whether or
    not the object is deleted.

    @see ScopedPointer
*/
template <class ObjectType>
class OptionalScopedPointer
{
public:
    //==============================================================================
    /** Creates an empty OptionalScopedPointer. */
    OptionalScopedPointer() : shouldDelete (false) {}

    /** Creates an OptionalScopedPointer to point to a given object, and specifying whether
        the OptionalScopedPointer will delete it.

        If takeOwnership is true, then the OptionalScopedPointer will act like a ScopedPointer,
        deleting the object when it is itself deleted. If this parameter is false, then the
        OptionalScopedPointer just holds a normal pointer to the object, and won't delete it.
    */
    OptionalScopedPointer (ObjectType* objectToHold, bool takeOwnership)
        : object (objectToHold), shouldDelete (takeOwnership)
    {
    }

    /** Takes ownership of the object that another OptionalScopedPointer holds.

        Like a normal ScopedPointer, the objectToTransferFrom object will become null,
        as ownership of the managed object is transferred to this object.

        The flag to indicate whether or not to delete the managed object is also
        copied from the source object.
    */
    OptionalScopedPointer (OptionalScopedPointer& objectToTransferFrom)
        : object (objectToTransferFrom.release()),
          shouldDelete (objectToTransferFrom.shouldDelete)
    {
    }

    /** Takes ownership of the object that another OptionalScopedPointer holds.

        Like a normal ScopedPointer, the objectToTransferFrom object will become null,
        as ownership of the managed object is transferred to this object.

        The ownership flag that says whether or not to delete the managed object is also
        copied from the source object.
    */
    OptionalScopedPointer& operator= (OptionalScopedPointer& objectToTransferFrom)
    {
        if (object != objectToTransferFrom.object)
        {
            clear();
            object = objectToTransferFrom.object;
        }

        shouldDelete = objectToTransferFrom.shouldDelete;
        return *this;
    }

    /** The destructor may or may not delete the object that is being held, depending on the
        takeOwnership flag that was specified when the object was first passed into an
        OptionalScopedPointer constructor.
    */
    ~OptionalScopedPointer()
    {
        clear();
    }

    //==============================================================================
    /** Returns the object that this pointer is managing. */
    inline operator ObjectType*() const noexcept                    { return object; }

    /** Returns the object that this pointer is managing. */
    inline ObjectType* get() const noexcept                         { return object; }

    /** Returns the object that this pointer is managing. */
    inline ObjectType& operator*() const noexcept                   { return *object; }

    /** Lets you access methods and properties of the object that this pointer is holding. */
    inline ObjectType* operator->() const noexcept                  { return object; }

    //==============================================================================
    /** Removes the current object from this OptionalScopedPointer without deleting it.
        This will return the current object, and set this OptionalScopedPointer to a null pointer.
    */
    ObjectType* release() noexcept                                  { return object.release(); }

    /** Resets this pointer to null, possibly deleting the object that it holds, if it has
        ownership of it.
    */
    void clear()
    {
        if (! shouldDelete)
            object.release();
    }

    /** Makes this OptionalScopedPointer point at a new object, specifying whether the
        OptionalScopedPointer will take ownership of the object.

        If takeOwnership is true, then the OptionalScopedPointer will act like a ScopedPointer,
        deleting the object when it is itself deleted. If this parameter is false, then the
        OptionalScopedPointer just holds a normal pointer to the object, and won't delete it.
    */
    void set (ObjectType* newObject, bool takeOwnership)
    {
        if (object != newObject)
        {
            clear();
            object = newObject;
        }

        shouldDelete = takeOwnership;
    }

    /** Makes this OptionalScopedPointer point at a new object, and take ownership of that object. */
    void setOwned (ObjectType* newObject)
    {
        set (newObject, true);
    }

    /** Makes this OptionalScopedPointer point at a new object, but will not take ownership of that object. */
    void setNonOwned (ObjectType* newObject)
    {
        set (newObject, false);
    }

    /** Returns true if the target object will be deleted when this pointer
        object is deleted.
    */
    bool willDeleteObject() const noexcept                          { return shouldDelete; }

    //==============================================================================
    /** Swaps this object with another OptionalScopedPointer.
        The two objects simply exchange their states.
    */
    void swapWith (OptionalScopedPointer<ObjectType>& other) noexcept
    {
        object.swapWith (other.object);
        std::swap (shouldDelete, other.shouldDelete);
    }

private:
    //==============================================================================
    ScopedPointer<ObjectType> object;
    bool shouldDelete;
};


#endif   // __JUCE_OPTIONALSCOPEDPOINTER_JUCEHEADER__
