#ifndef DB_VIEWER_H
#define DB_VIEWER_H

#include <wx/wxprec.h>
#include "database_manager.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/grid.h>

#if wxUSE_LOG && wxUSE_TEXTCTRL
    #define USE_LOG_WINDOW 1
#else
    #define USE_LOG_WINDOW 0
#endif

// ----------------------------------------------------------------------
// Define a new application
// ----------------------------------------------------------------------

class MyApp : public wxApp {
public:
    MyApp(){}
    bool OnInit() wxOVERRIDE;
};

// ----------------------------------------------------------------------
// The main frame class
// ----------------------------------------------------------------------

class MainFrame : public wxFrame {
public:
    // Constructor
    MainFrame();
    ~MainFrame();

    // Event handlers
    void OnAbout(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);

    // File Menu
    void OnConnectToDatabase(wxCommandEvent& event);
    void OnDisconnectFromDatabase(wxCommandEvent& event);

    // Display Menu
    void OnToggleLogDisplay(wxCommandEvent& event);
    void OnLayoutTable(wxCommandEvent& event);

    // Left Panel
    void OnTableSelect(wxCommandEvent& event);

    // Logging methods
    void LogMenuEvent(const wxCommandEvent& event);

    DatabaseManager* GetDatabaseManager() {
        if (db_manager != NULL)
            return db_manager;
        
        return NULL;
    }

    void NewDatabaseConnection(wxString db, wxString pw, wxString user);

private:
    void LoadTableFromDatabase(const string& table_name);

private:
    // Database manager
    DatabaseManager *db_manager;

    wxBoxSizer      *m_main_sizer,
                    *m_top_sizer,           // for wxGrid
                    *m_stored_proc_sizer;
    
    wxLog            *m_old_log;
    wxScrolledWindow *m_inner_panel;
    wxGrid           *m_grid;
 
    // Left Panel Controls 
    wxComboBox      *m_table_combo_box;
 
    // Labels 
    wxStaticText    *m_backend_lbl,
                    *m_user_lbl,
                    *m_database_lbl,
                    
                    *m_no_selection_lbl;
    
#if USE_LOG_WINDOW
    wxTextCtrl* m_log_textctrl;
#endif

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------
// Event handler class
// ----------------------------------------------------------------------
class MyEvtHandler : public wxEvtHandler {
public:
    MyEvtHandler(MainFrame* frame);

    void OnMenuEvent(wxCommandEvent& event);

private:
    MainFrame* m_frame;

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------
// Window IDs
// ----------------------------------------------------------------------
enum {
    FILE_CONNECT = 200,
    FILE_DISCONNECT,

    FRAME_COMBOBOX,

    DISPLAY_LOG = 300,
    DISPLAY_LAYOUT_TABLE,

    DIALOG_CONNECT_OK = 400,
    DIALOG_CONNECT_CANCEL,
    DIALOG_CONNECT_USERNAME_TEXT,
    DIALOG_CONNECT_PASSWORD_TEXT,
    DIALOG_CONNECT_DATABASE_TEXT,

    FRAME_ABOUT = wxID_ABOUT,
    FRAME_EXIT = wxID_EXIT,
};

#endif