/*

    IMPORTANT! This file is auto-generated each time you save your
    project - if you alter its contents, your changes may be overwritten!

    This is the header file that your files should include in order to get all the
    JUCE library headers. You should avoid including the JUCE headers directly in
    your own source files, because that wouldn't pick up the correct configuration
    options for your app.

*/

#pragma once


#include <juce_core/juce_core.h>


#if defined (JUCE_PROJUCER_VERSION) && JUCE_PROJUCER_VERSION < JUCE_VERSION
 /** If you've hit this error then the version of the Projucer that was used to generate this project is
     older than the version of the JUCE modules being included. To fix this error, re-save your project
     using the latest version of the Projucer or, if you aren't using the Projucer to manage your project,
     remove the JUCE_PROJUCER_VERSION define.
 */
 #error "This project was last saved using an outdated version of the Projucer! Re-save this project with the latest version to fix this error."
#endif

#if ! DONT_SET_USING_JUCE_NAMESPACE
 // If your code uses a lot of JUCE classes, then this will obviously save you
 // a lot of typing, but can be disabled by setting DONT_SET_USING_JUCE_NAMESPACE.
 using namespace juce;
#endif

#if ! JUCE_DONT_DECLARE_PROJECTINFO
namespace ProjectInfo
{
    const char* const  projectName    = "BinaryBuilder";
    const char* const  companyName    = "Raw Material Software Limited";
    const char* const  versionString  = "1.0.0";
    const int          versionNumber  = 0x10000;
}
#endif
