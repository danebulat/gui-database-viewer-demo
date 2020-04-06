#ifndef CONNECTION_DIALOG_H
#define CONNECTION_DIALOG_H

// ----------------------------------------------------------------------
// Connection Dialog
// ----------------------------------------------------------------------

class ConnectionDialog : public wxDialog {
public:
    ConnectionDialog(wxWindow* parent, const wxString& title);

private:
    wxTextValidator MakeTextValidator(long style, wxString& buffer);

private:
    void OnConnect(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    wxString    m_username,
                m_password,
                m_database;
    
    wxDECLARE_EVENT_TABLE();
};

#endif