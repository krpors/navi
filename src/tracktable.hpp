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
//#define wxUSE_DATAVIEWCTRL
#include <wx/dataview.h>

#include <vector>
#include <iostream>

namespace navi {



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

    // static callback methods, for sorting. sortData is always the `this' instance
    // of TrackTable.
    static int wxCALLBACK compareTrackNumber(long item1, long item2, long sortData);
    static int wxCALLBACK compareArtistName(long item1, long item2, long sortData);
    static int wxCALLBACK compareTitle(long item1, long item2, long sortData);
    static int wxCALLBACK compareAlbum(long item1, long item2, long sortData);

    /// The current selected item. May be NULL. Don't destroy this thing.
    TrackInfo* m_selectedItem;

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
    TrackInfo* getSelectedItem() throw();

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

    // Plx respond to events.
    DECLARE_EVENT_TABLE()
};

//================================================================================

/**
 * Encapsulation class to set within a wxCommandEvent. This is used in the 
 * UpdateThread (event.setClientObject(...)). It holds one TrackInfo instance. 
 *
 * TODO: why not subclass TrackInfo from wxClientData, instead of a wrappah?
 */
class UpdateClientData : public wxClientData {
private:
    /// The trackinfo.
    TrackInfo m_info;
public:
    /**
     * Constructor.
     *
     * @param info The trackinfo to set.
     */
    UpdateClientData(const TrackInfo& info);

    /**
     * Returns the trackinfo instance associated with this ClientData.
     *
     * @return the TrackInfo.
     */
    const TrackInfo getTrackInfo() const;
};

//================================================================================

/**
 * The UpdateThread is a joinable (not detached) thread to update the user interface
 * with new track infos. If we don't update the UI in another thread, the UI would
 * block until all files in a directory or the like are finished adding. This would
 * be severely problematic if you have a LOT of files in one directory.
 * 
 * The thread is created joinable so we can safely interrupt it be calling this
 * thread's public functions. If it was detached, it would be destroyed after it
 * has finished doing its work, and calling functions on the created instance will
 * then most certainly invoke terrorist attacks on the application.
 *
 * TODO: fer chrissake rename this thing. It's so generic.
 */
class UpdateThread : public wxThread {
private:
    /// The tracktable parent. We will be add pending events to this wxWindow.
    TrackTable* m_parent;

    /// The selected path to read files from.
    wxFileName m_selectedPath;

    /// Whether this thread should be active or not. This value is polled
    bool m_active;
public:
    /**
     * Creates the UpdateThread, with the 'parent' TrackTable (to add pending
     * events to) and the selected path to get a dir listing from.
     *
     * @param parent The TrackTable parent.
     * @param selectedPath The path to get a listing from.
     */
    UpdateThread(TrackTable* parent, const wxFileName& selectedPath);

    /**
     * Sets the 'activity' state of this thread. This is only useful right now
     * for deactivation (see the Entry() override function).
     *
     * @param active Set this to false to stop the thread gracefully.
     */
    void setActive(bool active);

    /**
     * Override from wxThread. This is the 'meat' of this subclass.
     *
     * @return ExitCode (a short or something)
     */
    virtual wxThread::ExitCode Entry(); 
};

//================================================================================


} // namespace navi 

#endif // TRACKTABLE_HPP

