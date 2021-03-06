//      tracktable.cpp //      //      Copyright 2010 Kevin Pors <krpors@users.sf.net> //      //      This program is free software; you can redistribute it and/or modify //      it under the terms of the GNU General Public License as published by //      the Free Software Foundation; either version 2 of the License, or //      (at your option) any later version.  //      //      This program is distributed in the hope that it will be useful, //      but WITHOUT ANY WARRANTY; without even the implied warranty of //      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#include "tracktable.hpp"
#include <sys/time.h>

namespace navi {

// Declared in misc.cpp
extern const wxEventType naviDirTraversedEvent;

//================================================================================

TrackTable::TrackTable(wxWindow* parent) :
        wxListCtrl(parent, TrackTable::ID_TRACKTABLE, wxDefaultPosition, 
        wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES | wxVSCROLL),
        m_currTrackItemIndex(0) {

    m_sortDirection[0] = false;
    m_sortDirection[1] = false;
    m_sortDirection[2] = false;
    m_sortDirection[3] = false;
    m_sortDirection[4] = false;

    wxListItem item;

    item.SetText(wxT("Track"));
    InsertColumn(0, item);
    SetColumnWidth(0, 40);

    item.SetText(wxT("Artist"));
    InsertColumn(1, item);
    SetColumnWidth(1, 150);

    item.SetText(wxT("Title"));
    InsertColumn(2, item);
    SetColumnWidth(2, 150);

    item.SetText(wxT("Album"));
    InsertColumn(3, item);
    SetColumnWidth(3, 150);

    item.SetText(wxT("Duration"));
    item.SetAlign(wxLIST_FORMAT_CENTRE);
    InsertColumn(4, item);
    SetColumnWidth(4, 150);
}

// compare functions:
int wxCALLBACK TrackTable::compareTrackNumber(long item1, long item2, long sortData) {
    // reinterpret the sortData to a TrackTable pointar. Wtf.
    TrackTable* roflol = reinterpret_cast<TrackTable*>(sortData);
    // make sure we have a point0r.
    if (roflol) {
        TrackInfo& one = roflol->getTrackInfo(item1);
        TrackInfo& two = roflol->getTrackInfo(item2);

        long tn1 = strToInt(one[TrackInfo::TRACK_NUMBER], 0);
        long tn2 = strToInt(two[TrackInfo::TRACK_NUMBER], 0);
        if (roflol->m_sortDirection[0]) {
            //return one[TrackInfo::TRACK_NUMBER].Cmp(two[TrackInfo::TRACK_NUMBER]);
            return tn2 - tn1;
        } else {
            //return two[TrackInfo::TRACK_NUMBER].Cmp(one[TrackInfo::TRACK_NUMBER]);
            return tn1 - tn2;
        }
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

        if (roflol->m_sortDirection[1]) {
            return one[TrackInfo::ARTIST].Cmp(two[TrackInfo::ARTIST]);
        } else {
            return two[TrackInfo::ARTIST].Cmp(one[TrackInfo::ARTIST]);
        }
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

        if (roflol->m_sortDirection[2]) {
            return one[TrackInfo::TITLE].Cmp(two[TrackInfo::TITLE]);
        } else {
            return two[TrackInfo::TITLE].Cmp(one[TrackInfo::TITLE]);
        }
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

        if (roflol->m_sortDirection[3]) {
            return one[TrackInfo::ALBUM].Cmp(two[TrackInfo::ALBUM]);
        } else {
            return two[TrackInfo::ALBUM].Cmp(one[TrackInfo::ALBUM]);
        }
    }

    return 0;
}

int wxCALLBACK TrackTable::compareDuration(long item1, long item2, long sortData) {
    // reinterpret the sortData to a TrackTable pointar. Wtf.
    TrackTable* roflol = reinterpret_cast<TrackTable*>(sortData);
    // make sure we have a point0r.
    if (roflol) {
        TrackInfo& one = roflol->getTrackInfo(item1);
        TrackInfo& two = roflol->getTrackInfo(item2);

        if (roflol->m_sortDirection[4]) {
            return one.getDurationSeconds() - two.getDurationSeconds();
        } else {
            return two.getDurationSeconds() - one.getDurationSeconds();
        }
    }

    return 0;
}

void TrackTable::addTrackInfo(TrackInfo& info, bool updateInternally = true) {
    wxListItem item;
    item.SetId(GetItemCount());
    long index = InsertItem(item);
    SetItem(index, 0, info[TrackInfo::TRACK_NUMBER]); 
    if (info[TrackInfo::ARTIST].IsEmpty()) {
        SetItem(index, 1, info.getSimpleName());
    } else {
        SetItem(index, 1, info[TrackInfo::ARTIST]); 
    }
    SetItem(index, 2, info[TrackInfo::TITLE]); 
    SetItem(index, 3, info[TrackInfo::ALBUM]); 
    SetItem(index, 4, formatSeconds(info.getDurationSeconds()));
    // SetItemData using the index variable is vital for getting the correct
    // selected item of this wxListCtrl (due to sorting and whatnot).
    SetItemData(index, index);

    if (updateInternally) {
        // add the info to our backing vector.
        m_trackInfos.push_back(info);

        // after each track, re-sort the whole list, if that option is given in 
        // the preferences. XXX: check if this performs well on large directories.
        bool autosort;
        wxConfigBase::Get()->Read(Preferences::AUTO_SORT, &autosort, true);
        if (autosort) {
            SortItems(TrackTable::compareTrackNumber, reinterpret_cast<long>(this));
        }
    }
}

TrackInfo TrackTable::getSelectedItem() throw() {
    // m_selectedItem may be NULL.
    return m_selectedItem;
}

TrackInfo TrackTable::getTrackBeforeOrAfterCurrent(int pos, bool markAsPlaying) throw() {
    // Magic happens here. Iterate over the current displayed items (sorted).
    // Then figure out which one is playing right now (by checking the current
    // track item index with the GetItemData() result). Once we find it, get the
    // next track in sequence. 

    for (int i = 0; i < GetItemCount(); i++) {
        wxColour colour = GetItemBackgroundColour(i);
        long d = GetItemData(i);
        if (d == m_currTrackItemIndex) {
            TrackInfo currentlyPlaying = m_trackInfos[d];

            long next = i + pos;
            if (next >= GetItemCount()) {
                next = 0; // 'rotate' to the first track
            } else if (next <= 0) {
                next = GetItemCount() - 1; // 'rotate' to the last track
            }


            long nextData = GetItemData(next);
            if (markAsPlaying) {
                // this marks the current playing track, based on the
                // zero index, sorted rows..
                markPlayedTrack(next);
                // and this sets the track item index based on the 
                // *item data* (GetItemData()).
                m_currTrackItemIndex = nextData;
            }
            TrackInfo nextx = m_trackInfos[nextData];
            return m_trackInfos[nextData];
        }
    }

    // XXX: return none??
    TrackInfo emptyone;
    return emptyone;
}

TrackInfo TrackTable::getPrev(bool markAsPlaying) throw() {
    return getTrackBeforeOrAfterCurrent(-1, markAsPlaying);
}

TrackInfo TrackTable::getNext(bool markAsPlaying) throw() {
    return getTrackBeforeOrAfterCurrent(1, markAsPlaying);
}

void TrackTable::markPlayedTrack(long newItemId) throw() {
    wxFont fontDef  = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
    wxFont fontMark = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
    fontMark.SetWeight(wxFONTWEIGHT_BOLD);

    // XXX: unmark all items... Looks like a bit of a brute force, so
    // test this thorougly with LOTS of items in the list to check if
    // this performs well enough.
    long item = -1;
    while(true) {
        item = GetNextItem(item);
        if (item == -1) {
            break;
        }

        SetItemFont(item, fontDef);
    }

    // last but not least, mark the selected item as playing.
    SetItemFont(newItemId, fontMark);
}

TrackInfo& TrackTable::getTrackInfo(int index) {
    return m_trackInfos[index];
}

void TrackTable::DeleteAllItems() {
    wxListCtrl::DeleteAllItems();
    m_trackInfos.clear();
}

void TrackTable::onActivate(wxListEvent& event) {
    // when an item is activated by double clicking, mark it as currently playing.
    markPlayedTrack(event.GetIndex());
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
            case 4: SortItems(TrackTable::compareDuration, (long) this); break;
            default: break;
        }

        // swap sorting direction yay!:
        m_sortDirection[event.GetColumn()] = !m_sortDirection[event.GetColumn()]; 
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
    m_selectedItem = info;
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

    event.Skip();
}

void TrackTable::onResize(wxSizeEvent& event) {
    int width, height;
    GetSize(&width, &height);

    // automatically set some widths here after resizing
    SetColumnWidth(0, 40); // track#
    SetColumnWidth(1, 0.3 * width); // title
    SetColumnWidth(2, 0.3 * width); // artist
    SetColumnWidth(3, 0.2 * width); // album
    SetColumnWidth(4, 0.1 * width); // duration

    // re-layout the control, to the column sizes are actually being done.
    Layout();

    // propagate this event up the chain. If we don't do this, the control
    // will not be automatically resized.
    event.Skip();
}

void TrackTable::shuffle() {
    // don't call our DeleteAllItems(), since it also clears the vector.
    wxListCtrl::DeleteAllItems();
    // randomize vector
    std::random_shuffle(m_trackInfos.begin(), m_trackInfos.end());
    // re-add them all.
    std::vector<TrackInfo>::iterator it = m_trackInfos.begin();
    while (it < m_trackInfos.end()) {
        TrackInfo info = *it;
        addTrackInfo(info, false);
        it++;
    }

}

BEGIN_EVENT_TABLE(TrackTable, wxListCtrl)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, TrackTable::onActivate)   
    EVT_LIST_ITEM_SELECTED(TrackTable::ID_TRACKTABLE, TrackTable::onSelected)
    EVT_LIST_COL_CLICK(TrackTable::ID_TRACKTABLE, TrackTable::onColumnClick)
    EVT_COMMAND(wxID_ANY, naviDirTraversedEvent, TrackTable::onAddTrackInfo)
    EVT_SIZE(TrackTable::onResize)
END_EVENT_TABLE()

//================================================================================

} // namespace navi

