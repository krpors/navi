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

#include "filetree.hpp"

#include <iostream>

namespace navi {

FileTreeItemData::FileTreeItemData(const wxFileName& fileName) :
        m_fileName(fileName) {
}

FileTreeItemData::~FileTreeItemData() {
}

const wxFileName& FileTreeItemData::getFileName() const {
    return m_fileName;
}

//==============================================================================

FileTree::FileTree(wxWindow* parent) :
    wxTreeCtrl(parent, FileTree::ID_TREE, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT),
    m_filesVisible(true),
    m_currentActiveItem(NULL) {

    // intialize the used icons in the wxTreeCtrl.
    initIcons();
}

FileTree::~FileTree() {
    // Imagelist will not get deleted by the wxTreeCtrl destructor, so lets do that
    // ourselves.
    delete m_imageList;
}

void FileTree::initIcons() {
    m_imageList = new wxImageList;

    m_iconRoot = wxArtProvider::GetIcon(wxART_HARDDISK);
    m_iconFolder = wxArtProvider::GetIcon(wxART_FOLDER);
    m_iconFolderOpen = wxArtProvider::GetIcon(wxART_FOLDER_OPEN);
    m_iconFile = wxArtProvider::GetIcon(wxART_NORMAL_FILE);


    if (m_iconFolder.IsOk()) {
        m_imageList->Add(m_iconFolder); // id 0
    }
    if (m_iconFolderOpen.IsOk()) {
        m_imageList->Add(m_iconFolderOpen); // id 1
    }
    if (m_iconFile.IsOk()) {
        m_imageList->Add(m_iconFile); // id 2
    }
    if (m_iconFolder.IsOk()) {
        m_imageList->Add(m_iconRoot); // id 3
    }

    SetImageList(m_imageList);
}

void FileTree::setBase(const wxString& basePath) {
    m_basePath = basePath;

    // delete them all before attempting to add a new root and shit.
    DeleteAllItems();
    // make sure the base path is now the root
    wxTreeItemId root = AddRoot(wxT("Music Library"), 3, -1, new FileTreeItemData(wxFileName(basePath)));
    // add the subdirs and files from the root here:
    addChildrenToDir(root);

}

int FileTree::OnCompareItems(const wxTreeItemId& one, const wxTreeItemId& two) {
    FileTreeItemData* dataOne = static_cast<FileTreeItemData*>(GetItemData(one));
    FileTreeItemData* dataTwo = static_cast<FileTreeItemData*>(GetItemData(two));
    bool isDirOne = wxFileName::DirExists(dataOne->getFileName().GetFullPath());
    bool isDirTwo = wxFileName::DirExists(dataTwo->getFileName().GetFullPath());

    if (isDirOne) {
        if (isDirTwo) {
            // compare the directories lexicographically
            return dataOne->getFileName().GetFullName() >= dataTwo->getFileName().GetFullName();
        } else {
            // pathTwo is just a file, which is `inferior'.
            return -1;
        }
    } else {
        // pathOne is a file... but pathTwo may be a dir!
        if (isDirTwo) {
            return 1;
        } else {
            // compare files lexicographically
            return dataOne->getFileName().GetFullName() >= dataTwo->getFileName().GetFullName();
        }
    }

    return 1;
}


void FileTree::onExpandItem(wxTreeEvent& event) {
    wxTreeItemId itemExpanded = event.GetItem();

    // check all it's chrilden. I am truley sorry for your lots. 
    wxTreeItemIdValue cookie;
    // This is only the FIRST child.
    wxTreeItemId child = GetFirstChild(itemExpanded, cookie);
    // IsOk checks if there was actually a child
    if (child.IsOk()) {
        addChildrenToDir(child);

        // This dildo part checks if there are more children than just the first one.
        wxTreeItemId nextChild = GetNextChild(itemExpanded, cookie);
        // while has more children:
        while(nextChild.IsOk()) {
            addChildrenToDir(nextChild);

            nextChild = GetNextChild(itemExpanded, cookie);
        }
    }

    SortChildren(itemExpanded);
}

/*
 * When an item is collapsing....
 */
void FileTree::onCollapseItem(wxTreeEvent& event) {
    wxTreeItemId itemCollapsed = event.GetItem();
    
    // FIXME: improve this part. Stupid behaviour when collapsing.

    wxTreeItemIdValue cookie;
    // This is only the FIRST child.
    wxTreeItemId child = GetFirstChild(itemCollapsed, cookie);
    // IsOk checks if there was actually a child
    if (child.IsOk()) {
        // remove childs of `child'
        DeleteChildren(child);

        // This dildo part checks if there are more children than just the first one.
        wxTreeItemId nextChild = GetNextChild(itemCollapsed, cookie);
        // while has more children:
        while(nextChild.IsOk()) {
            // remove childs
            DeleteChildren(nextChild);
            nextChild = GetNextChild(itemCollapsed, cookie);
        }
    }

}

void FileTree::onActivateItem(wxTreeEvent& event) {
    wxTreeItemId item = event.GetItem();

    SetItemBold(m_currentActiveItem, false);
    m_currentActiveItem = item;
    SetItemBold(m_currentActiveItem, true);

    // TODO: use callbacks/listeners and such things to notify interested
    // components that we're on the verge of playing songs in a directory.
    // TODO: start here again.
    //wxApp& app = wxGetApp;
    //NaviMainFrame* mainframe = 

    Refresh();
}

void FileTree::addChildrenToDir(wxTreeItemId& parent) {
    FileTreeItemData* data = static_cast<FileTreeItemData*>(GetItemData(parent));

    // If the `parent' holds an existing directory, get children.
    if (wxFileName::DirExists(data->getFileName().GetFullPath())) {
        //std::cout << "Existing dir, attempt to add children to parent " << data->getPath().mb_str() << std::endl;

        wxDir dir(data->getFileName().GetFullPath());
        wxString filename;
        // do we want to display files, directories, or both?
        int mode = m_filesVisible ? (wxDIR_DIRS | wxDIR_FILES) : wxDIR_DIRS;
        bool gotfiles = dir.GetFirst(&filename, wxEmptyString, mode);
        while (gotfiles) {
            wxString path = data->getFileName().GetFullPath();
            path << wxT("/") << filename;

            wxFileName fileobj(path);
            FileTreeItemData* data = new FileTreeItemData(fileobj);

            if (wxFileName::DirExists(path)) {
                AppendItem(parent, fileobj.GetFullName(), 0, -1, data);
            } else if(wxFileName::FileExists(path)) {
                // only add files when we want to..
                if (m_filesVisible) {
                    AppendItem(parent, fileobj.GetFullName(), 2, -1, data);
                }
            }

            // Get the next dir, if available. This is something like an iterator.
            gotfiles = dir.GetNext(&filename);
        }
    }
}

void FileTree::setFilesVisible(bool visible) {
    m_filesVisible = visible;    
    // reset the base to re-init all stuff.
    setBase(m_basePath);
}

const wxFileName& FileTree::getSelectedPath() const {
    wxTreeItemId selected = GetSelection();
    FileTreeItemData* data = static_cast<FileTreeItemData*>(GetItemData(selected));
    return data->getFileName();
}



// Event table.
BEGIN_EVENT_TABLE(FileTree, wxTreeCtrl)
    EVT_TREE_ITEM_ACTIVATED(FileTree::ID_TREE, FileTree::onActivateItem)
    EVT_TREE_ITEM_EXPANDING(FileTree::ID_TREE, FileTree::onExpandItem)
    EVT_TREE_ITEM_COLLAPSING(FileTree::ID_TREE, FileTree::onCollapseItem)
END_EVENT_TABLE()

} //namespace navi 

