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

#include "audio.hpp"
#include "main.hpp"
#include "tracktable.hpp"

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

#include <assert.h>

namespace navi {

class NaviMainFrame;
class DirTraversalThread;

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

    /// Thread for directory traversal (find all files). Used to not let
    /// the UI get 'stuck', i.e. waiting until it's finished.
    DirTraversalThread* m_dirTraversalThread;

    /// The Navi mainframe parent, top level window.
    NaviMainFrame* m_mainFrame;

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

    DirBrowser(wxWindow* parent, NaviMainFrame* frame);
    ~DirBrowser();

    wxString getBase() const;
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

    void onBrowseNewDir(wxCommandEvent& event);
public:
    static const short ID_BROWSE_DIR = 1030; 

    DirBrowserContainer(wxWindow* parent, NaviMainFrame* frame);

    DirBrowser* getDirBrowser() const;

    // Events plx for the buttons hurr durr
    DECLARE_EVENT_TABLE()
};

//================================================================================

/**
 * The DirTraversalThread is a joinable (not detached) thread to update the user interface
 * with new track infos. If we don't update the UI in another thread, the UI would
 * block until all files in a directory or the like are finished adding. This would
 * be severely problematic if you have a LOT of files in one directory.
 * 
 * The thread is created joinable so we can safely interrupt it be calling this
 * thread's public functions. If it was detached, it would be destroyed after it
 * has finished doing its work, and calling functions on the created instance will
 * then most certainly invoke terrorist attacks on the application.
 *
 * TODO: fer chrissake rename this thing. It's so generic.
 */
class DirTraversalThread : public wxThread, wxDirTraverser {
private:
    /// The tracktable parent. We will be add pending events to this wxWindow.
    TrackTable* m_parent;

    /// The selected path to read files from.
    wxFileName m_selectedPath;

    /// Whether this thread should be active or not. This value is polled
    bool m_active;

    wxArrayString m_files;

public:
    /**
     * Creates the DirTraversalThread, with the 'parent' TrackTable (to add pending
     * events to) and the selected path to get a dir listing from.
     *
     * @param parent The TrackTable parent.
     * @param selectedPath The path to get a listing from.
     */
    DirTraversalThread(TrackTable* parent, const wxFileName& selectedPath);

    /**
     * Sets the 'activity' state of this thread. This is only useful right now
     * for deactivation (see the Entry() override function).
     *
     * @param active Set this to false to stop the thread gracefully.
     */
    void setActive(bool active);

    /**
     * Override from wxThread. This is the 'meat' of this subclass.
     *
     * @return ExitCode (a short or something)
     */
    virtual wxThread::ExitCode Entry(); 

    virtual wxDirTraverseResult OnFile(const wxString& filename);
    virtual wxDirTraverseResult OnDir(const wxString& dirname);
};

} //namespace navi 

#endif // DIRBROWSER_HPP
