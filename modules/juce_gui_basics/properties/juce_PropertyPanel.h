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

#ifndef JUCE_PROPERTYPANEL_H_INCLUDED
#define JUCE_PROPERTYPANEL_H_INCLUDED


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

    /** Creates an empty property panel. */
    PropertyPanel (const String& name);

    /** Destructor. */
    ~PropertyPanel();

    //==============================================================================
    /** Deletes all property components from the panel. */
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
                     bool shouldSectionInitiallyBeOpen = true);

    /** Calls the refresh() method of all PropertyComponents in the panel */
    void refreshAll() const;

    /** Returns true if the panel contains no properties. */
    bool isEmpty() const;

    /** Returns the height that the panel needs in order to display all of its content
        without scrolling.
    */
    int getTotalContentHeight() const;

    //==============================================================================
    /** Returns a list of all the names of sections in the panel.
        These are the sections that have been added with addSection().
    */
    StringArray getSectionNames() const;

    /** Returns true if the section at this index is currently open.
        The index is from 0 up to the number of items returned by getSectionNames().
    */
    bool isSectionOpen (int sectionIndex) const;

    /** Opens or closes one of the sections.
        The index is from 0 up to the number of items returned by getSectionNames().
    */
    void setSectionOpen (int sectionIndex, bool shouldBeOpen);

    /** Enables or disables one of the sections.
        The index is from 0 up to the number of items returned by getSectionNames().
    */
    void setSectionEnabled (int sectionIndex, bool shouldBeEnabled);

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
    const String& getMessageWhenEmpty() const;

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;

private:
    class SectionComponent;

    Viewport viewport;
    class PropertyHolderComponent;
    PropertyHolderComponent* propertyHolderComponent;
    String messageWhenEmpty;

    void init();
    void updatePropHolderLayout() const;
    void updatePropHolderLayout (int width) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyPanel)
};


#endif   // JUCE_PROPERTYPANEL_H_INCLUDED
