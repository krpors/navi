//      streambrowser.cpp
//      
//      Copyright 2012 Kevin Pors <krpors@users.sf.net>
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

#include "streambrowser.hpp"

#include <iostream>

namespace navi {

// TODO: delete multiple items by multiple selection. Has some tricky
// method, because of shifting indexes during deletion.

StreamTable::StreamTable(wxWindow* parent) :
    wxListCtrl(parent, ID_STREAMTABLE, wxDefaultPosition, wxDefaultSize, 
        wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES | wxVSCROLL) {

    wxListItem item;

    item.SetText(wxT("Description"));
    InsertColumn(0, item);
    SetColumnWidth(0, 340);

    item.SetText(wxT("URL"));
    InsertColumn(1, item);
    SetColumnWidth(1, 340);

    loadFromFile();
}

const wxString StreamTable::getCellContents(long row, long col) const {
    wxListItem info;
    info.SetId(row); // which row do we want?
    info.SetColumn(col); // and which column?
    GetItem(info); // now FETCH IT (in the same object instance, weiiiiiird
    return info.GetText();
}

const wxString StreamTable::getDescription(long index) const {
    return getCellContents(index, 0); 
}

const wxString StreamTable::getLocation(long index) const {
    return getCellContents(index, 1); 
}

void StreamTable::onResize(wxSizeEvent& event) {
    int width, height;
    GetSize(&width, &height);

    // automatically set some widths here after resizing
    SetColumnWidth(0, 0.5 * width);
    SetColumnWidth(1, 0.5 * width);

    // re-layout the control, to make sure the column sizes are actually being done.
    Layout();

    // propagate this event up the chain. If we don't do this, the control
    // will not be automatically resized.
    event.Skip();
}

void StreamTable::onActivate(wxListEvent& event) {
    // find out what we have got in our second column:
    long index = event.GetIndex();
    const wxString& loc = getLocation(index);
    if (loc.IsEmpty()) {
        std::cerr << "Location is invalid (empty)" << std::endl;
        return;
    }

    try {
        TrackInfo* onDaHeap = new TrackInfo();
        onDaHeap->setLocation(loc);
        // We set a void* here. It's a pointer to the `onDaHeap' TrackInfo object.
        // Later on (see the `TrackStatusHandler::onStreamActivated' function), we
        // can cast it back to a TrackInfo* and work with it. That function must
        // also delete that very same pointer, or else we're fucked.
        event.SetClientObject(onDaHeap);

        // make sure this event is handled by event handlers up in the chain.
        event.Skip();
    } catch (AudioException& ex) {
        wxString msg;
        msg << wxT("Unable to open the URL `") << loc << wxT("'.\n\n");
        msg << wxT("GStreamer error description:\n") << ex.getAsWxString();
        wxMessageDialog dlg(this, msg, wxT("Error"), wxICON_ERROR | wxOK);
        dlg.ShowModal();
    }
}

void StreamTable::addStream(const wxString& desc, const wxString& loc) {
    wxListItem item;
    long index = InsertItem(item);

    SetItem(index, 0, desc);
    SetItem(index, 1, loc);
}

void StreamTable::removeSelectedStream() {
    // TODO: code this.
    long item = -1;
    item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item >= 0) {
        DeleteItem(item);
    }
}

void StreamTable::saveToFile() {
    StreamConfiguration sc;
    for(int i = 0; i < GetItemCount(); i++) {
        wxString desc = getDescription(i);
        wxString loca = getLocation(i);
        sc.addStream(desc, loca);
    }

    sc.save();
}

void StreamTable::loadFromFile() {
    StreamConfiguration sc;
    sc.load(); // load configured streams
    std::vector<std::pair<wxString, wxString> >::iterator it;
    it = sc.getStreams().begin();
    while(it < sc.getStreams().end()) {
        std::pair<wxString, wxString> p = *it;
        addStream(p.first, p.second);
        it++;
    }
}

BEGIN_EVENT_TABLE(StreamTable, wxListCtrl)
    EVT_LIST_ITEM_ACTIVATED(StreamTable::ID_STREAMTABLE, StreamTable::onActivate)   
    EVT_SIZE(StreamTable::onResize)
END_EVENT_TABLE()

//================================================================================

AddStreamDialog::AddStreamDialog(wxWindow* parent) :
        wxDialog(parent, wxID_ANY, wxT("Add new network stream")) {
    wxBoxSizer* sizerDialog = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizerDialog);

    // text fields:
    wxPanel* panelTextFields = new wxPanel(this); 
    wxFlexGridSizer* sizer = new wxFlexGridSizer(2, 2, 5, 5);
    panelTextFields->SetSizer(sizer);

    wxStaticText* lblDesc = new wxStaticText(panelTextFields, wxID_ANY, wxT("Description:"));
    m_txtDesc = new wxTextCtrl(panelTextFields, wxID_ANY);
    m_txtDesc->SetToolTip(wxT("Description of this stream"));
    wxStaticText* lblLoc = new wxStaticText(panelTextFields, wxID_ANY, wxT("Stream location:"));
    m_txtLoc = new wxTextCtrl(panelTextFields, wxID_ANY);
    m_txtLoc->SetToolTip(wxT("Stream location (e.g. 'http://example.org/stream.ogg')"));

    sizer->AddGrowableCol(1);
    sizer->SetFlexibleDirection(wxHORIZONTAL);
    
    sizer->Add(lblDesc, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    sizer->Add(m_txtDesc, wxSizerFlags().Expand().Top());
    sizer->Add(lblLoc, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    sizer->Add(m_txtLoc, wxSizerFlags().Expand().Bottom());

    // add all the components to the main panel sizer.
    sizerDialog->Add(panelTextFields, wxSizerFlags().Expand().Border(wxALL, 5));
    sizerDialog->Add(CreateButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Center());
    
    wxSize s = GetSize();
    SetSize(s.GetWidth(), 100);

    m_txtDesc->SetFocus();
}

const wxString AddStreamDialog::getDescription() const {
    return m_txtDesc->GetValue();
}

const wxString AddStreamDialog::getLocation() const {
    return m_txtLoc->GetValue();
}


//================================================================================

StreamBrowserContainer::StreamBrowserContainer(wxWindow* parent, NaviMainFrame* frame) :
        wxPanel(parent, wxID_ANY) {

     // Panel with the buttons
    wxPanel* panelBtns = new wxPanel(this);
    wxBoxSizer* sizerBtns = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap imgAdd = wxArtProvider::GetBitmap(wxT("gtk-add"));
    wxBitmap imgDel = wxArtProvider::GetBitmap(wxT("gtk-remove"));
 
    wxBitmapButton* btnAdd = new wxBitmapButton(panelBtns, ID_ADD, imgAdd, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    btnAdd->SetToolTip(wxT("Add network stream"));

    wxBitmapButton* btnDel = new wxBitmapButton(panelBtns, ID_REMOVE, imgDel, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    btnDel->SetToolTip(wxT("Delete selected network stream"));

    panelBtns->SetSizer(sizerBtns);
    sizerBtns->Add(btnAdd);
    sizerBtns->Add(btnDel);

    // Sizer of the main thing
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    sizer->Add(panelBtns, wxSizerFlags().Border(wxALL, 3));
    sizer->Add(createStreamPanel(this), wxSizerFlags(1).Expand());
   
}

wxPanel* StreamBrowserContainer::createStreamPanel(wxWindow* parent) {
    wxPanel* panel = new wxPanel(parent);
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    m_streamTable = new StreamTable(panel); 
    sizer->Add(m_streamTable, wxSizerFlags(1).Expand());
    return panel;
}

void StreamBrowserContainer::onAdd(wxCommandEvent& event) {
    AddStreamDialog d(this);
    if (d.ShowModal() == wxID_OK) {
        if (d.getLocation().IsEmpty()) {
            wxMessageDialog dlg(
                this, wxT("Stream location is empty."),
                wxT("Welp!"), wxOK | wxICON_EXCLAMATION);
            dlg.ShowModal();
        } else {
            m_streamTable->addStream(d.getDescription(), d.getLocation());
            m_streamTable->saveToFile();
        }
    }
}
void StreamBrowserContainer::onRemove(wxCommandEvent& event) {
    m_streamTable->removeSelectedStream();
    m_streamTable->saveToFile();
}

StreamTable* StreamBrowserContainer::getStreamTable() const {
    return m_streamTable;
}

// Event table.
BEGIN_EVENT_TABLE(StreamBrowserContainer, wxPanel)
    EVT_BUTTON(ID_ADD, StreamBrowserContainer::onAdd)
    EVT_BUTTON(ID_REMOVE, StreamBrowserContainer::onRemove)
END_EVENT_TABLE()

//================================================================================

} //namespace navi 

