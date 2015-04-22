/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

AudioFormat::AudioFormat (String name, StringArray extensions)
   : formatName (name), fileExtensions (extensions)
{
}

AudioFormat::AudioFormat (StringRef name, StringRef extensions)
   : formatName (name.text), fileExtensions (StringArray::fromTokens (extensions, false))
{
}

AudioFormat::~AudioFormat()
{
}

bool AudioFormat::canHandleFile (const File& f)
{
    for (int i = 0; i < fileExtensions.size(); ++i)
        if (f.hasFileExtension (fileExtensions[i]))
            return true;

    return false;
}

const String& AudioFormat::getFormatName() const                { return formatName; }
const StringArray& AudioFormat::getFileExtensions() const       { return fileExtensions; }
bool AudioFormat::isCompressed()                                { return false; }
StringArray AudioFormat::getQualityOptions()                    { return StringArray(); }

MemoryMappedAudioFormatReader* AudioFormat::createMemoryMappedReader (const File&)
{
    return nullptr;
}
