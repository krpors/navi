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

#include "tracktable.hpp"

namespace navi {

//================================================================================

TrackTable::TrackTable(wxWindow* parent) :
        wxListCtrl(parent, TrackTable::ID_TRACKTABLE, wxDefaultPosition, 
        wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES | wxVSCROLL),
        m_currTrackItemIndex(0) {

    wxListItem item;

    item.SetText(wxT("Track"));
    InsertColumn(0, item);
    SetColumnWidth(0, 40);

    item.SetText(wxT("Artist"));
    InsertColumn(1, item);
    SetColumnWidth(1, 80);

    item.SetText(wxT("Title"));
    InsertColumn(2, item);
    SetColumnWidth(2, 100);

    item.SetText(wxT("Album"));
    InsertColumn(3, item);
    SetColumnWidth(3, 150);

}

// compare functions:
int wxCALLBACK TrackTable::compareTrackNumber(long item1, long item2, long sortData) {
    // reinterpret the sortData to a TrackTable pointar. Wtf.
    TrackTable* roflol = reinterpret_cast<TrackTable*>(sortData);
    // make sure we have a point0r.
    if (roflol) {
        TrackInfo& one = roflol->getTrackInfo(item1);
        TrackInfo& two = roflol->getTrackInfo(item2);

        return one[TrackInfo::TRACK_NUMBER] > two[TrackInfo::TRACK_NUMBER];
    }

    return 0;
}

int wxCALLBACK TrackTable::compareArtistName(long item1, long item2, long sortData) {
    // reinterpret the sortData to a TrackTable pointar. Wtf.
    TrackTable* roflol = reinterpret_cast<TrackTable*>(sortData);
    // make sure we have a point0r.
    if (roflol) {
        TrackInfo& one = roflol->getTrackInfo(item1);
        TrackInfo& two = roflol->getTrackInfo(item2);

        return one[TrackInfo::ARTIST] > two[TrackInfo::ARTIST];
    }

    return 0;
}

int wxCALLBACK TrackTable::compareTitle(long item1, long item2, long sortData) {
    // reinterpret the sortData to a TrackTable pointar. Wtf.
    TrackTable* roflol = reinterpret_cast<TrackTable*>(sortData);
    // make sure we have a point0r.
    if (roflol) {
        TrackInfo& one = roflol->getTrackInfo(item1);
        TrackInfo& two = roflol->getTrackInfo(item2);

        return one[TrackInfo::TITLE] > two[TrackInfo::TITLE];
    }
    
    return 0;
}

int wxCALLBACK TrackTable::compareAlbum(long item1, long item2, long sortData) {
    // reinterpret the sortData to a TrackTable pointar. Wtf.
    TrackTable* roflol = reinterpret_cast<TrackTable*>(sortData);
    // make sure we have a point0r.
    if (roflol) {
        TrackInfo& one = roflol->getTrackInfo(item1);
        TrackInfo& two = roflol->getTrackInfo(item2);

        return one[TrackInfo::ALBUM] > two[TrackInfo::ALBUM];
    }

    return 0;
}

void TrackTable::addTrackInfo(TrackInfo& info) {
    wxListItem item;
    item.SetId(GetItemCount());
    long index = InsertItem(item);
    SetItem(index, 0, info[TrackInfo::TRACK_NUMBER]); 
    SetItem(index, 1, info[TrackInfo::ARTIST]); 
    SetItem(index, 2, info[TrackInfo::TITLE]); 
    SetItem(index, 3, info[TrackInfo::ALBUM]); 
    // SetItemData using the index variable is vital for getting the correct
    // selected item of this wxListCtrl (due to sorting and whatnot).
    SetItemData(index, index);

    // add the info to our backing vector.
    m_trackInfos.push_back(info);
}

TrackInfo* TrackTable::getSelectedItem() throw() {
    // m_selectedItem may be NULL.
    return m_selectedItem;
}

TrackInfo* TrackTable::getNext() throw() {
    // Magic happens here. Iterate over the current displayed items (sorted).
    // Then figure out which one is playing right now (by comparing
    // TODO: FINISH THIS UP
    wxListItem selectedItem; 
    selectedItem.SetId(0);
    selectedItem.SetColumn(1);
    GetItem(selectedItem);
    std::cout << "Selected item: " << selectedItem.GetText().mb_str() << std::endl;
    return NULL;
}

TrackInfo& TrackTable::getTrackInfo(int index) {
    return m_trackInfos[index];
}

void TrackTable::DeleteAllItems() {
    wxListCtrl::DeleteAllItems();
    m_trackInfos.clear();
}

void TrackTable::onActivate(wxListEvent& event) {
    //std::cout << "Activating item. " << event.GetData() << std::endl;
    m_currTrackItemIndex = event.GetData();
    // skip this when a listitem is activated (propagate it up the chain!)
    // In this case, main.cpp (NaviMainFrame) handles this event.
    event.Skip();
}

void TrackTable::onColumnClick(wxListEvent& event) {
    // XXX: This is fucking ridiculous. Using long as callback data?! wx, what
    // the fuck are you doing to me? Anyway, from PlasmaHH from #wxwidgets on freenode:
    //
    //      "it might be that sizeof(long) < sizeof(ObjectInstance) and 
    //       then you are screwed" 
    // 
    // So in this case, I'm just gonna blurt out some error message, until
    // I find a way to fix this.
    if (sizeof(long) != sizeof(this)) {
        std::cerr << "Major malfunction. sizeof(long) != sizeof(this)" << std::endl;
    } else {
        // commence sorting. We can somewhat 'guarantee' that casting a this 
        // to a long succeeds. Check which column has been clicked, then sort
        // appropriately.
        switch (event.GetColumn()) {
            case 0: SortItems(TrackTable::compareTrackNumber, (long) this); break;
            case 1: SortItems(TrackTable::compareArtistName, (long) this); break;
            case 2: SortItems(TrackTable::compareTitle, (long) this); break;
            case 3: SortItems(TrackTable::compareAlbum, (long) this); break;
            default: break;
        }
    }
}

void TrackTable::onSelected(wxListEvent& event) {
    // after a select event, we can safely determine the CORRECT selected
    // item, even after it has been sorted by artist, title, or whatever.
    // even.getData() returns the correct index, because of the SetItemData()
    // call we've done in addTrackInfo(TrackInfo&).
    long data = event.GetData();
    TrackInfo& info = getTrackInfo(data);
    // assign the address of this thing as the selected item.
    m_selectedItem = &info;
}

void TrackTable::onAddTrackInfo(wxCommandEvent& event) {
    TrackInfo* d = static_cast<TrackInfo*>(event.GetClientObject());
    if (d) {
        addTrackInfo(*d);
    } else {
        std::cerr << "TrackInfo should exist here, huh!" << std::endl;
    }

    // since the UCD instance was created on the heap in a thread and
    // added to the wxCommandEvent, we must also delete it manually, because
    // the event itself won't delete it in its destructor. This is conforming
    // the documentation from wxCommandEvent.
    delete d;
}


BEGIN_EVENT_TABLE(TrackTable, wxListCtrl)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, TrackTable::onActivate)   
    EVT_LIST_ITEM_SELECTED(TrackTable::ID_TRACKTABLE, TrackTable::onSelected)
    EVT_LIST_COL_CLICK(TrackTable::ID_TRACKTABLE, TrackTable::onColumnClick)
    EVT_COMMAND(wxID_ANY, NAVI_EVENT_DIR_TRAVERSED, TrackTable::onAddTrackInfo)
END_EVENT_TABLE()

//================================================================================

DirTraversalThread::DirTraversalThread(TrackTable* parent, const wxFileName& selectedPath) :
        wxThread(wxTHREAD_JOINABLE),
        m_parent(parent),
        m_selectedPath(selectedPath),
        m_active(true) {
}

void DirTraversalThread::setActive(bool active) {
    m_active = active;
}

wxThread::ExitCode DirTraversalThread::Entry() {
    wxDir thedir(m_selectedPath.GetFullPath());
    wxString filename;
    // only display files, and TODO: use a selector, as in: only mp3, ogg, flac, etc
    bool gotfiles = thedir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    // as long as more files are found, and the thread should remain active:
    while (gotfiles && m_active) {
        wxFileName fullFile;
        fullFile.Assign(m_selectedPath.GetFullPath(), filename);

        wxString uri = wxT("file://");
        uri << fullFile.GetFullPath();

        try {
            TagReader t(uri);
            // this info pointer must be deleted in the onAddTrackInfo() func
            // we're currently making a copy of the found TrackInfo object, because
            // of SetClientObject() and stuff.
            TrackInfo& info = t.getTrackInfo();
            // Make a copy on the heap, to use as a ClientObject. Must delete later!
            TrackInfo* derp = new TrackInfo(info);
            
            // use the NAVI_EVENT_DIR_TRAVERSED event type (new type)
            wxCommandEvent event(NAVI_EVENT_DIR_TRAVERSED);
            event.SetClientObject(derp);
            m_parent->AddPendingEvent(event);
        } catch (const AudioException& ex) {
            std::cerr << ex.what() << std::endl;
        }

        // Get the next dir, if available. This is something like an iterator.
        gotfiles = thedir.GetNext(&filename);
    }

    return 0;
}

//================================================================================

} // namespace navi

