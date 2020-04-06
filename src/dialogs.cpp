#include <wx/wx.h>
#include <wx/valtext.h>

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
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize), 
      m_username("root"), m_password(""), m_database("") {

    wxLogMessage(wxString::Format("Connection dialog created..."));

    wxBoxSizer* top_sizer = new wxBoxSizer(wxVERTICAL);

    // Username label
    wxStaticText* username_label = new wxStaticText(this, wxID_ANY, "Username: ", 
        wxDefaultPosition, wxDefaultSize);

    // Username text control
    wxTextCtrl* username_ctrl = new wxTextCtrl(this, DIALOG_CONNECT_USERNAME_TEXT, 
        wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, MakeTextValidator(wxFILTER_ASCII, m_username));
    
    // Password label
    wxStaticText* password_label = new wxStaticText(this, wxID_ANY, "Password: ", 
        wxDefaultPosition, wxDefaultSize);
    
    wxTextCtrl* password_ctrl = new wxTextCtrl(this, DIALOG_CONNECT_PASSWORD_TEXT, 
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, MakeTextValidator(wxFILTER_ASCII, m_password));
    
    // Database name label
    wxStaticText* database_label = new wxStaticText(this, wxID_ANY, "Database: ",
        wxDefaultPosition, wxDefaultSize);
    
    wxTextCtrl* database_ctrl = new wxTextCtrl(this, DIALOG_CONNECT_DATABASE_TEXT, 
        wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, MakeTextValidator(wxFILTER_ASCII, m_database));
    
    top_sizer->AddSpacer(20);
    top_sizer->Add(username_label, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(username_ctrl, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(password_label, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(password_ctrl, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(database_label, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());
    top_sizer->AddSpacer(10);
    top_sizer->Add(database_ctrl, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20).Expand());

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

    // Firstly call wxWindow::Validate, which returns false if any of the child 
    // window validators failed to validate the window data.
    // Secondly, call wxWindow::TransferDataFromWindow and return if this failed.
    
    if ( Validate() && TransferDataFromWindow() ) {

        if ( IsModal() ) {

            // Log message
            wxLogMessage(wxString::Format("Username from validator: %s", m_username));

            // Make database connection
            MyFrame* parent = static_cast<MyFrame*>(this->m_parent);

             // Close dialog if connection successful, otherwise display message box
            if (parent->NewDatabaseConnection(m_database, m_password, m_username)) {
                EndModal(DIALOG_CONNECT_OK);
            }
            else {
                (void)wxMessageBox(
                    "Unable to connect to database. Please try again.",
                    "Unable to Connect",
                    wxICON_EXCLAMATION, this);
            }
        }
        // Enter if this dialog is non-modal (for demonstration only)
        else {
            SetReturnCode(DIALOG_CONNECT_OK);
            this->Show(false);
        }
    }
}

void ConnectionDialog::OnCancel(wxCommandEvent& event) {
    Close();
}

// ----------------------------------------------------------------------
// Utilities
// ----------------------------------------------------------------------

wxTextValidator ConnectionDialog::MakeTextValidator(long style, wxString& buffer) {

    // const wchar_t chars[] = { '-', '_' };
    // wxArrayString include_list( 2, chars );
    // wxTextValidator text_validator(wxFILTER_ALPHANUMERIC | wxFILTER_INCLUDE_LIST, &m_username_str);
    // text_validator.SetCharIncludes(wxString("-_"));

    wxTextValidator val(style, &buffer);
    return val;
}