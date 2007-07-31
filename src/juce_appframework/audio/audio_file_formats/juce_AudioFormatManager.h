/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__
#define __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__

#include "juce_AudioFormat.h"
#include "../../../juce_core/basics/juce_Singleton.h"


//==============================================================================
/**
    A class for keeping a list of available audio formats, and for deciding which
    one to use to open a given file.

    You can either use this class as a singleton object, or create instances of it
    yourself. Once created, use its registerFormat() method to tell it which
    formats it should use.

    @see AudioFormat
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

    juce_DeclareSingleton (AudioFormatManager, false);

    //==============================================================================
    /** Adds a format to the manager's list of available file types.

        The object passed-in will be deleted by this object, so don't keep a pointer
        to it!

        If makeThisTheDefaultFormat is true, then the getDefaultFormat() method will
        return this one when called.
    */
    void registerFormat (AudioFormat* newFormat,
                         const bool makeThisTheDefaultFormat);

    /** Handy method to make it easy to register the formats that come with Juce.

        Currently, this will add WAV and AIFF to the list.
    */
    void registerBasicFormats();

    /** Clears the list of known formats. */
    void clearFormats();

    /** Returns the number of currently registered file formats. */
    int getNumKnownFormats() const;

    /** Returns one of the registered file formats. */
    AudioFormat* getKnownFormat (const int index) const;

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

        E.g. if might return "*.wav;*.aiff" if it just knows about wavs and aiffs.
    */
    const String getWildcardForAllFormats() const;

    //==============================================================================
    /** Searches through the known formats to try to create a suitable reader for
        this file.

        If none of the registered formats can open the file, it'll return 0. If it
        returns a reader, it's the caller's responsibility to delete the reader.
    */
    AudioFormatReader* createReaderFor (const File& audioFile);

    /** Searches through the known formats to try to create a suitable reader for
        this stream.

        The stream object that is passed-in will be deleted by this method or by the
        reader that is returned, so the caller should not keep any references to it.

        The stream that is passed-in must be capable of being repositioned so
        that all the formats can have a go at opening it.

        If none of the registered formats can open the stream, it'll return 0. If it
        returns a reader, it's the caller's responsibility to delete the reader.
    */
    AudioFormatReader* createReaderFor (InputStream* audioFileStream);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    VoidArray knownFormats;
    int defaultFormatIndex;
};


#endif   // __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__
