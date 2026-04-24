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
    A class for keeping a list of available audio formats, and for deciding which
    one to use to open a given file.

    After creating an AudioFormatManager object, you should call registerFormat()
    or registerBasicFormats() to give it a list of format types that it can use.

    @see AudioFormat

    @tags{Audio}
*/
class JUCE_API  AudioFormatManager
{
public:
    //==============================================================================
    /** Creates an empty format manager.

        Before it'll be any use, you'll need to call registerFormat() with all the
        formats you want it to be able to recognise.
    */
    AudioFormatManager();

    /** Destructor. */
    ~AudioFormatManager();

    //==============================================================================
    /** Adds a format to the manager's list of available file types.

        The object passed-in will be deleted by this object, so don't keep a pointer
        to it!

        If makeThisTheDefaultFormat is true, then the getDefaultFormat() method will
        return this one when called.
    */
    void registerFormat (AudioFormat* newFormat,
                         bool makeThisTheDefaultFormat);

    /** Handy method to make it easy to register the formats that come with JUCE.
        This will add WAV and AIFF to the list, along with any other formats enabled
        in either the Projucer or your application's preprocessor definitions.
    */
    void registerBasicFormats();

    /** Clears the list of known formats. */
    void clearFormats();

    /** Returns the number of currently registered file formats. */
    int getNumKnownFormats() const;

    /** Returns one of the registered file formats. */
    AudioFormat* getKnownFormat (int index) const;

    /** Iterator access to the list of known formats. */
    AudioFormat** begin() noexcept                       { return knownFormats.begin(); }

    /** Iterator access to the list of known formats. */
    AudioFormat* const* begin() const noexcept           { return knownFormats.begin(); }

    /** Iterator access to the list of known formats. */
    AudioFormat** end() noexcept                         { return knownFormats.end(); }

    /** Iterator access to the list of known formats. */
    AudioFormat* const* end() const noexcept             { return knownFormats.end(); }

    /** Looks for which of the known formats is listed as being for a given file
        extension.

        The extension may have a dot before it, so e.g. ".wav" or "wav" are both ok.
    */
    AudioFormat* findFormatForFileExtension (const String& fileExtension) const;

    /** Returns the format which has been set as the default one.

        You can set a format as being the default when it is registered. It's useful
        when you want to write to a file, because the best format may change between
        platforms, e.g. AIFF is preferred on the Mac, WAV on Windows.

        If none has been set as the default, this method will just return the first
        one in the list.
    */
    AudioFormat* getDefaultFormat() const;

    /** Returns a set of wildcards for file-matching that contains the extensions for
        all known formats.

        E.g. it might return "*.wav;*.aiff" if it just knows about wavs and aiffs.
    */
    String getWildcardForAllFormats() const;

    //==============================================================================
    /** Searches through the known formats to try to create a suitable reader for
        this file.

        If none of the registered formats can open the file, it'll return nullptr.
        It's the caller's responsibility to delete the reader that is returned.
    */
    AudioFormatReader* createReaderFor (const File& audioFile);

    /** Searches through the known formats to try to create a suitable reader for
        this stream.

        The stream object that is passed-in will be deleted by this method or by the
        reader that is returned, so the caller should not keep any references to it.

        The stream that is passed-in must be capable of being repositioned so
        that all the formats can have a go at opening it.

        If none of the registered formats can open the stream, it'll return nullptr.
        If it returns a reader, it's the caller's responsibility to delete the reader.
    */
    AudioFormatReader* createReaderFor (std::unique_ptr<InputStream> audioFileStream);

private:
    //==============================================================================
    OwnedArray<AudioFormat> knownFormats;
    int defaultFormatIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFormatManager)
};

} // namespace juce
