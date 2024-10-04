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

    /** Returns a version string such as "#version 150" suitable for prefixing a GLSL
        shader on this platform.
    */
    static String getGLSLVersionString();

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
