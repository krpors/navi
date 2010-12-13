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

#include <vector>
#include <iostream>

namespace navi {



//================================================================================

class TrackTable : public wxListCtrl {
private:
    std::vector<TrackInfo> m_trackInfos;
    void onActivate(wxListEvent& event);
    void onColumnClick(wxListEvent& event);
    void onNumberUpdate(wxCommandEvent& event);

public:
    static const wxWindowID ID_TRACKTABLE = 2;

    static int wxCALLBACK compareTrackNumber(long item1, long item2, long sortData);
    static int wxCALLBACK compareArtistName(long item1, long item2, long sortData);
    static int wxCALLBACK compareTitle(long item1, long item2, long sortData);
    static int wxCALLBACK compareAlbum(long item1, long item2, long sortData);

    TrackTable(wxWindow* parent);
    ~TrackTable();

    void addTrackInfo(TrackInfo& info);

    void DeleteAllItems();

    TrackInfo& getTrackInfo(int index);

    void addFromDir(const wxFileName& dir);

    int theSort(TrackInfo& one, TrackInfo& two, const char* field, bool ascending);

    DECLARE_EVENT_TABLE()
};

//================================================================================

class UpdateClientData : public wxClientData {
private:
    TrackInfo m_info;
public:
    UpdateClientData(const TrackInfo& info);

    const TrackInfo getTrackInfo() const;
};

//================================================================================

/**
 * The UpdateThread is a joinable (not detached) thread to update the user interface
 * with new track info's. If we don't update the UI in another thread, the UI would
 * block until all files in a directory or the like are finished adding.
 * 
 * The thread is created joinable so we can safely interrupt it be calling this
 * thread's public functions. If it was detached, it would be destroyed after it
 * has finished doing its work, and calling functions on the created instance will
 * then most certainly invoke terrorist attacks on the application.
 */
class UpdateThread : public wxThread {
private:
    TrackTable* m_parent;
    wxFileName m_selectedPath;
public:
    UpdateThread(TrackTable* parent, const wxFileName& selectedPath);

    virtual wxThread::ExitCode Entry(); 
};

//================================================================================

} // namespace navi 

#endif // TRACKTABLE_HPP

