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

/**
    Handles the opening and closing of DLLs.

    This class can be used to open a DLL and get some function pointers from it.
    Since the DLL is freed when this object is deleted, it's handy for managing
    library lifetimes using RAII.

    @tags{Core}
*/
class JUCE_API  DynamicLibrary
{
public:
    /** Creates an unopened DynamicLibrary object.
        Call open() to actually open one.
    */
    DynamicLibrary() = default;

    /**
    */
    DynamicLibrary (const String& name)  { open (name); }

    /** Move constructor */
    DynamicLibrary (DynamicLibrary&& other) noexcept
    {
        std::swap (handle, other.handle);
    }

    /** Destructor.
        If a library is currently open, it will be closed when this object is destroyed.
    */
    ~DynamicLibrary()   { close(); }

    /** Opens a DLL.
        The name and the method by which it gets found is of course platform-specific, and
        may or may not include a path, depending on the OS.
        If a library is already open when this method is called, it will first close the library
        before attempting to load the new one.
        @returns true if the library was successfully found and opened.
    */
    bool open (const String& name);

    /** Releases the currently-open DLL, or has no effect if none was open. */
    void close();

    /** Tries to find a named function in the currently-open DLL, and returns a pointer to it.
        If no library is open, or if the function isn't found, this will return a null pointer.
    */
    void* getFunction (const String& functionName) noexcept;

    /** Returns the platform-specific native library handle.
        You'll need to cast this to whatever is appropriate for the OS that's in use.
    */
    void* getNativeHandle() const noexcept     { return handle; }

private:
    void* handle = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicLibrary)
};

} // namespace juce
