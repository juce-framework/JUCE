#pragma once

#include "JuceHeader.h"

//==============================================================================
// The document controller is the central point of communication between the ARA host and our plug-in.
// While we're not customizing anything in this example, actual plug-ins will do a lot of work here.
class ARASampleProjectDocumentController    : public ARADocumentController
{
public:
    ARASampleProjectDocumentController (const ARA::ARADocumentControllerHostInstance* instance) noexcept
        :  ARADocumentController (instance)
    {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectDocumentController)
};
