//      tracktable.cpp
//      
//      Copyright 2010 Kevin Pors <krpors@users.sf.net>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#ifndef TRACKTABLE_HPP 
#define TRACKTABLE_HPP 

#include "audio.hpp"

#include <wx/listctrl.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/thread.h>
#include <wx/colour.h>
#include <wx/dataview.h>
#include <wx/settings.h>

#include <vector>
#include <iostream>

namespace navi {

//================================================================================

/// New event type for directory traversal (all UI things must be done on the
/// main thread). 

//================================================================================

class TrackTable : public wxListCtrl {
private:
    /// Vector holding the trackinfo objects.
    std::vector<TrackInfo> m_trackInfos;

    /// Executed when an item is activated (i.e. dbl clicked, entere'ed)
    void onActivate(wxListEvent& event);

    /// Executed when a column is clicked (sorting).
    void onColumnClick(wxListEvent& event);

    /// Executed when an item is selected (*not* doubleclixed)
    void onSelected(wxListEvent& event);

    /// Executed when track info is about to be added (from another thread).
    void onAddTrackInfo(wxCommandEvent& event);

    TrackInfo getTrackBeforeOrAfterCurrent(int pos, bool markAsPlaying) throw();

    void markPlayedTrack(long oldItemId, long newItemId) throw();

    // static callback methods, for sorting. sortData is always the `this' instance
    // of TrackTable.
    static int wxCALLBACK compareTrackNumber(long item1, long item2, long sortData);
    static int wxCALLBACK compareArtistName(long item1, long item2, long sortData);
    static int wxCALLBACK compareTitle(long item1, long item2, long sortData);
    static int wxCALLBACK compareAlbum(long item1, long item2, long sortData);

    /// The current selected item. May be NULL. Don't destroy this thing.
    TrackInfo m_selectedItem;

    /// The current track item index (in the vector)
    long m_currTrackItemIndex;

    /// The 
    long m_currTrackItemIndexInListCtrl;
    
    // array of bools for sort direction. 4 length, for each of the
    // four columns. true = ascending, false = descending.
    bool m_sortDirection[4];

public:
    /// The window ID for this track table.
    static const wxWindowID ID_TRACKTABLE = 2;

    static const wxWindowID ID_EVT_ADD_INFO = 10000;

    /**
     * Creates this tracktable.
     *
     * @param The parent wxWindow.
     */
    TrackTable(wxWindow* parent);

    /**
     * Adds tracking info to the list control.
     *
     * @param info The trackinfo to add.
     */
    void addTrackInfo(TrackInfo& info);

    /**
     * Gets the current (possibly) selected track. It may return a null
     * pointer, if nothing has been selected.
     */
    TrackInfo getSelectedItem() throw();

    /**
     * Get the previous track in line.
     */
    TrackInfo getPrev(bool markAsPlaying = false) throw();

    /**
     * Get the next track in line.
     */
    TrackInfo getNext(bool markAsPlaying = false) throw();

    /**
     * Override from wxListCtrl. In addition to deleting the items from the list
     * control itself, it also clears the backing std::vector.
     */
    void DeleteAllItems();

    /**
     * Gets a certain TrackInfo object at a specific index. Note that the given
     * index is not dependent on the current sorted order! It will simply return
     * the object which is at the same index in the backed vector.
     *
     * @param index The index of the TrackInfo to get.
     */
    TrackInfo& getTrackInfo(int index);

    /**
     * Resizes the headers automatically based on the current size of the widget.
     * This function is called by the EVT_SIZE event handling mapping.
     *
     * @param event The wxSizeEvent.
     */
    void onResize(wxSizeEvent& event);

    // Plx respond to events.
    DECLARE_EVENT_TABLE()
};




//================================================================================


} // namespace navi 

#endif // TRACKTABLE_HPP

