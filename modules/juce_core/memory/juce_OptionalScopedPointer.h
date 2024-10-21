/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Holds a pointer to an object which can optionally be deleted when this pointer
    goes out of scope.

    This acts in many ways like a std::unique_ptr, but allows you to specify whether or
    not the object is deleted.

    @tags{Core}
*/
template <class ObjectType>
class OptionalScopedPointer
{
public:
    //==============================================================================
    /** Creates an empty OptionalScopedPointer. */
    OptionalScopedPointer() = default;

    /** Creates an OptionalScopedPointer to point to a given object, and specifying whether
        the OptionalScopedPointer will delete it.

        If takeOwnership is true, then the OptionalScopedPointer will act like a std::unique_ptr,
        deleting the object when it is itself deleted. If this parameter is false, then the
        OptionalScopedPointer just holds a normal pointer to the object, and won't delete it.
    */
    OptionalScopedPointer (ObjectType* objectToHold, bool takeOwnership)
        : object (objectToHold),
          shouldDelete (takeOwnership)
    {
    }

    /** Takes ownership of the object that another OptionalScopedPointer holds.

        Like a normal std::unique_ptr, the objectToTransferFrom object will become null,
        as ownership of the managed object is transferred to this object.

        The flag to indicate whether or not to delete the managed object is also
        copied from the source object.
    */
    OptionalScopedPointer (OptionalScopedPointer&& other) noexcept
        : object (std::move (other.object)),
          shouldDelete (std::move (other.shouldDelete))
    {
    }

    /** Takes ownership of the object owned by `ptr`. */
    explicit OptionalScopedPointer (std::unique_ptr<ObjectType>&& ptr) noexcept
        : OptionalScopedPointer (ptr.release(), true)
    {
    }

    /** Points to the same object as `ref`, but does not take ownership. */
    explicit OptionalScopedPointer (ObjectType& ref) noexcept
        : OptionalScopedPointer (std::addressof (ref), false)
    {
    }

    /** Takes ownership of the object that another OptionalScopedPointer holds.

        Like a normal std::unique_ptr, the objectToTransferFrom object will become null,
        as ownership of the managed object is transferred to this object.

        The ownership flag that says whether or not to delete the managed object is also
        copied from the source object.
    */
    OptionalScopedPointer& operator= (OptionalScopedPointer&& other) noexcept
    {
        swapWith (other);
        other.reset();
        return *this;
    }

    /** The destructor may or may not delete the object that is being held, depending on the
        takeOwnership flag that was specified when the object was first passed into an
        OptionalScopedPointer constructor.
    */
    ~OptionalScopedPointer() noexcept
    {
        reset();
    }

    //==============================================================================
    /** Returns the object that this pointer is managing. */
    operator ObjectType*() const noexcept                    { return object.get(); }

    /** Returns the object that this pointer is managing. */
    ObjectType* get() const noexcept                         { return object.get(); }

    /** Returns the object that this pointer is managing. */
    ObjectType& operator*() const noexcept                   { return *object; }

    /** Lets you access methods and properties of the object that this pointer is holding. */
    ObjectType* operator->() const noexcept                  { return object.get(); }

    //==============================================================================
    /** Removes the current object from this OptionalScopedPointer without deleting it.
        This will return the current object, and set this OptionalScopedPointer to a null pointer.
    */
    ObjectType* release() noexcept                                  { return object.release(); }

    /** Resets this pointer to null, possibly deleting the object that it holds, if it has
        ownership of it.
    */
    void reset() noexcept
    {
        if (! shouldDelete)
            object.release();
        else
            object.reset();
    }

    /** Does the same thing as reset(). */
    void clear()                                                    { reset(); }

    /** Makes this OptionalScopedPointer point at a new object, specifying whether the
        OptionalScopedPointer will take ownership of the object.

        If takeOwnership is true, then the OptionalScopedPointer will act like a std::unique_ptr,
        deleting the object when it is itself deleted. If this parameter is false, then the
        OptionalScopedPointer just holds a normal pointer to the object, and won't delete it.
    */
    void set (ObjectType* newObject, bool takeOwnership)
    {
        if (object.get() != newObject)
        {
            reset();
            object.reset (newObject);
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
        std::swap (other.object, object);
        std::swap (other.shouldDelete, shouldDelete);
    }

private:
    //==============================================================================
    std::unique_ptr<ObjectType> object;
    bool shouldDelete = false;
};

} // namespace juce
