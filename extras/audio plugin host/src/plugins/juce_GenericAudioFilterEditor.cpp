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

#include "../../../../juce.h"
#include "juce_GenericAudioFilterEditor.h"


//==============================================================================
class FilterParameterPropertyComp   : public PropertyComponent,
                                      public AudioPluginParameterListener
{
public:
    FilterParameterPropertyComp (const String& name,
                                 AudioPluginInstance* const filter_,
                                 const int index_)
        : PropertyComponent (name),
          filter (filter_),
          index (index_)
    {
        addAndMakeVisible (slider = new PluginSlider (filter_, index_));
        filter->addListener (this);
    }

    ~FilterParameterPropertyComp()
    {
        filter->removeListener (this);
        deleteAllChildren();
    }

    void refresh()
    {
        slider->setValue (filter->getParameter (index), false);
    }

    void audioPluginChanged (AudioPluginInstance*)
    {
    }

    void audioPluginParameterChanged (AudioPluginInstance*, int parameterIndex)
    {
        if (parameterIndex == index)
            refresh();
    }
                                              
    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioPluginInstance* const filter;
    const int index;
    Slider* slider;

    //==============================================================================
    class PluginSlider  : public Slider
    {
    public:
        PluginSlider (AudioPluginInstance* const filter_, const int index_)
            : Slider (String::empty),
              filter (filter_),
              index (index_)
        {
            setRange (0.0, 1.0, 0.0);
            setSliderStyle (Slider::LinearBar);
            setTextBoxIsEditable (false);
            setScrollWheelEnabled (false);
        }

        ~PluginSlider()
        {
        }

        void valueChanged()
        {
            const float newVal = (float) getValue();

            if (filter->getParameter (index) != newVal)
                filter->setParameter (index, newVal);
        }

        const String getTextFromValue (double /*value*/)
        {
            return filter->getParameterText (index);
        }

        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        AudioPluginInstance* const filter;
        const int index;
    };
};


//==============================================================================
GenericAudioFilterEditor::GenericAudioFilterEditor (AudioPluginInstance* const filter)
    : AudioFilterEditor (filter)
{
    setOpaque (true);

    addAndMakeVisible (panel = new PropertyPanel());

    Array <PropertyComponent*> params;

    const int numParams = filter->getNumParameters();
    int totalHeight = 0;

    for (int i = 0; i < numParams; ++i)
    {
        String name (filter->getParameterName (i));
        if (name.trim().isEmpty())
            name = "Unnamed";

        FilterParameterPropertyComp* const pc = new FilterParameterPropertyComp (name, filter, i);
        params.add (pc);
        totalHeight += pc->getPreferredHeight();
    }

    panel->addProperties (params);

    setSize (400, jlimit (25, 400, totalHeight));
}

GenericAudioFilterEditor::~GenericAudioFilterEditor()
{
    deleteAllChildren();
}

void GenericAudioFilterEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void GenericAudioFilterEditor::resized()
{
    panel->setSize (getWidth(), getHeight());
}
