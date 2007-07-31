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

#ifndef __JUCE_PROPERTYPANEL_JUCEHEADER__
#define __JUCE_PROPERTYPANEL_JUCEHEADER__

#include "juce_PropertyComponent.h"
#include "../layout/juce_Viewport.h"


//==============================================================================
/**
    A panel that holds a list of PropertyComponent objects.

    This panel displays a list of PropertyComponents, and allows them to be organised
    into collapsible sections.

    To use, simply create one of these and add your properties to it with addProperties()
    or addSection().

    @see PropertyComponent
*/
class JUCE_API  PropertyPanel  : public Component
{
public:
    //==============================================================================
    /** Creates an empty property panel. */
    PropertyPanel();

    /** Destructor. */
    ~PropertyPanel();

    //==============================================================================
    /** Deletes all property components from the panel.
    */
    void clear();

    /** Adds a set of properties to the panel.

        The components in the list will be owned by this object and will be automatically
        deleted later on when no longer needed.

        These properties are added without them being inside a named section. If you
        want them to be kept together in a collapsible section, use addSection() instead.
    */
    void addProperties (const Array <PropertyComponent*>& newPropertyComponents);

    /** Adds a set of properties to the panel.

        These properties are added at the bottom of the list, under a section heading with
        a plus/minus button that allows it to be opened and closed.

        The components in the list will be owned by this object and will be automatically
        deleted later on when no longer needed.

        To add properies without them being in a section, use addProperties().
    */
    void addSection (const String& sectionTitle,
                     const Array <PropertyComponent*>& newPropertyComponents,
                     const bool shouldSectionInitiallyBeOpen = true);

    /** Calls the refresh() method of all PropertyComponents in the panel */
    void refreshAll() const;

    //==============================================================================
    /** Returns a list of all the names of sections in the panel.

        These are the sections that have been added with addSection().
    */
    const StringArray getSectionNames() const;

    /** Returns true if the section at this index is currently open.

        The index is from 0 up to the number of items returned by getSectionNames().
    */
    bool isSectionOpen (const int sectionIndex) const;

    /** Opens or closes one of the sections.

        The index is from 0 up to the number of items returned by getSectionNames().
    */
    void setSectionOpen (const int sectionIndex, const bool shouldBeOpen);

    //==============================================================================
    /** Saves the current state of open/closed sections so it can be restored later.

        The caller is responsible for deleting the object that is returned.

        To restore this state, use restoreOpennessState().

        @see restoreOpennessState
    */
    XmlElement* getOpennessState() const;

    /** Restores a previously saved arrangement of open/closed sections.

        This will try to restore a snapshot of the panel's state that was created by
        the getOpennessState() method. If any of the sections named in the original
        XML aren't present, they will be ignored.

        @see getOpennessState
    */
    void restoreOpennessState (const XmlElement& newState);

    //==============================================================================
    /** Sets a message to be displayed when there are no properties in the panel.

        The default message is "nothing selected".
    */
    void setMessageWhenEmpty (const String& newMessage);

    /** Returns the message that is displayed when there are no properties.
        @see setMessageWhenEmpty
    */
    const String& getMessageWhenEmpty() const throw();

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();

    juce_UseDebuggingNewOperator

private:
    Viewport* viewport;
    Component* propertyHolderComponent;
    String messageWhenEmpty;

    void updatePropHolderLayout() const;
    void updatePropHolderLayout (const int width) const;
};


#endif   // __JUCE_PROPERTYPANEL_JUCEHEADER__
