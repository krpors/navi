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
        wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES | wxVSCROLL) {

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

TrackTable::~TrackTable() {
}

bool cock = true;

// compare functions:
int wxCALLBACK TrackTable::compareTrackNumber(long item1, long item2, long sortData) {
    // reinterpret the sortData to a TrackTable pointar. Wtf.
    TrackTable* roflol = reinterpret_cast<TrackTable*>(sortData);
    // make sure we have a point0r.
    if (roflol) {
        TrackInfo& one = roflol->getTrackInfo(item1);
        TrackInfo& two = roflol->getTrackInfo(item2);
        return roflol->theSort(one, two, TrackInfo::TRACK_NUMBER, true);
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
        return roflol->theSort(one, two, TrackInfo::ARTIST, true);
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
        return roflol->theSort(one, two, TrackInfo::TITLE, true);
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
        return roflol->theSort(one, two, TrackInfo::ALBUM, true);
    }

    return 0;
}

void TrackTable::onActivate(wxListEvent& event) {
    event.Skip();
}

void TrackTable::onColumnClick(wxListEvent& event) {
    // XXX: This is fucking ridiculous. Using long as callback data?! wx, what
    // the fuck are you doing to me? Anyway, from PlasmaHH from #freenode:
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

void TrackTable::addTrackInfo(TrackInfo& info) {
    wxListItem item;
    item.SetId(GetItemCount());
    long index = InsertItem(item);
    SetItem(index, 0, info[TrackInfo::TRACK_NUMBER]); 
    SetItem(index, 1, info[TrackInfo::ARTIST]); 
    SetItem(index, 2, info[TrackInfo::TITLE]); 
    SetItem(index, 3, info[TrackInfo::ALBUM]); 
    SetItemData(index, index);

    // add the info to our backing vector.
    m_trackInfos.push_back(info);
}

TrackInfo& TrackTable::getTrackInfo(int index) {
    return m_trackInfos[index];
}

void TrackTable::DeleteAllItems() {
    wxListCtrl::DeleteAllItems();
    m_trackInfos.clear();
}

void TrackTable::addFromDir(const wxFileName& dir) {
    // first, remove existing items from the list and the vector.
    DeleteAllItems();
    m_trackInfos.clear();

    wxDir thedir(dir.GetFullPath());
    wxString filename;
    // only display files, and TODO: use a selector, as in: only mp3, ogg, flac, etc
    bool gotfiles = thedir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while (gotfiles) {
        wxFileName fullFile;
        fullFile.Assign(dir.GetFullPath(), filename);

        wxString uri = wxT("file://");
        uri << fullFile.GetFullPath();

        std::cout << "\t" << uri.mb_str() << std::endl;

        //file:///home/krpors/Desktop/mp3s/un.mp3
        // Read tags:
        try {
            TagReader t(uri);
            TrackInfo info = t.getTrackInfo();
            addTrackInfo(info);
        } catch (const AudioException& ex) {
            std::cerr << ex.what() << std::endl;
        }

        // Get the next dir, if available. This is something like an iterator.
        gotfiles = thedir.GetNext(&filename);
    }

}

void TrackTable::onNumberUpdate(wxCommandEvent& event) {
    UpdateClientData* d = dynamic_cast<UpdateClientData*>(event.GetClientObject());
    if (d) {
        TrackInfo ti = d->getTrackInfo();
        addTrackInfo(ti);
    }

    // since the UCD instance was created on the heap in a thread and
    // added to the wxCommandEvent, we must also delete it manually, because
    // the event itself won't delete it in its destructor.
    delete d;
}

int TrackTable::theSort(TrackInfo& one, TrackInfo& two, const char* field, bool ascending) {
    if (ascending) {
        return one[field] > two[field];
    } else {
        return one[field] < two[field];
    }
}

BEGIN_EVENT_TABLE(TrackTable, wxListCtrl)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, TrackTable::onActivate)   
    EVT_LIST_COL_CLICK(TrackTable::ID_TRACKTABLE, TrackTable::onColumnClick)
    EVT_COMMAND(10000, wxEVT_COMMAND_TEXT_UPDATED, TrackTable::onNumberUpdate)
END_EVENT_TABLE()

//================================================================================

UpdateClientData::UpdateClientData(const TrackInfo& info) :
        m_info(info) {
}

const TrackInfo UpdateClientData::getTrackInfo() const {
    return m_info;
}

//================================================================================

UpdateThread::UpdateThread(TrackTable* parent, const wxFileName& selectedPath) :
        wxThread(wxTHREAD_JOINABLE),
        m_parent(parent),
        m_selectedPath(selectedPath) {
}

wxThread::ExitCode UpdateThread::Entry() {
    wxDir thedir(m_selectedPath.GetFullPath());
    wxString filename;
    // only display files, and TODO: use a selector, as in: only mp3, ogg, flac, etc
    bool gotfiles = thedir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
    while (gotfiles) {
        wxFileName fullFile;
        fullFile.Assign(m_selectedPath.GetFullPath(), filename);

        wxString uri = wxT("file://");
        uri << fullFile.GetFullPath();

        //std::cout << "\t" << uri.mb_str() << std::endl;

        try {
            TagReader t(uri);
            TrackInfo info = t.getTrackInfo();
            std::cout << "\t Found track: " << info[TrackInfo::TITLE].mb_str() << std::endl;

            // add client data to the event, so we can use that to update 
            // the UI with the correct track information
            UpdateClientData* d = new UpdateClientData(info);

            wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, 10000);
            event.SetClientObject(d);
            m_parent->GetEventHandler()->AddPendingEvent(event);
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

