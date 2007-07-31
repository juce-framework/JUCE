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

#ifndef __JUCE_BLOWFISH_JUCEHEADER__
#define __JUCE_BLOWFISH_JUCEHEADER__


//==============================================================================
/**
    BlowFish encryption class.

*/
class JUCE_API  BlowFish
{
public:
    //==============================================================================
    /** Creates an object that can encode/decode based on the specified key. */
    BlowFish (const uint8* keyData, int keyBytes);

    /** Destructor. */
    ~BlowFish();

    //==============================================================================
    /** Encrypts a pair of 32-bit integers. */
    void encrypt (uint32& data1, uint32& data2) const;

    /** Decrypts a pair of 32-bit integers. */
    void decrypt (uint32& data1, uint32& data2) const;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    uint32* p;
    uint32* s[4];

    uint32 F (uint32 x) const;
};


#endif   // __JUCE_BLOWFISH_JUCEHEADER__
