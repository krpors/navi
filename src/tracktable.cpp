//      filetree.cpp
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

#include "tracktable.hpp"

namespace navi {

TrackTableItem::TrackTableItem() {
}

TrackTableItem::TrackTableItem(const TrackInfo& info) :
        m_trackInfo(info) {
}

TrackTableItem::~TrackTableItem() {
}

void TrackTableItem::setTrackInfo(const TrackInfo& info) {
    m_trackInfo = info;
}

TrackInfo TrackTableItem::getTrackInfo() {
    return m_trackInfo;
}

//================================================================================

TrackTable::TrackTable(wxWindow* parent) :
        wxListCtrl(parent, TrackTable::ID_TRACKTABLE, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VIRTUAL) {

    wxListItem item;
    item.SetText(wxT("Artist"));
    InsertColumn(0, item);
    SetColumnWidth(0, 80);

    item.SetText(wxT("Title"));
    InsertColumn(1, item);
    SetColumnWidth(1, 100);

    item.SetText(wxT("Album"));
    InsertColumn(2, item);
    SetColumnWidth(2, 150);


    TrackInfo t;
    t[TrackInfo::ARTIST] = wxT("Ninja please");
    t[TrackInfo::TITLE] = wxT("Say what!");
    t[TrackInfo::ALBUM] = wxT("Some Album.");

    TrackInfo t2;
    t2[TrackInfo::ARTIST] = wxT("Norah Jones");
    t2[TrackInfo::TITLE] = wxT("Come away with me");
    t2[TrackInfo::ALBUM] = wxT("Lullabys");

    addTrackInfo(t);
    addTrackInfo(t2);
}

TrackTable::~TrackTable() {
}

wxString TrackTable::OnGetItemText(long item, long column) const {
    wxString str;
    str << wxString::Format(wxT("%i, %i"), item, column);
    return str;
}

void TrackTable::onActivate(wxListEvent& event) {
}

void TrackTable::addTrackInfo(TrackInfo& info) {
    m_trackInfos.push_back(info);
    SetItemCount(GetItemCount() + 1);
}

BEGIN_EVENT_TABLE(TrackTable, wxListCtrl)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, TrackTable::onActivate)   
END_EVENT_TABLE()



//================================================================================

} // namespace navi

