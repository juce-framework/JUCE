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

#ifndef JUCE_OPENGLHELPERS_H_INCLUDED
#define JUCE_OPENGLHELPERS_H_INCLUDED


//==============================================================================
/**
    A set of miscellaneous openGL helper functions.
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

    static void enableScissorTest (const Rectangle<int>& clip);

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


#endif   // JUCE_OPENGLHELPERS_H_INCLUDED
