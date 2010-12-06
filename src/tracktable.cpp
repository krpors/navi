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

//================================================================================

TrackTable::TrackTable(wxWindow* parent) :
        wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL) {
    wxListItem item;
    item.SetText(wxT("Artist"));
    InsertColumn(0, item);
    SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);

    item.SetText(wxT("Title"));
    InsertColumn(1, item);
    SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);

    InsertItem(0, wxT("Marc Aurel"));
    SetItem(0, 0, wxT("Marc Aurel"));
    SetItem(0, 1, wxT("The Sound of Love"));

    InsertItem(1, wxT("Kingsize"));
    SetItem(1, 0, wxT("Kingsize1"));
    SetItem(1, 1, wxT("Galactic Storm"));

}

TrackTable::~TrackTable() {
}

//================================================================================

} // namespace navi
