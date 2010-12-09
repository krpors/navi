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

#include <vector>
#include <iostream>

namespace navi {

//================================================================================

class TrackTable : public wxListCtrl {
private:
    std::vector<TrackInfo> m_trackInfos;
    void onActivate(wxListEvent& event);
    void onColumnClick(wxListEvent& event);


public:
    static const short ID_TRACKTABLE = 2;

    static int wxCALLBACK compareTrackNumber(long item1, long item2, long sortData);
    static int wxCALLBACK compareArtistName(long item1, long item2, long sortData);
    static int wxCALLBACK compareTitle(long item1, long item2, long sortData);
    static int wxCALLBACK compareAlbum(long item1, long item2, long sortData);

    TrackTable(wxWindow* parent);
    ~TrackTable();

    void addTrackInfo(TrackInfo& info);

    TrackInfo& getTrackInfo(int index);

    void addFromDir(const wxFileName& dir);

    int theSort(TrackInfo& one, TrackInfo& two, const char* field, bool ascending);

    DECLARE_EVENT_TABLE()
};

} // namespace navi 

#endif // TRACKTABLE_HPP

