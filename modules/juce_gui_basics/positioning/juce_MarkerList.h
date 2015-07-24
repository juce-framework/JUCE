/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_MARKERLIST_H_INCLUDED
#define JUCE_MARKERLIST_H_INCLUDED


//==============================================================================
/**
    Holds a set of named marker points along a one-dimensional axis.

    This class is used to store sets of X and Y marker points in components.
    @see Component::getMarkers().
*/
class JUCE_API  MarkerList
{
public:
    //==============================================================================
    /** Creates an empty marker list. */
    MarkerList();
    /** Creates a copy of another marker list. */
    MarkerList (const MarkerList&);
    /** Copies another marker list to this one. */
    MarkerList& operator= (const MarkerList&);
    /** Destructor. */
    ~MarkerList();

    //==============================================================================
    /** Represents a marker in a MarkerList. */
    class JUCE_API  Marker
    {
    public:
        /** Creates a copy of another Marker. */
        Marker (const Marker&);
        /** Creates a Marker with a given name and position. */
        Marker (const String& name, const RelativeCoordinate& position);

        /** The marker's name. */
        String name;

        /** The marker's position.

            The expression used to define the coordinate may use the names of other
            markers, so that markers can be linked in arbitrary ways, but be careful
            not to create recursive loops of markers whose positions are based on each
            other! It can also refer to "parent.right" and "parent.bottom" so that you
            can set markers which are relative to the size of the component that contains
            them.

            To resolve the coordinate, you can use the MarkerList::getMarkerPosition() method.
        */
        RelativeCoordinate position;

        /** Returns true if both the names and positions of these two markers match. */
        bool operator== (const Marker&) const noexcept;
        /** Returns true if either the name or position of these two markers differ. */
        bool operator!= (const Marker&) const noexcept;
    };

    //==============================================================================
    /** Returns the number of markers in the list. */
    int getNumMarkers() const noexcept;

    /** Returns one of the markers in the list, by its index. */
    const Marker* getMarker (int index) const noexcept;

    /** Returns a named marker, or nullptr if no such name is found.
        Note that name comparisons are case-sensitive.
    */
    const Marker* getMarker (const String& name) const noexcept;

    /** Evaluates the given marker and returns its absolute position.
        The parent component must be supplied in case the marker's expression refers to
        the size of its parent component.
    */
    double getMarkerPosition (const Marker& marker, Component* parentComponent) const;

    /** Sets the position of a marker.

        If the name already exists, then the existing marker is moved; if it doesn't exist, then a
        new marker is added.
    */
    void setMarker (const String& name, const RelativeCoordinate& position);

    /** Deletes the marker at the given list index. */
    void removeMarker (int index);

    /** Deletes the marker with the given name. */
    void removeMarker (const String& name);

    /** Returns true if all the markers in these two lists match exactly. */
    bool operator== (const MarkerList&) const noexcept;
    /** Returns true if not all the markers in these two lists match exactly. */
    bool operator!= (const MarkerList&) const noexcept;

    //==============================================================================
    /**
        A class for receiving events when changes are made to a MarkerList.

        You can register a MarkerList::Listener with a MarkerList using the MarkerList::addListener()
        method, and it will be called when markers are moved, added, or deleted.

        @see MarkerList::addListener, MarkerList::removeListener
    */
    class JUCE_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() {}

        /** Called when something in the given marker list changes. */
        virtual void markersChanged (MarkerList* markerList) = 0;

        /** Called when the given marker list is being deleted. */
        virtual void markerListBeingDeleted (MarkerList* markerList);
    };

    /** Registers a listener that will be called when the markers are changed. */
    void addListener (Listener* listener);

    /** Deregisters a previously-registered listener. */
    void removeListener (Listener* listener);

    /** Synchronously calls markersChanged() on all the registered listeners. */
    void markersHaveChanged();

    //==============================================================================
    /** Forms a wrapper around a ValueTree that can be used for storing a MarkerList. */
    class ValueTreeWrapper
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        ValueTree& getState() noexcept      { return state; }
        int getNumMarkers() const;
        ValueTree getMarkerState (int index) const;
        ValueTree getMarkerState (const String& name) const;
        bool containsMarker (const ValueTree& state) const;
        MarkerList::Marker getMarker (const ValueTree& state) const;
        void setMarker (const MarkerList::Marker& marker, UndoManager* undoManager);
        void removeMarker (const ValueTree& state, UndoManager* undoManager);

        void applyTo (MarkerList& markerList);
        void readFrom (const MarkerList& markerList, UndoManager* undoManager);

        static const Identifier markerTag, nameProperty, posProperty;

    private:
        ValueTree state;
    };

private:
    //==============================================================================
    OwnedArray<Marker> markers;
    ListenerList<Listener> listeners;

    Marker* getMarkerByName (const String& name) const noexcept;

    JUCE_LEAK_DETECTOR (MarkerList)
};


#endif   // JUCE_MARKERLIST_H_INCLUDED
