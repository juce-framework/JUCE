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

#ifndef __JUCE_URL_JUCEHEADER__
#define __JUCE_URL_JUCEHEADER__

#include "../../text/juce_StringPairArray.h"
#include "../../text/juce_XmlElement.h"
#include "../juce_InputStream.h"


//==============================================================================
/**
    Represents a URL and has a bunch of useful functions to manipulate it.

    This class can be used to launch URLs in browsers, and also to create
    InputStreams that can read from remote http or ftp sources.
*/
class JUCE_API  URL
{
public:
    //==============================================================================
    /** Creates an empty URL. */
    URL() throw();

    /** Creates a URL from a string. */
    URL (const String& url);

    /** Creates a copy of another URL. */
    URL (const URL& other);

    /** Destructor. */
    ~URL() throw();

    /** Copies this URL from another one. */
    const URL& operator= (const URL& other);

    //==============================================================================
    /** Returns a string version of the URL.

        If includeGetParameters is true and any parameters have been set with the
        withParameter() method, then the string will have these appended on the
        end and url-encoded.
    */
    const String toString (const bool includeGetParameters) const;

    /** True if it seems to be valid. */
    bool isWellFormed() const;

    //==============================================================================
    /** Returns a copy of this URL, with a GET parameter added to the end.

        Any control characters in the value will be encoded.

        e.g. calling "withParameter ("amount", "some fish") for the url "www.fish.com"
        would produce a new url whose toString(true) method would return
        "www.fish.com?amount=some+fish".
    */
    const URL withParameter (const String& parameterName,
                             const String& parameterValue) const;

    /** Returns a set of all the parameters encoded into the url.

        E.g. for the url "www.fish.com?type=haddock&amount=some+fish", this array would
        contain two pairs: "type" => "haddock" and "amount" => "some fish".

        The values returned will have been cleaned up to remove any escape characters.

        @see getNamedParameter, withParameter
    */
    const StringPairArray& getParameters() const;

    //==============================================================================
    /** Tries to launch the system's default browser to open the URL.

        Returns true if this seems to have worked.
    */
    bool launchInDefaultBrowser() const;

    //==============================================================================
    /** Takes a guess as to whether a string might be a valid website address.

        This isn't foolproof!
    */
    static bool isProbablyAWebsiteURL (const String& possibleURL);

    /** Takes a guess as to whether a string might be a valid email address.

        This isn't foolproof!
    */
    static bool isProbablyAnEmailAddress (const String& possibleEmailAddress);

    //==============================================================================
    /** Attempts to open a stream that can read from this URL.

        @param usePostCommand   if true, it will try to do use a http 'POST' to pass
                                the paramters, otherwise it'll encode them into the
                                URL and do a 'GET'.
    */
    InputStream* createInputStream (const bool usePostCommand) const;

    /** Attempts to open a stream to read from this URL using a http POST command.

        Normally you'd use the createInputStream (true) method instead of this, as
        this will pass the given block of text instead of any parameters
        that were added to the this URL with the withParameter() method.
    */
    InputStream* createPostInputStream (const String& postText) const;

    //==============================================================================
    /** Tries to download the entire contents of this URL into a binary data block.

        If it succeeds, this will return true and append the data it read onto the end
        of the memory block.

        @param destData         the memory block to append the new data to
        @param usePostCommand   whether to use a POST command to get the data (uses
                                a GET command if this is false)
        @see readEntireTextStream, readEntireXmlStream
    */
    bool readEntireBinaryStream (MemoryBlock& destData,
                                 const bool usePostCommand = false) const;

    /** Tries to download the entire contents of this URL as a string.

        If it fails, this will return an empty string, otherwise it will return the
        contents of the downloaded file. If you need to distinguish between a read
        operation that fails and one that returns an empty string, you'll need to use
        a different method, such as readEntireBinaryStream().

        @param usePostCommand   whether to use a POST command to get the data (uses
                                a GET command if this is false)
        @see readEntireBinaryStream, readEntireXmlStream
    */
    const String readEntireTextStream (const bool usePostCommand = false) const;

    /** Tries to download the entire contents of this URL and parse it as XML.

        If it fails, or if the text that it reads can't be parsed as XML, this will
        return 0.

        When it returns a valid XmlElement object, the caller is responsibile for deleting
        this object when no longer needed.

        @param usePostCommand   whether to use a POST command to get the data (uses
                                a GET command if this is false)

        @see readEntireBinaryStream, readEntireTextStream
    */
    XmlElement* readEntireXmlStream (const bool usePostCommand = false) const;

    //==============================================================================
    /** Adds escape sequences to a string to encode any characters that aren't
        legal in a URL.

        E.g. any spaces will be replaced with "%20".

        This is the opposite of removeEscapeChars().

        @see removeEscapeChars
    */
    static const String addEscapeChars (const String& stringToAddEscapeCharsTo);

    /** Replaces any escape character sequences in a string with their original
        character codes.

        E.g. any instances of "%20" will be replaced by a space.

        This is the opposite of addEscapeChars().

        @see addEscapeChars
    */
    static const String removeEscapeChars (const String& stringToRemoveEscapeCharsFrom);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String url;
    StringPairArray parameters;

    const String getMangledParameters() const;
};


#endif   // __JUCE_URL_JUCEHEADER__
