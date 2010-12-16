//      dirbrowser.hpp
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

#ifndef DIRBROWSER_HPP
#define DIRBROWSER_HPP

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

namespace navi {

class DirBrowserItemData : public wxTreeItemData {
private:

    wxFileName m_fileName;
public:
    DirBrowserItemData(const wxFileName& fileName);
    ~DirBrowserItemData();

    const wxFileName& getFileName() const;

};

//==============================================================================

class DirBrowser : public wxTreeCtrl {
private:
    /// Image list for items in the tree.
    wxImageList* m_imageList;

    wxIcon m_iconFolder;
    wxIcon m_iconFolderOpen;
    wxIcon m_iconFile;
    wxIcon m_iconRoot;

    /// The base path to load the tree fromz0r.
    wxString m_basePath;

    /// Whether files should be visible in the tree. Defaults to false.
    bool m_filesVisible;

    /// The current active tree item (i.e. the directory which is played)
    /// This is used to reset it's state when a new tree item is activated.
    wxTreeItemId m_currentActiveItem;

    /**
     * This function will be invoked as a callback by the wxTreeCtrl.
     * When an item is collapsed, this will delete children of the children
     * of the item that was collapsed. This is to preserve memory and all
     * that stuff.
     *
     * @param event The wxTreeEvent, containing the item which generated the
     *  event.
     */
    void onCollapseItem(wxTreeEvent& event);

    /**
     * This callback function is invoked by the wxTreeCtrl parent, as soon
     * as an item is expanded. After expansion, the children of the children
     * of that item are fetched, and added at runtime to the wxTreeCtrl.
     *
     * @param event The wxTreeEvent with information about the wxTreeItemId
     *  and such which generate that event.
     */
    void onExpandItem(wxTreeEvent& event);

    /**
     * When an item is activated by either double clicking or hitting
     * the return key on an item, set its visible state to bold font, and
     * start playing songs from that dir... or something!
     *
     * @param event The wxTreeEvent propagated.
     */
    void onActivateItem(wxTreeEvent& event);

    /**
     * Compares child elements. Directories get prevalence, then files. 
     * This function therefore makes sure that the dirs get displayed first.
     * This is an overridden function from the wxTreeCtrl class.
     * 
     * @param one The first tree item.
     * @param two The second tree item.
     * @return A negative, zero or positive value if the first item is less 
     *  than, equal to or greater than the second one.
     */
    int OnCompareItems(const wxTreeItemId& one, const wxTreeItemId& two);

    /**
     * Adds children to a certain parent, by checking the DirBrowserItemData's
     * path etc.
     *
     * @param parent The parent to add children to.
     */
    void addChildrenToDir(wxTreeItemId& parent);

    /**
     * Initializes icons for this wxTreeCtrl.
     */
    void initIcons();

public:
    static const wxWindowID ID_NAVI_DIR_BROWSER = 1;

    DirBrowser(wxWindow* parent);
    ~DirBrowser();

    /**
     * Sets the base path for the DirBrowser.
     * TODO: throw exception when the basepath is not a valid dir.
     *
     * @param basePath The basepath. 
     */
    void setBase(const wxString& basePath);

    /**
     * Gets the current selected path as a wxFileName. The path value
     * is stored internally within a DirBrowserItemData instance.
     *
     * @return wxFileName containining the fully selected path.
     */
    const wxFileName& getSelectedPath() const;

    /**
     * Sets whether files are visible in the tree. This will refresh
     * the whole tree.
     *
     * @param visible true when files should be visible, false if not.
     */
    void setFilesVisible(bool visible);

    //void getFilesFromCurrentDi

    // wxWidgets macro: declare the event table... duh
    DECLARE_EVENT_TABLE()
};

//==============================================================================

/**
 * This DirBrowserContainer is actually the container of the DirBrowser class
 * itself, with a few additional action buttons, all neatly layed out.
 */
class DirBrowserContainer : public wxPanel {
private:
    DirBrowser* m_browser;
public:
    DirBrowserContainer(wxWindow* parent);

    DirBrowser* getDirBrowser() const;
};

} //namespace navi 

#endif // DIRBROWSER_HPP
