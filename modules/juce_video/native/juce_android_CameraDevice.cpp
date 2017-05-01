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

struct CameraDevice::Pimpl
{
    Pimpl (const String&, int /*index*/, int /*minWidth*/, int /*minHeight*/, int /*maxWidth*/, int /*maxHeight*/)
    {
    }

    ~Pimpl()
    {
    }

    void startRecordingToFile (const File&, int /*quality*/)
    {
    }

    void stopRecording()
    {
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return {};
    }

    void addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);
        listeners.addIfNotAlreadyThere (listenerToAdd);
    }

    void removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.removeFirstMatchingValue (listenerToRemove);
    }

    static StringArray getAvailableDevices()
    {
        StringArray results;

        return results;
    }

private:
    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

struct CameraDevice::ViewerComponent  : public Component
{
    ViewerComponent (CameraDevice&)
    {
    }

    JUCE_DECLARE_NON_COPYABLE (ViewerComponent)
};

String CameraDevice::getFileExtension()
{
    return ".mov";
}
