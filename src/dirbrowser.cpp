//      dirbrowser.cpp
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

#include "dirbrowser.hpp"

#include <iostream>

namespace navi {

// Declared in XXX.cpp
extern const wxEventType naviDirTraversedEvent;

DirBrowserItemData::DirBrowserItemData(const wxFileName& fileName) :
        m_fileName(fileName) {
}

DirBrowserItemData::~DirBrowserItemData() {
}

const wxFileName& DirBrowserItemData::getFileName() const {
    return m_fileName;
}

//==============================================================================

DirBrowser::DirBrowser(wxWindow* parent, NaviMainFrame* frame) :
        wxTreeCtrl(parent, DirBrowser::ID_NAVI_DIR_BROWSER, wxDefaultPosition, 
            wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT),
        m_filesVisible(true),
        m_currentActiveItem(NULL),
        m_dirTraversalThread(NULL),
        m_mainFrame(frame) {

    // intialize the used icons in the wxTreeCtrl.
    initIcons();
}

DirBrowser::~DirBrowser() {
    // Imagelist will not get deleted by the wxTreeCtrl destructor, so lets do that
    // ourselves.
    delete m_imageList;
}

void DirBrowser::initIcons() {
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


wxString DirBrowser::getBase() const {
    return m_basePath;
}

void DirBrowser::setBase(const wxString& basePath) {
    m_basePath = basePath;

    // delete them all before attempting to add a new root and shit.
    DeleteAllItems();
    // make sure the base path is now the root (second arg is the icon)
    wxTreeItemId root = AddRoot(wxT("Music Library"), 3, -1, new DirBrowserItemData(wxFileName(basePath)));
    // denote that our root "has children" (it will force a + on the item)
    SetItemHasChildren(root);
    // by default, expand the root.
    Expand(root);
}

int DirBrowser::OnCompareItems(const wxTreeItemId& one, const wxTreeItemId& two) {
    DirBrowserItemData* dataOne = static_cast<DirBrowserItemData*>(GetItemData(one));
    DirBrowserItemData* dataTwo = static_cast<DirBrowserItemData*>(GetItemData(two));
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


void DirBrowser::onExpandItem(wxTreeEvent& event) {
    wxTreeItemId itemExpanded = event.GetItem();
    addChildrenToDir(itemExpanded);
    SortChildren(itemExpanded);
}

void DirBrowser::onCollapseItem(wxTreeEvent& event) {
    wxTreeItemId itemCollapsed = event.GetItem();
    DeleteChildren(itemCollapsed);
}

void DirBrowser::onActivateItem(wxTreeEvent& event) {
    wxTreeItemId item = event.GetItem();

    // Highlight the current item.
    SetItemBold(m_currentActiveItem, false);
    m_currentActiveItem = item;
    SetItemBold(m_currentActiveItem, true);
    // Refresh the wxWindow (will display teh boldness lulz)
    Refresh();

    const wxFileName& selectedPath = getSelectedPath();

    if (m_dirTraversalThread != NULL) {
        m_dirTraversalThread->setActive(false);
        // wait for thread to finish doing its work.
        /*wxThread::ExitCode code = */
        m_dirTraversalThread->Wait();
    }
    // XXX: deleting all items does not seem to work reliably, i.e. always some
    // 'residue' seem to be left behind from the previous directory crap.
    TrackTable* tt = m_mainFrame->getTrackTable();
    tt->DeleteAllItems();

    m_dirTraversalThread = new DirTraversalThread(tt, selectedPath);
    wxThreadError err = m_dirTraversalThread->Create();
    if (err != wxTHREAD_NO_ERROR) {
        wxMessageBox(wxT("Couldn't create thread!"));
    }

    err = m_dirTraversalThread->Run();

    if (err != wxTHREAD_NO_ERROR) {
        wxMessageBox(wxT("Couldn't run thread!"));
    }


    // XXX: okay, we skip the event here, so the tree item is not expanded
    // automatically (or collapsed). 
    //event.Skip();
}

void DirBrowser::addChildrenToDir(wxTreeItemId& parent) {
    DirBrowserItemData* data = static_cast<DirBrowserItemData*>(GetItemData(parent));

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
            DirBrowserItemData* data = new DirBrowserItemData(fileobj);

            wxDir subdir(path);
            if (wxFileName::DirExists(path)) {
                wxTreeItemId subItem = AppendItem(parent, fileobj.GetFullName(), 0, -1, data);
                if (subdir.HasSubDirs()) {
                    // if this item has subdirs, pretend we have children so we
                    // can make it clickable.
                    SetItemHasChildren(subItem);
                }
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

void DirBrowser::setFilesVisible(bool visible) {
    m_filesVisible = visible;    
    // reset the base to re-init all stuff.
    setBase(m_basePath);
}

const wxFileName& DirBrowser::getSelectedPath() const {
    wxTreeItemId selected = GetSelection();
    DirBrowserItemData* data = static_cast<DirBrowserItemData*>(GetItemData(selected));
    return data->getFileName();
}



// Event table.
BEGIN_EVENT_TABLE(DirBrowser, wxTreeCtrl)
    EVT_TREE_ITEM_ACTIVATED(DirBrowser::ID_NAVI_DIR_BROWSER, DirBrowser::onActivateItem)
    EVT_TREE_ITEM_EXPANDING(DirBrowser::ID_NAVI_DIR_BROWSER, DirBrowser::onExpandItem)
    EVT_TREE_ITEM_COLLAPSING(DirBrowser::ID_NAVI_DIR_BROWSER, DirBrowser::onCollapseItem)
END_EVENT_TABLE()

//==============================================================================

DirBrowserContainer::DirBrowserContainer(wxWindow* parent, NaviMainFrame* frame) :
        wxPanel(parent, wxID_ANY) {

    // Panel with the buttons
    wxPanel* panelBtns = new wxPanel(this);
    wxBoxSizer* sizerBtns = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap imgOpen = wxArtProvider::GetBitmap(wxT("gtk-open"));
    wxBitmap imgAdd = wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK);
 
    wxBitmapButton* btn1 = new wxBitmapButton(panelBtns, ID_BROWSE_DIR, imgOpen, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    btn1->SetToolTip(wxT("Browse for new base directory"));

    wxBitmapButton* btn2 = new wxBitmapButton(panelBtns, wxID_ANY, imgAdd, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    btn2->SetToolTip(wxT("Add selected directory to favorites"));

    panelBtns->SetSizer(sizerBtns);
    sizerBtns->Add(btn1);
    sizerBtns->Add(btn2);

    m_browser = new DirBrowser(this, frame);

    // Sizer of the main thing
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    sizer->Add(panelBtns, wxSizerFlags().Left().Border(wxALL, 3));
    sizer->Add(m_browser, wxSizerFlags(1).Expand());
}

DirBrowser* DirBrowserContainer::getDirBrowser() const {
    return m_browser;
}

void DirBrowserContainer::onBrowseNewDir(wxCommandEvent& event) {
    // when button is clicked to browse for a new base dir, open a modal window
    // so the user can choose a new directory. Reset the base, etc.
    wxDirDialog* dlg = new wxDirDialog(this);
    dlg->SetPath(m_browser->getBase());
    if (dlg->ShowModal() == wxID_OK) {
        m_browser->setBase(dlg->GetPath());
    }
}

// Event table.
BEGIN_EVENT_TABLE(DirBrowserContainer, wxPanel)
    EVT_BUTTON(DirBrowserContainer::ID_BROWSE_DIR, DirBrowserContainer::onBrowseNewDir)
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
    
    thedir.Traverse(*this);
    std::cout << "Directory contains " << m_files.GetCount() << " addable files." << std::endl;

    for (unsigned int i = 0; i < m_files.GetCount(); i++) {
        if (m_active) {
            wxString filename = m_files[i];
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
                
                wxCommandEvent event(naviDirTraversedEvent);
                event.SetClientObject(derp);
                m_parent->AddPendingEvent(event);
            } catch (const AudioException& ex) {
                // this exception is thrown when for instance a file is trying to
                // be parsed when it's not a valid audio/video file. I don't want
                // to hide the exception cause, so I'm just printing it out to 
                // standard error.
                std::cerr << "DirTraversalThread() err : " << ex.what() << std::endl;
            }
        }
    }
#if 0
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
            
            wxCommandEvent event(naviDirTraversedEvent);
            event.SetClientObject(derp);
            m_parent->AddPendingEvent(event);
        } catch (const AudioException& ex) {
            // this exception is thrown when for instance a file is trying to
            // be parsed when it's not a valid audio/video file. I don't want
            // to hide the exception cause, so I'm just printing it out to 
            // standard error.
            std::cerr << "DirTraversalThread() err : " << ex.what() << std::endl;
        }

        // Get the next dir, if available. This is something like an iterator.
        gotfiles = thedir.GetNext(&filename);
    }
#endif
    return 0;
}

wxDirTraverseResult DirTraversalThread::OnFile(const wxString& filename) {
    // LOL UGLY. But it works. Ideally, I should add the list of allowed
    // extensions to a static set or vector, but I can't be arsed at this
    // moment to figure it out. C++ gets weirder, the more I am coding 
    // with it. ANYWAY: it's not that we need to have a crazy amount of 
    // performance with this part.
    wxArrayString allowed;
    allowed.Add(wxT(".ogg"));
    allowed.Add(wxT(".mp3"));
    allowed.Add(wxT(".aac"));
    allowed.Add(wxT(".wav"));
    allowed.Add(wxT(".flac"));

    for (unsigned int i = 0; i < allowed.GetCount(); i++) {
        if (filename.EndsWith(allowed[i])) {
            m_files.Add(filename);
            // no need to check the rest.
            return m_active ? wxDIR_CONTINUE : wxDIR_STOP;
        }
    }

    // when the thread is still active, continue looking.
    // if we set it to inactive (false), stop searching. This still may
    // result in some residue...
    return m_active ? wxDIR_CONTINUE : wxDIR_STOP;
}

wxDirTraverseResult DirTraversalThread::OnDir(const wxString& filename) {
    // During traversing, STOP reading into subdirectories for obvious
    // reasons. We are viewing the contents of the current directory
    // only, at all times. Cocks!
    return wxDIR_STOP;
}




} //namespace navi 

