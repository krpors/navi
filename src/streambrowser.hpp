//      streambrowser.hpp
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

#ifndef STREAMBROWSER_HPP 
#define STREAMBROWSER_HPP

#include "audio.hpp"
#include "main.hpp"

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>
#include <wx/mimetype.h>
#include <wx/icon.h>
#include <wx/iconloc.h>
#include <wx/artprov.h>
#include <wx/dirdlg.h>

namespace navi {

class NaviMainFrame;


class StreamTable: public wxListCtrl {
private:
    const wxString getCellContents(long row, long col) const;

public:

    /// The window ID for this track table.
    static const wxWindowID ID_STREAMTABLE = 1929;

    StreamTable(wxWindow* parent); 

    const wxString getDescription(long index) const;
    const wxString getLocation(long index) const;

    void onResize(wxSizeEvent& event);
    void onActivate(wxListEvent& event);
    void addStream(const wxString& desc, const wxString& loc);
    void removeSelectedStream();

    void saveToFile();
    void loadFromFile();

    DECLARE_EVENT_TABLE()
};


//================================================================================

class AddStreamDialog : public wxDialog {
private:
    wxTextCtrl* m_txtDesc;
    wxTextCtrl* m_txtLoc;

public:
    AddStreamDialog(wxWindow* parent);


    const wxString getDescription() const;
    const wxString getLocation() const;

};

//================================================================================
/**
 * This DirBrowserContainer is actually the container of the DirBrowser class
 * itself, with a few additional action buttons, all neatly layed out.
 */
class StreamBrowserContainer : public wxPanel {
private:
    static const int ID_ADD = 1;
    static const int ID_REMOVE = 2;

    StreamTable* m_streamTable;
    wxPanel* createStreamPanel(wxWindow* parent);
public:
    StreamBrowserContainer(wxWindow* parent, NaviMainFrame* frame);

    void onAdd(wxCommandEvent& event);
    void onRemove(wxCommandEvent& event);

    StreamTable* getStreamTable() const;

    // Events plx for the buttons hurr durr
    DECLARE_EVENT_TABLE()
};

} //namespace navi 

#endif // STREAMBROWSER_HPP 
