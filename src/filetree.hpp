//      filetree.hpp
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

#ifndef FILETREE_HPP
#define FILETREE_HPP

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>
#include <wx/mimetype.h>
#include <wx/icon.h>
#include <wx/iconloc.h>
#include <wx/artprov.h>

namespace navi {

class FileTreeItemData : public wxTreeItemData {
private:

    wxFileName m_fileName;
public:
    FileTreeItemData(const wxFileName& fileName);
    ~FileTreeItemData();

    const wxFileName& getFileName() const;

};

//==============================================================================

class FileTree : public wxTreeCtrl {
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
     * Adds children to a certain parent, by checking the FileTreeItemData's
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
    static const short ID_TREE = 1;

    FileTree(wxWindow* parent);
    ~FileTree();

    /**
     * Sets the base path for the FileTree.
     * TODO: throw exception when the basepath is not a valid dir.
     *
     * @param basePath The basepath. 
     */
    void setBase(const wxString& basePath);

    const wxFileName& getSelectedPath() const;

    /**
     * Sets whether files are visible in the tree. This will refresh
     * the whole tree.
     *
     * @param visible true when files should be visible, false if not.
     */
    void setFilesVisible(bool visible);

    // wxWidgets macro: declare the event table... duh
    DECLARE_EVENT_TABLE()
};

} //namespace navi 

#endif // FILETREE_HPP
