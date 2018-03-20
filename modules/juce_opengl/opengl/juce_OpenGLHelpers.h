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

namespace juce
{

//==============================================================================
/**
    A set of miscellaneous openGL helper functions.

    @tags{OpenGL}
*/
class JUCE_API  OpenGLHelpers
{
public:
    /** Clears the GL error state. */
    static void resetErrorState();

    /** Returns true if the current thread has an active OpenGL context. */
    static bool isContextActive();

    /** Clears the current context using the given colour. */
    static void clear (Colour colour);

    static void enableScissorTest (Rectangle<int> clip);

    /** Checks whether the current context supports the specified extension. */
    static bool isExtensionSupported (const char* extensionName);

    /** Returns the address of a named GL extension function */
    static void* getExtensionFunction (const char* functionName);

    /** Makes some simple textual changes to a shader program to try to convert old GLSL
        keywords to their v3 equivalents.

        Before doing this, the function will check whether the current context is actually
        using a later version of the language, and if not it will not make any changes.
        Obviously this is not a real parser, so will only work on simple code!
    */
    static String translateVertexShaderToV3 (const String&);

    /** Makes some simple textual changes to a shader program to try to convert old GLSL
        keywords to their v3 equivalents.

        Before doing this, the function will check whether the current context is actually
        using a later version of the language, and if not it will not make any changes.
        Obviously this is not a real parser, so will only work on simple code!
    */
    static String translateFragmentShaderToV3 (const String&);
};

} // namespace juce
