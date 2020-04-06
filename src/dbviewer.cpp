#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <vector>

#include "dbviewer.h"
#include "database_manager.h"
#include "dialogs.h"
#include "utils.h"

using std::endl;
using std::cout;


// ----------------------------------------------------------------------
// MyApp
// ----------------------------------------------------------------------

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {

    // Initialise base classes
    if (!wxApp::OnInit())
        return false;
    
    wxTheApp->SetVendorName(wxT("Dane Bulat"));
	wxTheApp->SetAppName(wxT("Database Viewer"));

    // Create the main frame window
    MyFrame* frame = new MyFrame();
    frame->Show(true);

    return true;
}

// ----------------------------------------------------------------------
// Event tables
// ----------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)

    // Menu bar events
    EVT_MENU(FRAME_ABOUT, MyFrame::OnAbout)
    EVT_MENU(FRAME_EXIT, MyFrame::OnExit)
    EVT_SIZE(MyFrame::OnSize)

    EVT_MENU(DISPLAY_LOG, MyFrame::OnToggleLogDisplay)
    EVT_MENU(DISPLAY_LAYOUT_TABLE, MyFrame::OnLayoutTable)

    EVT_MENU(FILE_CONNECT, MyFrame::OnConnectToDatabase)
    EVT_MENU(FILE_DISCONNECT, MyFrame::OnDisconnectFromDatabase)

    // Left panel events
    EVT_COMBOBOX(FRAME_COMBOBOX, MyFrame::OnTableSelect)

wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(MyEvtHandler, wxEvtHandler)
    EVT_MENU(wxID_ANY, MyEvtHandler::OnMenuEvent)
wxEND_EVENT_TABLE();

// ----------------------------------------------------------------------
// MyEvtHandler
// ----------------------------------------------------------------------

MyEvtHandler::MyEvtHandler(MyFrame* frame) : m_frame(frame) {}

void MyEvtHandler::OnMenuEvent(wxCommandEvent& event) {
    
    // Pass event to custom log function
    m_frame->LogMenuEvent(event);

    // Skip event so it's wxWindow (MyFrame) can handle it
    event.Skip();
}

void MyFrame::LogMenuEvent(const wxCommandEvent& event) {

    // Get event's ID to uniquely identify the item
    int id = event.GetId();

    // Start log message
    wxString msg = wxString::Format("Menu command %d", id);

    // Catch all checkable menubar items
    wxMenuItem* item = GetMenuBar()->FindItem(id);
    
    if (item && item->IsCheckable()) {
        msg += wxString::Format(" (the item is currently %schecked)",
            event.IsChecked() ? "" : "not ");
    }

    // Log message
    wxLogMessage(msg);
}

// ----------------------------------------------------------------------
// MyFrame
// ----------------------------------------------------------------------

// Constructor
MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, "Database Viewer"),
      m_main_sizer(NULL), 
      m_top_sizer(NULL), 
      m_stored_proc_sizer(NULL), 
      m_old_log(NULL), 
      m_inner_panel(NULL), 
      m_grid(NULL), 
      m_table_combo_box(NULL), 
      m_backend_lbl(NULL), 
      m_user_lbl(NULL), 
      m_database_lbl(NULL), 
      m_no_selection_lbl(NULL), 
      m_log_textctrl(NULL) {
          
    // Instantiate database manager
    db_manager = new DatabaseManager();

#if USE_LOG_WINDOW
    m_log_textctrl = NULL;
#endif

    // Create `File` menu
    wxMenu* file_menu = new wxMenu();
    file_menu->Append(FILE_CONNECT, "&Connect to database...\tCtrl-O", "Connect to a database");
    file_menu->Append(FILE_DISCONNECT, "&Disconnect from database...\tCtrl-D", "Disconnect from a database");
    file_menu->AppendSeparator();
    file_menu->Append(FRAME_EXIT, "E&xit", "Quit program");

    // Disable the disconnect item
    file_menu->Enable(FILE_DISCONNECT, false);

    // Create `Display` menu
    wxMenu* display_menu = new wxMenu();
    display_menu->Append(DISPLAY_LOG, "&Log Window\tCtrl-L", "Hide or show the log window", true);
    display_menu->AppendSeparator();
    display_menu->Append(DISPLAY_LAYOUT_TABLE, "Layout &Table\tCtrl-T", "Resize the table to fit panel", false);
    
    // Disable the `Layout table` item
    display_menu->Enable(DISPLAY_LAYOUT_TABLE, false);

    // Create `Help` menu
    wxMenu* help_menu = new wxMenu();
    help_menu->Append(FRAME_ABOUT, "&About", "About database viewer");

    // Create menu bar
    wxMenuBar* menu_bar = new wxMenuBar();
    menu_bar->Append(file_menu, "&File");
    menu_bar->Append(display_menu, "&Display");
    menu_bar->Append(help_menu, "&Help");

    // These items should be initially checked
    menu_bar->Check(DISPLAY_LOG, true);
    
    // Associate the menu bar with the frame
    SetMenuBar(menu_bar);

    // Intercept all menu events and log them in this custom event handler
    PushEventHandler(new MyEvtHandler(this));

    // Create status bar
#if wxUSE_STATUSBAR
    CreateStatusBar(2);
    SetStatusText("Welcome to database viewier");
#endif

    // Create layout
    m_top_sizer = new wxBoxSizer(wxHORIZONTAL);

    // --------------------------------------------------
    // Information panel
    // --------------------------------------------------

    // Create inner wxPanel
    m_inner_panel = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER);

    m_top_sizer->Add(m_inner_panel, wxSizerFlags().Proportion(1).Border(wxALL, 2).Expand());

    // (INNER) Vertical sizer
    wxBoxSizer* info_sizer = new wxBoxSizer(wxVERTICAL);

    // Vertical static box sizer
    wxBoxSizer* db_info_sizer = new wxStaticBoxSizer(
        new wxStaticBox(m_inner_panel, wxID_ANY, "Database information"), wxVERTICAL);
    

    wxFont bold_font = wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    
    wxGridSizer* grid_sizer = new wxGridSizer(2, 10, 5);

    // Database information
    m_backend_lbl = new wxStaticText(m_inner_panel, wxID_ANY, "MySQL");
    m_user_lbl = new wxStaticText(m_inner_panel, wxID_ANY, "-");
    m_database_lbl = new wxStaticText(m_inner_panel, wxID_ANY, "-"); 
    
    m_backend_lbl->SetFont(bold_font);
    m_user_lbl->SetFont(bold_font);
    m_database_lbl->SetFont(bold_font);

    grid_sizer->Add(
        new wxStaticText(m_inner_panel, wxID_ANY, "Backend:"), 
        wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));
    
    grid_sizer->Add(m_backend_lbl,
        wxSizerFlags().Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL));
    
    grid_sizer->Add(
        new wxStaticText(m_inner_panel, wxID_ANY, "User:"), 
        wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));
    
    grid_sizer->Add(m_user_lbl,
        wxSizerFlags().Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL));
    
    grid_sizer->Add(
        new wxStaticText(m_inner_panel, wxID_ANY, "Database:"), 
        wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));
    
    grid_sizer->Add(m_database_lbl,
        wxSizerFlags().Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL));
    
    db_info_sizer->Add(grid_sizer, wxSizerFlags().Expand());
    
    // Stored procedure information
    m_stored_proc_sizer = new wxStaticBoxSizer(
        new wxStaticBox(m_inner_panel, wxID_ANY, "Stored Procedures"), wxVERTICAL);

    m_stored_proc_sizer->Add(
        new wxStaticText(m_inner_panel, wxID_ANY, "-"),
        wxSizerFlags().Border(wxALL, 5));
    
    // Table information
    wxBoxSizer* table_info_sizer = new wxStaticBoxSizer(
        new wxStaticBox(m_inner_panel, wxID_ANY, "Select Table"), wxVERTICAL);
    
    // --------------------------------------------------
    // Combo box
    // --------------------------------------------------
    wxArrayString choices;

    m_table_combo_box = new wxComboBox(m_inner_panel, FRAME_COMBOBOX,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY);
    
    m_table_combo_box->Enable(false);
    
    table_info_sizer->Add(m_table_combo_box, wxSizerFlags().Border(wxLEFT | wxTOP, 10).Expand());

    info_sizer->AddSpacer(10);
    info_sizer->Add(db_info_sizer, wxSizerFlags().Expand());
    info_sizer->AddSpacer(10);
    info_sizer->Add(m_stored_proc_sizer, wxSizerFlags().Expand());
    info_sizer->AddSpacer(10);
    info_sizer->Add(table_info_sizer, wxSizerFlags().Expand());

    m_inner_panel->SetSizerAndFit(info_sizer);
    m_inner_panel->SetScrollRate(5, 5);

    // --------------------------------------------------
    // Table Section
    // --------------------------------------------------

    m_no_selection_lbl = new wxStaticText(this, wxID_ANY, "No table currently selected", 
            wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);

    m_top_sizer->Add(
        m_no_selection_lbl,
        wxSizerFlags().Proportion(3).Expand().Border(wxTOP, 20));

    // --------------------------------------------------
    // Logging Section
    // --------------------------------------------------

    m_main_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_sizer->Add(m_top_sizer, wxSizerFlags(1).Expand());

#if USE_LOG_WINDOW

    // The control used for logging
    m_log_textctrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    
    // Log text not editable
    m_log_textctrl->SetEditable(false);
    m_log_textctrl->SetMinSize(wxSize(-1, 120));

    // Disable timestamps
    //wxLog::DisableTimestamp();
    
    // Set a new active log target
    m_old_log = wxLog::SetActiveTarget(new wxLogTextCtrl(m_log_textctrl));

    wxLogMessage(wxString::Format("Welcome to the database viewer application!\n"
                 "Logging is enabled for this application build.\n"));
    
    m_main_sizer->Add(m_log_textctrl, wxSizerFlags().Border(wxLEFT | wxRIGHT | wxBOTTOM, 2).Expand());
    
#endif // USE_LOGGING_WINDOW

    // --------------------------------------------------
    // Layout Frame
    // --------------------------------------------------
    
    SetSizerAndFit(m_main_sizer);
    wxSize best_size = GetBestSize();
    
    SetClientSize(best_size*1.25);

    wxLogMessage(wxString::Format("Width of inner panel: %d", m_inner_panel->GetClientSize().GetWidth()));
    wxLogMessage(wxString::Format("Width of client area: %d", GetClientSize().GetWidth()));

    Show();
}

MyFrame::~MyFrame() {
    
    // De-allocate objects not maanged by wxWidgets
    SafeDelete(db_manager);    

    // Delete the event handler installed in the constructor
    PopEventHandler(true);

    // Restore old logger
#if USE_LOG_WINDOW
    delete wxLog::SetActiveTarget(m_old_log);
#endif
}

void MyFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {

    (void)wxMessageBox(
        "Connect to and view database tables.", 
        "About Database Viewer",
        wxOK | wxICON_INFORMATION);
}

void MyFrame::OnSize(wxSizeEvent& event){

    Layout();
        
    wxLogMessage(wxString::Format("Window resized..."));
}

void MyFrame::OnLayoutTable(wxCommandEvent& WXUNUSED(event)) {
    
    wxLogMessage(wxString::Format("Calling: MyFrame::OnLayoutTable"));

    int inner_panel_width = m_inner_panel->GetClientSize().GetWidth();
    int client_width = GetClientSize().GetWidth();

    //wxLogMessage(wxString::Format("inner_panel_width: %i\nclient width: %i",
    //    inner_panel_width, client_width));

    if (m_grid != NULL) {

        int available_width = 0;
        int grid_width = 0;

        // Calculate grid width by adding up each columns width
        int TOTAL_COLS = db_manager->GetColumnCount();

        for (int i = 0; i < TOTAL_COLS; ++i) {
            grid_width += m_grid->GetColSize(i);
        }

        //wxLogMessage(wxString::Format("Grid width: %i", grid_width));
        available_width = (client_width - inner_panel_width);
        //wxLogMessage(wxString::Format("Available width: %i", available_width));
        
        if (grid_width < available_width) {
            
            // Optional offset to remove horizontal scroll bar
            const int OFFSET = 0;

            int new_grid_width = available_width - OFFSET;
            int col_width = (new_grid_width / TOTAL_COLS);

            wxLogMessage(wxString::Format("New col width: %i", col_width));

            for (int i = 0; i < TOTAL_COLS; ++i) {
                m_grid->SetColSize(i, col_width);
            }
    
            Layout();
        }
    }
}

void MyFrame::OnConnectToDatabase(wxCommandEvent& WXUNUSED(event)) {

    ConnectionDialog dialog(this, "Connect to MySQL Database");

    dialog.ShowModal();
}

void MyFrame::OnDisconnectFromDatabase(wxCommandEvent& WXUNUSED(event)) {
    
    // Delete m_session in databas manager
    if (db_manager->DisconnectFromDatabase()) {
        wxLogMessage(wxString::Format("Success: Disconnected from database."));

        // Update `File` and `Display` menu items
        wxMenuBar* menu = GetMenuBar();
        wxMenu* file_menu = menu->GetMenu(menu->FindMenu("File"));
        wxMenu* display_menu = menu->GetMenu(menu->FindMenu("Display"));
        
        file_menu->Enable(FILE_CONNECT, true);
        file_menu->Enable(FILE_DISCONNECT, false);
        display_menu->Enable(DISPLAY_LAYOUT_TABLE, false);

        // Clear table combo box
        m_table_combo_box->Clear();
        m_table_combo_box->Enable(false);

        // Clear labels
        m_user_lbl->SetLabelText("-");
        m_database_lbl->SetLabelText("-");

        // Clear stored procedures
        m_stored_proc_sizer->DeleteWindows();
        m_stored_proc_sizer->Add(
            new wxStaticText(m_inner_panel, wxID_ANY, "-"), 
            wxSizerFlags().Border(wxALL, 5));
        
        // Delete grid and show label
        if (m_grid != NULL) {
            delete m_grid;
            m_grid = NULL;
        }

        m_no_selection_lbl->Show(true);

        // Layout the updated labels and table
        // wxSize client_size = GetClientSize();
        // SetSizerAndFit(m_main_sizer);
        // SetClientSize(client_size);
        Layout();
    }
    else {
        wxLogMessage(wxString::Format("Error: Unable to disconnect from database."));
    }
}

void MyFrame::OnTableSelect(wxCommandEvent& event) {

    wxLogMessage(wxString::Format("Calling: MyFrame::OnTableSelect"));

    int index = m_table_combo_box->GetSelection();

    if (index != wxNOT_FOUND) {
        wxString selection = m_table_combo_box->GetValue(); 
        wxLogMessage(wxString::Format("Selected table: %s", selection));

        // Load table rows from database
        LoadTableFromDatabase(selection.ToStdString());
        Layout();
    }
    else {
        wxLogMessage(wxString::Format("No table is selected"));
    }
}

void MyFrame::OnToggleLogDisplay(wxCommandEvent& event) {

    if (event.IsChecked()) {
        m_log_textctrl->Show(true);
        wxLogMessage(wxString::Format("Log panel shown"));
    }
    else {
        m_log_textctrl->Show(false);
        wxLogMessage(wxString::Format("Log panel hidden"));
    }

    // wxSize client_size = GetClientSize();
    // SetClientSize(client_size);
    Layout();
}

// ------------------------------------------------------------
// MyFrame::NewDatabaseConnection
// ------------------------------------------------------------

bool MyFrame::NewDatabaseConnection(wxString db, wxString pw, wxString user) {

    wxLogMessage(wxString::Format("MyFrame::NewDatabaseConnection: Entered..."));

    // Instantiate database manager
    if (db_manager != NULL) {

        bool good_connection = db_manager->ConnectToDatabase(
            db.ToStdString(),
            user.ToStdString(),
            pw.ToStdString()
        );

        // Handle connection result
        if (good_connection) {
                        
            // Update `File` and `Display` menu items
            wxMenuBar* menu = GetMenuBar();
            wxMenu* file_menu = menu->GetMenu(menu->FindMenu("File"));
            wxMenu* display_menu = menu->GetMenu(menu->FindMenu("Display"));

            file_menu->Enable(FILE_CONNECT, false);
            file_menu->Enable(FILE_DISCONNECT, true);
            display_menu->Enable(DISPLAY_LAYOUT_TABLE, true);

            // Update combo box with table names and enable
            m_table_combo_box->Clear();
            m_table_combo_box->Enable(true);

            //cout << "Getting table names..." << endl;
            std::vector<std::string> table_names;
            db_manager->GetTableNames(table_names);

            wxArrayString choices;
            for (int i = 0; i < table_names.size(); ++i) {
                choices.Add(table_names.at(i));
            }

            m_table_combo_box->Append(choices);
            m_table_combo_box->SetValue(choices[0]);

            // Update labels (username, database)
            m_user_lbl->SetLabelText(db_manager->GetUser());
            m_database_lbl->SetLabelText(db_manager->GetDatabase());

            // Update stored procedures list
            std::vector<std::string> stored_procedures;
            if (db_manager->GetStoredProcedures(stored_procedures) > 0) {
                
                m_stored_proc_sizer->DeleteWindows();

                for (size_t i = 0; i < stored_procedures.size(); ++i) {

                    m_stored_proc_sizer->Add(
                        new wxStaticText(m_inner_panel, wxID_ANY, stored_procedures.at(i)), 
                        wxSizerFlags().Border(wxALL, 5));
                }
            }
            
            // Load the first table
            LoadTableFromDatabase(table_names[0]);

            // Layout the updated labels and table
            // wxSize client_size = GetClientSize();
            // SetSizerAndFit(m_main_sizer);
            // SetClientSize(client_size);
            Layout();

            return true;
        }
        else {
            wxLogMessage(wxString::Format("Error: Unable to connect to database. Please try again."));
        }
    }

    return false;
}

void MyFrame::LoadTableFromDatabase(const string& table_name) {

    m_no_selection_lbl->Show(false);

    // Create a new grid
    if (m_grid != NULL) {
        delete m_grid;
        m_grid = NULL;    
    }

    m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxSize(400, 200));

    // Load table data into grid
    db_manager->LoadTableData(table_name, m_grid);

    // Layout controls
    m_top_sizer->Add(m_grid, 3, wxGROW|wxALL, 2);
    Layout();
}