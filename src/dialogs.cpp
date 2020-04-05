#include <wx/wx.h>

#include "dbviewer.h"
#include "dialogs.h"

// ----------------------------------------------------------------------
// Event Table
// ----------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(ConnectionDialog, wxDialog)
    EVT_BUTTON(DIALOG_CONNECT_OK, ConnectionDialog::OnConnect)
    EVT_BUTTON(DIALOG_CONNECT_CANCEL, ConnectionDialog::OnCancel)
wxEND_EVENT_TABLE();

// ----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------

ConnectionDialog::ConnectionDialog(wxWindow* parent, const wxString& title)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize) {

    wxLogMessage(wxString::Format("Connection dialog created..."));

    wxBoxSizer* top_sizer = new wxBoxSizer(wxVERTICAL);

    // Username label
    wxStaticText* username_label = new wxStaticText(this, wxID_ANY, "Username: ", 
        wxDefaultPosition, wxDefaultSize);

    // Username text control
    m_username = new wxTextCtrl(this, DIALOG_CONNECT_USERNAME_TEXT, 
        "root", wxDefaultPosition, wxDefaultSize);

    // Password label
    wxStaticText* password_label = new wxStaticText(this, wxID_ANY, "Password: ", 
        wxDefaultPosition, wxDefaultSize);
    
    m_password = new wxTextCtrl(this, DIALOG_CONNECT_PASSWORD_TEXT, 
        "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    
    // Database name label
    wxStaticText* database_label = new wxStaticText(this, wxID_ANY, "Database: ",
        wxDefaultPosition, wxDefaultSize);
    
    m_database = new wxTextCtrl(this, DIALOG_CONNECT_DATABASE_TEXT, 
        "classicmodels", wxDefaultPosition, wxDefaultSize);
    
    top_sizer->AddSpacer(20);
    top_sizer->Add(username_label, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(m_username, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(password_label, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(m_password, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(database_label, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(m_database, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());

    // Button sizer
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* connect_btn = new wxButton(this, DIALOG_CONNECT_OK, "Connect", 
        wxDefaultPosition, wxDefaultSize);
    
    wxButton* cancel_btn = new wxButton(this, DIALOG_CONNECT_CANCEL, "Cancel", 
        wxDefaultPosition, wxDefaultSize);
    
    button_sizer->Add(connect_btn, wxSizerFlags(1).Border(wxRIGHT, 5));
    button_sizer->Add(cancel_btn, wxSizerFlags(1).Border(wxLEFT, 5));

    top_sizer->AddSpacer(40);
    top_sizer->Add(button_sizer, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(20);

    SetSizerAndFit(top_sizer);
    SetClientSize(GetClientSize().GetWidth() * 1.5, GetClientSize().GetHeight());

    // Centres the dialog box on the display.
    Centre(wxBOTH);
}

// ----------------------------------------------------------------------
// Event Handlers
// ----------------------------------------------------------------------

void ConnectionDialog::OnConnect(wxCommandEvent& event) {
    
    wxLogMessage(wxString::Format("Attempt connection to database..."));

    // TODO: Validators (close dialog only if user has entered all information)

    // wxLogMessage(wxString::Format("Username: %s\nPassword: %s\nDatabase: %s",
    //     m_username->GetValue().ToStdString(),
    //     m_password->GetValue().ToStdString(),
    //     m_database->GetValue().ToStdString()));
    
    // Make database connection
    MainFrame* parent = static_cast<MainFrame*>(this->m_parent);
    
    wxString db_string(m_database->GetValue());
    wxString un_string(m_username->GetValue());
    wxString pw_string(m_password->GetValue());

    parent->NewDatabaseConnection(
        db_string,
        pw_string,
        un_string);

    Close();
}

void ConnectionDialog::OnCancel(wxCommandEvent& event) {
    Close();
}