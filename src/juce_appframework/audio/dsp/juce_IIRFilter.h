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

#ifndef __JUCE_IIRFILTER_JUCEHEADER__
#define __JUCE_IIRFILTER_JUCEHEADER__

#include "../../../juce_core/threads/juce_CriticalSection.h"


//==============================================================================
/**
    An IIR filter that can perform low, high, or band-pass filtering on an
    audio signal.

    @see IIRFilterAudioSource
*/
class JUCE_API  IIRFilter
{
public:
    //==============================================================================
    /** Creates a filter.

        Initially the filter is inactive, so will have no effect on samples that
        you process with it. Use the appropriate method to turn it into the type
        of filter needed.
    */
    IIRFilter() throw();

    /** Creates a copy of another filter. */
    IIRFilter (const IIRFilter& other) throw();

    /** Destructor. */
    ~IIRFilter() throw();

    //==============================================================================
    /** Resets the filter's processing pipeline, ready to start a new stream of data.

        Note that this clears the processing state, but the type of filter and
        its coefficients aren't changed. To put a filter into an inactive state, use
        the makeInactive() method.
    */
    void reset() throw();

    /** Performs the filter operation on the given set of samples.
    */
    void processSamples (float* const samples,
                         const int numSamples) throw();

    //==============================================================================
    /** Sets the filter up to act as a low-pass filter.
    */
    void makeLowPass (const double sampleRate,
                      const double frequency) throw();

    /** Sets the filter up to act as a high-pass filter.
    */
    void makeHighPass (const double sampleRate,
                       const double frequency) throw();

    //==============================================================================
    /** Sets the filter up to act as a low-pass shelf filter with variable Q and gain.
    */
    void makeLowShelf (const double sampleRate,
                       const double cutOffFrequency,
                       const double Q,
                       const float gainFactor) throw();

    /** Sets the filter up to act as a high-pass shelf filter with variable Q and gain.
    */
    void makeHighShelf (const double sampleRate,
                        const double cutOffFrequency,
                        const double Q,
                        const float gainFactor) throw();

    /** Sets the filter up to act as a band pass filter centred around a
        frequency, with a variable Q and gain.
    */
    void makeBandPass (const double sampleRate,
                       const double centreFrequency,
                       const double Q,
                       const float gainFactor) throw();

    /** Clears the filter's coefficients so that it becomes inactive.
    */
    void makeInactive() throw();

    //==============================================================================
    /** Makes this filter duplicate the set-up of another one.
    */
    void copyCoefficientsFrom (const IIRFilter& other) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator


protected:
    CriticalSection processLock;

    void setCoefficients (double c1, double c2, double c3,
                          double c4, double c5, double c6) throw();

    bool active;
    float coefficients[6];
    float x1, x2, y1, y2;

    // (use the copyCoefficientsFrom() method instead of this operator)
    const IIRFilter& operator= (const IIRFilter&);
};


#endif   // __JUCE_IIRFILTER_JUCEHEADER__
