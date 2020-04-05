#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <soci/soci.h>
#include <wx/grid.h>

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

using namespace soci;
using std::string;
using std::vector;


class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool ConnectToDatabase(string database, string username, string password);
    bool DisconnectFromDatabase();
    void LoadTableData(const string& table_name, wxGrid* grid);
    
    void GetTableNames(vector<string>& table_names);
    int GetStoredProcedures(vector<string>& stored_procedures);
    int GetColumnCount() const;

    string GetUser() const;
    string GetDatabase() const;

private:
    void MakeConnectionString(const string& database, const string& username, const string& password);

private:
    session *m_session;
    
    string  m_connection_string;
    string  m_user;
    string  m_database;

    int     m_column_count;
};

#endif