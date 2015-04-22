/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_DYNAMICLIBRARY_H_INCLUDED
#define JUCE_DYNAMICLIBRARY_H_INCLUDED

/**
    Handles the opening and closing of DLLs.

    This class can be used to open a DLL and get some function pointers from it.
    Since the DLL is freed when this object is deleted, it's handy for managing
    library lifetimes using RAII.
*/
class JUCE_API  DynamicLibrary
{
public:
    /** Creates an unopened DynamicLibrary object.
        Call open() to actually open one.
    */
    DynamicLibrary() noexcept : handle (nullptr) {}

    /**
    */
    DynamicLibrary (const String& name) : handle (nullptr) { open (name); }

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
    void* handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicLibrary)
};


#endif   // JUCE_DYNAMICLIBRARY_H_INCLUDED
