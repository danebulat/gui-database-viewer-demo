#include "database_manager.h"

#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>

#include <wx/grid.h>

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <ostream>

using namespace soci;
using std::cout;
using std::endl;
using std::cerr;
using std::exception;
using std::string;
using std::vector;

// ----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------

DatabaseManager::DatabaseManager() 
    : m_connection_string(""),
      m_user(""),
      m_database(""),
      m_column_count(0),
      m_session(NULL) 
{}

// ----------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------

DatabaseManager::~DatabaseManager() {

    cout << "DatabaseManager::~DatabaseManager: Entered..." << endl;

    // Delete session
    if (m_session != NULL) {
        delete m_session;
        m_session = NULL;

        cout << "Success: m_session deleted" << endl;
    }
}

// ----------------------------------------------------------------------
// Public Method Implementation
// ----------------------------------------------------------------------

bool DatabaseManager::ConnectToDatabase(string database, string username, string password) {

    try {
        // Construct connection string
        MakeConnectionString(database, username, password);

        // Connect to database
        m_session = new session(mysql, m_connection_string.c_str());

        m_user = username;
        m_database = database;

        // Debug message
        cout << "Connection successful! " << endl << "user: " << m_user << endl <<  "database: " << m_database << endl;

        return true;
    }
    catch (const soci::mysql_soci_error& e) {
        cerr << "MySQL error (error no: " << e.err_num_ << "): " << e.what() << endl;
        return false;
    }
    catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl; 
        return false;
    }

    return false;
}

bool DatabaseManager::DisconnectFromDatabase() {

    cout << "DatabaseManager::DisconnectFromSession: Entered..." << endl;
    
    // Delete session object if exists
    if (m_session != NULL) {
        
        delete m_session;
        m_session = NULL;

        cout << "DatabaseManager::DisconnectFromSession: m_session deleted..." << endl;
        return true;
    }

    return false;
}

void DatabaseManager::LoadTableData(const string& table_name, wxGrid* grid) {

    // Get column count
    (*m_session) << "SELECT COUNT(COLUMN_NAME) FROM information_schema.COLUMNS "
                    "WHERE table_name=:tn AND table_schema=:db",
                    use(table_name, "tn"), use(m_database, "db"), into(m_column_count);
    
    // Initialise column_names vector
    vector<string> column_names;
    column_names.resize(static_cast<size_t>(m_column_count));

    // Load column names
    (*m_session) << "SELECT COLUMN_NAME FROM information_schema.COLUMNS "
                    "WHERE table_name=:tn AND table_schema=:db",
                    use(table_name), use(m_database), into(column_names);
    
    // Set the column dimension of the grid and turn off row labels
    cout << "Column count before creating grid: " << m_column_count << endl;
    grid->CreateGrid(0, m_column_count);
    grid->SetRowLabelSize(0);

    // Set table's column labels
    if (column_names.size() > 0) {
        int index = 0;
        for_each(column_names.cbegin(), column_names.cend(),[&grid, &index](string column_name) {
            grid->SetColLabelValue(index, wxString::Format(column_name.c_str()));
            ++index;
        });
    }
    
    // Fetch rows dynamically
    // Calling soci::use with session::prepare causes a crash, so construct the 
    // query string before passing it to session::prepare
    string query_string = "SELECT * FROM " + table_name;
    rowset<row> rows = ((*m_session).prepare << query_string.c_str());
    
    // Populate grid with table data
    std::ostringstream document;
    unsigned int row_index = 0;
    
    for (rowset<row>::const_iterator it = rows.begin(); it != rows.end(); ++it) {
        const row& selected_row = *it;

        // Append a new row to the grid
        grid->AppendRows(1);

        int column_index;

        for (column_index = 0; column_index < m_column_count; ++column_index) {
            
            // Clear the ostringstream
            document.str("");  // reset the string to be empty
            document.clear();  // clear any error flags
            
            // Get column properties at current index
            const column_properties& props = selected_row.get_properties(static_cast<size_t>(column_index));

            const auto& indicator = selected_row.get_indicator(column_index);

            switch (indicator) {
            
                // The data was returned without problems
                case i_ok: {

                    switch (props.get_data_type()) {
                        case dt_date: {
                            std::tm when = selected_row.get<std::tm>(static_cast<size_t>(static_cast<size_t>(column_index)));

                            document << asctime(&when);
                            wxString cell_string(document.str());

                            grid->SetCellValue(row_index, column_index, cell_string);
                            break;
                        }
                        case dt_double: {
                            document << selected_row.get<double>(static_cast<size_t>(column_index));
                            wxString cell_string(document.str());

                            grid->SetCellValue(row_index, column_index, cell_string);
                            break;
                        }
                        case dt_integer: {
                            document << selected_row.get<int>(static_cast<size_t>(column_index));
                            wxString cell_string(document.str());
                            
                            grid->SetCellValue(row_index, column_index, cell_string);
                            break;
                        }
                        case dt_long_long: {
                            document << selected_row.get<long long>(static_cast<size_t>(column_index));
                            wxString cell_string(document.str());

                            grid->SetCellValue(row_index, column_index, cell_string);
                            break;
                        }
                        case dt_string: {
                            wxString cell_string(selected_row.get<string>(static_cast<size_t>(column_index)));
                            grid->SetCellValue(row_index, column_index, cell_string);
                            break;
                        }
                        case dt_unsigned_long_long: {
                            document << selected_row.get<unsigned long long>(static_cast<size_t>(column_index));
                            wxString cell_string(document.str());

                            grid->SetCellValue(row_index, column_index, document.str());
                            break;
                        }
                        default: {
                            grid->SetCellValue(row_index, column_index, wxString::Format("NULL"));
                            break;
                        }
                    } // switch (props.get_data_type())

                    break;
                }
            
                // There is a record, but the column is null
                case i_null: {
                    grid->SetCellValue(row_index, column_index, wxString::Format("NULL"));
                    break;
                }

                case i_truncated:
                    // the value was returned only in part,
                    // because the provided buffer was too short
                    // (not possible with std::string, but possible with char* and char[])
                    grid->SetCellValue(row_index, column_index, wxString::Format("TRUNC"));
                    break;
            }

            // Set cell as read-only
            grid->SetReadOnly(row_index, column_index);
        }

        ++row_index;
    }
}

void DatabaseManager::GetTableNames(vector<string>& table_names) {

    int count = 0;
    (*m_session) << "SELECT COUNT(table_name) FROM information_schema.tables "
                    "WHERE table_type='BASE TABLE' AND table_schema=:db",
                    use(m_database, "db"), into(count);


    cout << "Table name count: " << count << endl;

    // Important: Resize vector to avoid segmentation fault (11)
    table_names.resize(static_cast<size_t>(count));

    (*m_session) << "SELECT table_name FROM information_schema.tables "
                    "WHERE table_type='BASE TABLE' AND table_schema=:db",
                    use(m_database, "db"), into(table_names);
    
    cout << "No. of tables: " << table_names.size() << endl;
}

int DatabaseManager::GetStoredProcedures(vector<string>& stored_procedures) {

    int count = 0;
    (*m_session) << "SELECT COUNT(routine_name) FROM information_schema.routines "
                    "WHERE routine_type='PROCEDURE' AND routine_schema=:db",
                    use(m_database), into(count);
    
    if (count > 0) {

        // Important: Resize vector to avoid segmentation fault (11)
        stored_procedures.resize(static_cast<size_t>(count));

        (*m_session) << "SELECT routine_name FROM information_schema.routines "
                        "WHERE routine_type='PROCEDURE' AND routine_schema=:db",
                        use(m_database, "db"), into(stored_procedures);

        cout << "No. of stored procedures: " << stored_procedures.size() << endl;
    }

    return count;
}

// ----------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------

string DatabaseManager::GetUser() const {
    return m_user;
}

string DatabaseManager::GetDatabase() const {
    return m_database;
}

int DatabaseManager::GetColumnCount() const {
    return m_column_count;
}

// ----------------------------------------------------------------------
// Private Method Implementation
// ----------------------------------------------------------------------

void DatabaseManager::MakeConnectionString(const string& database, const string& username, const string& password) {
    m_connection_string = "db=" + database + " user=" + username + " password=" + password;
}
