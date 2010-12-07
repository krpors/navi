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
        wxListCtrl(parent, TrackTable::ID_TRACKTABLE, wxDefaultPosition, 
        wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL) {

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
    addTrackInfo(t);

    t[TrackInfo::ARTIST] = wxT("Live");
    t[TrackInfo::TITLE] = wxT("The Dolphin's Cry");
    t[TrackInfo::ALBUM] = wxT("The Distance to Here");
    addTrackInfo(t);

    t[TrackInfo::ARTIST] = wxT("Late Night Alumni");
    t[TrackInfo::TITLE] = wxT("Finally Found");
    t[TrackInfo::ALBUM] = wxT("Of Birds, Bees, and Butterflies");
    addTrackInfo(t);
}

TrackTable::~TrackTable() {
}

void TrackTable::onActivate(wxListEvent& event) {
    long data = event.GetData();
    TrackInfo& info = m_trackInfos[data];
    std::cout << "Selected track: " << info[TrackInfo::TITLE].mb_str()  << std::endl;
}

int wxCALLBACK wxListCompareFunction(long item1, long item2, long sortData) {
    std::cout << "Comparing " << item1 << " with " << item2 << std::endl;
    return (item1 > item2) ? -1 : 1;
}

void TrackTable::onColumnClick(wxListEvent& event) {
    std::cout << "Column has been clicked" << std::endl;
    SortItems(wxListCompareFunction, 2);

}

void TrackTable::addTrackInfo(TrackInfo& info) {
    wxListItem item;
    item.SetId(GetItemCount());
    long index = InsertItem(item);
    SetItem(index, 0, info[TrackInfo::ARTIST]); 
    SetItem(index, 1, info[TrackInfo::TITLE]); 
    SetItem(index, 2, info[TrackInfo::ALBUM]); 
    SetItemData(index, index);

    m_trackInfos.push_back(info);

    std::cout << "Inserted item " << index << std::endl;
}

BEGIN_EVENT_TABLE(TrackTable, wxListCtrl)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, TrackTable::onActivate)   
    EVT_LIST_COL_CLICK(TrackTable::ID_TRACKTABLE, TrackTable::onColumnClick)
END_EVENT_TABLE()



//================================================================================

} // namespace navi

